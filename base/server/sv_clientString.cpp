#include "client.h"
#include "l__language.h"
#include "server.h"
#include "client_string.h"

static sizebuf_t client_string_buf;
static byte client_string_message_buf[MAX_MSGLEN - 16];
static qboolean bNoContinueSent = FALSE;

static void CS_WriteByte(int c) { MSG_WriteByte(&client_string_buf, c); }
static void CS_WriteLong(int c) { MSG_WriteLong(&client_string_buf, c); }
static void CS_WriteFloat(float f) { MSG_WriteFloat(&client_string_buf, f); }
static void CS_WriteString(char *s) { MSG_WriteString(&client_string_buf, s); }

static char *CS_StringForResourceID(long str_list_index)
{
    unsigned long id = (unsigned long)str_list_index;
    int list = (int)((id >> 16) & 0xffffUL);
    int index = (int)(id & 0xffffUL);

#define CS_ARRAY_ITEM(array) \
    ((index >= 0 && index < (int)(sizeof(array) / sizeof((array)[0]))) ? (array)[index] : NULL)

    switch (list)
    {
    case CS_RES_TONGUE:             return CS_ARRAY_ITEM(tongue);
    case CS_RES_WEAPONS:            return CS_ARRAY_ITEM(tongue_weapons);
    case CS_RES_ARMORS:             return CS_ARRAY_ITEM(tongue_armors);
    case CS_RES_WORLD:              return CS_ARRAY_ITEM(tongue_world);
    case CS_RES_DEATHMSG_DAIKATANA: return CS_ARRAY_ITEM(tongue_deathmsg_daikatana);
    case CS_RES_DEATHMSG_SELF:      return CS_ARRAY_ITEM(tongue_deathmsg_self);
    case CS_RES_DEATHMSG_WEAPON1:   return CS_ARRAY_ITEM(tongue_deathmsg_weapon1);
    case CS_RES_DEATHMSG_WEAPON2:   return CS_ARRAY_ITEM(tongue_deathmsg_weapon2);
    case CS_RES_DEATHMSG_WEAPON3:   return CS_ARRAY_ITEM(tongue_deathmsg_weapon3);
    case CS_RES_DEATHMSG_WEAPON4:   return CS_ARRAY_ITEM(tongue_deathmsg_weapon4);
    case CS_RES_CTF:                return CS_ARRAY_ITEM(tongue_ctf);
    case CS_RES_DEATHTAG:           return CS_ARRAY_ITEM(tongue_deathtag);
    default:
        Com_Error(ERR_DROP, "CS_StringForResourceID: bad string resource ID %ld", str_list_index);
        return "";
    }
#undef CS_ARRAY_ITEM
}

static void CS_BeginSendString()
{
    SZ_Clear(&client_string_buf);
    bNoContinueSent = FALSE;
    CS_WriteByte(svc_client_string);
}

static void CS_EndSendString()
{
    SZ_Clear(&client_string_buf);
}

static void CS_Continue(qboolean bContinue)
{
    if (!bContinue)
        bNoContinueSent = TRUE;
    CS_WriteByte(bContinue ? 1 : 0);
}

static void CS_SendCount(byte count) { CS_WriteByte(count); }
static void CS_SendStringID(long strID) { CS_WriteByte(CS_data_str_resource); CS_WriteLong((int)strID); }
static void CS_SendString(char *str) { CS_WriteByte(CS_data_str_literal); CS_WriteString(str); }
static void CS_SendInteger(int val) { CS_WriteByte(CS_data_integer); CS_WriteLong(val); }
static void CS_SendFloat(float val) { CS_WriteByte(CS_data_float); CS_WriteFloat(val); }

static void CS_SendSpecifierStr(char *str, byte parmCount)
{
    CS_SendString(str);
    CS_SendCount(parmCount);
}

static void CS_SendSpecifierID(long strID, byte parmCount)
{
    CS_SendStringID(strID);
    CS_SendCount(parmCount);
}

static void *CS_DeleteSpace(sizebuf_t *buf, int length)
{
    void *data;
    if (buf->cursize < length)
        Com_Error(ERR_FATAL, "CS_DeleteSpace: length larger than buffer");
    buf->cursize -= length;
    data = buf->data + buf->cursize;
    return data;
}

static void CS_Unicast(edict_t *ent, byte print_type, float msg_time)
{
    int p;
    client_t *client;

    if (!bNoContinueSent)
        CS_Continue(FALSE);

    CS_WriteByte(print_type);
    if (print_type == CS_print_center)
        CS_WriteFloat(msg_time);

    if (ent)
    {
        p = NUM_FOR_EDICT(ent);
        if (p >= 1 && p <= maxclients->value)
        {
            client = svs.clients + (p - 1);
            if (client->state != cs_free && client->state != cs_zombie)
                SZ_Write(&client->netchan.message, client_string_buf.data, client_string_buf.cursize);
        }
    }

    CS_DeleteSpace(&client_string_buf, 1);
    if (print_type == CS_print_center)
        CS_DeleteSpace(&client_string_buf, 4);
}

static void CS_Multicast(byte print_type, float msg_time)
{
    client_t *client;
    int j;

    if (!bNoContinueSent)
        CS_Continue(FALSE);

    CS_WriteByte(print_type);
    if (print_type == CS_print_center)
        CS_WriteFloat(msg_time);

    for (j = 0, client = svs.clients; j < maxclients->value; j++, client++)
    {
        if (client->state == cs_free || client->state == cs_zombie)
            continue;
        SZ_Write(&client->netchan.message, client_string_buf.data, client_string_buf.cursize);
    }

    CS_DeleteSpace(&client_string_buf, 1);
    if (print_type == CS_print_center)
        CS_DeleteSpace(&client_string_buf, 4);
}

void CS_Init(game_import_t &import)
{
    SZ_Init(&client_string_buf, client_string_message_buf, sizeof(client_string_message_buf));

    import.cs.BeginSendString = CS_BeginSendString;
    import.cs.EndSendString = CS_EndSendString;
    import.cs.SendStringID = CS_SendStringID;
    import.cs.SendString = CS_SendString;
    import.cs.SendInteger = CS_SendInteger;
    import.cs.SendFloat = CS_SendFloat;
    import.cs.SendSpecifierStr = CS_SendSpecifierStr;
    import.cs.SendSpecifierID = CS_SendSpecifierID;
    import.cs.SendCount = CS_SendCount;
    import.cs.Continue = CS_Continue;
    import.cs.Multicast = CS_Multicast;
    import.cs.Unicast = CS_Unicast;
    import.cs.StringForID = CS_StringForResourceID;
}
