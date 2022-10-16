# Microsoft Developer Studio Project File - Name="world" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=world - Win32 Demo Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "world.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "world.mak" CFG="world - Win32 Demo Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "world - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "world - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "world - Win32 Demo Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "world - Win32 Demo Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Daikatana/dlls/world", CGBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "world - Win32 Release"

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
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\user" /I "..\..\libent" /D "DAIKATANA_OEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
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
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib wsock32.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /out:"Release/world.dll" /libpath:"..\..\libs"
# SUBTRACT LINK32 /profile /map /debug

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

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
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "..\..\base\dk_" /I "..\..\user" /I "..\..\libent" /D "DAIKATANA_OEM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs" /debug:full
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "world___Win32_Demo_Debug"
# PROP BASE Intermediate_Dir "world___Win32_Demo_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Demo_Debug"
# PROP Intermediate_Dir "Demo_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MTd /W3 /Gm /Gi /ZI /I "..\..\base\dk_" /I "..\..\user" /I "..\..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /MTd /W3 /Gm /Gi /ZI /I "..\..\base\dk_" /I "..\..\user" /I "..\..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DAIKATANA_DEMO" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs" /debug:full
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs" /debug:full
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "world___Win32_Demo_Release"
# PROP BASE Intermediate_Dir "world___Win32_Demo_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo_Release"
# PROP Intermediate_Dir "Demo_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\user" /I "..\..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /I "..\..\base\dk_" /I "..\..\user" /I "..\..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DAIKATANA_DEMO" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib wsock32.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /out:"Release/world.dll" /libpath:"..\..\libs"
# SUBTRACT BASE LINK32 /map /debug
# ADD LINK32 winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib wsock32.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\libs"
# SUBTRACT LINK32 /map /debug

!ENDIF 

# Begin Target

# Name "world - Win32 Release"
# Name "world - Win32 Debug"
# Name "world - Win32 Demo Debug"
# Name "world - Win32 Demo Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "AI Files"

# PROP Default_Filter "*.cpp"
# Begin Group "E1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\battleboar.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cambot.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crox.cpp
# End Source File
# Begin Source File

SOURCE=.\cryotech.cpp
# End Source File
# Begin Source File

SOURCE=.\deathsphere.cpp
# End Source File
# Begin Source File

SOURCE=.\fatworker.cpp
# End Source File
# Begin Source File

SOURCE=.\FireFly.cpp
# End Source File
# Begin Source File

SOURCE=.\froginator.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\inmater.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lasergat.cpp
# End Source File
# Begin Source File

SOURCE=.\mishimaguard.cpp
# End Source File
# Begin Source File

SOURCE=.\prisoner.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\protopod.cpp
# End Source File
# Begin Source File

SOURCE=.\psyclaw.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ragemaster.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rockgat.cpp
# End Source File
# Begin Source File

SOURCE=.\skeeter.cpp
# End Source File
# Begin Source File

SOURCE=.\skinnyworker.cpp
# End Source File
# Begin Source File

SOURCE=.\sludgeminion.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\surgeon.cpp
# End Source File
# Begin Source File

SOURCE=.\thunderskeet.cpp
# End Source File
# Begin Source File

SOURCE=.\vermin.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# End Group
# Begin Group "E2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\centurion.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cerberus.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\column.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ferryman.cpp
# End Source File
# Begin Source File

SOURCE=.\griffon.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\harpy.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\kminos.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\medusa.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\satyr.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\skeleton.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\smallspider.cpp
# End Source File
# Begin Source File

SOURCE=.\spider.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\thief.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# End Group
# Begin Group "E3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\buboid.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\doombat.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dragon.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dwarf.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fletcher.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\garroth.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\knight1.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\knight2.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lycanthir.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nharre.cpp
# End Source File
# Begin Source File

SOURCE=.\prat.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\priest.cpp
# End Source File
# Begin Source File

SOURCE=.\rotworm.cpp
# End Source File
# Begin Source File

