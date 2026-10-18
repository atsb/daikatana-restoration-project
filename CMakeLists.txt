cmake_minimum_required(VERSION 3.25)

project(DaikatanaV10 LANGUAGES C CXX)

# Keep every target in this build on the same MSVC runtime. This is set before
# project targets are created so every compiled object uses the same CRT
# (/MT in Release, /MTd in Debug).
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# -----------------------------------------------------------------------------
# Daikatana v1.0 restoration build
#
# The source tree itself is the source root.  Nothing here assumes the name or
# location of the ZIP file, and there are deliberately no absolute source
# paths.  The legacy VC6 DSP files are used only as source-membership records.
# -----------------------------------------------------------------------------

if(NOT WIN32)
    message(WARNING "Daikatana v1.0 is a Win32 build. Configure is allowed on non-Windows hosts only for source-tree validation.")
endif()

# The original engine was Win32-era code, but the source below has been audited
# so pointer-bearing state is pointer-sized while file/protocol/gameplay scalar
# widths remain unchanged. Allow both Win32 and Win64 builds.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_compile_definitions(DK_64BIT=1 DK_POINTER_BITS=64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_compile_definitions(DK_32BIT=1 DK_POINTER_BITS=32)
else()
    message(FATAL_ERROR "Unsupported pointer size: ${CMAKE_SIZEOF_VOID_P}")
endif()

set(DK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(DK_USER "${DK_ROOT}/user")
set(DK_BASE "${DK_ROOT}/base")
set(DK_CLIENT "${DK_BASE}/client")
set(DK_QCOMMON "${DK_BASE}/qcommon")
set(DK_SERVER "${DK_BASE}/server")
set(DK_WIN32 "${DK_BASE}/win32")
set(DK_DK_ "${DK_BASE}/dk_")
set(DK_REF_COMMON "${DK_BASE}/ref_common")
set(DK_AUDIO "${DK_BASE}/Audio")
set(DK_COLLECTION "${DK_USER}/Collection")
set(DK_LIBENT "${DK_ROOT}/libent")
set(DK_LIBS "${DK_ROOT}/Libs")

set(DK_MSS32_LIB "${DK_LIBS}/mss32.lib")
set(DK_BINK32_LIB "${DK_LIBS}/binkw32.lib")

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_net CONFIG REQUIRED)

# -----------------------------------------------------------------------------
# Local tactical decision engine
# -----------------------------------------------------------------------------
# The game-side decision layer is an in-process C/C++ implementation. It does
# not depend on an external inference runtime or a network service. The runtime
# cvar remains named dk_neural_ai for compatibility.
option(DK_ENABLE_NEURAL_AI "Build the Daikatana local tactical decision engine" ON)

