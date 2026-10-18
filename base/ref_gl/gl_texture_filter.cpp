#include "gl_local.h"
#include <SDL3/SDL.h>
#include <string.h>

#define GL_COMPILE_STATUS_TEXTURE_FILTER 0x8B81
#define GL_LINK_STATUS_TEXTURE_FILTER 0x8B82
#define GL_INFO_LOG_LENGTH_TEXTURE_FILTER 0x8B84
#define GL_VERTEX_SHADER_TEXTURE_FILTER 0x8B31
#define GL_FRAGMENT_SHADER_TEXTURE_FILTER 0x8B30

#define GL_TEXTURE0_SGIS_TEXTURE_FILTER 0x835E
#define GL_TEXTURE1_SGIS_TEXTURE_FILTER 0x835F
/* The shader path is only active for gl_texture_filter=2 (N64). */
typedef unsigned int TextureFilterGLuint;
typedef unsigned int TextureFilterGLenum;
typedef int TextureFilterGLint;
typedef int TextureFilterGLsizei;
typedef char TextureFilterGLchar;
typedef float TextureFilterGLfloat;

typedef TextureFilterGLuint (APIENTRY *PFNGLCREATESHADERPROC)(TextureFilterGLenum);
typedef void (APIENTRY *PFNGLSHADERSOURCEPROC)(TextureFilterGLuint, TextureFilterGLsizei, const TextureFilterGLchar * const *, const TextureFilterGLint *);
typedef void (APIENTRY *PFNGLCOMPILESHADERPROC)(TextureFilterGLuint);
typedef void (APIENTRY *PFNGLGETSHADERIVPROC)(TextureFilterGLuint, TextureFilterGLenum, TextureFilterGLint *);
typedef void (APIENTRY *PFNGLGETSHADERINFOLOGPROC)(TextureFilterGLuint, TextureFilterGLsizei, TextureFilterGLsizei *, TextureFilterGLchar *);
typedef void (APIENTRY *PFNGLDELETESHADERPROC)(TextureFilterGLuint);
typedef TextureFilterGLuint (APIENTRY *PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRY *PFNGLATTACHSHADERPROC)(TextureFilterGLuint, TextureFilterGLuint);
typedef void (APIENTRY *PFNGLLINKPROGRAMPROC)(TextureFilterGLuint);
typedef void (APIENTRY *PFNGLGETPROGRAMIVPROC)(TextureFilterGLuint, TextureFilterGLenum, TextureFilterGLint *);
typedef void (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC)(TextureFilterGLuint, TextureFilterGLsizei, TextureFilterGLsizei *, TextureFilterGLchar *);
typedef void (APIENTRY *PFNGLDELETEPROGRAMPROC)(TextureFilterGLuint);
typedef void (APIENTRY *PFNGLUSEPROGRAMPROC)(TextureFilterGLuint);
typedef TextureFilterGLint (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)(TextureFilterGLuint, const TextureFilterGLchar *);
typedef void (APIENTRY *PFNGLUNIFORM1IPROC)(TextureFilterGLint, TextureFilterGLint);
typedef void (APIENTRY *PFNGLUNIFORM1FPROC)(TextureFilterGLint, TextureFilterGLfloat);
typedef void (APIENTRY *PFNGLUNIFORM2FPROC)(TextureFilterGLint, TextureFilterGLfloat, TextureFilterGLfloat);
typedef void (APIENTRY *PFNGLUNIFORM4FPROC)(TextureFilterGLint, TextureFilterGLfloat, TextureFilterGLfloat, TextureFilterGLfloat, TextureFilterGLfloat);

static PFNGLCREATESHADERPROC tfCreateShader;
static PFNGLSHADERSOURCEPROC tfShaderSource;
static PFNGLCOMPILESHADERPROC tfCompileShader;
static PFNGLGETSHADERIVPROC tfGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC tfGetShaderInfoLog;
static PFNGLDELETESHADERPROC tfDeleteShader;
static PFNGLCREATEPROGRAMPROC tfCreateProgram;
static PFNGLATTACHSHADERPROC tfAttachShader;
static PFNGLLINKPROGRAMPROC tfLinkProgram;
static PFNGLGETPROGRAMIVPROC tfGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC tfGetProgramInfoLog;
static PFNGLDELETEPROGRAMPROC tfDeleteProgram;
static PFNGLUSEPROGRAMPROC tfUseProgram;
static PFNGLGETUNIFORMLOCATIONPROC tfGetUniformLocation;
static PFNGLUNIFORM1IPROC tfUniform1i;
static PFNGLUNIFORM1FPROC tfUniform1f;
static PFNGLUNIFORM2FPROC tfUniform2f;
static PFNGLUNIFORM4FPROC tfUniform4f;