SOURCE=.\stavros.cpp
# End Source File
# Begin Source File

SOURCE=.\wisp.cpp
# End Source File
# Begin Source File

SOURCE=.\wyndrax.cpp
# End Source File
# Begin Source File

SOURCE=.\WyndraxandWispShare.h
# End Source File
# End Group
# Begin Group "E4"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\blackpris.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chaingang.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\femgang.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ghost.cpp
# End Source File
# Begin Source File

SOURCE=.\kage.cpp
# End Source File
# Begin Source File

SOURCE=.\labmonkey.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mikiko_monster.cpp
# End Source File
# Begin Source File

SOURCE=.\piperat.cpp
# End Source File
# Begin Source File

SOURCE=.\rocketgang.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rocketmp.cpp
# End Source File
# Begin Source File

SOURCE=.\scomndo.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\seagull.cpp
# End Source File
# Begin Source File

SOURCE=.\sealcap.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sgirl.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shark.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\uzigang.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\whitepris.cpp
# End Source File
# End Group
# Begin Group "Bot"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bot.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BOT.H
# End Source File
# End Group
# Begin Group "Cinematic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cine_entities.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\action.cpp
# End Source File
# Begin Source File

SOURCE=.\action.h
# End Source File
# Begin Source File

SOURCE=.\ai.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\user\ai.h
# End Source File
# Begin Source File

SOURCE=.\ai_aim.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_aim.h
# End Source File
# Begin Source File

SOURCE=..\..\user\ai_common.h
# End Source File
# Begin Source File

SOURCE=.\AI_File.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_frames.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_frames.h
# End Source File
# Begin Source File

SOURCE=.\ai_func.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_func.h
# End Source File
# Begin Source File

SOURCE=.\ai_group.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_info.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_info.h
# End Source File
# Begin Source File

SOURCE=.\ai_move.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AI_MOVE.H
# End Source File
# Begin Source File

SOURCE=.\ai_save.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_save.h
# End Source File
# Begin Source File

SOURCE=.\ai_test.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_think.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_think.h
# End Source File
# Begin Source File

SOURCE=.\ai_utils.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_utils.h
# End Source File
# Begin Source File

SOURCE=.\ai_weapons.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_weapons.h
# End Source File
# Begin Source File

SOURCE=.\dopefish.cpp
# End Source File
# Begin Source File

SOURCE=.\DUMPSTAK.CPP
# End Source File
# Begin Source File

SOURCE=.\dumpstak.h
# End Source File
# Begin Source File

SOURCE=.\func2.cpp
# End Source File
# Begin Source File

SOURCE=.\goldfish.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\greyfish.cpp
# End Source File
# Begin Source File

SOURCE=.\GrphPath.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GrphPath.h
# End Source File
# Begin Source File

SOURCE=.\guppyone.cpp
# End Source File
# Begin Source File

SOURCE=.\guppytwo.cpp
# End Source File
# Begin Source File

SOURCE=.\hiro.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MonsterSound.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MonsterSound.h
# End Source File
# Begin Source File

SOURCE=.\NavTest.cpp
# End Source File
# Begin Source File

SOURCE=.\NavTest.h
# End Source File
# Begin Source File

SOURCE=.\nodelist.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NODELIST.H
# End Source File
# Begin Source File

SOURCE=.\octree.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\octree.h
# End Source File
# Begin Source File

SOURCE=.\PathTable.cpp
# End Source File
# Begin Source File

SOURCE=.\PathTable.h
# End Source File
# Begin Source File

SOURCE=.\SequenceMap.cpp
# End Source File
# Begin Source File

SOURCE=.\SequenceMap.h
# End Source File
# Begin Source File

SOURCE=.\SFX_ENTITIES.CPP
# End Source File
# Begin Source File

SOURCE=.\Sidekick.cpp
# End Source File
# Begin Source File

