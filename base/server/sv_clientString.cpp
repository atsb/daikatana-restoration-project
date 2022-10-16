#include    <windows.h>
#include	"client.h"
#include	"l__language.h"
#include	"server.h"

static sizebuf_t	client_string_buf;
static byte			client_string_message_buf[MAX_MSGLEN-16];
static bool			bNoContinueSent = false;

void CS_WriteByte (int c)		{MSG_WriteByte(&client_string_buf,c);}
void CS_WriteLong (int c)		{MSG_WriteLong(&client_string_buf,c);}
void CS_WriteFloat (float f)	{MSG_WriteFloat (&client_string_buf, f);}
void CS_WriteString (char *s)	{MSG_WriteString (&client_string_buf, s);}

char *CS_StringForResourceID(long str_list_index)
{
	long str_list = (str_list_index & 0xffff0000) >> 16;
	long str_index = str_list_index & 0x0000ffff;

	switch(str_list)
	{
	case 1:
		return tongue[str_index];

	case 2:
		return tongue_weapons[str_index];

	case 3:
		return tongue_armors[str_index];

	case 4:
		return tongue_world[str_index];

	case 5:
		return tongue_deathmsg_daikatana[str_index];

	case 6:
		return tongue_deathmsg_self[str_index];

	case 7:
		return tongue_deathmsg_weapon1[str_index];

	case 8:
		return tongue_deathmsg_weapon2[str_index];

	case 9:
		return tongue_deathmsg_weapon3[str_index];

	case 10:
		return tongue_deathmsg_weapon4[str_index];

	case 11:
		return tongue_ctf[str_index];

	case 12:
		return tongue_deathtag[str_index];

	default:
		Com_Error (ERR_DROP,"CL_ParseClientString: Bad string resource ID");
		break;
	}
	return "";
}

// call to stert sending the string
void CS_BeginSendString()
{
	// clear the buffer for the new message
	SZ_Clear(&client_string_buf);

	bNoContinueSent = false;	
	
	// start sending the message data
	CS_WriteByte(svc_client_string);
}

// call to end sending.  call after using unicast to send the message...
void CS_EndSendString()
{
	// we're done.  clear the buffer
	SZ_Clear(&client_string_buf);
}

// continue processing more parameters, using the previous results as the format string
void CS_Continue(bool bContinue)
{
	if (!bContinue)
		bNoContinueSent = true;

	CS_WriteByte(bContinue ? 1 : 0);
}

// send the parameter count for the next pass
void CS_SendCount(byte count)
{
	CS_WriteByte(count);
}

// functions to send parameters--------------------------------------------------
void CS_SendStringID(long strID)
{
	CS_WriteByte(CS_data_str_resource);
	CS_WriteLong(strID);
}

void CS_SendString(char *str)
{
	CS_WriteByte(CS_data_str_literal);
	CS_WriteString(str);
}

void CS_SendInteger(int val)
{
	CS_WriteByte(CS_data_integer);
	CS_WriteLong(val);
}

void CS_SendFloat(float val)
{
	CS_WriteByte(CS_data_float);
	CS_WriteFloat(val);
}
//-------------------------------------------------------------------------------

// funcations to send the format specifier string and parameter count------------
void CS_SendSpecifierStr(char *str, byte parmCount)
{
	CS_SendString(str);
	CS_SendCount(parmCount);
}

void CS_SendSpecifierID(long strID, byte parmCount)
{
	CS_SendStringID(strID);
	CS_SendCount(parmCount);
}
//-------------------------------------------------------------------------------

// functions for sending the message to clients----------------------------------

// removes 'length' bytes from the buffer
void *SZ_DeleteSpace (sizebuf_t *buf, int length)
{
	void	*data;
	
	// removing too much?
	if (buf->cursize < length)
	{
			_ASSERTE(FALSE);
			Com_Error (ERR_FATAL, "SZ_DeleteSpace: length larger than buffer"); 
	}

	buf->cursize -= length;
	data = buf->data + buf->cursize;
	
	return data;
}

// does NOT clear the buffer after calling
void CS_Unicast(edict_t *ent, byte print_type, float msg_time = 0.0f)
{
	if (!bNoContinueSent)
		CS_Continue(false);

	// specify how this client is to display this string
	CS_WriteByte(print_type);
	if (print_type == CS_print_center)
		CS_WriteFloat(msg_time);

	int		p;
	client_t	*client;
	if (ent)
	{
		p = NUM_FOR_EDICT(ent);
		if (p >= 1 && p <= maxclients->value)
		{
			client = svs.clients + (p-1);
			if (client->state != cs_free && client->state != cs_zombie)
			{
				SZ_Write (&client->netchan.message, client_string_message_buf, client_string_buf.cursize);
			}
		}
	}

	// pop the print specifier back off the buffer
	SZ_DeleteSpace(&client_string_buf,1);
	if (print_type == CS_print_center)
		SZ_DeleteSpace(&client_string_buf,4);
}

// acts as an endSendString after sending the message to all clients
void CS_Multicast(byte print_type, float msg_time = 0.0f)
{
	if (!bNoContinueSent)
		CS_Continue(false);

	// specify how this string should be displayed
	CS_WriteByte(print_type);
	if (print_type == CS_print_center)
		CS_WriteFloat(msg_time);

	client_t	*client;
	int j;
	// send the data to all relevent clients
	for (j = 0, client = svs.clients; j < maxclients->value; j++, client++)
	{
		if (client->state == cs_free || client->state == cs_zombie)
			continue;

		SZ_Write (&client->netchan.message, client_string_message_buf, client_string_buf.cursize);
	}

	// pop the print specifier back off the buffer
	SZ_DeleteSpace(&client_string_buf,1);
	if (print_type == CS_print_center)
		SZ_DeleteSpace(&client_string_buf,4);
}
//-------------------------------------------------------------------------------

void CS_Init(game_import_t &import)
{
	SZ_Init (&client_string_buf, client_string_message_buf, sizeof(client_string_message_buf));

	import.cs.SendStringID		= CS_SendStringID;
	import.cs.BeginSendString	= CS_BeginSendString;
	import.cs.EndSendString		= CS_EndSendString;

	import.cs.SendFloat			= CS_SendFloat;
	import.cs.SendInteger		= CS_SendInteger;
	import.cs.SendString		= CS_SendString;
	import.cs.SendStringID		= CS_SendStringID;

	import.cs.SendSpecifierStr	= CS_SendSpecifierStr;
	import.cs.SendSpecifierID	= CS_SendSpecifierID;		
	import.cs.SendSpecifierID	= CS_SendSpecifierID;
	import.cs.SendCount			= CS_SendCount;

	import.cs.Continue			= CS_Continue;

	import.cs.Multicast			= CS_Multicast;
	import.cs.Unicast			= CS_Unicast;


	import.cs.StringForID		= CS_StringForResourceID;
}

