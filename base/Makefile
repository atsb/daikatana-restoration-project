#=============================================================================
# Daikatana dedicated server makefile.
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

SRC_DIRS =			\
	$(BASE_DIR)		\
	$(DK__DIR)		\
	$(NULL_DIR)		\
	$(PHYSICS_DIR)	\
	$(QCOMMON_DIR)	\
	$(SERVER_DIR)	\
	$(USER_DIR)		\
	$(OS_DIR)

BUILD_DIR = $(BASE_BUILD)

#-----------------------------------------------------------------------------
# Compiler/linker options
#-----------------------------------------------------------------------------

CXX			= $(STD_CXX)
CXXFLAGS	= $(STD_CXXFLAGS) -w -I$(BASE_DIR) -I$(DK__DIR) -I$(NULL_DIR) \
			-I$(QCOMMON_DIR) -I$(SERVER_DIR) -I$(USER_DIR) \
			-I$(BASE_DIR)/client
LDFLAGS		= $(STD_LDFLAGS) $(RPATH)./$(DLLS_SUBDIR)
LIBS		= $(STD_LIBS) $(IONCOMMON)

#-----------------------------------------------------------------------------
# Files
#-----------------------------------------------------------------------------

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
OS_OBJS =				\
	q_shlinux.o			\
	sys_linux.o
endif

ifeq ($(UNAME_S),,Solaris)
OS_OBJS =				\
	q_shsolaris.o			\
	sys_solaris.o
endif

OBJS =					\
	dk_log.o			\
	dk_model.o			\
	dk_unix.o			\
	cd_null.o			\
	cl_null.o			\
	cmd.o				\
	cmodel_dynamic.o		\
	common.o			\
	cvar.o				\
	dk_screen_coord.o		\
	files.o				\
	glob.o				\
	key_null.o			\
	md4.o				\
	memmgr.o			\
	misc_null.o			\
	net_bsd.o			\
	net_chan.o			\
	pmove.o				\
	sv_ccmds.o			\
	sv_ents.o			\
	sv_game.o			\
	sv_init.o			\
	sv_main.o			\
	sv_send.o			\
	sv_user.o			\
	sv_world.o			\
	$(OS_OBJS)

OUT = $(DKDED)

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------

# Use general rules.
include $(DK_DIR)/Makefile-general

#-----------------------------------------------------------------------------
# End
#-----------------------------------------------------------------------------
