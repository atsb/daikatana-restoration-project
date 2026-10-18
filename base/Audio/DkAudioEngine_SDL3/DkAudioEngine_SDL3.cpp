#include <SDL3/SDL.h>
#if !defined(SDL_MAJOR_VERSION) || (SDL_MAJOR_VERSION < 3)
#error "Daikatana SDL platform code requires SDL3"
#endif

#include <SDL3_mixer/SDL_mixer.h>
#if !defined(SDL_MIXER_MAJOR_VERSION) || (SDL_MIXER_MAJOR_VERSION < 3)
#error "Daikatana SDL audio requires SDL3_mixer"
#endif

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#define DK_AUDIO_DLL_EXPORT __declspec(dllexport)
#else
#define DK_AUDIO_DLL_EXPORT
#endif

#define AE_N0_DEFINE_EXTERNS
#include "client.h"
#include "snd_loc.h"
#undef AE_N0_DEFINE_EXTERNS

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "AudioEngineImports.h"

struct sdl_sound_t
{
    std::string name;
    MIX_Audio* audio = nullptr;
    unsigned registration_sequence = 0;
};

struct sdl_music_t
{
    std::string name;
    MIX_Audio* audio = nullptr;
};

struct sdl_voice_t
{
    MIX_Track* track = nullptr;
    sdl_sound_t* sound = nullptr;
    CVector origin;
    int entnum = 0;
    int entchannel = 0;
    float volume = 1.0f;
    float dist_min = 0.0f;
    float dist_max = 0.0f;
    bool positional = false;
    bool fixed_origin = false;
    bool non_directional = false;
    bool music = false;
    bool autosound = false;
    bool refreshed = false;
    bool paused = false;
    bool persistent_music = false;
    std::string source_name;
};

audio_imports_t ai{};

static MIX_Mixer* g_mixer = nullptr;
static SDL_AudioSpec g_device_spec{};
static bool g_initialized = false;
static unsigned g_registration_sequence = 1;
static std::vector<std::string> g_mp3_decoders;
#if defined(_WIN32)
static bool g_mf_started = false;
static bool g_mf_com_initialized = false;
#endif
static std::vector<sdl_sound_t*> g_sounds;
static std::vector<sdl_music_t*> g_music;
static std::vector<sdl_voice_t> g_voices;
static CVector g_listener_origin(0.0f, 0.0f, 0.0f);
static CVector g_listener_forward(0.0f, 0.0f, 0.0f);
static CVector g_listener_right(0.0f, 0.0f, 0.0f);
static CVector g_listener_up(0.0f, 0.0f, 0.0f);
static cvar_t* s_volume = nullptr;
static cvar_t* s_music = nullptr;
static cvar_t* s_cine = nullptr;
static cvar_t* s_no_loop = nullptr;

static client_state_t* g_client_state = nullptr;
static entity_state_t* g_parse_entities = nullptr;
static client_static_t* g_client_static = nullptr;

extern "C" DK_AUDIO_DLL_EXPORT void S_StopMusic(int chan);
extern "C" DK_AUDIO_DLL_EXPORT void S_StartMP3(const char* name, int channel, float fvol, int play_count, qboolean bRestart);

static float SDL_ClampGain(float gain)
{
    return std::clamp(gain, 0.0f, 1.0f);
}

static bool SDL_PathHasPrefix(const char* path, const char* prefix)
{
    if (!path || !prefix)
        return false;
    const size_t len = std::strlen(prefix);
    return std::strncmp(path, prefix, len) == 0;
}

static bool SDL_HasExtension(const char* name, const char* ext)
{
    if (!name || !ext)
        return false;
    const char* dot = std::strrchr(name, '.');
    if (!dot)
        return false;
#if defined(_WIN32)
    return _stricmp(dot, ext) == 0;
#else
    return std::strcmp(dot, ext) == 0;
#endif
}

static const char* SDL_NormalizeSoundName(const char* name, char* buffer, size_t buffer_size)
{
    if (!name || !name[0])
        return name;

    /* A legacy Win64 game-side path can arrive with the first four
       characters of the global directory truncated ("globa/"). Keep the
       compatibility repair here so the engine's original sound tables and
       indices remain unchanged. */
    if (std::strncmp(name, "globa/", 6) == 0 && buffer && buffer_size > 0)
    {
        const int written = std::snprintf(buffer, buffer_size, "global/%s", name + 6);
        if (written >= 0 && (size_t)written < buffer_size)
            return buffer;
    }

    return name;
}

static bool SDL_BuildSoundPath(const char* name, char* path, size_t path_size)
{
    if (!name || !name[0] || !path || path_size == 0)
        return false;

    int written;
    if (name[0] == '#')
        written = std::snprintf(path, path_size, "%s", name + 1);
    else if (SDL_PathHasPrefix(name, "sounds/"))
        written = std::snprintf(path, path_size, "%s", name);
    else
        written = std::snprintf(path, path_size, "sounds/%s", name);

    return written >= 0 && (size_t)written < path_size;
}

static bool SDL_BuildMusicPath(const char* name, char* path, size_t path_size)
{
    if (!name || !name[0] || !path || path_size == 0)
        return false;

    /* FS_LoadFile() wants logical search-path names, not the on-disk data/
       prefix.  Normalize either spelling here. */
    const char* logical = name;
    if (SDL_PathHasPrefix(logical, "data/"))
        logical += 5;

    int written;
    if (SDL_PathHasPrefix(logical, "music/"))
        written = std::snprintf(path, path_size, "%s", logical);
    else if (SDL_HasExtension(logical, ".mp3"))
        written = std::snprintf(path, path_size, "music/%s", logical);
    else
        written = std::snprintf(path, path_size, "music/%s.mp3", logical);

    return written >= 0 && (size_t)written < path_size;
}

static bool SDL_BuildStreamPath(const char* name, char* path, size_t path_size)
{
    if (!name || !name[0] || !path || path_size == 0)
        return false;

    const char* logical = name;
    if (SDL_PathHasPrefix(logical, "data/"))
        logical += 5;

    if (SDL_PathHasPrefix(logical, "sounds/"))
    {
        const int written = std::snprintf(path, path_size, "%s", logical);
        return written >= 0 && (size_t)written < path_size;
    }

    const int written = std::snprintf(path, path_size, "sounds/%s", logical);
    return written >= 0 && (size_t)written < path_size;
}

static bool SDL_BuildMP3Path(const char* name, int channel, char* path, size_t path_size)
{
    if (!name || !name[0] || !path || path_size == 0)
        return false;

    const char* logical = name;
    if (SDL_PathHasPrefix(logical, "data/"))
        logical += 5;

    int written;
    if (SDL_PathHasPrefix(logical, "sounds/") || SDL_PathHasPrefix(logical, "music/"))
    {
        written = std::snprintf(path, path_size, "%s", logical);
    }
    else if (channel == CHAN_MUSIC_MAP || channel == CHAN_MUSIC_MENU)
    {
        written = std::snprintf(path, path_size, "music/%s", logical);
    }
    else
    {
        written = std::snprintf(path, path_size, "sounds/%s", logical);
    }

    return written >= 0 && (size_t)written < path_size;
}

