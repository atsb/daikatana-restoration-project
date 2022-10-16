# Microsoft Developer Studio Project File - Name="ref_d3d" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ref_d3d - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ref_d3d.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ref_d3d.mak" CFG="ref_d3d - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ref_d3d - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ref_d3d - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/Daikatana/base/ref_d3d", TAFAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ref_d3d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\client" /I "..\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib d3dim.lib dxguid.lib /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "ref_d3d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /Zi /Od /I "..\client" /I "..\qcommon" /I "..\ref_common" /I "..\dk_" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib advapi32.lib kernel32.lib user32.lib gdi32.lib ddraw.lib d3dim.lib dxguid.lib IonCommon.lib /nologo /subsystem:windows /dll /profile /debug /machine:I386

!ENDIF 

# Begin Target

# Name "ref_d3d - Win32 Release"
# Name "ref_d3d - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\d3d_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_enum.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_error.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_image.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_init.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_main.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_math.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_model.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_ref_main.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_texture.cpp

!IF  "$(CFG)" == "ref_d3d - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ref_d3d - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\d3denum.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dutil.cpp
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

SOURCE=..\ref_common\dk_ref_wal.cpp
# End Source File
# Begin Source File

SOURCE=..\win32\q_shwin.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\d3d_draw.h
# End Source File
# Begin Source File

SOURCE=.\d3d_enum.h
# End Source File
# Begin Source File

SOURCE=.\d3d_error.h
# End Source File
# Begin Source File

SOURCE=.\d3d_local.h
# End Source File
# Begin Source File

SOURCE=.\d3d_model.h
# End Source File
# Begin Source File

SOURCE=.\d3d_texture.h
# End Source File
# Begin Source File

SOURCE=.\d3denum.h
# End Source File
# Begin Source File

SOURCE=.\d3dutil.h
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_model.h
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_model.h
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_wal.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# End Group
# Begin Group "Library Files"

# PROP Default_Filter "*.lib"
# Begin Source File

SOURCE=..\..\Libs\IonCommon.lib
# End Source File
# End Group
# End Target
# End Project
