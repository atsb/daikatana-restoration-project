# Microsoft Developer Studio Project File - Name="ref_gl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ref_gl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ref_gl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ref_gl.mak" CFG="ref_gl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ref_gl - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ref_gl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Daikatana/base/ref_gl", SXAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ref_gl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\ref_gl__"
# PROP BASE Intermediate_Dir ".\ref_gl__"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "..\client" /I "..\qcommon" /I "..\..\user" /I "..\ref_common" /D "DAIKATANA_OEM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
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
# ADD LINK32 winmm.lib glu32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonRelease.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib:"libcmtd.lib" /libpath:"..\..\dlls\world\release" /libpath:"..\..\libs"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\ref_gl__"
# PROP BASE Intermediate_Dir ".\ref_gl__"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Gi /GX /ZI /Od /I "..\client" /I "..\qcommon" /I "..\..\user" /I "..\ref_common" /D "DAIKATANA_OEM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c /Tp
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 winmm.lib wsock32.lib advapi32.lib kernel32.lib user32.lib gdi32.lib IonCommonDebug.lib imagehlp.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /nodefaultlib:"libc.lib" /def:".\ref_gl.def" /libpath:"..\..\dlls\world\debug" /libpath:"..\..\libs"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "ref_gl - Win32 Release"
# Name "ref_gl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\..\user\dk_huffman.cpp
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_common.cpp
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_font.cpp
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_init.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_beams.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_fogsurf.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_icons.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_image.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_inventory.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_light.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_model.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_particle.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_rmain.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_rmisc.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_rsurf.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_state.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_surfsprite.cpp
# End Source File
# Begin Source File

SOURCE=.\gl_warp.cpp
# End Source File
# Begin Source File

SOURCE=..\win32\glw_imp.cpp
# End Source File
# Begin Source File

SOURCE=.\hierarchy.cpp
# End Source File
# Begin Source File

SOURCE=..\win32\q_shwin.cpp
# End Source File
# Begin Source File

SOURCE=..\win32\qgl_win.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\anorms.h
# End Source File
# Begin Source File

SOURCE=..\..\user\anorms.h
# End Source File
# Begin Source File

SOURCE=.\anormtab.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\CMODEL.H
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_array.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_huffman.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_model_new.h
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_common.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=.\gl_beams.h
# End Source File
# Begin Source File

SOURCE=.\gl_local.h
# End Source File
# Begin Source File

SOURCE=.\gl_model.h
# End Source File
# Begin Source File

SOURCE=.\gl_protex.h
# End Source File
# Begin Source File

SOURCE=.\gl_surfsprite.h
# End Source File
# Begin Source File

SOURCE=..\win32\glw_win.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qcommon.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qfiles.h
# End Source File
# Begin Source File

SOURCE=.\qgl.h
# End Source File
# Begin Source File

SOURCE=..\client\ref.h
# End Source File
# Begin Source File

SOURCE=.\warpsin.h
# End Source File
# Begin Source File

SOURCE=..\win32\winquake.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ref_gl.def

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
