#=============================================================================
# Daikatana world makefile.
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

SRC_DIRS	= 		\
	$(WORLD_DIR)	\
	$(USER_DIR)		\
	$(DK_DIR)/base/linux	\


BUILD_DIR	= $(WORLD_BUILD)

#-----------------------------------------------------------------------------
# Compiler/linker options
#-----------------------------------------------------------------------------

CXX			= $(STD_CXX)
CXXFLAGS	= $(STD_CXXFLAGS) -fPIC -w -I$(WORLD_DIR) -I$(USER_DIR) \
			-I$(DK__DIR) -I$(LIBENT_DIR) -I$(OS_DIR)
LDFLAGS		= $(STD_LDFLAGS) $(SHARED) $(SONAME)$(WORLD_SONAME)
LIBS		= $(STD_LIBS) $(IONCOMMON)

#-----------------------------------------------------------------------------
# Files
#-----------------------------------------------------------------------------

OBJS =					\
	AI_File.o			\
	GrphPath.o			\
	MonsterSound.o		\
	SequenceMap.o		\
	action.o			\
	actorlist.o			\
	ai.o				\
	ai_aim.o			\
	ai_frames.o			\
	ai_func.o			\
	ai_group.o			\
	ai_move.o			\
	ai_test.o			\
	ai_utils.o			\
	ai_weapons.o		\
	artifacts.o			\
	battleboar.o		\
	blackpris.o			\
	Bodylist.o			\
	bot.o				\
	buboid.o			\
	cambot.o			\
	centurion.o			\
	cerberus.o			\
	chaingang.o			\
	chasecam.o			\
	cin_playback.o		\
	client.o			\
	client_init.o		\
	column.o			\
	cryotech.o			\
	cyclops.o			\
	deathsphere.o		\
	doombat.o			\
	door.o				\
	dragon.o			\
	drop_monster.o		\
	dwarf.o				\
	effects.o			\
	entity_pointer.o	\
	femgang.o			\
	ferryman.o			\
	fatworker.o			\
	fletcher.o			\
	floater.o			\
	froginator.o		\
	func_anim.o			\
	func_monitor.o		\
	func_various.o		\
	garroth.o			\
	gib.o				\
	goldfish.o			\
	griffon.o			\
	harpy.o				\
	hiro.o				\
	Inmater.o			\
	items.o				\
	kage.o				\
	kminos.o			\
	knight1.o			\
	knight2.o			\
	labmonkey.o			\
	lasergat.o			\
	light.o				\
	lycanthir.o			\
	medusa.o			\
	memory.o			\
	minotaur.o			\
	misc.o				\
	mishimaguard.o			\
	monsterpaths.o		\
	NavTest.o			\
	nodelist.o			\
	npc.o				\
	octree.o			\
	prat.o				\
	priest.o			\
	prisoner.o			\
	protopod.o			\
	psyclaw.o			\
	ragemaster.o		\
	rocketgang.o		\
	rocketmp.o			\
	rotworm.o			\
	satyr.o				\
	scomndo.o			\
	sdiver.o			\
	sealcap.o			\
	sgirl.o				\
	shark.o				\
	skinnyworker.o			\
	skeeter.o			\
	skeleton.o			\
	skinnyworker.o		\
	sludgeminion.o		\
	smallspider.o		\
	spawn.o			\
	spider.o			\
	stavros.o			\
	target.o			\
	thief.o				\
	thinkFuncs.o		\
	thunderskeet.o		\
	trackattack.o		\
	transition.o		\
	triggers.o			\
	uzigang.o			\
	vermin.o			\
	viewthing.o			\
	whitepris.o			\
	wizard.o			\
	World.o				\
	wyndrax.o			\

OUT = $(WORLD)

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------

# Use general rules.
include $(DK_DIR)/Makefile-general

#-----------------------------------------------------------------------------
# End
#-----------------------------------------------------------------------------