SOURCE=.\Sidekick.h
# End Source File
# Begin Source File

SOURCE=.\spawn.cpp
# End Source File
# Begin Source File

SOURCE=.\spawn.h
# End Source File
# Begin Source File

SOURCE=.\thinkFuncs.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\thinkFuncs.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\actorlist.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\artifacts.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bodylist.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chasecam.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cin_playback.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\client.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\client_init.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common.cpp
# End Source File
# Begin Source File

SOURCE=.\coop.cpp
# End Source File
# Begin Source File

SOURCE=.\coop.h
# End Source File
# Begin Source File

SOURCE=.\CTF.cpp
# End Source File
# Begin Source File

SOURCE=.\CTF.h
# End Source File
# Begin Source File

SOURCE=.\deathtag.cpp
# End Source File
# Begin Source File

SOURCE=.\door.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\drop_monster.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\effects.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\entity_pointer.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\flag.cpp
# End Source File
# Begin Source File

SOURCE=.\func_event_generator.cpp
# End Source File
# Begin Source File

SOURCE=.\func_monitor.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\func_various.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gib.cpp
# End Source File
# Begin Source File

SOURCE=.\healthtree.cpp
# End Source File
# Begin Source File

SOURCE=.\hosportal.cpp
# End Source File
# Begin Source File

SOURCE=.\inventory.cpp
# End Source File
# Begin Source File

SOURCE=.\items.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\light.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\user\memory.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\misc.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\monsterpaths.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\target.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\transition.cpp
# End Source File
# Begin Source File

SOURCE=.\triggers.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\viewthing.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\world.cpp

!IF  "$(CFG)" == "world - Win32 Release"

# ADD CPP /Zi /Od

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# ADD BASE CPP /Zi /Od
# ADD CPP /Zi /Od

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\actorlist.h
# End Source File
# Begin Source File

SOURCE=.\BODYLIST.H
# End Source File
# Begin Source File

SOURCE=.\CHASECAM.H
# End Source File
# Begin Source File

SOURCE=.\CLIENT.H
# End Source File
# Begin Source File

SOURCE=.\client_init.h
# End Source File
# Begin Source File

SOURCE=..\..\user\collect.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=..\..\user\common.h
# End Source File
# Begin Source File

SOURCE=.\deathtag.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_cin_types.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_cin_world_interface.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_dll.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_entity_attributes.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_hashtable.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=.\entity_pointer.h
# End Source File
# Begin Source File

SOURCE=.\EPAIRS.H
# End Source File
# Begin Source File

SOURCE=.\flag.h
# End Source File
# Begin Source File

SOURCE=.\floater.h
# End Source File
# Begin Source File

SOURCE=..\..\user\hierarchy.h
# End Source File
# Begin Source File

SOURCE=..\..\user\HOOKS.H
# End Source File
# Begin Source File

SOURCE=.\items.h
# End Source File
# Begin Source File

SOURCE=..\..\user\levels.h
# End Source File
# Begin Source File

SOURCE=.\LIGHT.H
# End Source File
# Begin Source File

SOURCE=.\MISC.H
# End Source File
# Begin Source File

SOURCE=..\..\user\p_user.h
# End Source File
# Begin Source File

SOURCE=..\..\user\physics.h
# End Source File
# Begin Source File

SOURCE=.\satyr.h
# End Source File
# Begin Source File

SOURCE=.\transition.h
# End Source File
# Begin Source File

SOURCE=.\TRIGGERS.H
# End Source File
# Begin Source File

SOURCE=.\viewthing.h
# End Source File
# Begin Source File

SOURCE=..\..\user\weapondefs.h
# End Source File
# Begin Source File

SOURCE=.\WORLD.H
# End Source File
# End Group
# Begin Group "q2_reference"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Daikatana\game\p_client.c

!IF  "$(CFG)" == "world - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "world - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "world - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "world - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\q2_functions.txt
# End Source File
# End Group
# End Target
# End Project
