# Microsoft Developer Studio Project File - Name="daikatana" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=daikatana - Win32 Demo Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "daikatana.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "daikatana.mak" CFG="daikatana - Win32 Demo Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "daikatana - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "daikatana - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "daikatana - Win32 Demo Debug" (based on "Win32 (x86) Application")
!MESSAGE "daikatana - Win32 Demo Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Daikatana/base", OTAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "daikatana - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "." /I ".\dk_" /I ".\client" /I ".\win32" /I ".\qcommon" /I ".\server" /I "..\user" /I "..\audio" /I "..\user\collection" /I "..\libent" /D "DAIKATANA_OEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Z<none> /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib mss32.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\dlls\world\release" /libpath:"..\libs"
# SUBTRACT LINK32 /map /nodefaultlib

!ELSEIF  "$(CFG)" == "daikatana - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "." /I ".\dk_" /I ".\client" /I ".\win32" /I ".\qcommon" /I ".\server" /I "..\user" /I "..\user\collection" /I "..\audio" /I "..\libent" /I "../../dk_" /D "AUDIO_DEBUG" /D "DAIKATANA_OEM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX"client.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 imm32.lib winmm.lib wsock32.lib ole32.lib mss32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libc.lib" /libpath:"..\dlls\world\debug" /libpath:"..\libs"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "daikatana - Win32 Demo Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "daikatana___Win32_Demo_Debug"
# PROP BASE Intermediate_Dir "daikatana___Win32_Demo_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Demo_Debug"
# PROP Intermediate_Dir "Demo_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MTd /W3 /Gi /GX /ZI /Od /I "." /I ".\dk_" /I ".\client" /I ".\win32" /I ".\qcommon" /I ".\server" /I "..\user" /I "..\user\collection" /I "..\audio" /I "..\libent" /I "../../dk_" /D "AUDIO_DEBUG" /D "TONGUE_ENGLISH" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX"client.h" /FD /c
# ADD CPP /nologo /G6 /MTd /W3 /Gi /GX /ZI /Od /I "." /I ".\dk_" /I ".\client" /I ".\win32" /I ".\qcommon" /I ".\server" /I "..\user" /I "..\user\collection" /I "..\audio" /I "..\libent" /I "../../dk_" /D "AUDIO_DEBUG" /D "TONGUE_ENGLISH" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DAIKATANA_DEMO" /FR /YX"client.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib wsock32.lib ole32.lib mss32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libc.lib" /libpath:"..\dlls\world\debug" /libpath:"..\libs"
# SUBTRACT BASE LINK32 /pdb:none /incremental:no
# ADD LINK32 winmm.lib wsock32.lib ole32.lib mss32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libc.lib" /libpath:"..\dlls\world\debug" /libpath:"..\libs"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "daikatana - Win32 Demo Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "daikatana___Win32_Demo_Release"
# PROP BASE Intermediate_Dir "daikatana___Win32_Demo_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo_Release"
# PROP Intermediate_Dir "Demo_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MT /W3 /GX /O2 /I "." /I ".\dk_" /I ".\client" /I ".\win32" /I ".\qcommon" /I ".\server" /I "..\user" /I "..\audio" /I "..\user\collection" /I "..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /I "." /I ".\dk_" /I ".\client" /I ".\win32" /I ".\qcommon" /I ".\server" /I "..\user" /I "..\audio" /I "..\user\collection" /I "..\libent" /D "TONGUE_ENGLISH" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DAIKATANA_DEMO" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib mss32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\dlls\world\release" /libpath:"..\libs"
# SUBTRACT BASE LINK32 /map /debug /nodefaultlib
# ADD LINK32 winmm.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib mss32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\dlls\world\release" /libpath:"..\libs"
# SUBTRACT LINK32 /map /debug /nodefaultlib

!ENDIF 

# Begin Target

# Name "daikatana - Win32 Release"
# Name "daikatana - Win32 Debug"
# Name "daikatana - Win32 Demo Debug"
# Name "daikatana - Win32 Demo Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "Daikatana Source Files"

# PROP Default_Filter "dk_*.c;dk_*.cpp"
# Begin Source File

