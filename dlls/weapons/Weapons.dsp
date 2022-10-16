# Microsoft Developer Studio Project File - Name="weapons" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=weapons - Win32 Demo Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Weapons.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Weapons.mak" CFG="weapons - Win32 Demo Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "weapons - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "weapons - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "weapons - Win32 Demo Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "weapons - Win32 Demo Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Daikatana/dlls/weapons", YJBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "weapons - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\user" /I "..\..\base\audio" /I "..\..\user\Collection" /I "..\..\libent" /D "DAIKATANA_OEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Z<none> /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib kernel32.lib user32.lib gdi32.lib world.lib IonCommonRelease.lib wsock32.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /out:"release\weapons.dll" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT LINK32 /map /debug

!ELSEIF  "$(CFG)" == "weapons - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "..\..\user" /I "..\..\base\audio" /I "..\..\user\Collection" /I "..\..\libent" /D "DAIKATANA_OEM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib world.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\debug" /libpath:"..\..\libs"
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "weapons - Win32 Demo Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "weapons___Win32_Demo_Debug"
# PROP BASE Intermediate_Dir "weapons___Win32_Demo_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Demo_Debug"
# PROP Intermediate_Dir "Demo_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MTd /W3 /Gm /Gi /ZI /Od /I "..\..\user" /I "..\..\base\audio" /I "..\..\user\Collection" /I "..\..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fr /YX /FD /c
# ADD CPP /nologo /G6 /MTd /W3 /Gm /Gi /ZI /Od /I "..\..\user" /I "..\..\base\audio" /I "..\..\user\Collection" /I "..\..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib world.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\debug" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib ..\world\demo_debug\world.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\debug" /libpath:"..\..\libs"
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "weapons - Win32 Demo Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "weapons___Win32_Demo_Release"
# PROP BASE Intermediate_Dir "weapons___Win32_Demo_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo_Release"
# PROP Intermediate_Dir "Demo_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MT /W3 /GX /O2 /I "..\..\user" /I "..\..\base\audio" /I "..\..\user\Collection" /I "..\..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /I "..\..\user" /I "..\..\base\audio" /I "..\..\user\Collection" /I "..\..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib kernel32.lib user32.lib gdi32.lib world.lib IonCommonRelease.lib wsock32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /out:"release\weapons.dll" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /incremental:yes /map /debug
# ADD LINK32 winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib kernel32.lib user32.lib gdi32.lib ..\world\demo_release\world.lib IonCommonRelease.lib wsock32.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\world\release" /libpath:"..\..\libs"
# SUBTRACT LINK32 /incremental:yes /map /debug

!ENDIF 

# Begin Target

# Name "weapons - Win32 Release"
# Name "weapons - Win32 Debug"
# Name "weapons - Win32 Demo Debug"
# Name "weapons - Win32 Demo Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "e1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\c4.cpp
# End Source File
# Begin Source File

SOURCE=.\disruptor.cpp
# End Source File
# Begin Source File

SOURCE=.\gashands.cpp
# End Source File
# Begin Source File

SOURCE=.\ionblaster.cpp
# End Source File
# Begin Source File

SOURCE=.\shockwave.cpp
# End Source File
# Begin Source File

SOURCE=.\shotcycler.cpp
# End Source File
# Begin Source File

SOURCE=.\sidewinder.cpp
# End Source File
# End Group
# Begin Group "e2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\discus.cpp
# End Source File
# Begin Source File

SOURCE=.\hammer.cpp
# End Source File
# Begin Source File

SOURCE=.\sunflare.cpp
# End Source File
# Begin Source File

SOURCE=.\trident.cpp
# End Source File
# Begin Source File

SOURCE=.\venomous.cpp
# End Source File
# Begin Source File

SOURCE=.\zeus.cpp
# End Source File
# End Group
# Begin Group "e3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ballista.cpp
# End Source File
# Begin Source File

SOURCE=.\bolter.cpp
# End Source File
# Begin Source File

SOURCE=.\nightmare.cpp
# End Source File
# Begin Source File

