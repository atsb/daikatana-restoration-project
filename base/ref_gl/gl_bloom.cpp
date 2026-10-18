#include "gl_local.h"
#include <SDL3/SDL.h>
#include <string.h>

/* Some legacy OpenGL headers used by Daikatana do not expose the OpenGL 1.2
 * GL_CLAMP_TO_EDGE enum even though a 2.1 context supports it. */
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

/* GLSL bloom for the legacy OpenGL 2.1 renderer. */

typedef unsigned int GLuintBloom;
typedef unsigned int GLenumBloom;
typedef int GLintBloom;
typedef int GLsizeiBloom;
typedef char GLcharBloom;
typedef float GLfloatBloom;

typedef GLuintBloom (APIENTRY *PFNGLCREATESHADERPROC)(GLenumBloom);
typedef void (APIENTRY *PFNGLSHADERSOURCEPROC)(GLuintBloom, GLsizeiBloom, const GLcharBloom * const *, const GLintBloom *);
typedef void (APIENTRY *PFNGLCOMPILESHADERPROC)(GLuintBloom);
typedef void (APIENTRY *PFNGLGETSHADERIVPROC)(GLuintBloom, GLenumBloom, GLintBloom *);
typedef void (APIENTRY *PFNGLGETSHADERINFOLOGPROC)(GLuintBloom, GLsizeiBloom, GLsizeiBloom *, GLcharBloom *);
typedef void (APIENTRY *PFNGLDELETESHADERPROC)(GLuintBloom);
typedef GLuintBloom (APIENTRY *PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRY *PFNGLATTACHSHADERPROC)(GLuintBloom, GLuintBloom);
typedef void (APIENTRY *PFNGLLINKPROGRAMPROC)(GLuintBloom);
typedef void (APIENTRY *PFNGLGETPROGRAMIVPROC)(GLuintBloom, GLenumBloom, GLintBloom *);
typedef void (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC)(GLuintBloom, GLsizeiBloom, GLsizeiBloom *, GLcharBloom *);
typedef void (APIENTRY *PFNGLDELETEPROGRAMPROC)(GLuintBloom);
typedef void (APIENTRY *PFNGLUSEPROGRAMPROC)(GLuintBloom);
typedef GLintBloom (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)(GLuintBloom, const GLcharBloom *);
typedef void (APIENTRY *PFNGLUNIFORM1IPROC)(GLintBloom, GLintBloom);
typedef void (APIENTRY *PFNGLUNIFORM1FPROC)(GLintBloom, GLfloatBloom);
typedef void (APIENTRY *PFNGLUNIFORM2FPROC)(GLintBloom, GLfloatBloom, GLfloatBloom);

enum
{
    GL_COMPILE_STATUS_BLOOM = 0x8B81,
    GL_LINK_STATUS_BLOOM = 0x8B82,
    GL_INFO_LOG_LENGTH_BLOOM = 0x8B84,
    GL_VERTEX_SHADER_BLOOM = 0x8B31,
    GL_FRAGMENT_SHADER_BLOOM = 0x8B30
};

static PFNGLCREATESHADERPROC pglCreateShader;
static PFNGLSHADERSOURCEPROC pglShaderSource;
static PFNGLCOMPILESHADERPROC pglCompileShader;
static PFNGLGETSHADERIVPROC pglGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC pglGetShaderInfoLog;
static PFNGLDELETESHADERPROC pglDeleteShader;
static PFNGLCREATEPROGRAMPROC pglCreateProgram;
static PFNGLATTACHSHADERPROC pglAttachShader;
static PFNGLLINKPROGRAMPROC pglLinkProgram;
static PFNGLGETPROGRAMIVPROC pglGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC pglGetProgramInfoLog;
static PFNGLDELETEPROGRAMPROC pglDeleteProgram;
static PFNGLUSEPROGRAMPROC pglUseProgram;
static PFNGLGETUNIFORMLOCATIONPROC pglGetUniformLocation;
static PFNGLUNIFORM1IPROC pglUniform1i;
static PFNGLUNIFORM1FPROC pglUniform1f;
static PFNGLUNIFORM2FPROC pglUniform2f;

