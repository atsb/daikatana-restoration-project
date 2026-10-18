#ifndef DAIKATANA_CLIENT_STRING_H
#define DAIKATANA_CLIENT_STRING_H

// Resource-list identifiers used by the Daikatana 1.2 client-language
// message protocol. The low 16 bits contain the string index.
#define CS_RES_TONGUE                 1
#define CS_RES_WEAPONS                2
#define CS_RES_ARMORS                 3
#define CS_RES_WORLD                  4
#define CS_RES_DEATHMSG_DAIKATANA     5
#define CS_RES_DEATHMSG_SELF          6
#define CS_RES_DEATHMSG_WEAPON1       7
#define CS_RES_DEATHMSG_WEAPON2       8
#define CS_RES_DEATHMSG_WEAPON3       9
#define CS_RES_DEATHMSG_WEAPON4       10
#define CS_RES_CTF                    11
#define CS_RES_DEATHTAG               12

#define CS_MAKE_RESOURCE_ID(list_id, index) \
    ((long)((((unsigned long)(list_id)) << 16) | ((unsigned long)(index) & 0xffffUL)))

// Compatibility aliases used by the original 1.2 source.
#define CS_RES_tongue                 CS_RES_TONGUE
#define CS_RES_tongue_weapons         CS_RES_WEAPONS
#define CS_RES_tongue_armors          CS_RES_ARMORS
#define CS_RES_tongue_world           CS_RES_WORLD
#define CS_RES_tongue_deathmsg_daikatana CS_RES_DEATHMSG_DAIKATANA
#define CS_RES_tongue_deathmsg_self   CS_RES_DEATHMSG_SELF
#define CS_RES_tongue_deathmsg_weapon1 CS_RES_DEATHMSG_WEAPON1
#define CS_RES_tongue_deathmsg_weapon2 CS_RES_DEATHMSG_WEAPON2
#define CS_RES_tongue_deathmsg_weapon3 CS_RES_DEATHMSG_WEAPON3
#define CS_RES_tongue_deathmsg_weapon4 CS_RES_DEATHMSG_WEAPON4
#define CS_RES_tongue_ctf              CS_RES_CTF
#define CS_RES_tongue_deathtag         CS_RES_DEATHTAG

#define RESOURCE_ID_FOR_STRING(list, index) \
    CS_MAKE_RESOURCE_ID(CS_RES_##list, index)

#endif
