//misc macros and junk.
//#pragma once

#ifndef _DK_MISC_H
#define _DK_MISC_H

#define bound_max(var, val) if (var > (val)) var = (val);
#define bound_min(var, val) if (var < (val)) var = (val);

#define bound_min_inline(var, val) (((var) < (val)) ? (val) : (var))
#define bound_max_inline(var, val) (((var) > (val)) ? (val) : (var))

#define delca(arr) if (arr != NULL) {delete [] arr; arr = NULL;}
#define delc(var) if (var != NULL) {delete var; var = NULL;}
#define delcdec(var) if (var != NULL) {var->Disconnect(); var = NULL;}
#define setinc(var, val) if (val != NULL) {var = val; var->Connect();}
#define inc(obj) if (obj != NULL) {obj->Connect();}

#define PI 3.14159265358979323846f
#define RAD2DEG 57.29577951308f
#define TO_RAD 0.01745329251994f

__inline float to_radians(float degrees) {
    return degrees * TO_RAD;
}

//
//Nice litte macro that enables us to define a function-looking block that 
//gets executed when the program loads, before main/WinMain is called.
//The order that these are called in from multiple files is unknown.
//A typical usage looks like:
//FILE_INIT(our_cpp) {
//    global_var = new our_class();
//}
//
#define FILE_INIT(file) class file_init_class##file { public: file_init_class##file(); }; static file_init_class##file our_file_init_obj; file_init_class##file::file_init_class##file()

#if _MSC_VER
//misc windows functions.
extern "C" __declspec(dllimport) unsigned long __stdcall GetTickCount(void);
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char *lpOutputString);
#endif

#endif _DK_MISC_H
