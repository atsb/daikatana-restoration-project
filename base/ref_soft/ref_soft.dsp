# Microsoft Developer Studio Project File - Name="ref_soft" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ref_soft - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ref_soft.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ref_soft.mak" CFG="ref_soft - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ref_soft - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ref_soft - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/Daikatana/base/ref_soft", MYAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\ref_soft"
# PROP BASE Intermediate_Dir ".\ref_soft"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /I "..\client" /I "..\qcommon" /I "..\..\user" /I "..\ref_common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 winmm.lib advapi32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /map /machine:I386 /libpath:"..\..\dlls\world\release"
# SUBTRACT LINK32 /incremental:yes /debug

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\ref_soft"
# PROP BASE Intermediate_Dir ".\ref_soft"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /Zi /Od /I "..\client" /I "..\qcommon" /I "..\..\user" /I "..\ref_common" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 winmm.lib advapi32.lib kernel32.lib user32.lib gdi32.lib ..\..\libs\ioncommon.lib /nologo /subsystem:windows /dll /profile /debug /machine:I386 /nodefaultlib:"libc" /libpath:"..\..\dlls\world\debug"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "ref_soft - Win32 Release"
# Name "ref_soft - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\ref_common\dk_ref_common.cpp
DEP_CPP_DK_RE=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	"..\ref_common\dk_ref_common.h"\
	
NODEP_CPP_DK_RE=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_font.cpp
DEP_CPP_DK_REF=\
	"..\..\user\dk_array.h"\
	"..\..\user\dk_buffer.h"\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	"..\ref_common\dk_ref_common.h"\
	
NODEP_CPP_DK_REF=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_misc.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_init.cpp

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\user\dk_shared.cpp
DEP_CPP_DK_SH=\
	"..\..\user\dk_shared.h"\
	
NODEP_CPP_DK_SH=\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hierarchy.cpp
DEP_CPP_HIERA=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_HIERA=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\q_shwin.cpp
DEP_CPP_Q_SHW=\
	"..\..\user\dk_shared.h"\
	"..\qcommon\qcommon.h"\
	"..\win32\winquake.h"\
	
NODEP_CPP_Q_SHW=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_aclip.cpp
DEP_CPP_R_ACL=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_ACL=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_aclipa.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_aclipa.asm
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_aclipa.asm
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_alias.cpp
DEP_CPP_R_ALI=\
	"..\..\user\anorms.h"\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	"..\ref_common\alias_quant.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_ALI=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_beams.cpp
DEP_CPP_R_BEA=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_beams.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_BEA=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_bsp.cpp
DEP_CPP_R_BSP=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_BSP=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_draw.cpp
DEP_CPP_R_DRA=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	"..\ref_common\dk_ref_common.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_DRA=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_draw16.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_draw16.asm
InputName=r_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_draw16.asm
InputName=r_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_draw16f.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_draw16f.asm
InputName=r_draw16f

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_draw16f.asm
InputName=r_draw16f

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_draw8.cpp

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_drawa.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_drawa.asm
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_drawa.asm
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_drawstretch.cpp
DEP_CPP_R_DRAW=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	"..\ref_common\dk_ref_common.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_DRAW=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_edge.cpp
DEP_CPP_R_EDG=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_EDG=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_edgea.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_edgea.asm
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_edgea.asm
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_image.cpp
DEP_CPP_R_IMA=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	"..\ref_common\dk_ref_common.h"\
	".\r_local.h"\
	".\r_model.h"\
	".\r_protex.h"\
	
NODEP_CPP_R_IMA=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_inventory.cpp
DEP_CPP_R_INV=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	".\r_protex.h"\
	
NODEP_CPP_R_INV=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_light.cpp
DEP_CPP_R_LIG=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_LIG=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_main.cpp
DEP_CPP_R_MAI=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	"..\ref_common\dk_ref_common.h"\
	".\r_beams.h"\
	".\r_local.h"\
	".\r_model.h"\
	".\r_protex.h"\
	
