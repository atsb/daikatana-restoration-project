# Microsoft Developer Studio Project File - Name="daikatana" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DAIKATANA - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "daikatana.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "daikatana.mak" CFG="DAIKATANA - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "daikatana - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "daikatana - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "daikatana - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
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
# ADD CPP /nologo /G5 /MT /W3 /GX /Zd /O2 /I "." /I ".\dk_" /I ".\client" /I ".\win32" /I ".\qcommon" /I ".\server" /I "..\user" /I "..\audio" /I "..\user\collection" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /map /debug /machine:I386 /libpath:"..\dlls\world\release"
# SUBTRACT LINK32 /incremental:yes /nodefaultlib

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
# ADD CPP /nologo /G5 /MTd /W3 /GX /Zi /Od /I "." /I ".\dk_" /I ".\client" /I ".\win32" /I ".\qcommon" /I ".\server" /I "..\user" /I "..\user\collection" /I "..\audio" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /Gy /YX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 winmm.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib ole32.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /libpath:"..\dlls\world\debug"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "daikatana - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "daikata1"
# PROP BASE Intermediate_Dir "daikata1"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "profile"
# PROP Intermediate_Dir "profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /Zd /O2 /I "." /I ".\dk_" /I ".\client" /I ".\win32" /I ".\qcommon" /I ".\server" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /Zd /O2 /I "." /I ".\dk_" /I ".\client" /I ".\win32" /I ".\qcommon" /I ".\server" /I "..\user" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes /map /nodefaultlib
# ADD LINK32 winmm.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib world.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /pdb:none /incremental:yes /map /nodefaultlib

!ENDIF 

# Begin Target

# Name "daikatana - Win32 Release"
# Name "daikatana - Win32 Debug"
# Name "daikatana - Win32 Profile"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "Daikatana Source Files"

# PROP Default_Filter "dk_*.c;dk_*.cpp"
# Begin Source File

SOURCE=.\dk_\dk_beams.c
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

SOURCE=.\dk_\dk_inv.c
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_log.c
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_menu_controls.cpp
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_model.c
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_print.c
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_resource.c
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_scoreboard.c
# End Source File
# Begin Source File

SOURCE=.\dk_\dk_win.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\Audio\AudioSoundDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\cd_win.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_cin.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_ents.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_flash.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_fx.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_hierarchy.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_input.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_inv.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_main.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_parse.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_pred.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_pv.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_scrn.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_static.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_tent.c
# End Source File
# Begin Source File

SOURCE=.\client\cl_view.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\cmd.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\cmodel.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\qcommon\cmodel_dynamic.c
# End Source File
# Begin Source File

SOURCE=..\User\Collection\Col_Map_sp.cpp
# End Source File
# Begin Source File

SOURCE=..\User\Collection\Col_Plex.cpp
# End Source File
# Begin Source File

SOURCE=.\qcommon\common.c
# End Source File
# Begin Source File

SOURCE=.\win32\conproc.c
# End Source File
# Begin Source File

SOURCE=.\client\console.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\cvar.c
# End Source File
# Begin Source File

SOURCE=..\user\dk_shared.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\files.c
# End Source File
# Begin Source File

SOURCE=.\win32\in_win.c
# End Source File
# Begin Source File

SOURCE=..\User\Collection\IStringCore.cpp
# End Source File
# Begin Source File

SOURCE=.\client\keys.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\md4.c
# End Source File
# Begin Source File

SOURCE=.\client\menu.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\net_chan.c
# End Source File
# Begin Source File

SOURCE=.\win32\net_wins.c
# End Source File
# Begin Source File

SOURCE=.\qcommon\pmove.c
# End Source File
# Begin Source File

SOURCE=.\win32\q_shwin.c
# End Source File
# Begin Source File

SOURCE=.\client\qmenu.c
# End Source File
# Begin Source File

SOURCE=.\client\snd_dma.c
# End Source File
# Begin Source File

SOURCE=.\client\snd_mem.c
# End Source File
# Begin Source File

SOURCE=.\client\snd_mix.c
# End Source File
# Begin Source File

SOURCE=.\win32\snd_win.c
# End Source File
# Begin Source File

SOURCE=.\server\sv_ccmds.c
# End Source File
# Begin Source File

SOURCE=.\server\sv_ents.c
# End Source File
# Begin Source File

SOURCE=.\server\sv_game.c
# End Source File
# Begin Source File

SOURCE=.\server\sv_init.c
# End Source File
# Begin Source File

SOURCE=.\server\sv_main.c
# End Source File
# Begin Source File

SOURCE=.\server\sv_send.c
# End Source File
# Begin Source File

SOURCE=.\server\sv_user.c
# End Source File
# Begin Source File

SOURCE=.\server\sv_world.c
# End Source File
# Begin Source File

SOURCE=.\win32\sys_win.c
# End Source File
# Begin Source File

SOURCE=.\win32\vid_dll.c
# End Source File
# Begin Source File

SOURCE=.\win32\vid_menu.c
# End Source File
# Begin Source File

SOURCE=.\client\x86.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\user\anorms.h
# End Source File
# Begin Source File

SOURCE=.\Audio\AudioSoundDefs.h
# End Source File
# Begin Source File

SOURCE=.\client\cdaudio.h
# End Source File
# Begin Source File

SOURCE=.\client\client.h
# End Source File
# Begin Source File

SOURCE=.\qcommon\CMODEL.H
# End Source File
# Begin Source File

SOURCE=..\user\Collection\Col_Elements.h
# End Source File
# Begin Source File

SOURCE=..\user\Collection\Col_MFCDefs.h
# End Source File
# Begin Source File

SOURCE=..\user\Collection\CollectionClasses.h
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

SOURCE=..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=.\client\input.h
# End Source File
# Begin Source File

SOURCE=..\user\Collection\IString.h
# End Source File
# Begin Source File

SOURCE=.\client\keys.h
# End Source File
# Begin Source File

SOURCE=..\user\physics.h
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

SOURCE=.\Audio\Rsx.h
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
