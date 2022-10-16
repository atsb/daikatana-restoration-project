# Microsoft Developer Studio Project File - Name="PathTable" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PathTable - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PathTable.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PathTable.mak" CFG="PathTable - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PathTable - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PathTable - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/PathTable", SDHAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PathTable - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "PathTable - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PathTable___Win32_Debug"
# PROP BASE Intermediate_Dir "PathTable___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /MDd /W3 /GX /ZI /Od /I "..\..\base" /I "..\..\base\dk_" /I "..\..\base\client" /I "..\..\base\win32" /I "..\..\base\qcommon" /I "..\..\base\\server" /I "..\..\user" /I "..\..\user\collection" /I "..\..\base\audio" /I "..\..\libent" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "BUILD_PATH_TABLE" /FR /FD /GZ /c
# SUBTRACT CPP /nologo /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib wsock32.lib ole32.lib mss32.lib libent.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommon.lib /nologo /subsystem:windows /incremental:no /map /debug /machine:I386 /nodefaultlib:"libcd.lib" /pdbtype:sept /libpath:"..\dlls\world\debug" /libpath:"..\libs"

!ENDIF 

# Begin Target

# Name "PathTable - Win32 Release"
# Name "PathTable - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Daikatana Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\base\dk_\dk_beams.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_bitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_cin_playback.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_cl_curve.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_entities.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_headscript.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_gce_load.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_gce_main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_script.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_spline.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_matrix.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_advanced.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_controls.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_joystick.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_keyboard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_mouse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_multiplayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_newgame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_options.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_video.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_model.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_print.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_resource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_scoreboard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_win.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\base\win32\cd_win.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_cin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_ents.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_flash.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_fx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_icons.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_icons.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_input.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_inv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_inventory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_parse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_pred.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_pv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_scrn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_static.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_tent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_uvanim.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_view.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\cmd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\cmodel_dynamic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\common.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\win32\conproc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\console.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\cvar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\CEngine\darray.c
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_screen_coord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\Audio\DkAudioImpLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\files.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\Sdk\goautil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\CEngine\gserver.c
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\CEngine\gserverlist.c
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\CEngine\gutil.c
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\CEngine\hashtable.c
# End Source File
# Begin Source File

SOURCE=..\..\base\win32\in_win.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\keys.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\md4.cpp
# End Source File
# Begin Source File

SOURCE=..\..\user\memmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\menu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\net_chan.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\win32\net_wins.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\nonport.c
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\pmove.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\win32\q_shwin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\qmenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\server\sv_ccmds.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\server\sv_ents.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\server\sv_game.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\server\sv_init.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\server\sv_main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\server\sv_send.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\server\sv_user.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\server\sv_world.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\win32\sys_win.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\win32\vid_dll.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\win32\vid_menu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\base\client\x86.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\user\anorms.h
# End Source File
# Begin Source File

SOURCE=..\..\base\Audio\Audio.h
# End Source File
# Begin Source File

SOURCE=..\..\base\Audio\AudioBase.h
# End Source File
# Begin Source File

SOURCE=..\..\base\Audio\AudioDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\base\Audio\AudioEngineImports.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cdaudio.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\cl_inventory.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\client.h
# End Source File
# Begin Source File

SOURCE=..\..\base\qcommon\CMODEL.H
# End Source File
# Begin Source File

SOURCE=..\..\base\win32\conproc.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\console.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\daikatana.h
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\CEngine\darray.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_array.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_beams.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_bitmap.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_cin_playback.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_cin_types.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_cin_world_interface.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_cl_curve.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_defines.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_entities.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_headbob.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_interface.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_gce_main.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_script.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_gce_spline.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_inv.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_matrix.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menu_controls.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_menup.h
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

SOURCE=..\..\base\dk_\dk_pointer.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_\dk_scoreboard.h
# End Source File
# Begin Source File

SOURCE=..\..\base\dk_screen_coord.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\Sdk\goautil.h
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\CEngine\gserver.h
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\CEngine\gutil.h
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\CEngine\hashtable.h
# End Source File
# Begin Source File

SOURCE=..\..\user\memmgr.h
# End Source File
# Begin Source File

SOURCE=..\..\base\Goa\nonport.h
# End Source File
# Begin Source File

SOURCE=..\..\user\old_anorms.h
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

SOURCE=..\..\user\qfiles.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\qmenu.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\ref.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\screen.h
# End Source File
# Begin Source File

SOURCE=..\..\base\server\server.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\snd_loc.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\base\client\vid.h
# End Source File
# Begin Source File

SOURCE=..\..\base\win32\winquake.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