static GLuintBloom bloomSceneTexture;
static GLuintBloom bloomEffectTexture;
static GLuintBloom bloomPingTexture;
static GLuintBloom bloomCompositeProgram;
static GLuintBloom bloomExtractProgram;
static GLuintBloom bloomBlurProgram;
static int bloomWidth;
static int bloomHeight;
static int bloomSampleWidth;
static int bloomSampleHeight;
static qboolean bloomAvailable;
static GLintBloom bloomTextureUniform;
static GLintBloom bloomExtractTextureUniform;
static GLintBloom bloomExtractDownsampleTexelUniform;
static GLintBloom bloomExtractThresholdUniform;
static GLintBloom bloomIntensityUniform;
static GLintBloom bloomSaturationUniform;
static GLintBloom bloomBlurTextureUniform;
static GLintBloom bloomBlurDirUniform;

static cvar_t *bloomCvar;
static cvar_t *bloomIntensityCvar;
static cvar_t *bloomThresholdCvar;
static cvar_t *bloomSaturationCvar;
static cvar_t *bloomDebugCvar;

static const char *bloomVertexShader =
    "#version 120\n"
    "varying vec2 vTexCoord;\n"
    "void main() {\n"
    "    gl_Position = gl_Vertex;\n"
    "    vTexCoord = gl_MultiTexCoord0.st;\n"
    "}\n";

static const char *bloomCompositeShader =
    "#version 120\n"
    "uniform sampler2D uBloom;\n"
    "uniform float uIntensity;\n"
    "uniform float uSaturation;\n"
    "varying vec2 vTexCoord;\n"
    "\n"
    "void main() {\n"
    "    vec3 bloom = texture2D(uBloom, vTexCoord).rgb;\n"
    "    float luma = dot(bloom, vec3(0.2126, 0.7152, 0.0722));\n"
    "    bloom = mix(vec3(luma), bloom, clamp(uSaturation, 0.0, 1.0));\n"
    "    bloom *= 1.50 * uIntensity;\n"
    "    gl_FragColor = vec4(bloom, 1.0);\n"
    "}\n";

static const char *bloomExtractShader =
    "#version 120\n"
    "uniform sampler2D uScene;\n"
    "uniform vec2 uDownsampleTexel;\n"
    "uniform float uThreshold;\n"
    "varying vec2 vTexCoord;\n"
    "\n"
    "void main() {\n"
    "    vec2 d = uDownsampleTexel * 0.50;\n"
    "    vec3 c0 = texture2D(uScene, vTexCoord + vec2(-d.x, -d.y)).rgb;\n"
    "    vec3 c1 = texture2D(uScene, vTexCoord + vec2( d.x, -d.y)).rgb;\n"
    "    vec3 c2 = texture2D(uScene, vTexCoord + vec2(-d.x,  d.y)).rgb;\n"
    "    vec3 c3 = texture2D(uScene, vTexCoord + vec2( d.x,  d.y)).rgb;\n"
    "    vec3 c = (c0 + c1 + c2 + c3) * 0.25;\n"
    "    float lum = max(max(c.r, c.g), c.b);\n"
    "    float knee = max(0.18, uThreshold * 0.45);\n"
    "    float soft = smoothstep(uThreshold - knee, uThreshold + knee, lum);\n"
    "    gl_FragColor = vec4(c * soft, 1.0);\n"
    "}\n";

static const char *bloomBlurShader =
    "#version 120\n"
    "uniform sampler2D uBloom;\n"
    "uniform vec2 uDir;\n"
    "varying vec2 vTexCoord;\n"
    "\n"
    "void main() {\n"
    "    float w0 = 0.164000;\n"
    "    float w1 = 0.148000;\n"
    "    float w2 = 0.108000;\n"
    "    float w3 = 0.068000;\n"
    "    float w4 = 0.034000;\n"
    "    vec2 stepDir = uDir * 1.55;\n"
    "    vec3 sum = texture2D(uBloom, vTexCoord).rgb * w0;\n"
    "    sum += texture2D(uBloom, vTexCoord + stepDir * 1.0).rgb * w1;\n"
    "    sum += texture2D(uBloom, vTexCoord - stepDir * 1.0).rgb * w1;\n"
    "    sum += texture2D(uBloom, vTexCoord + stepDir * 2.0).rgb * w2;\n"
    "    sum += texture2D(uBloom, vTexCoord - stepDir * 2.0).rgb * w2;\n"
    "    sum += texture2D(uBloom, vTexCoord + stepDir * 3.0).rgb * w3;\n"
    "    sum += texture2D(uBloom, vTexCoord - stepDir * 3.0).rgb * w3;\n"
    "    sum += texture2D(uBloom, vTexCoord + stepDir * 4.0).rgb * w4;\n"
    "    sum += texture2D(uBloom, vTexCoord - stepDir * 4.0).rgb * w4;\n"
    "    gl_FragColor = vec4(sum, 1.0);\n"
    "}\n";