static MIX_Audio* SDL_LoadAudioFile(const char* display_name, const char* path);

static std::string SDL_Uppercase(const char* text)
{
    std::string result = text ? text : "";
    for (char& ch : result)
        ch = (char)std::toupper((unsigned char)ch);
    return result;
}

static void SDL_EnumerateAudioDecoders()
{
    g_mp3_decoders.clear();

    const int decoder_count = MIX_GetNumAudioDecoders();
    std::vector<std::string> other_mp3_decoders;

    for (int i = 0; i < decoder_count; ++i)
    {
        const char* decoder = MIX_GetAudioDecoder(i);
        if (!decoder)
            continue;

        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: decoder[%d] = %s\n", i, decoder);

        const std::string upper = SDL_Uppercase(decoder);
        if (upper == "MPG123")
            g_mp3_decoders.insert(g_mp3_decoders.begin(), decoder);
        else if (upper == "DRMP3")
            other_mp3_decoders.push_back(decoder);
    }

    g_mp3_decoders.insert(g_mp3_decoders.end(),
        other_mp3_decoders.begin(), other_mp3_decoders.end());
}

static MIX_Audio* SDL_LoadAudioPathWithDecoder(const char* display_name,
    const char* physical_path,
    const char* decoder,
    bool predecode)
{
    if (!g_mixer || !physical_path || !physical_path[0])
        return nullptr;

    SDL_IOStream* io = SDL_IOFromFile(physical_path, "rb");
    if (!io)
        return nullptr;

    SDL_PropertiesID props = SDL_CreateProperties();
    if (!props)
    {
        SDL_CloseIO(io);
        return nullptr;
    }

    bool ok = true;
    ok = ok && SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
    ok = ok && SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, true);
    ok = ok && SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_PREDECODE_BOOLEAN, predecode);
    ok = ok && SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, g_mixer);
    if (decoder && decoder[0])
        ok = ok && SDL_SetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, decoder);

    MIX_Audio* audio = nullptr;
    if (ok)
        audio = MIX_LoadAudioWithProperties(props);
    else
        SDL_CloseIO(io);

    SDL_DestroyProperties(props);

    if (!audio && ai.Com_DPrintf)
    {
        ai.Com_DPrintf("SDL3 Audio: MP3 decoder %s rejected %s: %s\n",
            decoder && decoder[0] ? decoder : "default",
            display_name ? display_name : physical_path,
            SDL_GetError());
    }

    return audio;
}

static MIX_Audio* SDL_LoadMP3AudioBuffer(const char* display_name,
    const char* path,
    void* file_data,
    int file_len)
{
    if (!g_mixer || !file_data || file_len <= 0)
        return nullptr;

    SDL_IOStream* io = SDL_IOFromConstMem(file_data, (size_t)file_len);
    if (!io)
        return nullptr;

    SDL_PropertiesID props = SDL_CreateProperties();
    if (!props)
    {
        SDL_CloseIO(io);
        return nullptr;
    }

    bool ok = true;
    ok = ok && SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
    ok = ok && SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, true);
    ok = ok && SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_PREDECODE_BOOLEAN, true);
    ok = ok && SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, g_mixer);
    if (!g_mp3_decoders.empty())
        ok = ok && SDL_SetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, g_mp3_decoders.front().c_str());

    MIX_Audio* audio = nullptr;
    if (ok)
        audio = MIX_LoadAudioWithProperties(props);
    else
        SDL_CloseIO(io);

    SDL_DestroyProperties(props);

    if (!audio && ai.Com_DPrintf)
    {
        ai.Com_DPrintf("SDL3 Audio: MP3 memory decode failed for %s (FS path: %s): %s\n",
            display_name ? display_name : "<audio>",
            path ? path : "<unknown>", SDL_GetError());
    }

    return audio;
}

#if defined(_WIN32)
static bool SDL_UTF8ToWidePath(const char* path, std::wstring& wide_path)
{
    if (!path || !path[0])
        return false;

    int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        path, -1, nullptr, 0);
    UINT code_page = CP_UTF8;
    DWORD flags = MB_ERR_INVALID_CHARS;

    if (length <= 0)
    {
        code_page = CP_ACP;
        flags = 0;
        length = MultiByteToWideChar(code_page, flags, path, -1, nullptr, 0);
    }

    if (length <= 0)
        return false;

    wide_path.resize((size_t)length - 1);
    if (MultiByteToWideChar(code_page, flags, path, -1,
        wide_path.data(), length) <= 0)
    {
        wide_path.clear();
        return false;
    }

    return true;
}

static bool SDL_GetAbsoluteWidePath(const char* path, std::wstring& wide_path)
{
    std::wstring input;
    if (!SDL_UTF8ToWidePath(path, input))
        return false;

    const DWORD required = GetFullPathNameW(input.c_str(), 0, nullptr, nullptr);
    if (required == 0)
    {
        wide_path = input;
        return true;
    }

    std::vector<wchar_t> buffer((size_t)required + 1);
    const DWORD written = GetFullPathNameW(input.c_str(),
        (DWORD)buffer.size(),
        buffer.data(), nullptr);
    if (written == 0 || written >= buffer.size())
    {
        wide_path = input;
        return true;
    }

    wide_path.assign(buffer.data(), written);
    return true;
}