if(DK_ENABLE_NEURAL_AI)
    add_library(dk_neural_ai STATIC
        "${DK_ROOT}/base/dk_neural_ai.cpp"
        "${DK_ROOT}/base/dk_neural_ai.h"
        "${DK_ROOT}/base/dk_neural_training_format.h"
    )
    target_include_directories(dk_neural_ai PRIVATE
        "${DK_ROOT}"
        "${DK_BASE}"
        "${DK_USER}"
        "${DK_QCOMMON}"
    )
    set_target_properties(dk_neural_ai PROPERTIES
        PROJECT_LABEL "Daikatana Tactical Decision Engine"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endif()

# -----------------------------------------------------------------------------
# Standalone offline 5M trainer
# -----------------------------------------------------------------------------
# This target is deliberately pure C and has no Python, PyTorch, llama.cpp,
# or other runtime dependency.  It consumes the binary samples captured by
# Advanced Reasoning and writes the exact 5M model format used by the game.
option(DK_BUILD_NEURAL_TRAINER "Build the standalone pure-C 5M tactical trainer" ON)

if(DK_BUILD_NEURAL_TRAINER)
    add_executable(dk_neural_trainer
        "${DK_ROOT}/base/dk_neural_trainer.c"
        "${DK_ROOT}/base/dk_neural_ai.h"
        "${DK_ROOT}/base/dk_neural_training_format.h"
    )
    target_include_directories(dk_neural_trainer PRIVATE
        "${DK_ROOT}"
        "${DK_BASE}"
    )
    if(MSVC)
        target_compile_options(dk_neural_trainer PRIVATE /W3 /TC)
    else()
        target_compile_options(dk_neural_trainer PRIVATE -Wall -Wextra -Wpedantic -std=c99)
        target_link_libraries(dk_neural_trainer PRIVATE m)
    endif()
    set_target_properties(dk_neural_trainer PROPERTIES
        PROJECT_LABEL "Daikatana 5M Tactical Trainer"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools"
    )
endif()

# Preserve the historical C/C++ view of the engine.  In particular, on an x86
# compiler _M_IX86 remains defined, but C_ONLY forces id386=0 in dk_defines.h,
# activating the renderer's portable C implementations instead of MASM/x86
# code paths.
add_compile_definitions(WIN32 _WINDOWS TONGUE_ENGLISH C_ONLY)

if(MSVC)
    # Compatibility with the old code's loop-variable expectations.
    add_compile_options(/W3 /Zc:forScope- /EHsc /GR-)
endif()

# -----------------------------------------------------------------------------
# Legacy DSP parser
# -----------------------------------------------------------------------------
# We do not copy path strings out of the old projects.  Instead we resolve each
# SOURCE= entry relative to the DSP that owns it, at CMake configure time.
# MASM .asm entries are intentionally ignored; the C_ONLY branches provide the
# renderer implementation in C/C++.
function(dk_collect_dsp_sources OUT_VAR DSP_FILE)
    if(NOT EXISTS "${DSP_FILE}")
        message(FATAL_ERROR "Missing legacy DSP file: ${DSP_FILE}")
    endif()

    get_filename_component(DSP_DIR "${DSP_FILE}" DIRECTORY)
    file(STRINGS "${DSP_FILE}" DSP_LINES)
    set(SOURCES)

    foreach(LINE IN LISTS DSP_LINES)
        if(LINE MATCHES "^SOURCE=(.*)$")
            set(REL "${CMAKE_MATCH_1}")
            string(REPLACE "\\" "/" REL "${REL}")
            get_filename_component(EXT "${REL}" EXT)
            string(TOLOWER "${EXT}" EXT_LOWER)

            if(EXT_LOWER STREQUAL ".c" OR
               EXT_LOWER STREQUAL ".cc" OR
               EXT_LOWER STREQUAL ".cpp" OR
               EXT_LOWER STREQUAL ".cxx")
                cmake_path(ABSOLUTE_PATH REL BASE_DIRECTORY "${DSP_DIR}" NORMALIZE OUTPUT_VARIABLE ABS)
                if(NOT EXISTS "${ABS}")
                    # The historical DSPs contain case-only filename differences
                    # which work on normal Windows filesystems but not on Linux
                    # validation hosts. Resolve an exact sibling case-insensitively
                    # before deciding the entry is genuinely missing.
                    get_filename_component(_parent "${ABS}" DIRECTORY)
                    get_filename_component(_name "${ABS}" NAME)
                    string(TOLOWER "${_name}" _name_lower)
                    set(_case_match "")
                    if(IS_DIRECTORY "${_parent}")
                        file(GLOB _siblings LIST_DIRECTORIES false "${_parent}/*")
                        foreach(_sibling IN LISTS _siblings)
                            get_filename_component(_sibling_name "${_sibling}" NAME)
                            string(TOLOWER "${_sibling_name}" _sibling_lower)
                            if(_sibling_lower STREQUAL _name_lower)
                                set(_case_match "${_sibling}")
                                break()
                            endif()
                        endforeach()
                    endif()

                    if(_case_match)
                        list(APPEND SOURCES "${_case_match}")
                    else()
                        # One stale SOURCE= entry exists in the clean v1.0 world.dsp
                        # for ../Daikatana/game/p_client.c. That file is not present
                        # in the public archive, so do not manufacture a source file.
                        get_filename_component(_missing_name "${REL}" NAME)
                        if(_missing_name STREQUAL "p_client.c")
                            message(WARNING "Ignoring absent legacy DSP entry '${REL}' in '${DSP_FILE}'; no such source is present in the clean v1.0 archive.")
                        else()
                            message(FATAL_ERROR "DSP source does not exist: '${REL}' from '${DSP_FILE}' (resolved to '${ABS}')")
                        endif()
                    endif()
                else()
                    list(APPEND SOURCES "${ABS}")
                endif()
            elseif(EXT_LOWER STREQUAL ".asm")
                # Historical x86/MASM source; intentionally excluded in C_ONLY.
            endif()
        endif()
    endforeach()

    list(REMOVE_DUPLICATES SOURCES)
    set(${OUT_VAR} "${SOURCES}" PARENT_SCOPE)
endfunction()

function(dk_target_include_dirs OUT_VAR DSP_FILE)
    get_filename_component(DSP_DIR "${DSP_FILE}" DIRECTORY)

    # Use only paths derived from the current source tree.  We intentionally
    # avoid trying to interpret the old VC6 command-line text here; the legacy
    # projects contain a mix of configuration-specific include paths.  This
    # stable superset mirrors the actual source layout and prevents archive-
    # filename/extraction-path assumptions.
    set(INCLUDES
        "${DSP_DIR}"
        "${DK_ROOT}"
        "${DK_USER}"
        "${DK_CLIENT}"
        "${DK_QCOMMON}"
        "${DK_SERVER}"
        "${DK_WIN32}"
        "${DK_DK_}"
        "${DK_REF_COMMON}"
        "${DK_AUDIO}"
        "${DK_COLLECTION}"
        "${DK_LIBENT}"
        "${DK_BASE}"
    )
    list(FILTER INCLUDES EXCLUDE REGEX "^$")
    list(REMOVE_DUPLICATES INCLUDES)
    set(${OUT_VAR} "${INCLUDES}" PARENT_SCOPE)
endfunction()

function(dk_configure_target TARGET DSP_FILE)
    dk_target_include_dirs(INCLUDES "${DSP_FILE}")
    target_include_directories(${TARGET} PRIVATE ${INCLUDES})
    if(MSVC)
        # /Y- disables old VC6 automatic precompiled-header assumptions.
        target_compile_options(${TARGET} PRIVATE /Y-)
    endif()
endfunction()

# -----------------------------------------------------------------------------
# Source lists directly from the clean v1.0 DSP files.
# -----------------------------------------------------------------------------
dk_collect_dsp_sources(IONCOMMON_SRC     "${DK_ROOT}/IonCommon/IonCommon.dsp")
dk_collect_dsp_sources(LIBENT_SRC       "${DK_ROOT}/libent/libent.dsp")
dk_collect_dsp_sources(PHYSICS_SRC      "${DK_ROOT}/base/physics/physics.dsp")
dk_collect_dsp_sources(WORLD_SRC        "${DK_ROOT}/dlls/world/world.dsp")
dk_collect_dsp_sources(GCE_SRC          "${DK_ROOT}/dlls/gce/gce.dsp")
dk_collect_dsp_sources(WEAPONS_SRC      "${DK_ROOT}/dlls/weapons/Weapons.dsp")
dk_collect_dsp_sources(DAIKATANA_SRC    "${DK_ROOT}/base/daikatana.dsp")
dk_collect_dsp_sources(REF_SOFT_SRC     "${DK_ROOT}/base/ref_soft/ref_soft.dsp")
dk_collect_dsp_sources(REF_GL_SRC       "${DK_ROOT}/base/ref_gl/ref_gl.dsp")

# Explicit Restoration Edition renderer sources. Keep these independent of the
# legacy DSP so bloom and N64 texture filtering remain in the ref_gl target.
if(EXISTS "${DK_ROOT}/base/ref_gl/gl_bloom.cpp")
    list(APPEND REF_GL_SRC "${DK_ROOT}/base/ref_gl/gl_bloom.cpp")
else()
    message(FATAL_ERROR "Missing OpenGL bloom source: ${DK_ROOT}/base/ref_gl/gl_bloom.cpp")
endif()

if(EXISTS "${DK_ROOT}/base/ref_gl/gl_texture_filter.cpp")
    list(APPEND REF_GL_SRC "${DK_ROOT}/base/ref_gl/gl_texture_filter.cpp")
else()
    message(FATAL_ERROR "Missing OpenGL texture filtering source: ${DK_ROOT}/base/ref_gl/gl_texture_filter.cpp")
endif()

list(REMOVE_DUPLICATES REF_GL_SRC)
dk_collect_dsp_sources(AUDIO_MILES_SRC  "${DK_ROOT}/base/Audio/DkAudioEngine_Miles/DkAudioEngine_Miles.dsp")

# Restoration Edition 1.2 sources. These files were restored after the original
# legacy DSP membership was established, so keep them explicit here rather than
# relying solely on the historical project file. This also guarantees that a
# fresh CMake/Visual Studio generation links the implementations, not just the
# headers/callers.
list(APPEND DAIKATANA_SRC
    "${DK_ROOT}/base/dk_/dk_menu_weapons.cpp"
    "${DK_ROOT}/base/server/sv_clientString.cpp"
)

dk_collect_dsp_sources(REF_D3D_SRC      "${DK_ROOT}/base/ref_d3d/ref_d3d.dsp")
dk_collect_dsp_sources(REF_GLIDE_SRC    "${DK_ROOT}/base/ref_glide/ref_glide.dsp")

# -----------------------------------------------------------------------------
# Core libraries / DLLs.
# -----------------------------------------------------------------------------
add_library(IonCommon STATIC ${IONCOMMON_SRC})
dk_configure_target(IonCommon "${DK_ROOT}/IonCommon/IonCommon.dsp")
target_link_libraries(IonCommon PUBLIC SDL3_net::SDL3_net)
set_target_properties(IonCommon PROPERTIES OUTPUT_NAME "IonCommon")

add_library(libent STATIC ${LIBENT_SRC})
dk_configure_target(libent "${DK_ROOT}/libent/libent.dsp")
set_target_properties(libent PROPERTIES OUTPUT_NAME "libent")
target_link_libraries(libent PRIVATE IonCommon)

add_library(physics SHARED ${PHYSICS_SRC})
dk_configure_target(physics "${DK_ROOT}/base/physics/physics.dsp")
target_link_libraries(physics PRIVATE IonCommon
    winmm winspool comdlg32 shell32 ole32 oleaut32
    advapi32 kernel32 user32 gdi32
)

add_library(world SHARED ${WORLD_SRC})
dk_configure_target(world "${DK_ROOT}/dlls/world/world.dsp")
target_link_libraries(world PRIVATE IonCommon
    winmm winspool comdlg32 shell32 ole32 oleaut32 uuid
    advapi32 kernel32 user32 gdi32
)
set_target_properties(world PROPERTIES OUTPUT_NAME "world")

add_library(gce SHARED ${GCE_SRC})
dk_configure_target(gce "${DK_ROOT}/dlls/gce/gce.dsp")
target_link_libraries(gce PRIVATE IonCommon
    odbc32 odbccp32 winspool comdlg32 shell32 ole32 oleaut32 uuid
    advapi32 kernel32 user32 gdi32 winmm
)
set_target_properties(gce PROPERTIES OUTPUT_NAME "gce")

add_library(weapons SHARED ${WEAPONS_SRC})
dk_configure_target(weapons "${DK_ROOT}/dlls/weapons/Weapons.dsp")
target_link_libraries(weapons PRIVATE world IonCommon
    winmm winspool comdlg32 shell32 ole32 oleaut32 uuid
    advapi32 kernel32 user32 gdi32
)
set_target_properties(weapons PROPERTIES OUTPUT_NAME "weapons")

# The original game executable loads the game-side DLLs at runtime.
# On Win64 the legacy cd_win.cpp cannot be linked because it depends on
# mss32. Replace only that source with a no-op compatibility shim.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    list(FILTER DAIKATANA_SRC EXCLUDE REGEX "/base/win32/cd_win\.cpp$")
    list(APPEND DAIKATANA_SRC "${DK_ROOT}/base/win32/cdaudio_stub.cpp")
endif()

list(REMOVE_ITEM DAIKATANA_SRC "${DK_ROOT}/base/win32/net_wins.cpp")
list(REMOVE_ITEM DAIKATANA_SRC "${DK_ROOT}/linux/net_bsd.cpp")
list(APPEND DAIKATANA_SRC "${DK_ROOT}/base/qcommon/net_sdl3.cpp")

# Replace the legacy Win32 input backend with the SDL3 implementation.
list(REMOVE_ITEM DAIKATANA_SRC "${DK_ROOT}/base/win32/in_win.cpp")
list(APPEND DAIKATANA_SRC
    "${DK_ROOT}/base/client/in_sdl3.cpp"
)

add_executable(daikatana WIN32 ${DAIKATANA_SRC})
dk_configure_target(daikatana "${DK_ROOT}/base/daikatana.dsp")
target_link_libraries(daikatana PRIVATE IonCommon
    SDL3::SDL3
    SDL3_net::SDL3_net
    winmm ole32 advapi32 kernel32 user32 gdi32
)
if(DK_ENABLE_NEURAL_AI)
    add_dependencies(daikatana dk_neural_ai)
    target_link_libraries(daikatana PRIVATE dk_neural_ai)
endif()
if(DK_BUILD_NEURAL_TRAINER)
    # Build the offline trainer as part of the normal Daikatana solution without
    # linking its own main() into the game executable.
    add_dependencies(daikatana dk_neural_trainer)
endif()
set_target_properties(daikatana PROPERTIES OUTPUT_NAME "daikatana")


# The Miles library is a 32-bit-only dependency. Keep it available only for
# the unchanged Win32 build. Win64 uses the SDL3 audio DLL below instead.
if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND EXISTS "${DK_MSS32_LIB}")
    add_library(DkAudioEngine_Miles SHARED ${AUDIO_MILES_SRC})
    dk_configure_target(DkAudioEngine_Miles "${DK_ROOT}/base/Audio/DkAudioEngine_Miles/DkAudioEngine_Miles.dsp")
    target_link_libraries(DkAudioEngine_Miles PRIVATE IonCommon user32 "${DK_MSS32_LIB}")
    set_target_properties(DkAudioEngine_Miles PROPERTIES OUTPUT_NAME "audio")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING "${DK_MSS32_LIB} is absent; DkAudioEngine_Miles will not be built.")
