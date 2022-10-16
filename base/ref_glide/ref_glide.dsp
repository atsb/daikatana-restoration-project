# Microsoft Developer Studio Project File - Name="ref_glide" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ref_glide - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ref_glide.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ref_glide.mak" CFG="ref_glide - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ref_glide - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ref_glide - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/Daikatana/base/ref_glide", CQEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ref_glide - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "y:\daikatana"
# PROP Intermediate_Dir "z:\daikatana\ref_glide\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /Ob2 /I "..\client" /I "..\qcommon" /I "..\..\user" /I "..\dk_" /I "..\ref_common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib glide2x.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\Release/ref_glide.dll"

!ELSEIF  "$(CFG)" == "ref_glide - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "y:\daikatana"
# PROP Intermediate_Dir "z:\daikatana\ref_glide\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "..\client" /I "..\qcommon" /I "..\..\user" /I "..\dk_" /I "..\ref_common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /WX /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"y:\daikatana/ref_glideD.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib glide2x.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\Debug/ref_glide.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ref_glide - Win32 Release"
# Name "ref_glide - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=..\..\user\dk_matrix.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_model_frame_cache.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_model_new.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_point.cpp
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_common.cpp
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_font.cpp
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_pic.cpp
# End Source File
# Begin Source File

SOURCE=.\glide_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\glide_frame_buffer.cpp

!IF  "$(CFG)" == "ref_glide - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_glide - Win32 Debug"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\glide_frame_rate.cpp
# End Source File
# Begin Source File

SOURCE=.\glide_frustrum.cpp
# End Source File
# Begin Source File

SOURCE=.\glide_main.cpp
# End Source File
# Begin Source File

SOURCE=.\glide_model.cpp
# End Source File
# Begin Source File

SOURCE=.\glide_palettes.cpp
# End Source File
# Begin Source File

SOURCE=.\glide_ref_main.cpp
# End Source File
# Begin Source File

SOURCE=.\glide_render.cpp
# End Source File
# Begin Source File

SOURCE=.\glide_texture_memory.cpp
# End Source File
# Begin Source File

SOURCE=.\glide_viewport.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\..\user\anorms.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\CMODEL.H
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_array.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_list.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_matrix.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_model_new.h
# End Source File
# Begin Source File

SOURCE=..\dk_\dk_object_reference.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_point.h
# End Source File
# Begin Source File

SOURCE=..\dk_\dk_pointer.h
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_common.h
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_pic.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=.\glide_draw.h
# End Source File
# Begin Source File

SOURCE=.\glide_draw_point.h
# End Source File
# Begin Source File

SOURCE=.\glide_frame_buffer.h
# End Source File
# Begin Source File

SOURCE=.\glide_frame_rate.h
# End Source File
# Begin Source File

SOURCE=.\glide_frustrum.h
# End Source File
# Begin Source File

SOURCE=.\glide_main.h
# End Source File
# Begin Source File

SOURCE=.\glide_model.h
# End Source File
# Begin Source File

SOURCE=.\glide_palettes.h
# End Source File
# Begin Source File

SOURCE=.\glide_plane.h
# End Source File
# Begin Source File

SOURCE=.\glide_ref_main.h
# End Source File
# Begin Source File

SOURCE=.\glide_texture_memory.h
# End Source File
# Begin Source File

SOURCE=.\glide_viewport.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qcommon.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qfiles.h
# End Source File
# Begin Source File

SOURCE=..\client\ref.h
# End Source File
# End Group
# End Target
# End Project