static TextureFilterGLuint tfProgram;
static qboolean tfAvailable;
static qboolean tfGradient;
static TextureFilterGLint tfBaseSampler;
static TextureFilterGLint tfSecondarySampler;
static TextureFilterGLint tfBaseTexSize;
static TextureFilterGLint tfSecondaryTexSize;
static TextureFilterGLint tfMode;
static TextureFilterGLint tfEnvColor;
static TextureFilterGLint tfFogEnabled;
static TextureFilterGLint tfFogStart;
static TextureFilterGLint tfFogEnd;
static TextureFilterGLint tfFogColor;
static TextureFilterGLint tfAlphaTest;
static TextureFilterGLint tfAlphaRef;

static qboolean tfProgramBound;
static qboolean tfCacheValid;
static int tfLastBaseTexnum;
static float tfLastBaseWidth;
static float tfLastBaseHeight;
static int tfLastSecondaryTexnum;
static float tfLastSecondaryWidth;
static float tfLastSecondaryHeight;
static int tfLastMode;
static float tfLastEnvColor;
static int tfLastFogEnabled;
static float tfLastFogStart;
static float tfLastFogEnd;
static float tfLastFogR;
static float tfLastFogG;
static float tfLastFogB;
static int tfLastAlphaTest;
static float tfLastAlphaRef;

static const char *tfVertexShader =
    "#version 120\n"
    "varying vec2 vTex0;\n"
    "varying vec2 vTex1;\n"
    "varying vec4 vColor;\n"
    "varying float vFogDepth;\n"
    "void main() {\n"
    "    vec4 eye = gl_ModelViewMatrix * gl_Vertex;\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "    vTex0 = (gl_TextureMatrix[0] * gl_MultiTexCoord0).st;\n"
    "    vTex1 = (gl_TextureMatrix[1] * gl_MultiTexCoord1).st;\n"
    "    vColor = gl_Color;\n"
    "    vFogDepth = abs(eye.z);\n"
    "}\n";