static void *BloomProc(const char *name)
{
    return SDL_GL_GetProcAddress(name);
}

static qboolean BloomLoadProcs(void)
{
    pglCreateShader = (PFNGLCREATESHADERPROC)BloomProc("glCreateShader");
    pglShaderSource = (PFNGLSHADERSOURCEPROC)BloomProc("glShaderSource");
    pglCompileShader = (PFNGLCOMPILESHADERPROC)BloomProc("glCompileShader");
    pglGetShaderiv = (PFNGLGETSHADERIVPROC)BloomProc("glGetShaderiv");
    pglGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)BloomProc("glGetShaderInfoLog");
    pglDeleteShader = (PFNGLDELETESHADERPROC)BloomProc("glDeleteShader");
    pglCreateProgram = (PFNGLCREATEPROGRAMPROC)BloomProc("glCreateProgram");
    pglAttachShader = (PFNGLATTACHSHADERPROC)BloomProc("glAttachShader");
    pglLinkProgram = (PFNGLLINKPROGRAMPROC)BloomProc("glLinkProgram");
    pglGetProgramiv = (PFNGLGETPROGRAMIVPROC)BloomProc("glGetProgramiv");
    pglGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)BloomProc("glGetProgramInfoLog");
    pglDeleteProgram = (PFNGLDELETEPROGRAMPROC)BloomProc("glDeleteProgram");
    pglUseProgram = (PFNGLUSEPROGRAMPROC)BloomProc("glUseProgram");
    pglGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)BloomProc("glGetUniformLocation");
    pglUniform1i = (PFNGLUNIFORM1IPROC)BloomProc("glUniform1i");
    pglUniform1f = (PFNGLUNIFORM1FPROC)BloomProc("glUniform1f");
    pglUniform2f = (PFNGLUNIFORM2FPROC)BloomProc("glUniform2f");

    return pglCreateShader && pglShaderSource && pglCompileShader && pglGetShaderiv &&
           pglGetShaderInfoLog && pglDeleteShader && pglCreateProgram && pglAttachShader &&
           pglLinkProgram && pglGetProgramiv && pglGetProgramInfoLog && pglDeleteProgram &&
           pglUseProgram && pglGetUniformLocation && pglUniform1i && pglUniform1f &&
           pglUniform2f;
}

static GLuintBloom BloomCompileShader(GLenumBloom type, const char *source)
{
    GLuintBloom shader;
    GLintBloom ok;
    GLintBloom logLength;
    char log[2048];
    const char *sources[1];

    shader = pglCreateShader(type);
    if (!shader)
        return 0;

    sources[0] = source;
    pglShaderSource(shader, 1, sources, NULL);
    pglCompileShader(shader);
    pglGetShaderiv(shader, GL_COMPILE_STATUS_BLOOM, &ok);
    if (ok)
        return shader;

    pglGetShaderiv(shader, GL_INFO_LOG_LENGTH_BLOOM, &logLength);
    if (logLength <= 0)
        logLength = 1;
    if (logLength > (GLintBloom)sizeof(log))
        logLength = (GLintBloom)sizeof(log);
    memset(log, 0, sizeof(log));
    pglGetShaderInfoLog(shader, logLength, NULL, log);
    ri.Con_Printf(PRINT_ALL, "Bloom shader compile failed: %s\n", log);
    pglDeleteShader(shader);
    return 0;
}