SOURCE=.\silverclaw.cpp
# End Source File
# Begin Source File

SOURCE=.\stavros.cpp
# End Source File
# Begin Source File

SOURCE=.\wyndrax.cpp
# End Source File
# End Group
# Begin Group "e4"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\glock.cpp
# End Source File
# Begin Source File

SOURCE=.\kineticore.cpp
# End Source File
# Begin Source File

SOURCE=.\metamaser.cpp
# End Source File
# Begin Source File

SOURCE=.\novabeam.cpp
# End Source File
# Begin Source File

SOURCE=.\ripgun.cpp
# End Source File
# Begin Source File

SOURCE=.\slugger.cpp
# End Source File
# End Group
# Begin Group "global"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\daikatana.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\w_attribute.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon_funcs.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\user\ai.h
# End Source File
# Begin Source File

SOURCE=..\world\ai_frames.h
# End Source File
# Begin Source File

SOURCE=..\world\ai_func.h
# End Source File
# Begin Source File

SOURCE=.\ballista.h
# End Source File
# Begin Source File

SOURCE=.\bolter.h
# End Source File
# Begin Source File

SOURCE=.\c4.h
# End Source File
# Begin Source File

SOURCE=.\celestril.h
# End Source File
# Begin Source File

SOURCE=..\world\CHASECAM.H
# End Source File
# Begin Source File

SOURCE=..\..\user\common.h
# End Source File
# Begin Source File

SOURCE=.\daikatana.h
# End Source File
# Begin Source File

SOURCE=.\discus.h
# End Source File
# Begin Source File

SOURCE=.\disruptor.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_dll.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=.\flashlight.h
# End Source File
# Begin Source File

SOURCE=.\gashands.h
# End Source File
# Begin Source File

SOURCE=.\GLOCK.H
# End Source File
# Begin Source File

SOURCE=.\greekfire.h
# End Source File
# Begin Source File

SOURCE=.\hammer.h
# End Source File
# Begin Source File

SOURCE=..\..\user\hierarchy.h
# End Source File
# Begin Source File

SOURCE=..\..\user\HOOKS.H
# End Source File
# Begin Source File

SOURCE=.\ionblaster.h
# End Source File
# Begin Source File

SOURCE=.\kineticore.h
# End Source File
# Begin Source File

SOURCE=.\metamaser.h
# End Source File
# Begin Source File

SOURCE=.\midas.h
# End Source File
# Begin Source File

SOURCE=.\nightmare.h
# End Source File
# Begin Source File

SOURCE=.\novabeam.h
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

SOURCE=.\ripgun.h
# End Source File
# Begin Source File

SOURCE=.\shockwave.h
# End Source File
# Begin Source File

SOURCE=.\shotcycler.h
# End Source File
# Begin Source File

SOURCE=.\sidewinder.h
# End Source File
# Begin Source File

SOURCE=.\silverclaw.h
# End Source File
# Begin Source File

SOURCE=.\slugger.h
# End Source File
# Begin Source File

SOURCE=.\stavros.h
# End Source File
# Begin Source File

SOURCE=.\sunflare.h
# End Source File
# Begin Source File

SOURCE=.\tazerhook.h
# End Source File
# Begin Source File

SOURCE=.\testweapon.h
# End Source File
# Begin Source File

SOURCE=.\trident.h
# End Source File
# Begin Source File

SOURCE=..\..\user\vector.h
# End Source File
# Begin Source File

SOURCE=.\venomous.h
# End Source File
# Begin Source File

SOURCE=.\w_attribute.h
# End Source File
# Begin Source File

SOURCE=.\weapon_funcs.h
# End Source File
# Begin Source File

SOURCE=..\..\user\weapondefs.h
# End Source File
# Begin Source File

SOURCE=.\weapons.h
# End Source File
# Begin Source File

SOURCE=.\wyndrax.h
# End Source File
# Begin Source File

SOURCE=.\zeus.h
# End Source File
# End Group
# End Target
# End Project