static const char *tfFragmentShaderFallback =
    "#version 120\n"
    "uniform sampler2D uBase;\n"
    "uniform sampler2D uSecondary;\n"
    "uniform vec2 uBaseTexSize;\n"
    "uniform vec2 uSecondaryTexSize;\n"
    "uniform int uMode;\n"
    "uniform float uEnvColor;\n"
    "uniform int uFogEnabled;\n"
    "uniform float uFogStart;\n"
    "uniform float uFogEnd;\n"
    "uniform vec4 uFogColor;\n"
    "uniform int uAlphaTest;\n"
    "uniform float uAlphaRef;\n"
    "#define N64_FILTER_STRENGTH 1.20\n"
    "varying vec2 vTex0;\n"
    "varying vec2 vTex1;\n"
    "varying vec4 vColor;\n"
    "varying float vFogDepth;\n"
    "vec4 N64Blend(vec4 a, vec4 b, vec4 c, float wa, float wb, float wc) {\n"
    "    float alpha = a.a * wa + b.a * wb + c.a * wc;\n"
    "    vec3 rgb = a.rgb * a.a * wa + b.rgb * b.a * wb + c.rgb * c.a * wc;\n"
    "    if (alpha > 0.00001) rgb /= alpha;\n"
    "    else rgb = vec3(0.0);\n"
    "    return vec4(rgb, alpha);\n"
    "}\n"
    "vec4 N64Sample(sampler2D tex, vec2 size, vec2 uv) {\n"
    "    vec2 p = uv * size - 0.5;\n"
    "    vec2 ip = floor(p);\n"
    "    vec2 f = p - ip;\n"
    "    vec2 baseUV = (ip + vec2(0.5, 0.5)) / size;\n"
    "    vec2 texel = 1.0 / size;\n"
    "    vec2 texStep = texel * N64_FILTER_STRENGTH;\n"
    "    float s = f.x + f.y;\n"
    "    vec4 a;\n"
    "    vec4 b;\n"
    "    vec4 c;\n"
    "    if (s < 1.0) {\n"
    "        a = texture2D(tex, baseUV);\n"
    "        b = texture2D(tex, baseUV + vec2(texStep.x, 0.0));\n"
    "        c = texture2D(tex, baseUV + vec2(0.0, texStep.y));\n"
    "        return N64Blend(a, b, c, 1.0 - f.x - f.y, f.x, f.y);\n"
    "    }\n"
    "    a = texture2D(tex, baseUV + texStep);\n"
    "    b = texture2D(tex, baseUV + vec2(0.0, texStep.y));\n"
    "    c = texture2D(tex, baseUV + vec2(texStep.x, 0.0));\n"
    "    return N64Blend(a, b, c, f.x + f.y - 1.0, 1.0 - f.x, 1.0 - f.y);\n"
    "}\n"
    "void main() {\n"
    "    vec4 base = N64Sample(uBase, uBaseTexSize, vTex0);\n"
    "    vec4 color;\n"
    "    if (uMode == 1)\n"
    "        color = base * N64Sample(uSecondary, uSecondaryTexSize, vTex1) * vColor;\n"
    "    else if (uMode == 2)\n"
    "        color = base * (1.0 - uEnvColor) + N64Sample(uSecondary, uSecondaryTexSize, vTex1) * uEnvColor;\n"
    "    else\n"
    "        color = base * vColor;\n"
    "    if (uAlphaTest != 0 && color.a <= uAlphaRef) discard;\n"
    "    if (uFogEnabled != 0 && uFogEnd > uFogStart) {\n"
    "        float fog = clamp((uFogEnd - vFogDepth) / (uFogEnd - uFogStart), 0.0, 1.0);\n"
    "        color.rgb = mix(uFogColor.rgb, color.rgb, fog);\n"
    "    }\n"
    "    gl_FragColor = color;\n"
    "}\n";