static GLuintBloom BloomCreateProgram(const char *fragmentSource)
{
    GLuintBloom vertex;
    GLuintBloom fragment;
    GLuintBloom program;
    GLintBloom ok;
    GLintBloom logLength;
    char log[2048];

    vertex = BloomCompileShader(GL_VERTEX_SHADER_BLOOM, bloomVertexShader);
    if (!vertex)
        return 0;

    fragment = BloomCompileShader(GL_FRAGMENT_SHADER_BLOOM, fragmentSource);
    if (!fragment)
    {
        pglDeleteShader(vertex);
        return 0;
    }

    program = pglCreateProgram();
    if (!program)
    {
        pglDeleteShader(vertex);
        pglDeleteShader(fragment);
        return 0;
    }

    pglAttachShader(program, vertex);
    pglAttachShader(program, fragment);
    pglLinkProgram(program);
    pglDeleteShader(vertex);
    pglDeleteShader(fragment);

    pglGetProgramiv(program, GL_LINK_STATUS_BLOOM, &ok);
    if (ok)
        return program;

    pglGetProgramiv(program, GL_INFO_LOG_LENGTH_BLOOM, &logLength);
    if (logLength <= 0)
        logLength = 1;
    if (logLength > (GLintBloom)sizeof(log))
        logLength = (GLintBloom)sizeof(log);
    memset(log, 0, sizeof(log));
    pglGetProgramInfoLog(program, logLength, NULL, log);
    ri.Con_Printf(PRINT_ALL, "Bloom program link failed: %s\n", log);
    pglDeleteProgram(program);
    return 0;
}

static void BloomDeleteTexture(GLuintBloom *texture)
{
    if (*texture)
    {
        qglDeleteTextures(1, texture);
        *texture = 0;
    }
}

static void BloomDestroyTargets(void)
{
    BloomDeleteTexture(&bloomSceneTexture);
    BloomDeleteTexture(&bloomEffectTexture);
    BloomDeleteTexture(&bloomPingTexture);
    bloomWidth = 0;
    bloomHeight = 0;
    bloomSampleWidth = 0;
    bloomSampleHeight = 0;
}

