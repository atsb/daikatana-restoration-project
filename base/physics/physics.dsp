# Microsoft Developer Studio Project File - Name="physics" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=physics - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "physics.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "physics.mak" CFG="physics - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "physics - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "physics - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "physics - Win32 French" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "physics - Win32 German" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "physics - Win32 Italian" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "physics - Win32 Spanish" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "physics - Win32 Japanese" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "physics - Win32 German Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Daikatana/base/physics", KNEAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "physics - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "TONGUE_ENGLISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
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
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib  /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "physics - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MTd /W3 /Gi /GX /ZI /Od /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "TONGUE_ENGLISH" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX"p_user.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libc.lib" /libpath:"..\..\libs"

!ELSEIF  "$(CFG)" == "physics - Win32 French"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "physics___Win32_French"
# PROP BASE Intermediate_Dir "physics___Win32_French"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\French"
# PROP Intermediate_Dir "French"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "TONGUE_FRENCH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib  libentRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib  IonCommonFrench.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "physics - Win32 German"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "physics___Win32_German"
# PROP BASE Intermediate_Dir "physics___Win32_German"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\German"
# PROP Intermediate_Dir "German"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "TONGUE_GERMAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib  libentRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib  IonCommonGerman.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "physics - Win32 Italian"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "physics___Win32_Italian"
# PROP BASE Intermediate_Dir "physics___Win32_Italian"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Italian"
# PROP Intermediate_Dir "Italian"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "TONGUE_ITALIAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib  libentRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib  IonCommonItalian.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "physics - Win32 Spanish"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "physics___Win32_Spanish"
# PROP BASE Intermediate_Dir "physics___Win32_Spanish"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Spanish"
# PROP Intermediate_Dir "Spanish"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "TONGUE_SPANISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib  libentRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib  IonCommonSpanish.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "physics - Win32 Japanese"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "physics___Win32_Japanese"
# PROP BASE Intermediate_Dir "physics___Win32_Japanese"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Japanese"
# PROP Intermediate_Dir "Japanese"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "TONGUE_JAPANESE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib  libentRelease.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib  IonCommonJapanese.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "physics - Win32 German Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "physics___Win32_German_Debug"
# PROP BASE Intermediate_Dir "physics___Win32_German_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\German_Debug"
# PROP Intermediate_Dir "German_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "TONGUE_GERMAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /I "..\..\user" /I "..\audio" /I "..\..\user\collection" /I "..\..\libent" /I "../dk_" /D "TONGUE_GERMAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAs /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib  IonCommonGerman.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib  IonCommonGermanDebug.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"

!ENDIF 

# Begin Target

# Name "physics - Win32 Release"
# Name "physics - Win32 Debug"
# Name "physics - Win32 French"
# Name "physics - Win32 German"
# Name "physics - Win32 Italian"
# Name "physics - Win32 Spanish"
# Name "physics - Win32 Japanese"
# Name "physics - Win32 German Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.c;*.cpp"
# Begin Source File

SOURCE=.\com_friction.cpp
# End Source File
# Begin Source File

SOURCE=.\com_heap.cpp
# End Source File
# Begin Source File

SOURCE=.\com_list.cpp
# End Source File
# Begin Source File

SOURCE=.\com_main.cpp
# End Source File
# Begin Source File

SOURCE=.\com_sub.cpp
# End Source File
# Begin Source File

SOURCE=.\com_teamplay.cpp
# End Source File
# Begin Source File

SOURCE=.\com_weapons.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_dll.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_sent.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_ServerState.cpp
# End Source File
# Begin Source File

SOURCE=.\p_client.cpp
# End Source File
# Begin Source File

SOURCE=.\p_concmds.cpp
# End Source File
# Begin Source File

SOURCE=.\p_float.cpp
# End Source File
# Begin Source File

SOURCE=.\p_inventory.cpp
# End Source File
# Begin Source File

SOURCE=.\p_main.cpp
# End Source File
# Begin Source File

SOURCE=.\p_phys.cpp
# End Source File
# Begin Source File

SOURCE=.\p_save.cpp
# End Source File
# Begin Source File

SOURCE=.\p_selector.cpp
# End Source File
# Begin Source File

SOURCE=.\p_stub.cpp
# End Source File
# Begin Source File

SOURCE=.\p_user.cpp

!IF  "$(CFG)" == "physics - Win32 Release"

!ELSEIF  "$(CFG)" == "physics - Win32 Debug"

# ADD CPP /Yc"p_user.h"

!ELSEIF  "$(CFG)" == "physics - Win32 French"

!ELSEIF  "$(CFG)" == "physics - Win32 German"

!ELSEIF  "$(CFG)" == "physics - Win32 Italian"

!ELSEIF  "$(CFG)" == "physics - Win32 Spanish"

!ELSEIF  "$(CFG)" == "physics - Win32 Japanese"

!ELSEIF  "$(CFG)" == "physics - Win32 German Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\p_utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h;*.inc"
# Begin Source File

SOURCE=..\..\user\ai.h
# End Source File
# Begin Source File

SOURCE=.\com_teamplay.h
# End Source File
# Begin Source File

SOURCE=..\..\user\common.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_dll.h
# End Source File
# Begin Source File

SOURCE=..\dk_\dk_log.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=..\..\user\HOOKS.H
# End Source File
# Begin Source File

SOURCE=.\p_client.h
# End Source File
# Begin Source File

SOURCE=.\p_concmds.h
# End Source File
# Begin Source File

SOURCE=..\..\user\p_global.h
# End Source File
# Begin Source File

SOURCE=.\p_inventory.h
# End Source File
# Begin Source File

SOURCE=.\p_selector.h
# End Source File
# Begin Source File

SOURCE=..\..\user\p_user.h
# End Source File
# Begin Source File

SOURCE=..\..\user\physics.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\cvardoc.txt
# End Source File
# End Target
# End Project
