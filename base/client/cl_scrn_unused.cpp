//===============================================================

/*
#define STAT_MINUS		10	// num frame for '-' stats digit
char		*sb_nums[2][11] = 
{
	{"num_0", "num_1", "num_2", "num_3", "num_4", "num_5",
	"num_6", "num_7", "num_8", "num_9", "num_minus"},
	{"anum_0", "anum_1", "anum_2", "anum_3", "anum_4", "anum_5",
	"anum_6", "anum_7", "anum_8", "anum_9", "anum_minus"}
};


#define	ICON_WIDTH	24
#define	ICON_HEIGHT	24
#define	CHAR_WIDTH	16
#define	ICON_SPACE	8




//================
//SizeHUDString
//
//Allow embedded \n in the string
//================
void SizeHUDString (char *string, int *w, int *h)
{
	int		lines, width, current;

	lines = 1;
	width = 0;

	current = 0;
	while (*string)
	{
		if (*string == '\n')
		{
			lines++;
			current = 0;
		}
		else
		{
			current++;
			if (current > width)
				width = current;
		}
		string++;
	}

	*w = width * 8;
	*h = lines * 8;
}



void DrawHUDString (char *string, int x, int y, int centerwidth, int xor)
{
	int		margin;
	char	line[1024];
	int		width;
	int		i;

	margin = x;

	while (*string)
	{
		// scan out one line of text from the string
		width = 0;
		while (*string && *string != '\n')
			line[width++] = *string++;
		line[width] = 0;

		if (centerwidth)
			x = margin + (centerwidth - width*8)/2;
		else
			x = margin;
		for (i=0 ; i<width ; i++)
		{
			re.DrawChar (x, y, line[i]^xor);
			x += 8;
		}
		if (*string)
		{
			string++;	// skip the \n
			x = margin;
			y += 8;
		}
	}
}

*/



/*
//==============
//SCR_DrawField
//==============

void SCR_DrawField (int x, int y, int color, int width, int value)
{
	char	num[16], *ptr;
	int		l;
	int		frame;

	if (width < 1)
		return;

	// draw number string
	if (width > 5)
		width = 5;

	SCR_AddDirtyPoint (x, y);
	SCR_AddDirtyPoint (x+width*CHAR_WIDTH+2, y+23);

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + CHAR_WIDTH*(width - l);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		DRAWSTRUCT drawStruct;
		drawStruct.pImage = re.RegisterPic( sb_nums[color][frame], NULL, NULL, RESOURCE_LEVEL );
		drawStruct.nFlags = DSFLAG_BLEND;
		drawStruct.nXPos = x;
		drawStruct.nYPos = y;
		re.DrawPic( drawStruct );

		x += CHAR_WIDTH;
		ptr++;
		l--;
	}
}
*/