static GLuintBloom BloomCreateTexture(int width, int height)
{
    GLuintBloom texture;

    qglGenTextures(1, &texture);
    qglBindTexture(GL_TEXTURE_2D, texture);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    qglBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

static int BloomSampleDimension(int value, int limit)
{
    int result;

    result = value;
    if (result < 64)
        result = 64;
    if (result > limit)
        result = limit;
    return result;
}

static qboolean BloomCreateTargets(int width, int height)
{
    float scale;

    BloomDestroyTargets();

    /* Match the Yamagi/Quake bloom layout: work at half resolution. */
    scale = 0.5f;

    bloomWidth = width;
    bloomHeight = height;
    bloomSampleWidth = BloomSampleDimension((int)(width * scale + 0.5f), width);
    bloomSampleHeight = BloomSampleDimension((int)(height * scale + 0.5f), height);

    bloomSceneTexture = BloomCreateTexture(bloomWidth, bloomHeight);
    bloomEffectTexture = BloomCreateTexture(bloomSampleWidth, bloomSampleHeight);
    bloomPingTexture = BloomCreateTexture(bloomSampleWidth, bloomSampleHeight);

    if (!bloomSceneTexture || !bloomEffectTexture || !bloomPingTexture)
    {
        BloomDestroyTargets();
        return false;
    }

    /* Target creation binds textures directly rather than through GL_Bind(). */
    gl_state.nTexnum[0] = -1;
    gl_state.nTexEnv[0] = -1;
    gl_state.nTMU = 0;

    return true;
}

static void BloomSetOrtho(void)
{
    qglMatrixMode(GL_PROJECTION);
    qglPushMatrix();
    qglLoadIdentity();
    qglMatrixMode(GL_MODELVIEW);
    qglPushMatrix();
    qglLoadIdentity();
}

static void BloomRestoreOrtho(void)
{
    qglMatrixMode(GL_MODELVIEW);
    qglPopMatrix();
    qglMatrixMode(GL_PROJECTION);
    qglPopMatrix();
    qglMatrixMode(GL_MODELVIEW);
}

static void BloomDrawQuad(void)
{
    qglBegin(GL_QUADS);
    qglTexCoord2f(0.0f, 0.0f); qglVertex2f(-1.0f, -1.0f);
    qglTexCoord2f(1.0f, 0.0f); qglVertex2f( 1.0f, -1.0f);
    qglTexCoord2f(1.0f, 1.0f); qglVertex2f( 1.0f,  1.0f);
    qglTexCoord2f(0.0f, 1.0f); qglVertex2f(-1.0f,  1.0f);
    qglEnd();
}

void R_BloomSetCvars(cvar_t *bloom, cvar_t *intensity, cvar_t *threshold, cvar_t *saturation)
{
    bloomCvar = bloom;
    bloomIntensityCvar = intensity;
    bloomThresholdCvar = threshold;
    bloomSaturationCvar = saturation;
}

void R_BloomSetDebugCvar(cvar_t *debug)
{
    bloomDebugCvar = debug;
}

void R_BloomInit(void)
{
    bloomAvailable = false;

    if (!BloomLoadProcs())
    {
        ri.Con_Printf(PRINT_ALL, "...GLSL bloom unavailable (shader entry points missing)\n");
        return;
    }

    bloomCompositeProgram = BloomCreateProgram(bloomCompositeShader);
    bloomExtractProgram = BloomCreateProgram(bloomExtractShader);
    bloomBlurProgram = BloomCreateProgram(bloomBlurShader);
    if (!bloomCompositeProgram || !bloomExtractProgram || !bloomBlurProgram)
    {
        R_BloomShutdown();
        return;
    }

    bloomTextureUniform = pglGetUniformLocation(bloomCompositeProgram, "uBloom");
    bloomIntensityUniform = pglGetUniformLocation(bloomCompositeProgram, "uIntensity");
    bloomSaturationUniform = pglGetUniformLocation(bloomCompositeProgram, "uSaturation");

    bloomExtractTextureUniform = pglGetUniformLocation(bloomExtractProgram, "uScene");
    bloomExtractDownsampleTexelUniform = pglGetUniformLocation(bloomExtractProgram, "uDownsampleTexel");
    bloomExtractThresholdUniform = pglGetUniformLocation(bloomExtractProgram, "uThreshold");
    bloomBlurTextureUniform = pglGetUniformLocation(bloomBlurProgram, "uBloom");
    bloomBlurDirUniform = pglGetUniformLocation(bloomBlurProgram, "uDir");

    if (bloomTextureUniform < 0 || bloomIntensityUniform < 0 ||
        bloomSaturationUniform < 0 || bloomExtractTextureUniform < 0 ||
        bloomExtractDownsampleTexelUniform < 0 || bloomExtractThresholdUniform < 0 ||
        bloomBlurTextureUniform < 0 ||
        bloomBlurDirUniform < 0 || !BloomCreateTargets(vid.width, vid.height))
    {
        R_BloomShutdown();
        return;
    }

    bloomAvailable = true;
    ri.Con_Printf(PRINT_ALL, "...GLSL bloom enabled\n");
}

void R_BloomShutdown(void)
{
    BloomDestroyTargets();

    if (bloomCompositeProgram)
        pglDeleteProgram(bloomCompositeProgram);
    if (bloomExtractProgram)
        pglDeleteProgram(bloomExtractProgram);
    if (bloomBlurProgram)
        pglDeleteProgram(bloomBlurProgram);
    bloomCompositeProgram = 0;
    bloomExtractProgram = 0;
    bloomBlurProgram = 0;
    bloomAvailable = false;
}

void R_BloomFrame(void)
{
    int width;
    int height;
    float intensity;
    float threshold;
    float saturation;

    if (!bloomAvailable || !bloomCompositeProgram || !bloomExtractProgram || !bloomBlurProgram ||
        !bloomCvar || !bloomIntensityCvar || !bloomThresholdCvar || !bloomSaturationCvar ||
        !vid.width || !vid.height || !bloomCvar->value)
        return;

    /* Cinematic and no-world-model views are not normal 3D frames. */
    if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_LETTERBOX))
        return;

    if (gl_drawbuffer == NULL || stricmp(gl_drawbuffer->string, "GL_BACK") != 0)
        return;

    if (gl_state.stereo_enabled)
        return;

    width = vid.width;
    height = vid.height;
    if (width != bloomWidth || height != bloomHeight)
    {
        if (!BloomCreateTargets(width, height))
            return;
    }

    intensity = bloomIntensityCvar->value;
    threshold = bloomThresholdCvar->value;
    saturation = bloomSaturationCvar->value;
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 4.0f) intensity = 4.0f;
    if (threshold < 0.05f) threshold = 0.05f;
    if (threshold > 0.95f) threshold = 0.95f;
    if (saturation < 0.0f) saturation = 0.0f;
    if (saturation > 1.0f) saturation = 1.0f;

    qglDisable(GL_DEPTH_TEST);
    qglDisable(GL_CULL_FACE);
    qglDisable(GL_ALPHA_TEST);
    qglDisable(GL_BLEND);
    qglDisable(GL_SCISSOR_TEST);
    qglDisable(GL_FOG);
    qglEnable(GL_TEXTURE_2D);
    qglColor4f(1, 1, 1, 1);

    BloomSetOrtho();

    /* Always render and sample through texture unit 0. SGIS multitexture is
       optional in this renderer and is only used to select the legacy unit. */
    if (qglSelectTextureSGIS)
        qglSelectTextureSGIS(GL_TEXTURE0_SGIS);
    gl_state.nTMU = 0;
    qglEnable(GL_TEXTURE_2D);

    /* Preserve the finished 3D frame. */
    qglBindTexture(GL_TEXTURE_2D, bloomSceneTexture);
    qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);

    /* Bright pass: full scene -> half-resolution bright texture. */
    qglViewport(0, 0, bloomSampleWidth, bloomSampleHeight);
    pglUseProgram(bloomExtractProgram);
    pglUniform1i(bloomExtractTextureUniform, 0);
    pglUniform2f(bloomExtractDownsampleTexelUniform,
                 1.0f / (float)width, 1.0f / (float)height);
    pglUniform1f(bloomExtractThresholdUniform, threshold);
    BloomDrawQuad();
    pglUseProgram(0);

    qglEnable(GL_TEXTURE_2D);
    qglBindTexture(GL_TEXTURE_2D, bloomEffectTexture);
    qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
                         bloomSampleWidth, bloomSampleHeight);

    /* Horizontal Gaussian blur. */
    qglBindTexture(GL_TEXTURE_2D, bloomEffectTexture);
    pglUseProgram(bloomBlurProgram);
    pglUniform1i(bloomBlurTextureUniform, 0);
    pglUniform2f(bloomBlurDirUniform,
                 1.0f / (float)bloomSampleWidth, 0.0f);
    BloomDrawQuad();
    pglUseProgram(0);

    qglEnable(GL_TEXTURE_2D);
    qglBindTexture(GL_TEXTURE_2D, bloomPingTexture);
    qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
                         bloomSampleWidth, bloomSampleHeight);

    /* Vertical Gaussian blur. */
    qglBindTexture(GL_TEXTURE_2D, bloomPingTexture);
    pglUseProgram(bloomBlurProgram);
    pglUniform1i(bloomBlurTextureUniform, 0);
    pglUniform2f(bloomBlurDirUniform,
                 0.0f, 1.0f / (float)bloomSampleHeight);
    BloomDrawQuad();
    pglUseProgram(0);

    qglEnable(GL_TEXTURE_2D);
    qglBindTexture(GL_TEXTURE_2D, bloomEffectTexture);
    qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
                         bloomSampleWidth, bloomSampleHeight);

    /* Restore the unmodified scene before applying bloom. Explicitly enabling
       GL_TEXTURE_2D is essential because R_RenderView() can leave it disabled. */
    qglViewport(0, 0, width, height);
    qglEnable(GL_TEXTURE_2D);
    qglBindTexture(GL_TEXTURE_2D, bloomSceneTexture);
    qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    qglColor4f(1, 1, 1, 1);
    BloomDrawQuad();

    /* Add the blurred bright image over the restored scene. */
    if (bloomDebugCvar && bloomDebugCvar->value)
    {
        qglEnable(GL_TEXTURE_2D);
        qglBindTexture(GL_TEXTURE_2D, bloomEffectTexture);
        qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        qglColor4f(1, 1, 1, 1);
        BloomDrawQuad();
    }
    else
    {
        qglEnable(GL_TEXTURE_2D);
        qglBindTexture(GL_TEXTURE_2D, bloomEffectTexture);
        qglEnable(GL_BLEND);
        qglBlendFunc(GL_ONE, GL_ONE);

        pglUseProgram(bloomCompositeProgram);
        pglUniform1i(bloomTextureUniform, 0);
        pglUniform1f(bloomIntensityUniform, intensity);
        pglUniform1f(bloomSaturationUniform, saturation);
        BloomDrawQuad();
        pglUseProgram(0);
    }

    qglDisable(GL_BLEND);
    qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    qglBindTexture(GL_TEXTURE_2D, 0);
    gl_state.nTexnum[0] = -1;
    gl_state.nTexEnv[0] = -1;
    gl_state.nTMU = 0;

    /* Direct GL calls above bypass the renderer state cache. */
    gl_state.nStateFlags = 0;
    GL_SetState(GLSTATE_ALPHA_TEST | GLSTATE_TEXTURE_2D);

    BloomRestoreOrtho();
    qglViewport(0, 0, width, height);
}