endif()

# SDL3 audio is used only for Win64 in this stage. This replaces the 32-bit-only
# Miles dependency without changing the known-good x86 audio path.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_package(SDL3_mixer CONFIG REQUIRED)

    set(SDL3_AUDIO_SRC
        "${DK_ROOT}/base/Audio/DkAudioEngine_SDL3/DkAudioEngine_SDL3.cpp"
        "${DK_ROOT}/base/Audio/DkAudioEngine_SDL3/DllMain.cpp"
    )
    add_library(DkAudioEngine_SDL3 SHARED ${SDL3_AUDIO_SRC})
    dk_configure_target(DkAudioEngine_SDL3 "${DK_ROOT}/base/Audio/DkAudioEngine_Miles/DkAudioEngine_Miles.dsp")
    target_compile_features(DkAudioEngine_SDL3 PRIVATE cxx_std_17)
    target_link_libraries(DkAudioEngine_SDL3 PRIVATE
        IonCommon
        SDL3::SDL3
        SDL3_mixer::SDL3_mixer
        user32
        mfplat
        mfreadwrite
        mfuuid
        ole32
    )
    target_include_directories(DkAudioEngine_SDL3 PRIVATE
        "${DK_ROOT}/base/Audio"
        "${DK_ROOT}/base/Audio/DkAudioEngine_SDL3"
    )
    set_target_properties(DkAudioEngine_SDL3 PROPERTIES OUTPUT_NAME "audio")