NODEP_CPP_R_MAI=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_misc.cpp
DEP_CPP_R_MIS=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_MIS=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_model.cpp
DEP_CPP_R_MOD=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_MOD=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_part.cpp
DEP_CPP_R_PAR=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_PAR=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_poly.cpp
DEP_CPP_R_POL=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_POL=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_polyf.cpp
DEP_CPP_R_POLY=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_POLY=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_polysa.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_polysa.asm
InputName=r_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_polysa.asm
InputName=r_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_polysa33.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_polysa33.asm
InputName=r_polysa33

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_polysa33.asm
InputName=r_polysa33

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_polysa33f.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_polysa33f.asm
InputName=r_polysa33f

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_polysa33f.asm
InputName=r_polysa33f

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_polysa66.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_polysa66.asm
InputName=r_polysa66

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_polysa66.asm
InputName=r_polysa66

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_polysa66f.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_polysa66f.asm
InputName=r_polysa66f

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_polysa66f.asm
InputName=r_polysa66f

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_polysaf.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_polysaf.asm
InputName=r_polysaf

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_polysaf.asm
InputName=r_polysaf

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_polyse.cpp
DEP_CPP_R_POLYS=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\adivtab.h"\
	".\r_local.h"\
	".\r_model.h"\
	".\rand1k.h"\
	
NODEP_CPP_R_POLYS=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_protex.cpp
DEP_CPP_R_PRO=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	".\r_protex.h"\
	
NODEP_CPP_R_PRO=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_rast.cpp
DEP_CPP_R_RAS=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_RAS=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_scan.cpp
DEP_CPP_R_SCA=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_SCA=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_scana.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_scana.asm
InputName=r_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_scana.asm
InputName=r_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_sprite.cpp
DEP_CPP_R_SPR=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	".\r_protex.h"\
	
NODEP_CPP_R_SPR=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_surf.cpp
DEP_CPP_R_SUR=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_R_SUR=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_surf8.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_surf8.asm
InputName=r_surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_surf8.asm
InputName=r_surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_surfsprite.cpp
DEP_CPP_R_SURF=\
	"..\..\user\dk_array.h"\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	".\r_local.h"\
	".\r_model.h"\
	".\r_surfSprite.h"\
	
NODEP_CPP_R_SURF=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_misc.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_varsa.asm

!IF  "$(CFG)" == "ref_soft - Win32 Release"

# Begin Custom Build
OutDir=.\..\Release
InputPath=.\r_varsa.asm
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

# Begin Custom Build
OutDir=.\..\Debug
InputPath=.\r_varsa.asm
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ref_soft.def
# End Source File
# Begin Source File

SOURCE=..\win32\rw_ddraw.cpp
DEP_CPP_RW_DD=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	"..\win32\rw_win.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_RW_DD=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\rw_imp.cpp
DEP_CPP_RW_IM=\
	"..\..\user\dk_shared.h"\
	"..\client\ref.h"\
	"..\qcommon\CMODEL.H"\
	"..\qcommon\qcommon.h"\
	"..\win32\rw_win.h"\
	"..\win32\winquake.h"\
	".\r_local.h"\
	".\r_model.h"\
	
NODEP_CPP_RW_IM=\
	"..\qcommon\qfiles.h"\
	".\k_defines.h"\
	".\k_matrix.h"\
	".\k_point.h"\
	".\ogfile.h"\
	

!IF  "$(CFG)" == "ref_soft - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_soft - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\adivtab.h
# End Source File
# Begin Source File

SOURCE=..\..\user\anorms.h
# End Source File
# Begin Source File

SOURCE=..\client\asm_i386.h
# End Source File
# Begin Source File

SOURCE=..\ref_common\dk_ref_common.h
# End Source File
# Begin Source File

SOURCE=..\..\user\dk_shared.h
# End Source File
# Begin Source File

SOURCE=.\qasm.inc
# End Source File
# Begin Source File

SOURCE=..\qcommon\qcommon.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qfiles.h
# End Source File
# Begin Source File

SOURCE=.\r_beams.h
# End Source File
# Begin Source File

SOURCE=.\r_bmp.h
# End Source File
# Begin Source File

SOURCE=.\r_local.h
# End Source File
# Begin Source File

SOURCE=.\r_model.h
# End Source File
# Begin Source File

SOURCE=.\r_protex.h
# End Source File
# Begin Source File

SOURCE=.\rand1k.h
# End Source File
# Begin Source File

SOURCE=..\client\ref.h
# End Source File
# Begin Source File

SOURCE=..\win32\rw_win.h
# End Source File
# Begin Source File

SOURCE=..\win32\winquake.h
# End Source File
# End Group
# End Target
# End Project
