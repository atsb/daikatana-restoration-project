#=============================================================================
# Daikatana Makefile.
#
# Created by Mike Danylchuk
#=============================================================================

# Daikatana directory. (Required by Makefile-header.)
DK_DIR	= .

# Include global definitions.
include $(DK_DIR)/Makefile-header

# No default sub-make arguments.
ARGS =

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------

.PHONY : all
all : dkded IonCommon libent physics world weapons

.PHONY : dkded
dkded : IonCommon
	@$(MAKE) $(ARGS) --directory $(BASE_DIR)

.PHONY : IonCommon
IonCommon : 
	@$(MAKE) $(ARGS) --directory $(IONCOMMON_DIR)

.PHONY : libent
libent : 
	@$(MAKE) $(ARGS) --directory $(LIBENT_DIR)

.PHONY : physics
physics : IonCommon libent
	@$(MAKE) $(ARGS) --directory $(PHYSICS_DIR)

.PHONY : world
world : IonCommon
	@$(MAKE) $(ARGS) --directory $(WORLD_DIR)

.PHONY : weapons
weapons : IonCommon world
	@$(MAKE) $(ARGS) --directory $(WEAPONS_DIR)

#-----------------------------------------------------------------------------
# Maintenance
#-----------------------------------------------------------------------------

.PHONY : tags
tags : 
	etags $(ALL_TAGS)

# Pass custom targets down to individual makefiles.
.DEFAULT :  
	@$(MAKE) ARGS=$@ --no-print-directory

#-----------------------------------------------------------------------------
# End
#-----------------------------------------------------------------------------
