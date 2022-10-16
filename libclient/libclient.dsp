# Microsoft Developer Studio Project File - Name="libclient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=libclient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libclient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libclient.mak" CFG="libclient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libclient - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libclient - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/Daikatana/libclient", EIGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libclient - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\user" /I "..\base\dk_" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "libclient - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "..\user" /I "..\base\dk_" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ioncommon.lib winmm.lib mss32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../base/Debug/libclient.dll" /implib:"../libs/libclient.lib" /pdbtype:sept /libpath:"..\libs"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "libclient - Win32 Release"
# Name "libclient - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CEPhysics.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientEntity.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientEntityManager.cpp
# End Source File
# Begin Source File

SOURCE=.\IonBlast.cpp
# End Source File
# Begin Source File

SOURCE=.\Particle.cpp
# End Source File
# Begin Source File

SOURCE=.\Particle_IonTrail.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleFX_IonSpray.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleFX_Smoke.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleFX_Smoke.h
# End Source File
# Begin Source File

SOURCE=.\Projectile.cpp
# End Source File
# Begin Source File

SOURCE=.\Rocket.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundFX.cpp
# End Source File
# Begin Source File

SOURCE=.\TrackFlare.cpp
# End Source File
# Begin Source File

SOURCE=.\TrackingEntity.cpp
# End Source File
# Begin Source File

SOURCE=.\TrackLight.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CEPhysics.h
# End Source File
# Begin Source File

SOURCE=.\ClientEntity.h
# End Source File
# Begin Source File

SOURCE=.\ClientEntityManager.h
# End Source File
# Begin Source File

SOURCE=.\IonBlast.h
# End Source File
# Begin Source File

SOURCE=..\user\libclient.h
# End Source File
# Begin Source File

SOURCE=.\libclientlocals.h
# End Source File
# Begin Source File

SOURCE=.\Particle.h
# End Source File
# Begin Source File

SOURCE=.\Particle_IonTrail.h
# End Source File
# Begin Source File

SOURCE=.\ParticleFX.h
# End Source File
# Begin Source File

SOURCE=.\ParticleFX_IonSpray.h
# End Source File
# Begin Source File

SOURCE=.\Projectile.h
# End Source File
# Begin Source File

SOURCE=.\Rocket.h
# End Source File
# Begin Source File

SOURCE=.\SoundFX.h
# End Source File
# Begin Source File

SOURCE=.\TrackFlare.h
# End Source File
# Begin Source File

SOURCE=.\TrackingEntity.h
# End Source File
# Begin Source File

SOURCE=.\TrackLight.h
# End Source File
# End Group
# End Target
# End Project
