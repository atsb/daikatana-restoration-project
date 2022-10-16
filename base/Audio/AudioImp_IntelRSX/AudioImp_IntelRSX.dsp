# Microsoft Developer Studio Project File - Name="AudioImp_IntelRSX" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=AudioImp_IntelRSX - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AudioImp_IntelRSX.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AudioImp_IntelRSX.mak" CFG="AudioImp_IntelRSX - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AudioImp_IntelRSX - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "AudioImp_IntelRSX - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/Daikatana/base/Audio/AudioImp_IntelRSX", ZSFAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AudioImp_IntelRSX - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Release\"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\user\collection" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x06000000" /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "AudioImp_IntelRSX - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\user\collection" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "AUDIO_DEBUG" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x06000000" /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "AudioImp_IntelRSX - Win32 Release"
# Name "AudioImp_IntelRSX - Win32 Debug"
# Begin Source File

SOURCE=..\AudioDebug.cpp
# End Source File
# Begin Source File

SOURCE=..\AudioDeviceEnum.cpp
# End Source File
# Begin Source File

SOURCE=..\AudioDSDebug.cpp
# End Source File
# Begin Source File

SOURCE=..\AudioEmitter.cpp
# End Source File
# Begin Source File

SOURCE=..\AudioGlobals.cpp
# End Source File
# Begin Source File

SOURCE=..\AudioImp_Base.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioImp_IntelRSX.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioImp_IntelRSX.def
# End Source File
# Begin Source File

SOURCE=.\AudioImp_IntelRSX.h
# End Source File
# Begin Source File

SOURCE=.\AudioImp_IntelRSX_GUIDs.c
# End Source File
# Begin Source File

SOURCE=.\AudioImp_IntelRSX_Main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\User\Collection\Col_Array_p.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\user\Collection\Col_Elements.h
# End Source File
# Begin Source File

SOURCE=..\..\..\User\Collection\Col_List_p.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\User\Collection\Col_Map_sp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\User\Collection\Col_Plex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\User\Collection\IStringCore.cpp
# End Source File
# End Target
# End Project