static MIX_Audio* SDL_LoadMP3AudioMediaFoundation(const char* display_name,
    const char* physical_path)
{
    if (!g_mixer || !g_mf_started || !physical_path || !physical_path[0])
        return nullptr;

    std::wstring wide_path;
    if (!SDL_GetAbsoluteWidePath(physical_path, wide_path))
        return nullptr;

    IMFSourceReader* reader = nullptr;
    IMFMediaType* requested_type = nullptr;
    IMFMediaType* current_type = nullptr;
    HRESULT hr = MFCreateSourceReaderFromURL(wide_path.c_str(), nullptr, &reader);
    if (FAILED(hr))
    {
        if (ai.Com_DPrintf)
            ai.Com_DPrintf("SDL3 Audio: Media Foundation could not open %s (hr=0x%08lX)\n",
                display_name ? display_name : physical_path,
                (unsigned long)hr);
        return nullptr;
    }

    hr = reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
    if (SUCCEEDED(hr))
        hr = reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);

    if (SUCCEEDED(hr))
        hr = MFCreateMediaType(&requested_type);
    if (SUCCEEDED(hr))
        hr = requested_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    if (SUCCEEDED(hr))
        hr = requested_type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    if (SUCCEEDED(hr))
        hr = requested_type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);

    if (SUCCEEDED(hr))
        hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            nullptr, requested_type);
    if (SUCCEEDED(hr))
        hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            &current_type);

    UINT32 channels = 0;
    UINT32 sample_rate = 0;
    UINT32 bits = 0;
    if (SUCCEEDED(hr))
        hr = current_type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
    if (SUCCEEDED(hr))
        hr = current_type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sample_rate);
    if (SUCCEEDED(hr))
        hr = current_type->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bits);

    std::vector<Uint8> pcm;
    if (SUCCEEDED(hr) && channels > 0 && channels <= 8 && sample_rate > 0 && bits == 16)
    {
        for (;;)
        {
            DWORD stream_index = 0;
            DWORD flags = 0;
            LONGLONG timestamp = 0;
            IMFSample* sample = nullptr;

            hr = reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                0, &stream_index, &flags,
                &timestamp, &sample);
            if (FAILED(hr))
                break;

            if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
            {
                if (sample)
                    sample->Release();
                break;
            }

            if (!sample)
                continue;

            IMFMediaBuffer* buffer = nullptr;
            hr = sample->ConvertToContiguousBuffer(&buffer);
            if (SUCCEEDED(hr))
            {
                BYTE* data = nullptr;
                DWORD max_length = 0;
                DWORD data_length = 0;
                hr = buffer->Lock(&data, &max_length, &data_length);
                if (SUCCEEDED(hr) && data && data_length > 0)
                    pcm.insert(pcm.end(), data, data + data_length);
                if (SUCCEEDED(hr))
                    buffer->Unlock();
                buffer->Release();
            }

            sample->Release();
            if (FAILED(hr))
                break;
        }
    }

    MIX_Audio* audio = nullptr;
    if (SUCCEEDED(hr) && !pcm.empty())
    {
        SDL_AudioSpec spec{};
        spec.format = SDL_AUDIO_S16;
        spec.channels = (int)channels;
        spec.freq = (int)sample_rate;
        audio = MIX_LoadRawAudio(g_mixer, pcm.data(), pcm.size(), &spec);
    }

    if (!audio && ai.Com_DPrintf)
    {
        ai.Com_DPrintf("SDL3 Audio: Media Foundation could not decode %s (hr=0x%08lX, channels=%u, rate=%u, bits=%u, bytes=%zu)\n",
            display_name ? display_name : physical_path,
            (unsigned long)hr, (unsigned)channels, (unsigned)sample_rate,
            (unsigned)bits, pcm.size());
    }
    else if (audio && ai.Com_DPrintf)
    {
        ai.Com_DPrintf("SDL3 Audio: Media Foundation decoded DK MP3 %s (%u Hz, %u ch, %zu bytes PCM)\n",
            display_name ? display_name : physical_path,
            (unsigned)sample_rate, (unsigned)channels, pcm.size());
    }

    if (current_type)
        current_type->Release();
    if (requested_type)
        requested_type->Release();
    if (reader)
        reader->Release();

    return audio;
}
#endif

static MIX_Audio* SDL_LoadMP3AudioFile(const char* display_name, const char* logical_path)
{
    if (!g_mixer || !logical_path || !logical_path[0])
        return nullptr;

    /*
     * Daikatana's MP3 assets were originally played through Miles as streams.
     * Keep the primary SDL_mixer load path streamed rather than predecoded.
     *
     * SDL3_mixer can contain multiple MP3 decoders. Prefer MPG123, then try
     * any other MP3 decoder exposed by the installed SDL3_mixer, and finally
     * let SDL_mixer perform its normal decoder selection. This is deliberately
     * runtime-driven: the available decoder set depends on how SDL3_mixer was
     * built on the target machine.
     */
    const char* logical = logical_path;
    if (SDL_PathHasPrefix(logical, "data/"))
        logical += 5;

    char physical_path[1024];
    const int written = std::snprintf(physical_path, sizeof(physical_path),
        "data/%s", logical);
    if (written >= 0 && (size_t)written < sizeof(physical_path))
    {
        for (const std::string& decoder : g_mp3_decoders)
        {
            MIX_Audio* audio = SDL_LoadAudioPathWithDecoder(display_name, physical_path,
                decoder.c_str(), false);
            if (audio)
                return audio;
        }

        MIX_Audio* audio = SDL_LoadAudioPathWithDecoder(display_name, physical_path,
            nullptr, false);
        if (audio)
            return audio;

        /* A few decoder builds are more reliable when asked to fully decode
           the file at load time, so keep a predecode compatibility fallback. */
        for (const std::string& decoder : g_mp3_decoders)
        {
            audio = SDL_LoadAudioPathWithDecoder(display_name, physical_path,
                decoder.c_str(), true);
            if (audio)
                return audio;
        }

        audio = SDL_LoadAudioPathWithDecoder(display_name, physical_path,
            nullptr, true);
        if (audio)
            return audio;

#if defined(_WIN32)
        /*
         * Some original Daikatana MP3 assets are accepted by the legacy
         * Miles decoder but rejected by the MP3 decoders bundled with the
         * installed SDL3_mixer.  On Win64, use Media Foundation as the
         * compatibility decoder and feed the resulting PCM into the same
         * SDL3_mixer mixer.  This keeps the output/mixing path entirely SDL3.
         */
        audio = SDL_LoadMP3AudioMediaFoundation(display_name, physical_path);
        if (audio)
            return audio;
#endif
    }

    /* PAK/mod fallback. The bytes returned by FS_LoadFile are owned by the
       engine, so this path must predecode before we release that buffer. */
    if (!ai.FS_LoadFile)
        return nullptr;

    void* file_data = nullptr;
    const int file_len = ai.FS_LoadFile(logical, &file_data);
    if (file_len > 0 && file_data)
    {
        MIX_Audio* audio = nullptr;

        /* Try every MP3 decoder exposed by this SDL3_mixer build when the
           asset came from a PAK/search path. */
        for (const std::string& decoder : g_mp3_decoders)
        {
            SDL_IOStream* io = SDL_IOFromConstMem(file_data, (size_t)file_len);
            if (!io)
                break;

            SDL_PropertiesID props = SDL_CreateProperties();
            if (!props)
            {
                SDL_CloseIO(io);
                break;
            }

            bool ok = true;
            ok = ok && SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io);
            ok = ok && SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, true);
            ok = ok && SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_PREDECODE_BOOLEAN, true);
            ok = ok && SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, g_mixer);
            ok = ok && SDL_SetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING, decoder.c_str());

            if (ok)
                audio = MIX_LoadAudioWithProperties(props);
            else
                SDL_CloseIO(io);

            SDL_DestroyProperties(props);
            if (audio)
                break;
        }

        if (!audio)
            audio = SDL_LoadMP3AudioBuffer(display_name, logical, file_data, file_len);

        ai.FS_FreeFile(file_data);
        if (audio)
            return audio;
    }

    if (ai.Com_Printf)
        ai.Com_Printf("SDL3 Audio: Could not decode %s (FS path: %s): %s\n",
            display_name ? display_name : "<audio>", logical,
            SDL_GetError());
    return nullptr;
}