static const char *tfFragmentShaderGrad =
    "#version 120\n"
    "#extension GL_ARB_shader_texture_lod : enable\n"
    "uniform sampler2D uBase;\n"
    "uniform sampler2D uSecondary;\n"
    "uniform vec2 uBaseTexSize;\n"
    "uniform vec2 uSecondaryTexSize;\n"
    "uniform int uMode;\n"
    "uniform float uEnvColor;\n"
    "uniform int uFogEnabled;\n"
    "uniform float uFogStart;\n"
    "uniform float uFogEnd;\n"
    "uniform vec4 uFogColor;\n"
    "uniform int uAlphaTest;\n"
    "uniform float uAlphaRef;\n"
    "#define N64_FILTER_STRENGTH 1.20\n"
    "varying vec2 vTex0;\n"
    "varying vec2 vTex1;\n"
    "varying vec4 vColor;\n"
    "varying float vFogDepth;\n"
    "vec4 N64Blend(vec4 a, vec4 b, vec4 c, float wa, float wb, float wc) {\n"
    "    float alpha = a.a * wa + b.a * wb + c.a * wc;\n"
    "    vec3 rgb = a.rgb * a.a * wa + b.rgb * b.a * wb + c.rgb * c.a * wc;\n"
    "    if (alpha > 0.00001) rgb /= alpha;\n"
    "    else rgb = vec3(0.0);\n"
    "    return vec4(rgb, alpha);\n"
    "}\n"
    "vec4 N64Sample(sampler2D tex, vec2 size, vec2 uv) {\n"
    "    vec2 p = uv * size - 0.5;\n"
    "    vec2 ip = floor(p);\n"
    "    vec2 f = p - ip;\n"
    "    vec2 baseUV = (ip + vec2(0.5, 0.5)) / size;\n"
    "    vec2 texel = 1.0 / size;\n"
    "    vec2 texStep = texel * N64_FILTER_STRENGTH;\n"
    "    vec2 dX = dFdx(uv);\n"
    "    vec2 dY = dFdy(uv);\n"
    "    float s = f.x + f.y;\n"
    "    vec4 a;\n"
    "    vec4 b;\n"
    "    vec4 c;\n"
    "    if (s < 1.0) {\n"
    "        a = texture2DGradARB(tex, baseUV, dX, dY);\n"
    "        b = texture2DGradARB(tex, baseUV + vec2(texStep.x, 0.0), dX, dY);\n"
    "        c = texture2DGradARB(tex, baseUV + vec2(0.0, texStep.y), dX, dY);\n"
    "        return N64Blend(a, b, c, 1.0 - f.x - f.y, f.x, f.y);\n"
    "    }\n"
    "    a = texture2DGradARB(tex, baseUV + texStep, dX, dY);\n"
    "    b = texture2DGradARB(tex, baseUV + vec2(0.0, texStep.y), dX, dY);\n"
    "    c = texture2DGradARB(tex, baseUV + vec2(texStep.x, 0.0), dX, dY);\n"
    "    return N64Blend(a, b, c, f.x + f.y - 1.0, 1.0 - f.x, 1.0 - f.y);\n"
    "}\n"
    "void main() {\n"
    "    vec4 base = N64Sample(uBase, uBaseTexSize, vTex0);\n"
    "    vec4 color;\n"
    "    if (uMode == 1)\n"
    "        color = base * N64Sample(uSecondary, uSecondaryTexSize, vTex1) * vColor;\n"
    "    else if (uMode == 2)\n"
    "        color = base * (1.0 - uEnvColor) + N64Sample(uSecondary, uSecondaryTexSize, vTex1) * uEnvColor;\n"
    "    else\n"
    "        color = base * vColor;\n"
    "    if (uAlphaTest != 0 && color.a <= uAlphaRef) discard;\n"
    "    if (uFogEnabled != 0 && uFogEnd > uFogStart) {\n"
    "        float fog = clamp((uFogEnd - vFogDepth) / (uFogEnd - uFogStart), 0.0, 1.0);\n"
    "        color.rgb = mix(uFogColor.rgb, color.rgb, fog);\n"
    "    }\n"
    "    gl_FragColor = color;\n"
    "}\n";



static void *TextureFilterProc(const char *name)
{
    return SDL_GL_GetProcAddress(name);
}

static qboolean TextureFilterLoadProcs(void)
{
    tfCreateShader = (PFNGLCREATESHADERPROC)TextureFilterProc("glCreateShader");
    tfShaderSource = (PFNGLSHADERSOURCEPROC)TextureFilterProc("glShaderSource");
    tfCompileShader = (PFNGLCOMPILESHADERPROC)TextureFilterProc("glCompileShader");
    tfGetShaderiv = (PFNGLGETSHADERIVPROC)TextureFilterProc("glGetShaderiv");
    tfGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)TextureFilterProc("glGetShaderInfoLog");
    tfDeleteShader = (PFNGLDELETESHADERPROC)TextureFilterProc("glDeleteShader");
    tfCreateProgram = (PFNGLCREATEPROGRAMPROC)TextureFilterProc("glCreateProgram");
    tfAttachShader = (PFNGLATTACHSHADERPROC)TextureFilterProc("glAttachShader");
    tfLinkProgram = (PFNGLLINKPROGRAMPROC)TextureFilterProc("glLinkProgram");
    tfGetProgramiv = (PFNGLGETPROGRAMIVPROC)TextureFilterProc("glGetProgramiv");
    tfGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)TextureFilterProc("glGetProgramInfoLog");
    tfDeleteProgram = (PFNGLDELETEPROGRAMPROC)TextureFilterProc("glDeleteProgram");
    tfUseProgram = (PFNGLUSEPROGRAMPROC)TextureFilterProc("glUseProgram");
    tfGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)TextureFilterProc("glGetUniformLocation");
    tfUniform1i = (PFNGLUNIFORM1IPROC)TextureFilterProc("glUniform1i");
    tfUniform1f = (PFNGLUNIFORM1FPROC)TextureFilterProc("glUniform1f");
    tfUniform2f = (PFNGLUNIFORM2FPROC)TextureFilterProc("glUniform2f");
    tfUniform4f = (PFNGLUNIFORM4FPROC)TextureFilterProc("glUniform4f");

    return tfCreateShader && tfShaderSource && tfCompileShader && tfGetShaderiv &&
           tfGetShaderInfoLog && tfDeleteShader && tfCreateProgram && tfAttachShader &&
           tfLinkProgram && tfGetProgramiv && tfGetProgramInfoLog && tfDeleteProgram &&
           tfUseProgram && tfGetUniformLocation && tfUniform1i && tfUniform1f &&
           tfUniform2f && tfUniform4f;
}

