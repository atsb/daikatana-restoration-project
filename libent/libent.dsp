# Microsoft Developer Studio Project File - Name="libent" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libent - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libent.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libent.mak" CFG="libent - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libent - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libent - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libent - Win32 French" (based on "Win32 (x86) Static Library")
!MESSAGE "libent - Win32 German" (based on "Win32 (x86) Static Library")
!MESSAGE "libent - Win32 Spanish" (based on "Win32 (x86) Static Library")
!MESSAGE "libent - Win32 Italian" (based on "Win32 (x86) Static Library")
!MESSAGE "libent - Win32 Japanese" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Daikatana/libent", XJGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libent - Win32 Release"

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
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "TONGUE_ENGLISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\libs\libentRelease.lib"

!ELSEIF  "$(CFG)" == "libent - Win32 Debug"

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
# ADD CPP /nologo /G5 /MTd /W3 /GX /Z7 /Od /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "TONGUE_ENGLISH" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\libs\libentDebug.lib"

!ELSEIF  "$(CFG)" == "libent - Win32 French"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libent___Win32_French"
# PROP BASE Intermediate_Dir "libent___Win32_French"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "French"
# PROP Intermediate_Dir "French"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TONGUE_FRENCH" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\libs\libentRelease.lib"
# ADD LIB32 /nologo /out:"..\libs\libentFrench.lib"

!ELSEIF  "$(CFG)" == "libent - Win32 German"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libent___Win32_German"
# PROP BASE Intermediate_Dir "libent___Win32_German"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "German"
# PROP Intermediate_Dir "German"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TONGUE_GERMAN" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\libs\libentRelease.lib"
# ADD LIB32 /nologo /out:"..\libs\libentGerman.lib"

!ELSEIF  "$(CFG)" == "libent - Win32 Spanish"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libent___Win32_Spanish"
# PROP BASE Intermediate_Dir "libent___Win32_Spanish"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Spanish"
# PROP Intermediate_Dir "Spanish"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TONGUE_SPANISH" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\libs\libentRelease.lib"
# ADD LIB32 /nologo /out:"..\libs\libentSpanish.lib"

!ELSEIF  "$(CFG)" == "libent - Win32 Italian"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libent___Win32_Italian"
# PROP BASE Intermediate_Dir "libent___Win32_Italian"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Italian"
# PROP Intermediate_Dir "Italian"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TONGUE_ITALIAN" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\libs\libentRelease.lib"
# ADD LIB32 /nologo /out:"..\libs\libentItalian.lib"

!ELSEIF  "$(CFG)" == "libent - Win32 Japanese"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libent___Win32_Japanese"
# PROP BASE Intermediate_Dir "libent___Win32_Japanese"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Japanese"
# PROP Intermediate_Dir "Japanese"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\user" /I "..\base\client" /I "..\base\qcommon" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "TONGUE_JAPANESE" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\libs\libentRelease.lib"
# ADD LIB32 /nologo /out:"..\libs\libentJapanese.lib"

!ENDIF 

# Begin Target

# Name "libent - Win32 Release"
# Name "libent - Win32 Debug"
# Name "libent - Win32 French"
# Name "libent - Win32 German"
# Name "libent - Win32 Spanish"
# Name "libent - Win32 Italian"
# Name "libent - Win32 Japanese"
# Begin Group "Source Files"

# PROP Default_Filter "cpp"
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

SOURCE=.\FadeEnt.cpp
# End Source File
# Begin Source File

SOURCE=.\HUD.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDChar.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDCSpeeds.cpp
# End Source File
# Begin Source File

SOURCE=.\HUDGraphic.cpp
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
# Begin Source File

SOURCE=.\TrackLightBlinking.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
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

SOURCE=.\FadeEnt.h
# End Source File
# Begin Source File

SOURCE=.\HUD.h
# End Source File
# Begin Source File

SOURCE=.\HUDChar.h
# End Source File
# Begin Source File

SOURCE=.\HUDCSpeeds.h
# End Source File
# Begin Source File

SOURCE=.\HUDGraphic.h
# End Source File
# Begin Source File

SOURCE=.\IonBlast.h
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

SOURCE=.\ParticleFX_Smoke.h
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
# Begin Source File

SOURCE=.\TrackLightBlinking.h
# End Source File
# End Group
# End Target
# End Project