static MIX_Audio* SDL_LoadAudioFile(const char* display_name, const char* path)
{
    if (!g_mixer || !ai.FS_LoadFile)
        return nullptr;

    if (SDL_HasExtension(path, ".mp3"))
        return SDL_LoadMP3AudioFile(display_name, path);

    void* file_data = nullptr;
    const char* load_path = path;
    int file_len = ai.FS_LoadFile(load_path, &file_data);

    char logical_path[1024];
    if ((file_len <= 0 || !file_data) && SDL_PathHasPrefix(path, "data/"))
    {
        std::snprintf(logical_path, sizeof(logical_path), "%s", path + 5);
        file_data = nullptr;
        file_len = ai.FS_LoadFile(logical_path, &file_data);
        if (file_len > 0 && file_data)
            load_path = logical_path;
    }
    else if (file_len <= 0 || !file_data)
    {
        std::snprintf(logical_path, sizeof(logical_path), "data/%s", path);
        file_data = nullptr;
        const int fallback_len = ai.FS_LoadFile(logical_path, &file_data);
        if (fallback_len > 0 && file_data)
        {
            file_len = fallback_len;
            load_path = logical_path;
        }
    }

    if (file_len <= 0 || !file_data)
    {
        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: Could not load %s (FS path: %s)\n", display_name, path);
        return nullptr;
    }

    SDL_IOStream* io = SDL_IOFromConstMem(file_data, (size_t)file_len);
    if (!io)
    {
        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: Could not create IO stream for %s: %s\n", display_name, SDL_GetError());
        ai.FS_FreeFile(file_data);
        return nullptr;
    }

    /* Non-MP3 assets remain fully predecoded as before. */
    MIX_Audio* audio = MIX_LoadAudio_IO(g_mixer, io, true, true);
    if (!audio)
    {
        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: Could not decode %s (FS path: %s): %s\n",
                display_name, load_path, SDL_GetError());
        ai.FS_FreeFile(file_data);
        return nullptr;
    }

    ai.FS_FreeFile(file_data);
    return audio;
}

static void SDL_FreeSound(sdl_sound_t* sound)
{
    if (!sound)
        return;
    MIX_DestroyAudio(sound->audio);
    delete sound;
}

static void SDL_FreeMusic(sdl_music_t* music)
{
    if (!music)
        return;
    MIX_DestroyAudio(music->audio);
    delete music;
}

static float SDL_Attenuation(const sdl_voice_t& voice)
{
    if (!voice.sound || !voice.positional)
        return 1.0f;

    CVector source = voice.origin;
    if (!voice.fixed_origin && voice.entnum > 0 && ai.CL_GetEntitySoundOrigin)
        ai.CL_GetEntitySoundOrigin(voice.entnum, source);

    const float dx = source.x - g_listener_origin.x;
    const float dy = source.y - g_listener_origin.y;
    const float dz = source.z - g_listener_origin.z;
    const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (voice.dist_max <= voice.dist_min)
        return 1.0f;
    if (distance <= voice.dist_min)
        return 1.0f;
    if (distance >= voice.dist_max)
        return 0.0f;
    return 1.0f - ((distance - voice.dist_min) / (voice.dist_max - voice.dist_min));
}

static void SDL_SetMusicGain(sdl_voice_t& voice)
{
    if (!voice.track)
        return;

    float master = 1.0f;
    if (voice.entchannel == CHAN_MUSIC_MAP || voice.entchannel == CHAN_MUSIC_MENU)
        master = s_music ? SDL_ClampGain(s_music->value) : 1.0f;
    else if (voice.entchannel >= CHAN_CINE_1 && voice.entchannel <= CHAN_CINE_6)
        master = s_cine ? SDL_ClampGain(s_cine->value) : 1.0f;

    MIX_SetTrackGain(voice.track, SDL_ClampGain(voice.volume * master));
}

static void SDL_SetVoiceSpatialization(sdl_voice_t& voice)
{
    if (!voice.track)
        return;

    if (voice.music)
    {
        /* Music is not spatialized.  Volume changes must only touch the
           track gain so playback position and loop state are untouched. */
        SDL_SetMusicGain(voice);
        return;
    }

    float gain = voice.volume * SDL_Attenuation(voice);
    CVector source = voice.origin;
    if (!voice.fixed_origin && voice.entnum > 0 && ai.CL_GetEntitySoundOrigin)
        ai.CL_GetEntitySoundOrigin(voice.entnum, source);

    float pan = 0.5f;
    if (voice.positional && !voice.non_directional)
    {
        const float dx = source.x - g_listener_origin.x;
        const float dy = source.y - g_listener_origin.y;
        const float dz = source.z - g_listener_origin.z;
        const float length = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (length > 0.001f)
        {
            const float nx = dx / length;
            const float ny = dy / length;
            const float nz = dz / length;
            const float dot = nx * g_listener_right.x +
                ny * g_listener_right.y +
                nz * g_listener_right.z;
            pan = 0.5f * (1.0f + std::clamp(dot, -1.0f, 1.0f));
        }
    }

    MIX_StereoGains stereo{};
    stereo.left = std::sqrt(max(0.0f, 1.0f - pan));
    stereo.right = std::sqrt(max(0.0f, pan));
    MIX_SetTrackStereo(voice.track, &stereo);

    const float master = s_volume ? SDL_ClampGain(s_volume->value) : 1.0f;
    MIX_SetTrackGain(voice.track, SDL_ClampGain(gain * master));
}

static void SDL_DestroyVoice(sdl_voice_t& voice)
{
    if (voice.track)
    {
        MIX_StopTrack(voice.track, 0);
        MIX_DestroyTrack(voice.track);
    }
    voice.track = nullptr;
    voice.sound = nullptr;
}

static void SDL_RemoveFinishedVoices()
{
    for (size_t i = 0; i < g_voices.size();)
    {
        sdl_voice_t& voice = g_voices[i];
        if (!voice.track || (!voice.persistent_music &&
            !MIX_TrackPlaying(voice.track) && !MIX_TrackPaused(voice.track)))
        {
            SDL_DestroyVoice(voice);
            g_voices.erase(g_voices.begin() + (ptrdiff_t)i);
        }
        else
        {
            ++i;
        }
    }
}

static void SDL_RefreshVoiceGains()
{
    for (sdl_voice_t& voice : g_voices)
    {
        if (voice.music)
            SDL_SetMusicGain(voice);
        else
            SDL_SetVoiceSpatialization(voice);
    }

    if (g_mixer)
        MIX_SetMixerGain(g_mixer, 1.0f);
}

static sdl_sound_t* SDL_FindSound(const char* name)
{
    if (!name || !name[0])
        return nullptr;
    for (sdl_sound_t* sound : g_sounds)
    {
        if (sound && sound->name == name)
            return sound;
    }
    return nullptr;
}

static float SDL_AudioDurationSeconds(MIX_Audio* audio)
{
    if (!audio)
        return 5.0f;

    const Sint64 frames = MIX_GetAudioDuration(audio);
    SDL_AudioSpec spec{};
    if (frames > 0 && MIX_GetAudioFormat(audio, &spec) && spec.freq > 0)
        return (float)frames / (float)spec.freq;

    return 5.0f;
}

