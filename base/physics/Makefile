#=============================================================================
# Daikatana physics makefile.
#
# Created by Mike Danylchuk
#=============================================================================

# Daikatana directory. (Required by Makefile-header.)
DK_DIR	= ../..

# Include global definitions.
include $(DK_DIR)/Makefile-header

#-----------------------------------------------------------------------------
# Directories
#-----------------------------------------------------------------------------

SRC_DIRS	= $(PHYSICS_DIR)	\
			= $(OS_DIR)			\

BUILD_DIR	= $(PHYSICS_BUILD)

#-----------------------------------------------------------------------------
# Compiler/linker options
#-----------------------------------------------------------------------------

CXX			= $(STD_CXX)
CXXFLAGS	= $(STD_CXXFLAGS) -fPIC -w -I$(USER_DIR) -I$(LIBENT_DIR) \
			-I$(OS_DIR)
LDFLAGS		= $(STD_LDFLAGS) $(RPATH)./dlls $(SHARED) \
			$(SONAME)$(PHYSICS_SONAME)
LIBS		= $(STD_LIBS) $(IONCOMMON) $(LIBENT)

#-----------------------------------------------------------------------------
# Files
#-----------------------------------------------------------------------------

OBJS =					\
	com_friction.o		\
	com_heap.o			\
	com_list.o			\
	com_main.o			\
	com_sub.o			\
	com_teamplay.o		\
	com_weapons.o		\
	dk_ServerState.o	\
	dk_sent.o			\
	dk_so.o				\
	p_client.o			\
	p_concmds.o			\
	p_float.o			\
	p_inventory.o		\
	p_main.o			\
	p_phys.o			\
	p_save.o			\
	p_stub.o			\
	p_utils.o			\

OUT = $(PHYSICS)

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------

# Use general rules.
include $(DK_DIR)/Makefile-general

#-----------------------------------------------------------------------------
# End
#-----------------------------------------------------------------------------