SOURCE=.\dk_\dk_beams.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_cin_playback.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_cl_curve.cpp
# End Source File
# Begin Source File

SOURCE=..\user\dk_gce_entities.cpp
# End Source File
# Begin Source File

SOURCE=..\user\dk_gce_headscript.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_gce_load.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_gce_main.cpp
# End Source File
# Begin Source File

SOURCE=..\user\dk_gce_script.cpp
# End Source File
# Begin Source File

SOURCE=..\user\dk_gce_spline.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_log.cpp
# End Source File
# Begin Source File

SOURCE=..\user\dk_matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_advanced.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_controls.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_demos.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_joystick.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_loadgame.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_multiplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_newgame.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_options.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_quit.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_savegame.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_video.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_weapons.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_model.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_print.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_resource.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_scoreboard.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_win.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\mp_launch.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Artifact_fx.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\cd_win.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_ents.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_flash.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_fx.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_icons.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_icons.h
# End Source File
# Begin Source File

SOURCE=.\client\cl_input.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_inventory.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_main.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_parse.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_pred.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_pv.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_scrn.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_selector.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_sidekick.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_tent.cpp
# End Source File
# Begin Source File

SOURCE=.\client\cl_view.cpp
# End Source File
# Begin Source File

SOURCE=.\client.cpp

!IF  "$(CFG)" == "daikatana - Win32 Release"

!ELSEIF  "$(CFG)" == "daikatana - Win32 Debug"

# ADD CPP /Yc"client.h"

!ELSEIF  "$(CFG)" == "daikatana - Win32 Demo Debug"

# ADD BASE CPP /Yc"client.h"
# ADD CPP /Yc"client.h"

!ELSEIF  "$(CFG)" == "daikatana - Win32 Demo Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\qcommon\cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\qcommon\cmodel_dynamic.cpp
# End Source File
# Begin Source File

SOURCE=.\qcommon\common.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\conproc.cpp
# End Source File
# Begin Source File

SOURCE=.\client\console.cpp
# End Source File
# Begin Source File

SOURCE=.\qcommon\cvar.cpp
# End Source File
# Begin Source File

SOURCE=.\Goa\CEngine\darray.c
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_screen_coord.cpp
# End Source File
# Begin Source File

SOURCE=.\Audio\DkAudioImpLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\qcommon\files.cpp
# End Source File
# Begin Source File

SOURCE=.\Goa\Sdk\goautil.cpp
# End Source File
# Begin Source File

SOURCE=.\Goa\CEngine\gserver.c
# End Source File
# Begin Source File

SOURCE=.\Goa\CEngine\gserverlist.c
# End Source File
# Begin Source File

SOURCE=.\Goa\CEngine\gutil.c
# End Source File
# Begin Source File

SOURCE=.\Goa\CEngine\hashtable.c
# End Source File
# Begin Source File

SOURCE=.\win32\Imeui.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\in_win.cpp
# End Source File
# Begin Source File

SOURCE=.\client\keys.cpp
# End Source File
# Begin Source File

SOURCE=.\qcommon\md4.cpp
# End Source File
# Begin Source File

SOURCE=..\user\memmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\client\menu.cpp
# End Source File
# Begin Source File

SOURCE=.\qcommon\net_chan.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\net_wins.cpp
# End Source File
# Begin Source File

SOURCE=.\Goa\nonport.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\pmove.cpp
# End Source File
# Begin Source File

SOURCE=.\Projectile_fx.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\q_shwin.cpp
# End Source File
# Begin Source File

SOURCE=.\client\qmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\server\sv_ccmds.cpp
# End Source File
# Begin Source File

SOURCE=.\server\sv_clientString.cpp
# End Source File
# Begin Source File

SOURCE=.\server\sv_ents.cpp
# End Source File
# Begin Source File

SOURCE=.\server\sv_game.cpp
# End Source File
# Begin Source File

SOURCE=.\server\sv_init.cpp
# End Source File
# Begin Source File

SOURCE=.\server\sv_main.cpp
# End Source File
# Begin Source File

SOURCE=.\server\sv_send.cpp
# End Source File
# Begin Source File