static void SDL_CheckForSubtitle(const char* name, MIX_Audio* audio)
{
    if (!name || !name[0] || !ai.FS_FOpenFile || !ai.FS_Read || !ai.FS_FCloseFile ||
        !ai.SCR_SubtitlePrint)
        return;

    if (ai.Cvar_Get)
    {
        cvar_t* subtitle_cvar = ai.Cvar_Get("subtitles", "0", CVAR_ARCHIVE);
        if (!subtitle_cvar || subtitle_cvar->value == 0.0f)
            return;
    }

    const char* base = std::strrchr(name, '/');
    base = base ? base + 1 : name;

    char subtitle_path[1024];
    int written = std::snprintf(subtitle_path, sizeof(subtitle_path),
        "subtitles/%s.txt", base);
    if (written < 0 || (size_t)written >= sizeof(subtitle_path))
        return;

    FILE* file = nullptr;
    int file_len = ai.FS_FOpenFile(subtitle_path, &file);

    if ((file_len <= 0 || !file) && std::strrchr(base, '.') != nullptr)
    {
        char stem[512];
        const size_t base_len = std::strlen(base);
        const char* extension = std::strrchr(base, '.');
        const size_t stem_len = (size_t)(extension - base);

        if (stem_len > 0 && stem_len < sizeof(stem) && stem_len < base_len)
        {
            std::memcpy(stem, base, stem_len);
            stem[stem_len] = '\0';
            written = std::snprintf(subtitle_path, sizeof(subtitle_path),
                "subtitles/%s.txt", stem);
            if (written >= 0 && (size_t)written < sizeof(subtitle_path))
                file_len = ai.FS_FOpenFile(subtitle_path, &file);
        }
    }

    if (file_len <= 0 || !file)
        return;

    char text[1024];
    std::memset(text, 0, sizeof(text));
    const int read_len = min(file_len, (int)sizeof(text) - 1);
    ai.FS_Read(text, read_len, file);
    ai.FS_FCloseFile(file);

    if (text[0] != '\0')
        ai.SCR_SubtitlePrint(text, SDL_AudioDurationSeconds(audio));
}

static sdl_music_t* SDL_FindMusic(const char* name)
{
    if (!name || !name[0])
        return nullptr;
    for (sdl_music_t* music : g_music)
    {
        if (music && music->name == name)
            return music;
    }
    return nullptr;
}

static sdl_sound_t* SDL_LoadSound(const char* name)
{
    if (!name || !name[0] || name[0] == '*')
        return nullptr;

    char normalized_name[1024];
    const char* load_name = SDL_NormalizeSoundName(name, normalized_name, sizeof(normalized_name));

    sdl_sound_t* existing = SDL_FindSound(load_name);
    if (existing)
    {
        existing->registration_sequence = g_registration_sequence;
        return existing;
    }

    char path[1024];
    if (!SDL_BuildSoundPath(load_name, path, sizeof(path)))
        return nullptr;

    MIX_Audio* audio = SDL_LoadAudioFile(load_name, path);
    if (!audio)
        return nullptr;

    sdl_sound_t* sound = new sdl_sound_t;
    sound->name = load_name;
    sound->audio = audio;
    sound->registration_sequence = g_registration_sequence;
    g_sounds.push_back(sound);
    return sound;
}

static sdl_music_t* SDL_LoadMusic(const char* name, const char* path_override = nullptr)
{
    if (!name || !name[0])
        return nullptr;

    sdl_music_t* existing = SDL_FindMusic(name);
    if (existing)
        return existing;

    char path[1024];
    const char* path_to_load = path_override;
    if (!path_to_load)
    {
        if (!SDL_BuildMusicPath(name, path, sizeof(path)))
            return nullptr;
        path_to_load = path;
    }

    MIX_Audio* audio = SDL_LoadAudioFile(name, path_to_load);
    if (!audio)
        return nullptr;

    sdl_music_t* music = new sdl_music_t;
    music->name = name;
    music->audio = audio;
    g_music.push_back(music);
    return music;
}

static bool SDL_StopSoundChannel(int entnum, int entchannel)
{
    bool stopped = false;
    for (size_t i = 0; i < g_voices.size();)
    {
        if (!g_voices[i].music && g_voices[i].entnum == entnum && g_voices[i].entchannel == entchannel)
        {
            SDL_DestroyVoice(g_voices[i]);
            g_voices.erase(g_voices.begin() + (ptrdiff_t)i);
            stopped = true;
        }
        else
            ++i;
    }
    return stopped;
}

static void SDL_StopMusicChannel(int channel, bool destroy)
{
    for (size_t i = 0; i < g_voices.size();)
    {
        sdl_voice_t& voice = g_voices[i];
        if (voice.music && voice.entchannel == channel)
        {
            if (destroy)
            {
                SDL_DestroyVoice(voice);
                g_voices.erase(g_voices.begin() + (ptrdiff_t)i);
                continue;
            }
            if (voice.track)
            {
                MIX_PauseTrack(voice.track);
                voice.paused = true;
            }
        }
        ++i;
    }
}

static sdl_voice_t* SDL_PlayAudio(MIX_Audio* audio, sdl_sound_t* sound,
    const char* source_name, const CVector& origin,
    int entnum, int entchannel, float volume,
    float dist_min, float dist_max, bool music,
    int loop_count)
{
    if (!audio || !g_mixer)
        return nullptr;

    MIX_Track* track = MIX_CreateTrack(g_mixer);
    if (!track)
    {
        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: Could not create playback track for %s: %s\n",
                source_name ? source_name : "<audio>", SDL_GetError());
        return nullptr;
    }

    if (!MIX_SetTrackAudio(track, audio))
    {
        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: Could not attach %s: %s\n",
                source_name ? source_name : "<audio>", SDL_GetError());
        MIX_DestroyTrack(track);
        return nullptr;
    }

    if (loop_count != 0 && !MIX_SetTrackLoops(track, loop_count))
    {
        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: Could not configure loops for %s: %s\n",
                source_name ? source_name : "<audio>", SDL_GetError());
        MIX_DestroyTrack(track);
        return nullptr;
    }

    sdl_voice_t voice;
    voice.track = track;
    voice.sound = sound;
    voice.origin = origin;
    voice.entnum = entnum;
    voice.entchannel = entchannel;
    voice.volume = max(0.0f, volume);
    voice.dist_min = max(0.0f, dist_min);
    voice.dist_max = max(0.0f, dist_max);
    /* Match legacy Miles behaviour for the local player: a sound emitted
       by entity 1 with no explicit origin is treated as 2D. */
    voice.positional = (dist_min > 0.0f || dist_max > 0.0f) &&
        !((entnum == 1) && (origin.Length() == 0.0f));
    voice.fixed_origin = (entnum == 0);
    voice.non_directional = false;
    voice.music = music;
    voice.autosound = false;
    voice.refreshed = false;
    voice.paused = false;
    voice.persistent_music = music &&
        (entchannel == CHAN_MUSIC_MAP || entchannel == CHAN_MUSIC_MENU);
    voice.source_name = source_name ? source_name : "";

    g_voices.push_back(voice);
    sdl_voice_t* result = &g_voices.back();
    SDL_SetVoiceSpatialization(*result);

    if (!MIX_PlayTrack(track, 0))
    {
        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: Could not play %s: %s\n",
                source_name ? source_name : "<audio>", SDL_GetError());
        SDL_DestroyVoice(*result);
        g_voices.pop_back();
        return nullptr;
    }

    return result;
}