endif()

# -----------------------------------------------------------------------------
# Software renderer -- C implementation, no MASM.
# -----------------------------------------------------------------------------
option(DK_BUILD_REF_SOFT "Build the software renderer" ON)
if(DK_BUILD_REF_SOFT)
    add_library(ref_soft SHARED ${REF_SOFT_SRC})
    dk_configure_target(ref_soft "${DK_ROOT}/base/ref_soft/ref_soft.dsp")
    target_link_libraries(ref_soft PRIVATE IonCommon
        winmm advapi32 kernel32 user32 gdi32
    )
    target_link_options(ref_soft PRIVATE "/DEF:${DK_ROOT}/base/ref_soft/ref_soft.def")
    set_target_properties(ref_soft PROPERTIES OUTPUT_NAME "ref_soft")

    # Explicitly document that MASM has not slipped back into this target.
    set_target_properties(ref_soft PROPERTIES PROJECT_LABEL "ref_soft (C_ONLY)")
endif()

# OpenGL renderer is the supported hardware renderer for the restoration.
# SDL3 owns the video window, OpenGL context, fullscreen mode and buffer
# presentation. The renderer itself continues to use the original fixed-
# function OpenGL API. No Glide SDK is required.
option(DK_BUILD_REF_GL "Build the OpenGL renderer" ON)
if(DK_BUILD_REF_GL)
    add_library(ref_gl SHARED ${REF_GL_SRC})
    dk_configure_target(ref_gl "${DK_ROOT}/base/ref_gl/ref_gl.dsp")
    target_link_libraries(ref_gl PRIVATE IonCommon
        SDL3::SDL3
        winmm advapi32 kernel32 user32
    )
    target_link_options(ref_gl PRIVATE "/DEF:${DK_ROOT}/base/ref_gl/ref_gl.def")
    set_target_properties(ref_gl PROPERTIES OUTPUT_NAME "ref_gl")
