# Microsoft Developer Studio Project File - Name="DkAudioEngine_Miles" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DkAudioEngine_Miles - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DkAudioEngine_Miles.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DkAudioEngine_Miles.mak" CFG="DkAudioEngine_Miles - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DkAudioEngine_Miles - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DkAudioEngine_Miles - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DkAudioEngine_Miles - Win32 German" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DkAudioEngine_Miles - Win32 French" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DkAudioEngine_Miles - Win32 Italian" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DkAudioEngine_Miles - Win32 Spanish" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DkAudioEngine_Miles - Win32 Japanese" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DkAudioEngine_Miles - Win32 German Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Daikatana/base/Audio/DkAudioEngine_Miles", PGGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DkAudioEngine_Miles - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "TONGUE_ENGLISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 user32.lib mss32.lib IonCommonRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\Release/audio.dll" /libpath:"..\..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 Debug"

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
# ADD CPP /nologo /G6 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX"MilesSound.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mss32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\Debug/audio.dll" /libpath:"..\..\..\libs"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 German"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DkAudioEngine_Miles___Win32_German"
# PROP BASE Intermediate_Dir "DkAudioEngine_Miles___Win32_German"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\German"
# PROP Intermediate_Dir "German"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "TONGUE_GERMAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib mss32.lib IonCommonRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\Release/audio.dll" /libpath:"..\..\..\libs"
# ADD LINK32 user32.lib mss32.lib IonCommonGerman.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\German/audio.dll" /libpath:"..\..\..\libs"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 French"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DkAudioEngine_Miles___Win32_French"
# PROP BASE Intermediate_Dir "DkAudioEngine_Miles___Win32_French"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\French"
# PROP Intermediate_Dir "French"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "TONGUE_FRENCH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib mss32.lib IonCommonRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\Release/audio.dll" /libpath:"..\..\..\libs"
# ADD LINK32 user32.lib mss32.lib IonCommonFrench.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\French/audio.dll" /libpath:"..\..\..\libs"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 Italian"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DkAudioEngine_Miles___Win32_Italian"
# PROP BASE Intermediate_Dir "DkAudioEngine_Miles___Win32_Italian"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Italian"
# PROP Intermediate_Dir "Italian"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "TONGUE_ITALIAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib mss32.lib IonCommonRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\Release/audio.dll" /libpath:"..\..\..\libs"
# ADD LINK32 user32.lib mss32.lib IonCommonItalian.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\Italian/audio.dll" /libpath:"..\..\..\libs"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 Spanish"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DkAudioEngine_Miles___Win32_Spanish"
# PROP BASE Intermediate_Dir "DkAudioEngine_Miles___Win32_Spanish"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Spanish"
# PROP Intermediate_Dir "Spanish"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "TONGUE_SPANISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib mss32.lib IonCommonRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\Release/audio.dll" /libpath:"..\..\..\libs"
# ADD LINK32 user32.lib mss32.lib IonCommonSpanish.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\Spanish/audio.dll" /libpath:"..\..\..\libs"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 Japanese"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DkAudioEngine_Miles___Win32_Japanese"
# PROP BASE Intermediate_Dir "DkAudioEngine_Miles___Win32_Japanese"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Japanese"
# PROP Intermediate_Dir "Japanese"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "TONGUE_JAPANESE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib mss32.lib IonCommonRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\Release/audio.dll" /libpath:"..\..\..\libs"
# ADD LINK32 user32.lib mss32.lib IonCommonJapanese.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\Japanese/audio.dll" /libpath:"..\..\..\libs"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 German Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DkAudioEngine_Miles___Win32_German_Debug"
# PROP BASE Intermediate_Dir "DkAudioEngine_Miles___Win32_German_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\German_Debug"
# PROP Intermediate_Dir "German_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "TONGUE_GERMAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /I "..\..\dk_" /I "..\..\client" /I "..\..\win32" /I "..\..\qcommon" /I "..\..\..\user" /I "." /I "..\..\..\libclient" /D "TONGUE_GERMAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAs /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib mss32.lib IonCommonGerman.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\German/audio.dll" /libpath:"..\..\..\libs"
# ADD LINK32 user32.lib mss32.lib IonCommonGermanDebug.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\German/audio.dll" /libpath:"..\..\..\libs"

!ENDIF 

# Begin Target

# Name "DkAudioEngine_Miles - Win32 Release"
# Name "DkAudioEngine_Miles - Win32 Debug"
# Name "DkAudioEngine_Miles - Win32 German"
# Name "DkAudioEngine_Miles - Win32 French"
# Name "DkAudioEngine_Miles - Win32 Italian"
# Name "DkAudioEngine_Miles - Win32 Spanish"
# Name "DkAudioEngine_Miles - Win32 Japanese"
# Name "DkAudioEngine_Miles - Win32 German Debug"
# Begin Group "Main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DllMain.cpp
# End Source File
# Begin Source File

SOURCE=.\MilesSound.cpp

!IF  "$(CFG)" == "DkAudioEngine_Miles - Win32 Release"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 Debug"

# ADD CPP /Yc"MilesSound.h"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 German"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 French"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 Italian"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 Spanish"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 Japanese"

!ELSEIF  "$(CFG)" == "DkAudioEngine_Miles - Win32 German Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\S_Calls.cpp
# End Source File
# Begin Source File

SOURCE=.\Support.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=.\MilesSound.h
# End Source File
# Begin Source File

SOURCE=..\..\client\sound.h
# End Source File
# End Group
# Begin Group "Classes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sample.cpp
# End Source File
# Begin Source File

SOURCE=.\Sample.h
# End Source File
# End Group
# End Target
# End Project