static TextureFilterGLuint TextureFilterCompileShader(TextureFilterGLenum type, const char *source)
{
    TextureFilterGLuint shader;
    TextureFilterGLint ok;
    TextureFilterGLint logLength;
    TextureFilterGLchar log[2048];
    const TextureFilterGLchar *sources[1];

    shader = tfCreateShader(type);
    if (!shader)
        return 0;

    sources[0] = source;
    tfShaderSource(shader, 1, sources, NULL);
    tfCompileShader(shader);
    tfGetShaderiv(shader, GL_COMPILE_STATUS_TEXTURE_FILTER, &ok);
    if (ok)
        return shader;

    tfGetShaderiv(shader, GL_INFO_LOG_LENGTH_TEXTURE_FILTER, &logLength);
    if (logLength <= 0)
        logLength = 1;
    if (logLength > (TextureFilterGLint)sizeof(log))
        logLength = (TextureFilterGLint)sizeof(log);
    memset(log, 0, sizeof(log));
    tfGetShaderInfoLog(shader, logLength, NULL, log);
    ri.Con_Printf(PRINT_ALL, "N64 texture filter shader compile failed: %s\n", log);
    tfDeleteShader(shader);
    return 0;
}

static TextureFilterGLuint TextureFilterCreateProgram(const char *fragmentSource)
{
    TextureFilterGLuint vertex;
    TextureFilterGLuint fragment;
    TextureFilterGLuint program;
    TextureFilterGLint ok;
    TextureFilterGLint logLength;
    TextureFilterGLchar log[2048];

    vertex = TextureFilterCompileShader(GL_VERTEX_SHADER_TEXTURE_FILTER, tfVertexShader);
    if (!vertex)
        return 0;

    fragment = TextureFilterCompileShader(GL_FRAGMENT_SHADER_TEXTURE_FILTER, fragmentSource);
    if (!fragment)
    {
        tfDeleteShader(vertex);
        return 0;
    }

    program = tfCreateProgram();
    if (!program)
    {
        tfDeleteShader(vertex);
        tfDeleteShader(fragment);
        return 0;
    }

    tfAttachShader(program, vertex);
    tfAttachShader(program, fragment);
    tfLinkProgram(program);
    tfGetProgramiv(program, GL_LINK_STATUS_TEXTURE_FILTER, &ok);

    tfDeleteShader(vertex);
    tfDeleteShader(fragment);

    if (!ok)
    {
        tfGetProgramiv(program, GL_INFO_LOG_LENGTH_TEXTURE_FILTER, &logLength);
        if (logLength <= 0)
            logLength = 1;
        if (logLength > (TextureFilterGLint)sizeof(log))
            logLength = (TextureFilterGLint)sizeof(log);
        memset(log, 0, sizeof(log));
        tfGetProgramInfoLog(program, logLength, NULL, log);
        ri.Con_Printf(PRINT_ALL, "N64 texture filter program link failed: %s\n", log);
        tfDeleteProgram(program);
        return 0;
    }

    return program;
}

static void TextureFilterInvalidateCache(void)
{
    tfCacheValid = false;
    tfLastBaseTexnum = -1;
    tfLastBaseWidth = 0.0f;
    tfLastBaseHeight = 0.0f;
    tfLastSecondaryTexnum = -1;
    tfLastSecondaryWidth = 0.0f;
    tfLastSecondaryHeight = 0.0f;
    tfLastMode = -1;
    tfLastEnvColor = 0.0f;
    tfLastFogEnabled = -1;
    tfLastFogStart = 0.0f;
    tfLastFogEnd = 0.0f;
    tfLastFogR = 0.0f;
    tfLastFogG = 0.0f;
    tfLastFogB = 0.0f;
    tfLastAlphaTest = -1;
    tfLastAlphaRef = -1.0f;
}