extern "C" DK_AUDIO_DLL_EXPORT int AEDLL_Init(DkAEInterfaceData_t* dkData, AEInfo_t* pEntryPts)
{
    if (dkData)
    {
        ai.Com_Printf = dkData->Com_Printf;
        ai.Com_sprintf = dkData->Com_sprintf;
        ai.CL_GetEntitySoundOrigin = dkData->CL_GetEntitySoundOrigin;
        ai.Cmd_AddCommand = dkData->Cmd_AddCommand;
        ai.Cmd_RemoveCommand = dkData->Cmd_RemoveCommand;
        ai.GetArgc = dkData->GetArgc;
        ai.GetArgv = dkData->GetArgv;
        ai.Cvar_Get = dkData->Cvar_Get;
        ai.Cvar_Set = dkData->Cvar_Set;
        ai.Cvar_SetValue = dkData->Cvar_SetValue;
        ai.SCR_SubtitlePrint = dkData->SCR_SubtitlePrint;
        ai.FS_FOpenFile = dkData->FS_Open;
        ai.FS_FOpenTextFile = dkData->FS_OpenText;
        ai.FS_FCloseFile = dkData->FS_Close;
        ai.FS_LoadFile = dkData->FS_LoadFile;
        ai.FS_FreeFile = dkData->FS_FreeFile;
        ai.FS_FileLength = dkData->FS_FileLength;
        ai.FS_Read = dkData->FS_Read;
        ai.FS_Seek = dkData->FS_Seek;
        ai.FS_Tell = dkData->FS_Tell;
        ai.FS_Getc = dkData->FS_Getc;
        ai.FS_Gets = dkData->FS_Gets;
        ai.Com_DPrintf = dkData->Com_DPrintf;
        ai.Com_Error = dkData->Com_Error;
        ai.Com_PageInMemory = dkData->Com_PageInMemory;
        ai.X_Malloc = dkData->X_Malloc;
        ai.X_Free = dkData->X_Free;
        ai.DKM_InMenu = dkData->DKM_InMenu;

        g_client_state = static_cast<client_state_t*>(dkData->pcl);
        g_parse_entities = static_cast<entity_state_t*>(dkData->cl_parse_entities);
        g_client_static = static_cast<client_static_t*>(dkData->cls);
    }

    if (pEntryPts)
    {
        pEntryPts->uVersionNum = 1;
        pEntryPts->pszDesc = "Daikatana SDL3 Audio Engine";
    }

    return 1;
}

extern "C" DK_AUDIO_DLL_EXPORT void S_Init(void* hWnd)
{
    (void)hWnd;
    if (g_initialized)
        return;

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: SDL_InitSubSystem failed: %s\n", SDL_GetError());
        return;
    }

    if (!MIX_Init())
    {
        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: MIX_Init failed: %s\n", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }

#if defined(_WIN32)
    {
        const HRESULT com_hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (com_hr == S_OK || com_hr == S_FALSE)
            g_mf_com_initialized = true;

        if (com_hr == S_OK || com_hr == S_FALSE || com_hr == RPC_E_CHANGED_MODE)
        {
            const HRESULT mf_hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
            if (SUCCEEDED(mf_hr))
                g_mf_started = true;
            else if (ai.Com_DPrintf)
                ai.Com_DPrintf("SDL3 Audio: Media Foundation startup failed: hr=0x%08lX\n",
                    (unsigned long)mf_hr);
        }
        else if (ai.Com_DPrintf)
        {
            ai.Com_DPrintf("SDL3 Audio: COM initialization failed: hr=0x%08lX\n",
                (unsigned long)com_hr);
        }
    }
#endif

    SDL_EnumerateAudioDecoders();

    g_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!g_mixer)
    {
        if (ai.Com_Printf)
            ai.Com_Printf("SDL3 Audio: Could not create mixer device: %s\n", SDL_GetError());
        MIX_Quit();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return;
    }

    MIX_GetMixerFormat(g_mixer, &g_device_spec);

    if (ai.Cvar_Get)
    {
        s_volume = ai.Cvar_Get("s_volume", "0.7", CVAR_ARCHIVE);
        s_music = ai.Cvar_Get("s_music", "0.5", CVAR_ARCHIVE);
        s_cine = ai.Cvar_Get("s_cine", "0.5", CVAR_ARCHIVE);
        s_no_loop = ai.Cvar_Get("s_no_loop", "0", 0);
    }

    if (g_mixer)
        MIX_SetMixerGain(g_mixer, 1.0f);

    g_initialized = true;

    if (ai.Com_Printf)
        ai.Com_Printf("SDL3 Audio: mixer ready at %d Hz, %d channels\n",
            g_device_spec.freq, g_device_spec.channels);
}

extern "C" DK_AUDIO_DLL_EXPORT void S_Shutdown(void)
{
    for (sdl_voice_t& voice : g_voices)
        SDL_DestroyVoice(voice);
    g_voices.clear();

    for (sdl_sound_t* sound : g_sounds)
        SDL_FreeSound(sound);
    g_sounds.clear();

    for (sdl_music_t* music : g_music)
        SDL_FreeMusic(music);
    g_music.clear();

    if (g_mixer)
    {
        MIX_DestroyMixer(g_mixer);
        g_mixer = nullptr;
    }

    if (g_initialized)
    {
        MIX_Quit();
#if defined(_WIN32)
        if (g_mf_started)
        {
            MFShutdown();
            g_mf_started = false;
        }
        if (g_mf_com_initialized)
        {
            CoUninitialize();
            g_mf_com_initialized = false;
        }
#endif
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        g_mp3_decoders.clear();
        g_initialized = false;
        g_client_state = nullptr;
        g_parse_entities = nullptr;
        g_client_static = nullptr;
    }
}

extern "C" DK_AUDIO_DLL_EXPORT void S_BeginRegistration(void)
{
    ++g_registration_sequence;
}

extern "C" DK_AUDIO_DLL_EXPORT void* S_RegisterSound(const char* pszName)
{
    return SDL_LoadSound(pszName);
}

extern "C" DK_AUDIO_DLL_EXPORT void S_EndRegistration(int nFreeSequenceCode)
{
    (void)nFreeSequenceCode;
    for (size_t i = 0; i < g_sounds.size();)
    {
        sdl_sound_t* sound = g_sounds[i];
        if (sound && sound->registration_sequence != g_registration_sequence)
        {
            bool in_use = false;
            for (const sdl_voice_t& voice : g_voices)
            {
                if (voice.sound == sound)
                {
                    in_use = true;
                    break;
                }
            }
            if (!in_use)
            {
                SDL_FreeSound(sound);
                g_sounds.erase(g_sounds.begin() + (ptrdiff_t)i);
                continue;
            }
        }
        ++i;
    }
}

