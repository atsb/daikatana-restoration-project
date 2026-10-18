#include <string.h>
#include <SDL3/SDL.h>
#include "ref.h"
#include "dk_array.h"
#include "dk_buffer.h"
#include "dk_menup.h"

array_obj<gl_driver_info_t> gl_driver_info;
int gl_driver_info_count = 0;

#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

typedef const unsigned char *(*dk_glGetString_t)(unsigned int name);

static qboolean AddCurrentSDLGLDriver(void)
{
    gl_driver_info_t driver;
    dk_glGetString_t get_string;
    const unsigned char *value;

    if (gl_driver_info_count != 0)
        return true;

    get_string = (dk_glGetString_t)SDL_GL_GetProcAddress("glGetString");
    if (get_string == NULL)
        return false;

    memset(&driver, 0, sizeof(driver));
    driver.szFilename.Set("opengl32.dll");

    value = get_string(GL_VENDOR);
    if (value != NULL)
        driver.szVendor.Set((const char *)value);

    value = get_string(GL_RENDERER);
    if (value != NULL)
        driver.szRenderer.Set((const char *)value);

    value = get_string(GL_VERSION);
    if (value != NULL)
        driver.szVersion.Set((const char *)value);

    value = get_string(GL_EXTENSIONS);
    if (value != NULL)
        driver.szExtensions.Set((const char *)value);

    gl_driver_info.Add(driver);
    gl_driver_info_count = 1;
    return true;
}

void DK_EnumerateGLDrivers()
{
    gl_driver_info.Length(0);
    gl_driver_info_count = 0;
    AddCurrentSDLGLDriver();
}