endif()

# These historical renderers depend on SDKs not shipped with the public
# archive. Keep them available as opt-in source graphs rather than making the
# normal x86 restoration build depend on unavailable third-party SDKs.
option(DK_BUILD_REF_D3D "Build the legacy Direct3D renderer" OFF)
if(DK_BUILD_REF_D3D)
    add_library(ref_d3d SHARED ${REF_D3D_SRC})
    dk_configure_target(ref_d3d "${DK_ROOT}/base/ref_d3d/ref_d3d.dsp")
    target_link_libraries(ref_d3d PRIVATE ddraw d3dim dxguid winmm advapi32 kernel32 user32 gdi32)
    set_target_properties(ref_d3d PROPERTIES OUTPUT_NAME "ref_d3d")
endif()

option(DK_BUILD_REF_GLIDE "Build the historical Glide renderer" OFF)
if(DK_BUILD_REF_GLIDE)
    add_library(ref_glide SHARED ${REF_GLIDE_SRC})
    dk_configure_target(ref_glide "${DK_ROOT}/base/ref_glide/ref_glide.dsp")
    target_link_libraries(ref_glide PRIVATE glide2x kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 uuid ole32 oleaut32 odbc32 odbccp32)
    set_target_properties(ref_glide PROPERTIES OUTPUT_NAME "ref_glide")
