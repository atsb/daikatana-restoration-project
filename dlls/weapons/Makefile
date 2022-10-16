#=============================================================================
# Daikatana weapons makefile.
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

SRC_DIRS	= $(WEAPONS_DIR)
BUILD_DIR	= $(WEAPONS_BUILD)

#-----------------------------------------------------------------------------
# Compiler/linker options
#-----------------------------------------------------------------------------

CXX			= $(STD_CXX)
CXXFLAGS	= $(STD_CXXFLAGS) -fPIC -w -I$(USER_DIR) -I$(WORLD_DIR) \
			-I$(QCOMMON_DIR) -I$(LIBENT_DIR) -I$(OS_DIR)
LDFLAGS		= $(STD_LDFLAGS) $(RPATH)./dlls -Wl,-Bsymbolic $(SHARED) \
			$(SONAME)$(WEAPONS_SONAME)
LIBS		= $(STD_LIBS) $(IONCOMMON) $(WORLD)

#-----------------------------------------------------------------------------
# Files
#-----------------------------------------------------------------------------

OBJS =				\
	ballista.o		\
	bolter.o		\
	C4.o			\
	celestril.o		\
	daikatana.o		\
	discus.o		\
	disruptor.o		\
	dk_banish.o		\
	dk_barrier.o	\
	dk_dance.o		\
	dk_melee.o		\
	dk_posession.o	\
	dk_proteus.o	\
	flashlight.o	\
	glock.o			\
	hammer.o		\
	ionblaster.o	\
	kineticore.o	\
	Main.o			\
	metamaser.o		\
	midas.o			\
	nightmare.o		\
	novabeam.o		\
	ripgun.o		\
	shockwave.o		\
	shotcycler.o	\
	sidewinder.o	\
	silverclaw.o	\
	slugger.o		\
	stavros.o		\
	sunflare.o		\
	swinging_hook.o	\
	tazerhook.o		\
	testweapon.o	\
	trident.o		\
	venomous.o		\
	w_attribute.o	\
	weapon_funcs.o	\
	wyndrax.o		\
	zeus.o			\

OUT = $(WEAPONS)

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------

# Use general rules.
include $(DK_DIR)/Makefile-general

#-----------------------------------------------------------------------------
# End
#-----------------------------------------------------------------------------