extern "C" DK_AUDIO_DLL_EXPORT void* S_StartSound(const CVector& origin, int entnum, int entchannel,
    void* pAudioStream, float fvol, float dist_min, float dist_max)
{
    sdl_sound_t* sound = (sdl_sound_t*)pAudioStream;
    if (!sound)
        return nullptr;

    if (entchannel == CHAN_LOOP)
    {
        /* Cinematic loop shutdown is expressed by replaying the loop channel
           with zero volume.  Treat that as a stop, not as a request to resume
           the loop. */
        if (fvol <= 0.0f)
        {
            SDL_StopSoundChannel(entnum, entchannel);
            return nullptr;
        }

        for (sdl_voice_t& voice : g_voices)
        {
            if (voice.music || voice.entnum != entnum || voice.entchannel != entchannel)
                continue;

            if (voice.sound == sound && voice.track)
            {
                voice.origin = origin;
                voice.volume = fvol;
                voice.dist_min = dist_min;
                voice.dist_max = dist_max;
                voice.positional = (dist_min > 0.0f || dist_max > 0.0f) &&
                    !((entnum == 1) && (origin.Length() == 0.0f));
                voice.fixed_origin = (entnum == 0);
                voice.paused = false;
                MIX_ResumeTrack(voice.track);
                SDL_SetVoiceSpatialization(voice);
                return &voice;
            }
        }

        SDL_StopSoundChannel(entnum, entchannel);
    }
    else if (entchannel != 0)
    {
        SDL_StopSoundChannel(entnum, entchannel);
    }

    SDL_CheckForSubtitle(sound->name.c_str(), sound->audio);

    return SDL_PlayAudio(sound->audio, sound, sound->name.c_str(), origin,
        entnum, entchannel, fvol, dist_min, dist_max,
        false, entchannel == CHAN_LOOP ? -1 : 0);
}

extern "C" DK_AUDIO_DLL_EXPORT void* S_StartSoundQuake2(const CVector& origin, int entnum, int entchannel,
    void* pAudioStream, float fvol, float attenuation, float timeofs)
{
    (void)timeofs;

    float min = 0.0f;
    float max = 0.0f;
    if (attenuation == ATTN_IDLE)
    {
        min = 80.0f;
        max = 1000.0f;
    }
    else if (attenuation == ATTN_STATIC)
    {
        min = 64.0f;
        max = 224.0f;
    }
    else
    {
        min = ATTN_NORM_MIN;
        max = ATTN_NORM_MAX;
    }

    return S_StartSound(origin, entnum, entchannel, pAudioStream, fvol, min, max);
}

extern "C" DK_AUDIO_DLL_EXPORT void S_StartLocalSound(const char* s)
{
    sdl_sound_t* sound = SDL_FindSound(s);
    if (!sound)
        sound = SDL_LoadSound(s);
    if (!sound)
        return;

    CVector zero(0, 0, 0);
    SDL_CheckForSubtitle(sound->name.c_str(), sound->audio);
    SDL_PlayAudio(sound->audio, sound, sound->name.c_str(), zero,
        0, 0, 1.0f, 0.0f, 0.0f, false, 0);
}

extern "C" DK_AUDIO_DLL_EXPORT void S_StopAllSounds(void)
{
    /* Music is managed separately from ordinary sound effects by the engine.
       DKM_Enter() calls S_StopAllSounds() before CMenuMain::Enter() pauses the
       map music, and DKM_Leave() calls it again before S_StartMusic(NULL, ...)
       resumes the map track.  Therefore S_StopAllSounds() must never destroy
       music voices; S_StopMusic/S_StopMP3 explicitly control those channels. */
    for (size_t i = 0; i < g_voices.size();)
    {
        sdl_voice_t& voice = g_voices[i];

        if (voice.music)
        {
            ++i;
            continue;
        }

        SDL_DestroyVoice(voice);
        g_voices.erase(g_voices.begin() + (ptrdiff_t)i);
    }
}


static void SDL_RemoveStaleLoopSounds()
{
    for (size_t i = 0; i < g_voices.size();)
    {
        sdl_voice_t& voice = g_voices[i];
        if (voice.autosound && !voice.refreshed)
        {
            SDL_DestroyVoice(voice);
            g_voices.erase(g_voices.begin() + (ptrdiff_t)i);
            continue;
        }

        if (voice.autosound)
            voice.refreshed = false;
        ++i;
    }
}

static sdl_voice_t* SDL_FindLoopSound(int entnum)
{
    for (sdl_voice_t& voice : g_voices)
    {
        if (voice.autosound && voice.entnum == entnum && !voice.music)
            return &voice;
    }
    return nullptr;
}

static void SDL_AddLoopSounds()
{
    if (!g_client_state || !g_parse_entities || !g_client_static)
        return;

    if (g_client_static->state != ca_active)
        return;

    if (!g_client_state->sound_prepped)
        return;

    if (s_no_loop && s_no_loop->value)
        return;

    const int num_entities = g_client_state->frame.num_entities;
    for (int i = 0; i < num_entities; ++i)
    {
        const int index =
            (g_client_state->frame.parse_entities + i) & (MAX_PARSE_ENTITIES - 1);
        entity_state_t* ent = &g_parse_entities[index];

        if (!ent->sound)
            continue;

        if (ent->sound < 0 || ent->sound >= MAX_SOUNDS)
            continue;

        /* sound_precache contains the handle returned by S_RegisterSound().
           The SDL3 backend stores sdl_sound_t pointers there, not the legacy
           Miles sfx_t objects used by the old backend. */
        sdl_sound_t* sound = static_cast<sdl_sound_t*>(
            g_client_state->sound_precache[ent->sound]);
        if (!sound || !sound->audio || sound->name.empty())
            continue;

        CVector source = ent->origin;
        if (ai.CL_GetEntitySoundOrigin)
            ai.CL_GetEntitySoundOrigin(ent->number, source);

        sdl_voice_t* existing = SDL_FindLoopSound(ent->number);
        if (existing && existing->sound == sound)
        {
            existing->refreshed = true;
            existing->origin = source;
            existing->volume = ent->volume ? ent->volume : 1.0f;
            existing->dist_min = ent->dist_min ? ent->dist_min : ATTN_NORM_MIN;
            existing->dist_max = ent->dist_max ? ent->dist_max : ATTN_NORM_MAX;
            existing->non_directional = (ent->snd_flags & SND_NONDIRECTIONAL) != 0;
            existing->positional = (existing->dist_min > 0.0f || existing->dist_max > 0.0f) &&
                !(existing->entnum == 1 && source.Length() == 0.0f);
            SDL_SetVoiceSpatialization(*existing);
            continue;
        }

        if (existing)
        {
            SDL_DestroyVoice(*existing);
            for (size_t v = 0; v < g_voices.size(); ++v)
            {
                if (&g_voices[v] == existing)
                {
                    g_voices.erase(g_voices.begin() + (ptrdiff_t)v);
                    break;
                }
            }
        }

        const float dist_min = ent->dist_min ? ent->dist_min : ATTN_NORM_MIN;
        const float dist_max = ent->dist_max ? ent->dist_max : ATTN_NORM_MAX;
        const float volume = ent->volume ? ent->volume : 1.0f;

        sdl_voice_t* voice = SDL_PlayAudio(sound->audio, sound, sound->name.c_str(),
            source, ent->number, 0, volume,
            dist_min, dist_max, false, -1);
        if (!voice)
            continue;

        voice->autosound = true;
        voice->refreshed = true;
        voice->non_directional = (ent->snd_flags & SND_NONDIRECTIONAL) != 0;
        voice->positional = (dist_min > 0.0f || dist_max > 0.0f) &&
            !(ent->number == 1 && source.Length() == 0.0f);
        SDL_SetVoiceSpatialization(*voice);
    }
}

