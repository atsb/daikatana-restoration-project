#=============================================================================
# IonCommon makefile.
#
# Created by Mike Danylchuk
#=============================================================================

# Daikatana directory. (Required by Makefile-header.)
DK_DIR	= ..

# Include global definitions.
include $(DK_DIR)/Makefile-header

#-----------------------------------------------------------------------------
# Directories
#-----------------------------------------------------------------------------

SRC_DIRS	= $(USER_DIR)	\
			= $(OS_DIR)		\

BUILD_DIR	= $(IONCOMMON_BUILD)

#-----------------------------------------------------------------------------
# Compiler/linker options
#-----------------------------------------------------------------------------

CXX			= $(STD_CXX)
CXXFLAGS	= $(STD_CXXFLAGS) -fPIC -I $(DK__DIR)
LDFLAGS		= $(STD_LDFLAGS) $(SHARED) $(SONAME)$(IONCOMMON_SONAME) 
LIBS		= $(STD_LIBS)

#-----------------------------------------------------------------------------
# Files
#-----------------------------------------------------------------------------

OBJS = \
	collect.o		\
	dk_matrix.o		\
	dk_point.o		\
	dk_shared.o		\
	dk_std.o		\
	dk_system.o		\
	map_ip.o		\
	map_sp.o		\
	List_p.o		\
	log_unix.o		\
	udpclientbsd.o	\
	filebuffer.o	\
	Lzari.o			\
	md5c.o			\
	dk_io.o			\
	csv.o			\

OUT = $(IONCOMMON)

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------

# Use general rules.
include $(DK_DIR)/Makefile-general

#-----------------------------------------------------------------------------
# End
#-----------------------------------------------------------------------------