endif()

# -----------------------------------------------------------------------------
# Optional legacy MFC collection project.
# It is not part of the ordinary game build and requires MFC.
# -----------------------------------------------------------------------------
option(DK_BUILD_IONCOLLECTION "Build the legacy MFC IonCollection library" OFF)
if(DK_BUILD_IONCOLLECTION AND MSVC)
    dk_collect_dsp_sources(IONCOLLECTION_SRC "${DK_ROOT}/user/Collection/IonCollection/IonCollection.dsp")
    add_library(IonCollection STATIC ${IONCOLLECTION_SRC})
    dk_configure_target(IonCollection "${DK_ROOT}/user/Collection/IonCollection/IonCollection.dsp")
    set_target_properties(IonCollection PROPERTIES OUTPUT_NAME "IonCollection")
else()
    if(DK_BUILD_IONCOLLECTION)
        message(WARNING "IonCollection requires MSVC/MFC; target disabled on this host.")
    endif()
endif()

# -----------------------------------------------------------------------------
# Output layout
# -----------------------------------------------------------------------------
# Keep all runtime binaries together. The restoration can still copy the DLLs
# next to the executable or into the historical directory layout afterward.
set(DK_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(DK_LIB_DIR "${CMAKE_BINARY_DIR}/lib")
set(DK_RUNTIME_TARGETS daikatana dk_neural_ai world physics gce weapons ref_soft ref_gl DkAudioEngine_Miles DkAudioEngine_SDL3 ref_d3d)
foreach(T IN LISTS DK_RUNTIME_TARGETS)
    if(TARGET ${T})
        set_target_properties(${T} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${DK_BIN_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${DK_BIN_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${DK_LIB_DIR}"
        )
    endif()
endforeach()

# Helpful IDE grouping when CMake generates Visual Studio.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
foreach(T IonCommon libent dk_neural_ai physics world gce weapons daikatana DkAudioEngine_Miles DkAudioEngine_SDL3 ref_soft ref_gl ref_d3d IonCollection)
    if(TARGET ${T})
        set_target_properties(${T} PROPERTIES FOLDER "Daikatana v1.0")
    endif()
endforeach()

message(STATUS "Daikatana v1.0 CMake source root: ${DK_ROOT}")
message(STATUS "Daikatana v1.0 target ABI: ${CMAKE_SIZEOF_VOID_P} byte pointers, C_ONLY=1; SDL3 audio enabled; Tactical Decision Engine=${DK_ENABLE_NEURAL_AI}")