static void TextureFilterUseProgram(TextureFilterGLuint program)
{
    if (program != 0)
    {
        if (tfProgramBound)
            return;
        tfUseProgram(program);
        tfProgramBound = true;
    }
    else
    {
        if (!tfProgramBound)
            return;
        tfUseProgram(0);
        tfProgramBound = false;
    }
}

static void TextureFilterSetCommonUniforms(void)
{
    int fogEnabled;
    int alphaTest;
    float alphaRef;
    float fogR;
    float fogG;
    float fogB;

    fogEnabled = (gl_state.nStateFlags & GLSTATE_FOG) ? 1 : 0;
    if (currententity && (currententity->flags & RF_WEAPONMODEL))
        fogEnabled = 0;

    alphaTest = (gl_state.nStateFlags & GLSTATE_ALPHA_TEST) ? 1 : 0;
    alphaRef = gl_state.fAlphaFunc;
    if (alphaRef < 0.0f)
        alphaRef = 0.0f;

    fogR = r_newrefdef.foginfo.r;
    fogG = r_newrefdef.foginfo.g;
    fogB = r_newrefdef.foginfo.b;

    if (!tfCacheValid || tfLastFogEnabled != fogEnabled)
    {
        tfUniform1i(tfFogEnabled, fogEnabled);
        tfLastFogEnabled = fogEnabled;
    }
    if (!tfCacheValid || tfLastFogStart != r_newrefdef.foginfo.start)
    {
        tfUniform1f(tfFogStart, r_newrefdef.foginfo.start);
        tfLastFogStart = r_newrefdef.foginfo.start;
    }
    if (!tfCacheValid || tfLastFogEnd != r_newrefdef.foginfo.end)
    {
        tfUniform1f(tfFogEnd, r_newrefdef.foginfo.end);
        tfLastFogEnd = r_newrefdef.foginfo.end;
    }
    if (!tfCacheValid || tfLastFogR != fogR || tfLastFogG != fogG || tfLastFogB != fogB)
    {
        tfUniform4f(tfFogColor, fogR, fogG, fogB, 1.0f);
        tfLastFogR = fogR;
        tfLastFogG = fogG;
        tfLastFogB = fogB;
    }
    if (!tfCacheValid || tfLastAlphaTest != alphaTest)
    {
        tfUniform1i(tfAlphaTest, alphaTest);
        tfLastAlphaTest = alphaTest;
    }
    if (!tfCacheValid || tfLastAlphaRef != alphaRef)
    {
        tfUniform1f(tfAlphaRef, alphaRef);
        tfLastAlphaRef = alphaRef;
    }

    tfCacheValid = true;
}

static void TextureFilterSetTextureUniform(TextureFilterGLint sizeUniform,
                                           const image_t *image)
{
    float width = 1.0f;
    float height = 1.0f;
    int texnum = 0;
    qboolean base = (sizeUniform == tfBaseTexSize);

    if (image)
    {
        width = (float)(image->upload_width > 0 ? image->upload_width : image->width);
        height = (float)(image->upload_height > 0 ? image->upload_height : image->height);
        texnum = image->texnum;
    }

    if (base)
    {
        if (tfCacheValid && tfLastBaseTexnum == texnum &&
            tfLastBaseWidth == width && tfLastBaseHeight == height)
            return;

        tfUniform2f(sizeUniform, width, height);
        tfLastBaseTexnum = texnum;
        tfLastBaseWidth = width;
        tfLastBaseHeight = height;
    }
    else
    {
        if (tfCacheValid && tfLastSecondaryTexnum == texnum &&
            tfLastSecondaryWidth == width && tfLastSecondaryHeight == height)
            return;

        tfUniform2f(sizeUniform, width, height);
        tfLastSecondaryTexnum = texnum;
        tfLastSecondaryWidth = width;
        tfLastSecondaryHeight = height;
    }
}