SOURCE=.\server\sv_user.cpp
# End Source File
# Begin Source File

SOURCE=.\server\sv_world.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\sys_win.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\vid_dll.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\vid_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\client\x86.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\user\anorms.h
# End Source File
# Begin Source File

SOURCE=.\Audio\Audio.h
# End Source File
# Begin Source File

SOURCE=.\Audio\AudioBase.h
# End Source File
# Begin Source File

SOURCE=.\Audio\AudioDebug.h
# End Source File
# Begin Source File

SOURCE=.\Audio\AudioEngineImports.h
# End Source File
# Begin Source File

SOURCE=.\client\cdaudio.h
# End Source File
# Begin Source File

SOURCE=.\client\cl_inventory.h
# End Source File
# Begin Source File

SOURCE=.\client\cl_selector.h
# End Source File
# Begin Source File

SOURCE=.\client\cl_sidekick.h
# End Source File
# Begin Source File

SOURCE=.\client\client.h
# End Source File
# Begin Source File

SOURCE=.\qcommon\CMODEL.H
# End Source File
# Begin Source File

SOURCE=.\win32\conproc.h
# End Source File
# Begin Source File

SOURCE=.\client\console.h
# End Source File
# Begin Source File

SOURCE=.\dk_\daikatana.h
# End Source File
# Begin Source File

SOURCE=.\Goa\CEngine\darray.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_array.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_beams.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_bitmap.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_buffer.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_cin_playback.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_cin_types.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_cin_world_interface.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_cl_curve.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_gce_entities.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_gce_entity_attributes.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_gce_headbob.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_gce_interface.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_gce_main.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_gce_script.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_gce_spline.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_inv.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_matrix.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_controls.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menup.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_misc.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_model.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_point.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_pointer.h
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_scoreboard.h
# End Source File
# Begin Source File

SOURCE=.\dk_screen_coord.h
# End Source File
# Begin Source File

SOURCE=..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=.\Goa\CEngine\goaceng.h
# End Source File
# Begin Source File

SOURCE=.\Goa\Sdk\goautil.h
# End Source File
# Begin Source File

SOURCE=.\Goa\CEngine\gserver.h
# End Source File
# Begin Source File

SOURCE=.\Goa\CEngine\gutil.h
# End Source File
# Begin Source File

SOURCE=.\Goa\CEngine\hashtable.h
# End Source File
# Begin Source File

SOURCE=.\win32\ImeFull.h
# End Source File
# Begin Source File

SOURCE=.\client\input.h
# End Source File
# Begin Source File

SOURCE=.\client\keys.h
# End Source File
# Begin Source File

SOURCE=..\user\l__language.h
# End Source File
# Begin Source File

SOURCE=..\user\l_english.h
# End Source File
# Begin Source File

SOURCE=..\user\memmgr.h
# End Source File
# Begin Source File

SOURCE=.\dk_\mp_launch.h
# End Source File
# Begin Source File

SOURCE=.\Goa\nonport.h
# End Source File
# Begin Source File

SOURCE=..\user\old_anorms.h
# End Source File
# Begin Source File

SOURCE=..\user\p_user.h
# End Source File
# Begin Source File

SOURCE=..\user\physics.h
# End Source File
# Begin Source File

SOURCE=.\Projectile_fx.h
# End Source File
# Begin Source File

SOURCE=.\qcommon\qcommon.h
# End Source File
# Begin Source File

SOURCE=.\qcommon\qfiles.h
# End Source File
# Begin Source File

SOURCE=.\client\qmenu.h
# End Source File
# Begin Source File

SOURCE=.\client\ref.h
# End Source File
# Begin Source File

SOURCE=.\client\screen.h
# End Source File
# Begin Source File

SOURCE=.\server\server.h
# End Source File
# Begin Source File

SOURCE=.\client\snd_loc.h
# End Source File
# Begin Source File

SOURCE=.\client\sound.h
# End Source File
# Begin Source File

SOURCE=.\client\vid.h
# End Source File
# Begin Source File

SOURCE=.\win32\winquake.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\katana.ico
# End Source File
# End Group
# End Target
# End Project
