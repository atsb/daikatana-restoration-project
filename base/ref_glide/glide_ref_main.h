#pragma once

extern "C" refimport_t ri;

extern int32 current_video_width;
extern int32 current_video_height;


bool InitRefresh();


void SetResourceDir(char *name);
void DrawChar(int x, int y, int c);
int Draw_AlphaChar(int x, int y, int num, int startMsec, int endMsec, int curMsec, 
    int timeOffset, vec3_t rgbBlend, int flags, void *font);
void DrawStretchPic(int x, int y, int w, int h, const char *name, resource_t resource);
void *RegisterPic(const char *name, resource_t resource);
void *RegisterModel(const char *name, resource_t resource);
void SetInterfacePalette(const unsigned char *palette);