static void TextureFilterSetTextureUniformValues(qboolean base, int texnum, float width, float height)
{
    if (base)
    {
        if (tfCacheValid && tfLastBaseTexnum == texnum &&
            tfLastBaseWidth == width && tfLastBaseHeight == height)
            return;

        tfUniform2f(tfBaseTexSize, width, height);
        tfLastBaseTexnum = texnum;
        tfLastBaseWidth = width;
        tfLastBaseHeight = height;
    }
    else
    {
        if (tfCacheValid && tfLastSecondaryTexnum == texnum &&
            tfLastSecondaryWidth == width && tfLastSecondaryHeight == height)
            return;

        tfUniform2f(tfSecondaryTexSize, width, height);
        tfLastSecondaryTexnum = texnum;
        tfLastSecondaryWidth = width;
        tfLastSecondaryHeight = height;
    }
}

static void TextureFilterSetModeUniform(int mode)
{
    if (!tfCacheValid || tfLastMode != mode)
    {
        tfUniform1i(tfMode, mode);
        tfLastMode = mode;
    }
}

static void TextureFilterSetEnvColorUniform(float envColor)
{
    if (!tfCacheValid || tfLastEnvColor != envColor)
    {
        tfUniform1f(tfEnvColor, envColor);
        tfLastEnvColor = envColor;
    }
}

void R_TextureFilterInit(void)
{
    tfProgram = 0;
    tfAvailable = false;
    tfGradient = false;
    tfProgramBound = false;
    TextureFilterInvalidateCache();
    if (!TextureFilterLoadProcs())
        return;

    {
        const char *extensions = gl_config.extensions_string;
        if (extensions && strstr(extensions, "GL_ARB_shader_texture_lod"))
        {
            tfProgram = TextureFilterCreateProgram(tfFragmentShaderGrad);
            if (tfProgram)
                tfGradient = true;
        }

        if (!tfProgram)
            tfProgram = TextureFilterCreateProgram(tfFragmentShaderFallback);
    }

    if (!tfProgram)
        return;

    tfBaseSampler = tfGetUniformLocation(tfProgram, "uBase");
    tfSecondarySampler = tfGetUniformLocation(tfProgram, "uSecondary");
    tfBaseTexSize = tfGetUniformLocation(tfProgram, "uBaseTexSize");
    tfSecondaryTexSize = tfGetUniformLocation(tfProgram, "uSecondaryTexSize");
    tfMode = tfGetUniformLocation(tfProgram, "uMode");
    tfEnvColor = tfGetUniformLocation(tfProgram, "uEnvColor");
    tfFogEnabled = tfGetUniformLocation(tfProgram, "uFogEnabled");
    tfFogStart = tfGetUniformLocation(tfProgram, "uFogStart");
    tfFogEnd = tfGetUniformLocation(tfProgram, "uFogEnd");
    tfFogColor = tfGetUniformLocation(tfProgram, "uFogColor");
    tfAlphaTest = tfGetUniformLocation(tfProgram, "uAlphaTest");
    tfAlphaRef = tfGetUniformLocation(tfProgram, "uAlphaRef");

    if (tfBaseSampler < 0 || tfSecondarySampler < 0 || tfBaseTexSize < 0 ||
        tfSecondaryTexSize < 0 || tfMode < 0 || tfEnvColor < 0 || tfFogEnabled < 0 || tfFogStart < 0 ||
        tfFogEnd < 0 || tfFogColor < 0 || tfAlphaTest < 0 || tfAlphaRef < 0)
    {
        tfDeleteProgram(tfProgram);
        tfProgram = 0;
        return;
    }

    TextureFilterUseProgram(tfProgram);
    tfUniform1i(tfBaseSampler, 0);
    tfUniform1i(tfSecondarySampler, 1);
    TextureFilterUseProgram(0);
    TextureFilterInvalidateCache();
    tfAvailable = true;
    ri.Con_Printf(PRINT_ALL, "...N64 3-point texture filtering shader available%s\n",
                   tfGradient ? " (Doom64EX-Plus sampling model + explicit gradients)" :
                                 " (Doom64EX-Plus sampling model, no gradient extension)");
}

