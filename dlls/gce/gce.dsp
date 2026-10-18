# Microsoft Developer Studio Project File - Name="gce" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=gce - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gce.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gce.mak" CFG="gce - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gce - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gce - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gce - Win32 French" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gce - Win32 German" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gce - Win32 Italian" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gce - Win32 Spanish" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gce - Win32 Japanese" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gce - Win32 German Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Daikatana/dlls/gce", RPCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gce - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "TONGUE_ENGLISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "gce - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  winmm.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\debug" /libpath:"..\..\libs"
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "gce - Win32 French"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "gce___Win32_French"
# PROP BASE Intermediate_Dir "gce___Win32_French"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "French"
# PROP Intermediate_Dir "French"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "TONGUE_FRENCH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonFrench.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "gce - Win32 German"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "gce___Win32_German"
# PROP BASE Intermediate_Dir "gce___Win32_German"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "German"
# PROP Intermediate_Dir "German"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "TONGUE_GERMAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib winmm.lib IonCommonGerman.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "gce - Win32 Italian"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "gce___Win32_Italian"
# PROP BASE Intermediate_Dir "gce___Win32_Italian"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Italian"
# PROP Intermediate_Dir "Italian"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "TONGUE_ITALIAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib winmm.lib IonCommonItalian.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "gce - Win32 Spanish"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "gce___Win32_Spanish"
# PROP BASE Intermediate_Dir "gce___Win32_Spanish"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Spanish"
# PROP Intermediate_Dir "Spanish"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "TONGUE_SPANISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonSpanish.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "gce - Win32 Japanese"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "gce___Win32_Japanese"
# PROP BASE Intermediate_Dir "gce___Win32_Japanese"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Japanese"
# PROP Intermediate_Dir "Japanese"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "TONGUE_JAPANESE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonJapanese.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "gce - Win32 German Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "gce___Win32_German_Debug"
# PROP BASE Intermediate_Dir "gce___Win32_German_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "German_Debug"
# PROP Intermediate_Dir "German_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "TONGUE_GERMAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\qcommon" /I "..\..\user" /I "..\..\libclient" /D "TONGUE_GERMAN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FAs /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib winmm.lib IonCommonGerman.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 odbc32.lib odbccp32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib  advapi32.lib kernel32.lib user32.lib gdi32.lib winmm.lib IonCommonGermanDebug.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"

!ENDIF 

# Begin Target

# Name "gce - Win32 Release"
# Name "gce - Win32 Debug"
# Name "gce - Win32 French"
# Name "gce - Win32 German"
# Name "gce - Win32 Italian"
# Name "gce - Win32 Spanish"
# Name "gce - Win32 Japanese"
# Name "gce - Win32 German Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=..\..\user\dk_gce_entities.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_headscript.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_script.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_spline.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_commands.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_entities.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_entity_save.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_fields.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_headscript.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_hud.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_hud_entities.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_hud_headbob.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_hud_position.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_hud_script.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_hud_sequence.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_hud_sounds.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_input.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_main.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_position.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_save.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_script.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_sequence.cpp
# End Source File
# Begin Source File

SOURCE=.\gce_sound.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\..\base\client\cdaudio.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\client.h
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\CMODEL.H
# End Source File
# Begin Source File

SOURCE=..\..\base\client\console.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_array.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_beams.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_cin_types.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_dll.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_entities.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_entity_attributes.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_headbob.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_interface.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_script.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_spline.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_model.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_point.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=.\gce_entities.h
# End Source File
# Begin Source File

SOURCE=.\gce_entity_save.h
# End Source File
# Begin Source File

SOURCE=.\gce_fields.h
# End Source File
# Begin Source File

SOURCE=.\gce_headscript.h
# End Source File
# Begin Source File

SOURCE=.\gce_hud.h
# End Source File
# Begin Source File

SOURCE=.\gce_input.h
# End Source File
# Begin Source File

SOURCE=.\gce_main.h
# End Source File
# Begin Source File

SOURCE=.\gce_position.h
# End Source File
# Begin Source File

SOURCE=.\gce_script.h
# End Source File
# Begin Source File

SOURCE=.\gce_sequence.h
# End Source File
# Begin Source File

SOURCE=.\gce_sound.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\input.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\keys.h
# End Source File
# Begin Source File

SOURCE=..\..\user\p_user.h
# End Source File
# Begin Source File

SOURCE=..\..\user\physics.h
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\qcommon.h
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\qfiles.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\ref.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\screen.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\vid.h
# End Source File
# End Group
# End Target
# End Project
