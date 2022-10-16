# Microsoft Developer Studio Project File - Name="IonCommon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=IonCommon - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IonCommon.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IonCommon.mak" CFG="IonCommon - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IonCommon - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "IonCommon - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Daikatana/IonCommon", ECGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IonCommon - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /D "DAIKATANA_OEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\libs\IonCommonRelease.lib"

!ELSEIF  "$(CFG)" == "IonCommon - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "..\user" /D "DAIKATANA_OEM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\libs\IonCommonDebug.lib"

!ENDIF 

# Begin Target

# Name "IonCommon - Win32 Release"
# Name "IonCommon - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.c**"
# Begin Source File

SOURCE=..\user\Array_p.cpp
# End Source File
# Begin Source File

SOURCE=..\user\collect.cpp
# End Source File
# Begin Source File

SOURCE=..\user\csv.cpp
# End Source File
# Begin Source File

SOURCE=..\user\dk_io.cpp
# End Source File
# Begin Source File

SOURCE=..\user\dk_matrix.cpp
# End Source File
# Begin Source File

SOURCE=..\user\dk_point.cpp
# End Source File
# Begin Source File

SOURCE=..\user\dk_shared.cpp
# End Source File
# Begin Source File

SOURCE=..\user\dk_thread.cpp
# End Source File
# Begin Source File

SOURCE=..\user\filebuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\user\List_p.cpp
# End Source File
# Begin Source File

SOURCE=..\user\logfile.cpp
# End Source File
# Begin Source File

SOURCE=..\user\Lzari.cpp
# End Source File
# Begin Source File

SOURCE=..\user\map_ip.cpp
# End Source File
# Begin Source File

SOURCE=..\user\map_sp.cpp
# End Source File
# Begin Source File

SOURCE=..\user\md5c.cpp
# End Source File
# Begin Source File

SOURCE=..\user\Swmrg.cpp
# End Source File
# Begin Source File

SOURCE=..\user\udpclient.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h**"
# Begin Source File

SOURCE=..\user\CmnHdr.h
# End Source File
# Begin Source File

SOURCE=..\user\collect.h
# End Source File
# Begin Source File

SOURCE=..\user\csv.h
# End Source File
# Begin Source File

SOURCE=..\user\decode.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_defines.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_io.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_matrix.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_point.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_thread.h
# End Source File
# Begin Source File

SOURCE=..\user\filebuffer.h
# End Source File
# Begin Source File

SOURCE=..\user\logfile.h
# End Source File
# Begin Source File

SOURCE=..\user\md5.h
# End Source File
# Begin Source File

SOURCE=..\user\Swmrg.h
# End Source File
# Begin Source File

SOURCE=..\user\udpclient.h
# End Source File
# End Group
# End Target
# End Project