void R_TextureFilterShutdown(void)
{
    if (tfProgram)
    {
        TextureFilterUseProgram(0);
        tfDeleteProgram(tfProgram);
        tfProgram = 0;
    }
    tfProgramBound = false;
    tfAvailable = false;
    tfGradient = false;
    TextureFilterInvalidateCache();
}

qboolean R_TextureFilterUsesGradient(void)
{
    return tfGradient;
}

void R_TextureFilterSetMode(void)
{
    TextureFilterUseProgram(0);
    TextureFilterInvalidateCache();
    if (gl_texturemode && gl_texturemode->string)
        GL_TextureMode(gl_texturemode->string);
}

void R_TextureFilterBeginBase(const image_t *image)
{
    if (!tfAvailable || !gl_texture_filter)
        return;

    /* The main menu temporarily selects Bilinear (filter 0) while it draws.
       The previous N64 mode left texture objects at nearest filtering, so
       restore ordinary bilinear filtering on the currently bound base texture
       instead of falling through with stale GL state. */
    if (gl_texture_filter->value == 0.0f)
    {
        GL_SelectTexture(GL_TEXTURE0_SGIS_TEXTURE_FILTER);
        if (image)
            GL_Bind(image->texnum);
        qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        TextureFilterUseProgram(0);
        return;
    }

    if (gl_texture_filter->value != 2.0f)
        return;

    GL_SelectTexture(GL_TEXTURE0_SGIS_TEXTURE_FILTER);
    if (image)
        GL_Bind(image->texnum);

    if (image && image->type == it_pic)
    {
        qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    TextureFilterUseProgram(tfProgram);
    TextureFilterSetTextureUniform(tfBaseTexSize, image);
    TextureFilterSetTextureUniformValues(false, 0, 1.0f, 1.0f);
    TextureFilterSetModeUniform(0);
    TextureFilterSetEnvColorUniform(0.0f);
    TextureFilterSetCommonUniforms();
}

void R_TextureFilterBeginLightmapped(const image_t *image, float lightmap_width, float lightmap_height)
{
    if (!tfAvailable || !gl_texture_filter || gl_texture_filter->value != 2.0f)
        return;

    TextureFilterUseProgram(tfProgram);
    TextureFilterSetTextureUniform(tfBaseTexSize, image);
    TextureFilterSetTextureUniformValues(false, 0, lightmap_width, lightmap_height);
    TextureFilterSetModeUniform(1);
    TextureFilterSetEnvColorUniform(0.0f);
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL_SelectTexture(GL_TEXTURE0_SGIS_TEXTURE_FILTER);
    TextureFilterSetCommonUniforms();
}

void R_TextureFilterBeginEnvmap(const image_t *image, const image_t *envmap, float env_color)
{
    if (!tfAvailable || !gl_texture_filter || gl_texture_filter->value != 2.0f)
        return;

    TextureFilterUseProgram(tfProgram);
    TextureFilterSetTextureUniform(tfBaseTexSize, image);
    TextureFilterSetTextureUniform(tfSecondaryTexSize, envmap);
    TextureFilterSetModeUniform(2);
    TextureFilterSetEnvColorUniform(env_color);
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL_SelectTexture(GL_TEXTURE0_SGIS_TEXTURE_FILTER);
    TextureFilterSetCommonUniforms();
}

void R_TextureFilterEnd(void)
{
    if (!tfAvailable || !gl_texture_filter || gl_texture_filter->value != 2.0f)
        return;

    TextureFilterUseProgram(0);
}
void R_TextureFilterBeginSingle(int texnum, float width, float height)
{
    if (!tfAvailable || !gl_texture_filter || gl_texture_filter->value != 2.0f)
        return;

    GL_SelectTexture(GL_TEXTURE0_SGIS_TEXTURE_FILTER);
    GL_Bind(texnum);
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    TextureFilterUseProgram(tfProgram);
    TextureFilterSetTextureUniformValues(true, texnum, width, height);
    TextureFilterSetTextureUniformValues(false, 0, 1.0f, 1.0f);
    TextureFilterSetModeUniform(0);
    TextureFilterSetEnvColorUniform(0.0f);
    TextureFilterSetCommonUniforms();
}