static void SDL_ApplyChangedMusicVolumes()
{
    const bool music_changed = s_music && s_music->modified;
    const bool cine_changed = s_cine && s_cine->modified;

    if (!music_changed && !cine_changed)
        return;

    for (sdl_voice_t& voice : g_voices)
    {
        if (!voice.music)
            continue;

        if (music_changed &&
            (voice.entchannel == CHAN_MUSIC_MAP || voice.entchannel == CHAN_MUSIC_MENU))
        {
            SDL_SetMusicGain(voice);
        }
        else if (cine_changed &&
            voice.entchannel >= CHAN_CINE_1 && voice.entchannel <= CHAN_CINE_6)
        {
            SDL_SetMusicGain(voice);
        }
    }

    if (s_music && music_changed)
        s_music->modified = false;
    if (s_cine && cine_changed)
        s_cine->modified = false;
}

extern "C" DK_AUDIO_DLL_EXPORT void S_Update(CVector& origin, CVector& v_forward, CVector& v_right, CVector& v_up)
{
    g_listener_origin = origin;
    g_listener_forward = v_forward;
    g_listener_right = v_right;
    g_listener_up = v_up;

    SDL_ApplyChangedMusicVolumes();
    SDL_RemoveFinishedVoices();

    for (sdl_voice_t& voice : g_voices)
    {
        if (voice.autosound)
            voice.refreshed = false;
    }

    SDL_AddLoopSounds();
    SDL_RemoveStaleLoopSounds();
    SDL_RefreshVoiceGains();
}

extern "C" DK_AUDIO_DLL_EXPORT void S_ReverbPreset(int nPresetID)
{
    (void)nPresetID;
}

extern "C" DK_AUDIO_DLL_EXPORT void S_StartStream(const char* name, float vol)
{
    if (!name || !name[0])
        return;

    char path[1024];
    if (!SDL_BuildStreamPath(name, path, sizeof(path)))
        return;

    const std::string key = std::string("stream:") + name;
    sdl_music_t* music = SDL_LoadMusic(key.c_str(), path);
    if (!music)
        return;

    SDL_StopMusicChannel(0, true);
    CVector zero(0, 0, 0);
    SDL_CheckForSubtitle(name, music->audio);
    SDL_PlayAudio(music->audio, nullptr, name, zero, 0, 0, vol,
        0.0f, 0.0f, true, 0);
}

extern "C" DK_AUDIO_DLL_EXPORT void S_StartMusic(const char* name, int chan)
{
    if (!name || !name[0])
    {
        for (sdl_voice_t& voice : g_voices)
        {
            if (voice.music && voice.entchannel == chan)
            {
                if (voice.track && voice.paused)
                {
                    MIX_ResumeTrack(voice.track);
                    voice.paused = false;
                }
                SDL_SetMusicGain(voice);
                return;
            }
        }
        return;
    }

    char musicname[1024];
    int written;
    if (SDL_PathHasPrefix(name, "data/") || SDL_PathHasPrefix(name, "music/"))
    {
        written = std::snprintf(musicname, sizeof(musicname), "%s", name);
    }
    else if (SDL_HasExtension(name, ".mp3"))
    {
        written = std::snprintf(musicname, sizeof(musicname), "music/%s", name);
    }
    else
    {
        written = std::snprintf(musicname, sizeof(musicname), "music/%s.mp3", name);
    }

    if (written < 0 || (size_t)written >= sizeof(musicname))
        return;

    for (sdl_voice_t& voice : g_voices)
    {
        if (voice.music && voice.entchannel == chan && voice.source_name == musicname)
        {
            if (voice.track && voice.paused)
            {
                MIX_ResumeTrack(voice.track);
                voice.paused = false;
            }
            SDL_SetMusicGain(voice);
            return;
        }
    }

    SDL_StopMusicChannel(chan, true);
    S_StartMP3(musicname, chan, 1.0f, 0, true);
}

extern "C" DK_AUDIO_DLL_EXPORT void S_StopMusic(int chan)
{
    SDL_StopMusicChannel(chan, false);
}

extern "C" DK_AUDIO_DLL_EXPORT void S_StartMP3(const char* name, int channel, float fvol,
    int play_count, qboolean bRestart)
{
    if (!name || !name[0])
        return;

    float final_volume = fvol;
    if (channel >= CHAN_CINE_1 && channel <= CHAN_CINE_6)
        final_volume *= s_cine ? s_cine->value : 1.0f;

    for (size_t i = 0; i < g_voices.size();)
    {
        sdl_voice_t& voice = g_voices[i];
        if (voice.music && voice.entchannel == channel)
        {
            if (!bRestart && voice.source_name == name)
            {
                if (voice.track && voice.paused)
                {
                    MIX_ResumeTrack(voice.track);
                    voice.paused = false;
                }
                SDL_SetMusicGain(voice);
                return;
            }

            SDL_DestroyVoice(voice);
            g_voices.erase(g_voices.begin() + (ptrdiff_t)i);
            continue;
        }
        ++i;
    }

    sdl_music_t* music = SDL_FindMusic(name);
    if (!music)
    {
        MIX_Audio* audio = SDL_LoadMP3AudioFile(name, name);
        if (!audio)
            return;

        music = new sdl_music_t;
        music->name = name;
        music->audio = audio;
        g_music.push_back(music);
    }
    if (!music)
        return;

    SDL_CheckForSubtitle(name, music->audio);

    int loops = 0;
    if (play_count == 0)
        loops = -1;
    else if (play_count > 1)
        loops = play_count - 1;

    CVector zero(0, 0, 0);
    SDL_PlayAudio(music->audio, nullptr, name, zero,
        0, channel, final_volume, 0.0f, 0.0f,
        true, loops);
}

extern "C" DK_AUDIO_DLL_EXPORT void S_StopMP3(int channel)
{
    SDL_StopMusicChannel(channel, true);
}

extern "C" DK_AUDIO_DLL_EXPORT void S_PauseStreams(qboolean bOnOff)
{
    for (sdl_voice_t& voice : g_voices)
    {
        if (!voice.music || !voice.track)
            continue;

        if (bOnOff)
        {
            MIX_PauseTrack(voice.track);
            voice.paused = true;
        }
        else
        {
            MIX_ResumeTrack(voice.track);
            voice.paused = false;
        }
    }
}
