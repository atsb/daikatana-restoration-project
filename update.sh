#!/bin/sh

#######################################################################
# Updates binaries for UN*X compilations of the game
# Assumes that the game directory is /usr/local/games/katana/daikatana
#
# On my system, that directory is an SMB mount from hadron/kata
echo "Updating daikatana runtime"
DK_DIR=/usr/local/games/katana/daikatana
SRC_DIR=bin/debug-i386
echo "Daikatana runtime dir: $DK_DIR"
echo "Daikatana build dir:   $SRC_DIR"
echo " "

echo "Dedicated server executable: dkded"
cp $SRC_DIR/dkded $DK_DIR/dkded

echo "Common routines lib:         ioncommon.so"
cp $SRC_DIR/dlls/ioncommon.so $DK_DIR/dlls/ioncommon.so

echo "Common entity lib:           libent.so"
cp $SRC_DIR/dlls/libent.so $DK_DIR/dlls/libent.so

echo "Server-side physics lib:     physics.so"
cp $SRC_DIR/dlls/physics.so $DK_DIR/dlls/physics.so

echo "Weapons lib:                 weapons.so"
cp $SRC_DIR/dlls/weapons.so $DK_DIR/dlls/weapons.so

echo "AI, player and world lib:    world.so"
cp $SRC_DIR/dlls/world.so $DK_DIR/dlls/world.so

echo "Update complete"

