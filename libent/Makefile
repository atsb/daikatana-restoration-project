#=============================================================================
# Libent makefile.
#
# Created by Mike Danylchuk
#=============================================================================

# Daikatana directory. (Required by Makefile-header.)
DK_DIR  = ..

# Include global definitions.
include $(DK_DIR)/Makefile-header

#-----------------------------------------------------------------------------
# Directories
#-----------------------------------------------------------------------------

SRC_DIRS    =               \
            $(LIBENT_DIR)   \
            $(USER_DIR)     \

BUILD_DIR   = $(LIBENT_BUILD)

#-----------------------------------------------------------------------------
# Compiler/linker options
#-----------------------------------------------------------------------------

CXX         = $(STD_CXX)
CXXFLAGS    = $(STD_CXXFLAGS) -fPIC -w -I$(USER_DIR) -I$(QCOMMON_DIR) \
            -I$(CLIENT_DIR)
LDFLAGS     = $(STD_LDFLAGS) $(RPATH)./dlls $(SHARED) \
            $(SONAME)$(LIBENT_SONAME)
LIBS        = $(STD_LIBS)

#-----------------------------------------------------------------------------
# Files
#-----------------------------------------------------------------------------

OBJS =                      \
    CEPhysics.o             \
    ClientEntity.o          \
    ClientEntityManager.o   \
    dk_system.o             \
    FadeEnt.o		    	\
    HUD.o		    		\
	HUDChar.o				\
	HUDCSpeeds.o			\
	HUDGraphic.o			\
    IonBlast.o              \
    Particle.o              \
    Particle_IonTrail.o     \
    ParticleFX.o            \
    ParticleFX_IonSpray.o   \
    ParticleFX_Smoke.o      \
    Projectile.o            \
    Rocket.o                \
    SoundFX.o               \
    TrackFlare.o            \
    TrackingEntity.o        \
    TrackLight.o            \
	TrackLightBlinking.o \

OUT = $(LIBENT)

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------

# Use general rules.
include $(DK_DIR)/Makefile-general

#-----------------------------------------------------------------------------
# End
#-----------------------------------------------------------------------------