/*
================
SCR_ExecuteLayoutString 

================


#pragma warning(disable: 4090)
void SCR_ExecuteLayoutString (const char *s)
{
	int		x, y;
	int		value;
	char	*token;
	int		width;
	int		index;
	clientinfo_t	*ci;

	if (cls.state != ca_active || !cl.refresh_prepped)
		return;

	if (!s[0])
		return;

	x = 0;
	y = 0;
	width = 3;

	while (s)
	{
		token = COM_Parse ((char **)&s);
		if (!strcmp(token, "xl"))
		{
			token = COM_Parse ((char **)&s);
			x = atoi(token);
			continue;
		}
		if (!strcmp(token, "xr"))
		{
			token = COM_Parse ((char **)&s);
			x = viddef.width + atoi(token);
			continue;
		}
		if (!strcmp(token, "xv"))
		{
			token = COM_Parse ((char **)&s);
			x = viddef.width/2 - 160 + atoi(token);
			continue;
		}

		if (!strcmp(token, "yt"))
		{
			token = COM_Parse ((char **)&s);
			y = atoi(token);
			continue;
		}
		if (!strcmp(token, "yb"))
		{
			token = COM_Parse ((char **)&s);
			y = viddef.height + atoi(token);
			continue;
		}
		if (!strcmp(token, "yv"))
		{
			token = COM_Parse ((char **)&s);
			y = viddef.height/2 - 120 + atoi(token);
			continue;
		}

		if (!strcmp(token, "pic"))
		{	// draw a pic from a stat number
			token = COM_Parse ((char **)&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			if (value >= MAX_IMAGES)
				Com_Error (ERR_DROP, "Pic >= MAX_IMAGES");
			if (cl.configstrings[CS_IMAGES+value])
			{
				SCR_AddDirtyPoint (x, y);
				SCR_AddDirtyPoint (x+23, y+23);
				re.DrawPic (x, y, cl.configstrings[CS_IMAGES+value], RESOURCE_LEVEL);
			}
			continue;
		}

		if (!strcmp(token, "client"))
		{	// draw a deathmatch client block
			int		score, ping, time;

			token = COM_Parse ((char **)&s);
			x = viddef.width/2 - 160 + atoi(token);
			token = COM_Parse ((char **)&s);
			y = viddef.height/2 - 120 + atoi(token);
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x+159, y+31);

			token = COM_Parse ((char **)&s);
			value = atoi(token);
			if (value >= MAX_CLIENTS || value < 0)
				Com_Error (ERR_DROP, "client >= MAX_CLIENTS");
			ci = &cl.clientinfo[value];

			token = COM_Parse ((char **)&s);
			score = atoi(token);

			token = COM_Parse ((char **)&s);
			ping = atoi(token);

			token = COM_Parse ((char **)&s);
			time = atoi(token);

			DrawAltString (x+32, y, ci->name);
			DrawString (x+32, y+8,  "Score: ");
			DrawAltString (x+32+7*8, y+8,  va("%i", score));
			DrawString (x+32, y+16, va("Ping:  %i", ping));
			DrawString (x+32, y+24, va("Time:  %i", time));

			if (!ci->icon)
				ci = &cl.baseclientinfo;
			re.DrawPic (x, y, ci->iconname, RESOURCE_LEVEL);
			continue;
		}

		if (!strcmp(token, "ctf"))
		{	// draw a ctf client block
			int		score, ping;
			char	block[80];

			token = COM_Parse ((char **)&s);
			x = viddef.width/2 - 160 + atoi(token);
			token = COM_Parse ((char **)&s);
			y = viddef.height/2 - 120 + atoi(token);
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x+159, y+31);

			token = COM_Parse ((char **)&s);
			value = atoi(token);
			if (value >= MAX_CLIENTS || value < 0)
				Com_Error (ERR_DROP, "client >= MAX_CLIENTS");
			ci = &cl.clientinfo[value];

			token = COM_Parse ((char **)&s);
			score = atoi(token);

			token = COM_Parse ((char **)&s);
			ping = atoi(token);
			if (ping > 999)
				ping = 999;

			sprintf(block, "%3d %3d %-12.12s", score, ping, ci->name);

			if (value == cl.playernum)
				DrawAltString (x, y, block);
			else
				DrawString (x, y, block);
			continue;
		}

		if (!strcmp(token, "picn"))
		{	// draw a pic from a name
			token = COM_Parse ((char **)&s);
			SCR_AddDirtyPoint (x, y);
			SCR_AddDirtyPoint (x+23, y+23);
			re.DrawPic (x, y, token, RESOURCE_LEVEL);
			continue;
		}

		if (!strcmp(token, "num"))
		{	// draw a number
			token = COM_Parse ((char **)&s);
			width = atoi(token);
			token = COM_Parse ((char **)&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			SCR_DrawField (x, y, 0, width, value);
			continue;
		}

		if (!strcmp(token, "hnum"))
		{	// health number
			int		color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_HEALTH];
			if (value > 25)
				color = 0;	// green
			else if (value > 0)
				color = (cl.frame.serverframe>>2) & 1;		// flash
			else
				color = 1;

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 1)
				re.DrawPic (x, y, "field_3", RESOURCE_LEVEL);

			SCR_DrawField (x, y, color, width, value);
			continue;
		}

		if (!strcmp(token, "anum"))
		{	// ammo number
			int		color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_AMMO];
			if (value > 5)
				color = 0;	// green
			else if (value >= 0)
				color = (cl.frame.serverframe>>2) & 1;		// flash
			else
				continue;	// negative number = don't show

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 4)
				re.DrawPic (x, y, "field_3", RESOURCE_LEVEL);

			SCR_DrawField (x, y, color, width, value);
			continue;
		}

		if (!strcmp(token, "rnum"))
		{	// armor number
			int		color;

			width = 3;
			value = cl.frame.playerstate.stats[STAT_ARMOR];
			if (value < 1)
				continue;

			color = 0;	// green

			if (cl.frame.playerstate.stats[STAT_FLASHES] & 2)
				re.DrawPic (x, y, "field_3", RESOURCE_LEVEL);

			SCR_DrawField (x, y, color, width, value);
			continue;
		}


		if (!strcmp(token, "stat_string"))
		{
			token = COM_Parse ((char **)&s);
			index = atoi(token);
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error (ERR_DROP, "Bad stat_string index");
			index = cl.frame.playerstate.stats[index];
			if (index < 0 || index >= MAX_CONFIGSTRINGS)
				Com_Error (ERR_DROP, "Bad stat_string index");
			DrawString (x, y, cl.configstrings[index]);
			continue;
		}

		if (!strcmp(token, "cstring"))
		{
			token = COM_Parse ((char **)&s);
			DrawHUDString (token, x, y, 320, 0);
			continue;
		}

		if (!strcmp(token, "string"))
		{
			token = COM_Parse ((char **)&s);
			DrawString (x, y, token);
			continue;
		}

		if (!strcmp(token, "cstring2"))
		{
			token = COM_Parse ((char **)&s);
			DrawHUDString (token, x, y, 320,0x80);
			continue;
		}

		if (!strcmp(token, "string2"))
		{
			token = COM_Parse ((char **)&s);
			DrawAltString (x, y, token);
			continue;
		}

		if (!strcmp(token, "if"))
		{	// draw a number
			token = COM_Parse ((char **)&s);
			value = cl.frame.playerstate.stats[atoi(token)];
			if (!value)
			{	// skip to endif
				while (s && strcmp(token, "endif") )
				{
					token = COM_Parse ((char **)&s);
				}
			}

			continue;
		}


	}
}
*/
