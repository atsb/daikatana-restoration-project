#include "dk_shared.h"
#include "client.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_menu_controls.h"

#include "keys.h"

#include "l__language.h"

///////////////////////////////////////////////////////////////////////////////////
//  Load Menu
///////////////////////////////////////////////////////////////////////////////////


// defines
#define CENTERED(left,width,cwidth)							(left + (0.5*width) - (0.5*cwidth))

#define SCREEN_WIDTH										370
#define SCREEN_HEIGHT										300
#define SCREEN_LEFT											100
#define SCREEN_TOP											115
#define SCREEN_RIGHT										(SCREEN_LEFT + SCREEN_WIDTH)
#define SCREEN_BOTTOM										(SCREEN_TOP + SCREEN_HEIGHT)

// the buttons
#define BUTTON_HEIGHT										(25)
#define BUTTON_SHORT_WIDTH									(63)
#define BUTTON_MED_WIDTH									(115)
#define BUTTON_LONG_WIDTH									(200)

// some cool stuff for centering buttons
#define BUTTON_SPACE(width,eawidth,max)						((width - (max * eawidth))/(max + 1))
#define BUTTON_L(left,width,num,max,eawidth)				(left + (num * BUTTON_SPACE(width,eawidth,max)) + ((num-1)*eawidth))
#define BUTTON_MED_LEFT(num,max)							(BUTTON_L(SCREEN_LEFT,FILE_LIST_WIDTH,num,max,BUTTON_MED_WIDTH))

#define NAME_LEFT											(SCREEN_LEFT)
#define EPISODE_LEFT										(NAME_LEFT)
#define MAP_LEFT											(NAME_LEFT + 100)

#define CAPTION_TOP											(SCREEN_TOP + 20)
#define CAPTION_LEFT										(EPISODE_LEFT)

#define LOAD_TOP											(CAPTION_TOP)
#define LOAD_LEFT											(SCREEN_RIGHT - BUTTON_MED_WIDTH)

#define SEPARATOR_TOP										(CAPTION_TOP + BUTTON_HEIGHT)

#define NAME_COLUMN_HEIGHT									(20)

#define INFO_HEADER_TOP										(SCREEN_SHOT_TOP)
#define INFO_SEPARATOR_TOP									(INFO_HEADER_TOP + NAME_COLUMN_HEIGHT - 3)
#define INFO_SEPARATOR_RIGHT								(_STAT_COLUMN_LEFT(3))

#define PLAYER_STAT_TOP										(INFO_HEADER_TOP + NAME_COLUMN_HEIGHT)
#define SUPERFLY_STAT_TOP									(PLAYER_STAT_TOP + NAME_COLUMN_HEIGHT)
#define MIKIKO_STAT_TOP										(SUPERFLY_STAT_TOP + NAME_COLUMN_HEIGHT)

#define _STAT_COLUMN_LEFT(idx)								(NAME_LEFT + 90 + (idx * 50))
#define STAT_COLUMN_LEFT(idx)								(_STAT_COLUMN_LEFT(idx) + 15)

#define MONSTERS_LEFT										(NAME_LEFT)
#define MONSTERS_TOP										(MIKIKO_STAT_TOP + 45)

#define SECRETS_LEFT										(NAME_LEFT)//(_STAT_COLUMN_LEFT(1) - 10)
#define SECRETS_TOP											(MONSTERS_TOP + NAME_COLUMN_HEIGHT)//(MONSTERS_TOP)

#define TIME_LEFT											(_STAT_COLUMN_LEFT(1) - 0)//(NAME_LEFT)
#define TIME_TOP											(MONSTERS_TOP)//(MONSTERS_TOP + NAME_COLUMN_HEIGHT)

#define TOTAL_TIME_LEFT										(TIME_LEFT)//(SECRETS_LEFT)
#define	TOTAL_TIME_TOP										(SECRETS_TOP)//(TIME_TOP)

#define SCREEN_SHOT_HEIGHT									(100)
#define SCREEN_SHOT_WIDTH									(120)
#define SCREEN_SHOT_TOP										(SEPARATOR_TOP + 10)
#define SCREEN_SHOT_RIGHT									(SCREEN_RIGHT - 2)
#define SCREEN_SHOT_LEFT									(SCREEN_SHOT_RIGHT - SCREEN_SHOT_WIDTH)
#define SCREEN_SHOT_BOTTOM									(SCREEN_SHOT_TOP + SCREEN_SHOT_HEIGHT)

#define SCREEN_SHOT_IMAGE_WIDTH	256.0
#define SCREEN_SHOT_IMAGE_HEIGHT 256.0

#define FILE_LIST_LENGTH									5
#define FILE_LIST_SPACING									14
#define FILE_LIST_WIDTH										(SCREEN_WIDTH)

#define config_line_y(i)									(SECRETS_TOP + 20 + (i) * FILE_LIST_SPACING)

#define file_list_line_y(i)									(config_line_y(i + 1))
#define FILE_LIST_LEFT										(SCREEN_LEFT)
#define FILE_LIST_TOP										(file_list_line_y(-0.5) - 3)
#define FILE_LIST_RIGHT										(FILE_LIST_LEFT + FILE_LIST_WIDTH)
#define FILE_LIST_BOTTOM									(file_list_line_y( FILE_LIST_LENGTH ) + 4)

#define FILE_NAME_CHARS										22
#define FILE_NAME_START										0
#define FILE_DATE_CHARS										9
#define FILE_DATE_START										22
#define FILE_TIME_CHARS										9
#define FILE_TIME_START										33

CMenuSubLoadgame::CMenuSubLoadgame()
{
}

int FileExists(char *path, char *file)
{
	int result = 0;
	if (!path || !file)
		return 0;

	char search[MAX_OSPATH];
	Com_sprintf(search,sizeof(search),"%s%s",path,file);

	if (Sys_FindFirst(search,0,0))
		result = 1;
	
	Sys_FindClose();

	return result;
}

int validSaveGame(char *path)
{
	return (FileExists(path,"game.ssv") && FileExists(path,"server.ssv")) ? TRUE : FALSE;
}

void Sys_DateTime(char *path, char *date, int dateSize, char *time, int timeSize);
int LoadGameVerify(char *name, char *field1, char *field2, char *comment)
{
	if (!name)
		return 0;

	if( strstr( name, "current" ) || strstr( name, "save1" ))
	{
		return 0;
	}

	char path[MAX_OSPATH];
/*
	cvar_t *sv_savegamedir = Cvar_Get ("sv_savegamedir", "", CVAR_ARCHIVE);

	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
		Com_sprintf(path,sizeof(path),"%s/save/%s/",sv_savegamedir->string,name);
	else
		Com_sprintf(path,sizeof(path),"%s/save/%s/",FS_Gamedir(),name);
*/

	Com_sprintf(path,sizeof(path),"%s/save/%s/",FS_SaveGameDir(),name);

	// see if the files exist (game.ssv, server.ssv) and fill the comment field if available
	if (validSaveGame(path))
	{
		char path2[MAX_OSPATH];
		char date[MAX_FIELD];
		char time[MAX_FIELD];
		Com_sprintf(path2,sizeof(path2),"%s%s",path,"game.ssv");
		Sys_DateTime(path2, date, MAX_FIELD, time, MAX_FIELD);
		Com_sprintf(field1,MAX_FIELD,"%s",date);
		Com_sprintf(field2,MAX_FIELD,"%s",time);

		// if the header exists, set the comment field to the mapTitle in the header file...
//		Com_sprintf( path2,sizeof(path2), "%sinfo.hdr",path);
		Com_sprintf( path2,sizeof(path2), "save/%s/info.hdr", name );
		
		byte	*buffer;
		int length = FS_LoadFile (path2, (void **)&buffer);
		if (!buffer || length != sizeof(save_header_t))
		{
			return 1;
		}

		save_header_t *header = (save_header_t *)buffer;
		if (strlen(header->mapTitle))
		{
			Com_sprintf(comment,MAX_FIELD,header->mapTitle);
		}
		FS_FreeFile (buffer);

		return 1;
	}

	return 0;
}

void CMenuSubLoadgame::Enter()
{
	// initialize stuff
	memset(&game_info,0,4 * sizeof(game_info_t));
	memset(&title,0,sizeof(title));
	strcpy(game_info[0].name,tongue_menu[T_MENU_MODELNAME]);	// actually says 'character'

	monsters = -1;
	tmonsters = -1;
	secrets = -1;
	tsecrets = -1;
	time = -1;
	total = -1;
	episode = -1;
	memset( title, 0, MAX_TITLE );

	float xScale = (viddef.width / 640.0);
	float yScale = (viddef.height / 480.0);
	picDrawStruct.nFlags = DSFLAG_SCALE;
	picDrawStruct.fScaleX = (SCREEN_SHOT_WIDTH * xScale) / SCREEN_SHOT_IMAGE_WIDTH;
	picDrawStruct.fScaleY = (SCREEN_SHOT_HEIGHT * yScale) / SCREEN_SHOT_IMAGE_HEIGHT;
	picDrawStruct.nXPos = SCREEN_SHOT_LEFT * xScale;
	picDrawStruct.nYPos = SCREEN_SHOT_TOP * yScale;
	picDrawStruct.pImage = re.RegisterPic( "pics/noscreen_avail.tga", NULL, NULL, RESOURCE_INTERFACE );

	char file_filter[MAX_OSPATH];
/*
	cvar_t *sv_savegamedir = Cvar_Get ("sv_savegamedir", "", CVAR_ARCHIVE);
	// the file list

	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
		Com_sprintf(file_filter,sizeof(file_filter),"%s/save/",sv_savegamedir->string);
	else
		Com_sprintf(file_filter,sizeof(file_filter),"%s/save/",FS_Gamedir());
*/
	Com_sprintf(file_filter,sizeof(file_filter),"%s/save/",FS_SaveGameDir());

	files.Init(FILE_LIST_LEFT,FILE_LIST_TOP,FILE_LIST_RIGHT,FILE_LIST_BOTTOM,2,file_filter,"*.",NULL,FILE_LIST_SPACING,FILE_LIST_LENGTH);
	files.SetFields(FILE_NAME_START,FILE_NAME_CHARS,FILE_DATE_START,FILE_DATE_CHARS,FILE_TIME_START,FILE_TIME_CHARS);
	files.Strip(true,true);
	files.SortBy(CInterfaceFileBox::SORT_FIELD1_DN,CInterfaceFileBox::SORT_FIELD2_UP,CInterfaceFileBox::SORT_NAME_DN);
	files.Verify(LoadGameVerify);
	files.Refresh();
	files.Select(0,false);


	// SCG[11/9/99]: Initialize the screen shot box
	screen_shot_box.Init( SCREEN_SHOT_LEFT - 2, SCREEN_SHOT_TOP - 2, SCREEN_SHOT_RIGHT + 2, SCREEN_SHOT_BOTTOM + 2, 2 );

	// SCG[11/10/99]: Initialize the seperator
//	seperator.Init( LEVEL_NAME_TOP + 20, NAME_COLUMN_LEFT, 2, NULL );
    separator.Init( SCREEN_LEFT, SEPARATOR_TOP, SCREEN_RIGHT, 2 );
    separator2.Init( NAME_LEFT, INFO_SEPARATOR_TOP, INFO_SEPARATOR_RIGHT, 1 );

    load.Init(LOAD_LEFT, LOAD_TOP, menu_font, menu_font_bright, button_font);
    load.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
    load.SetText(tongue_menu[T_MENU_LOAD_GAME], true);

	picDrawStruct.pImage = re.RegisterPic( "pics/noscreen_avail.tga", NULL, NULL, RESOURCE_INTERFACE );
	curSelection = -1;
	RetrieveData();
}


void CMenuSubLoadgame::Animate(int32 elapsed_time)
{
	files.Animate(elapsed_time);
	load.Animate(elapsed_time);
}

void CMenuSubLoadgame::DrawBackground()
{
	load.DrawBackground();
	files.DrawBackground();

	// SCG[11/9/99]: Draw the box containing the screenshot
	screen_shot_box.DrawBackground();

	// SCG[11/10/99]: Draw the seperator
	separator.DrawBackground();
	separator2.DrawBackground();
}

void CMenuSubLoadgame::DrawForeground()
{
	load.DrawForeground();
	files.DrawForeground();

	char	string[128];

	// SCG[11/9/99]: Draw the box containing the screenshot
	screen_shot_box.DrawForeground();

	GetPic();
	re.DrawPic( picDrawStruct );

	// SCG[11/10/99]: Draw the seperator
	separator.DrawForeground();
	separator2.DrawForeground();

	// SCG[11/9/99]: Draw the level stats
	if( episode != -1 )
	{
		Com_sprintf(string,sizeof(string),"%s %d: (%s)",tongue_menu[T_MENU_EPISODE],episode, bCoop ? "COOP" : "SP");
		DKM_DrawString( EPISODE_LEFT, SCREEN_TOP, string, NULL, false, false );
	}
	else
	{
		DKM_DrawString( EPISODE_LEFT, SCREEN_TOP, "", NULL, false, false );
	}

	// SCG[11/9/99]: Draw the level name
	if( title[0] != NULL )
	{
		DKM_DrawString( CAPTION_LEFT, CAPTION_TOP, title, NULL, false, false );
	}

	int32 top = INFO_HEADER_TOP;
	DKM_DrawString(NAME_LEFT,top,game_info[0].name,NULL,false,false);
	DKM_DrawString(_STAT_COLUMN_LEFT(0),top,tongue_menu[T_MENU_ARMOR],NULL,false,false);
	DKM_DrawString(_STAT_COLUMN_LEFT(1),top,tongue_menu[T_MENU_HEALTH],NULL,false,false);
	DKM_DrawString(_STAT_COLUMN_LEFT(2),top,tongue_menu[T_MENU_LEVEL],NULL,false,false);
	

	top = PLAYER_STAT_TOP;
	for (int i = 1; i <= 3; i++)
	{
		DKM_DrawString(NAME_LEFT,top,game_info[i].name,NULL,false,false);
		
		for (int j = 0; j < 3; j++)
		{
			DKM_DrawString(STAT_COLUMN_LEFT(j),top,game_info[i].stats[j],NULL,false,false);
		}

		top += NAME_COLUMN_HEIGHT;
	}

	// SCG[11/9/99]: Monsters
	if( (monsters != -1) && tmonsters )
	{
		Com_sprintf(string, sizeof(string) ,"%s %d/%d",tongue_menu[T_MENU_MONSTERS],monsters,tmonsters);
		DKM_DrawString( MONSTERS_LEFT, MONSTERS_TOP, string, NULL, false, false );
	}

	// SCG[11/9/99]: Secrets
	if( (secrets != -1)  && tsecrets )
	{
		Com_sprintf(string,sizeof(string), "%s %d/%d",tongue_menu[T_MENU_SECRETS],secrets,tsecrets);
		DKM_DrawString( SECRETS_LEFT, SECRETS_TOP, string, NULL, false, false );
	}

	if( (time != -1) && time && total )
	{
		int nMinutes	= (int)( time / 60 ) % 60;
		int nHours		= ( time / 3600 );
		int nSeconds	= ( time % 60 );

		int nTMinutes	= (int)( total / 60 ) % 60;
		int nTHours		= ( total / 3600 );
		int nTSeconds	= ( total % 60 );

		// SCG[11/9/99]: Draw the Time
		Com_sprintf(string,sizeof(string),  "%s %02d:%02d:%02d",tongue_menu[T_MENU_TIME],nHours,nMinutes,nSeconds);
		DKM_DrawString( TIME_LEFT, TIME_TOP, string, NULL, false, false );

		// SCG[11/9/99]: Draw the Total Time
		Com_sprintf(string, sizeof(string),"%s %02d:%02d:%02d",tongue_menu[T_MENU_TOTAL_TIME],nTHours,nTMinutes,nTSeconds);
		DKM_DrawString( TOTAL_TIME_LEFT, TOTAL_TIME_TOP, string, NULL, false, false );
	}
}


void CMenuSubLoadgame::PlaceEntities()
{
	load.PlaceEntities();
	files.PlaceEntities();

	screen_shot_box.PlaceEntities();

	separator.PlaceEntities();
	separator2.PlaceEntities();
}


bool CMenuSubLoadgame::Keydown(int32 key)
{
	if (files.Keydown(key))
	{
		RetrieveData();
		return true;
	}

	switch (key)
	{
    case '`' :
    case K_ESCAPE :
		if( Cvar_VariableValue( "console" ) == 0 )
		{
			return false;
		}
		DKM_Shutdown(); // exit entire interface

		return true;
	case 'l':
		DoLoad();
		return true;
	}

	return false;
}




bool CMenuSubLoadgame::MousePos(int32 norm_x, int32 norm_y)
{
	if (load.MousePos(norm_x,norm_y)) return true;

	bool ret = false;

	if (files.MousePos(norm_x,norm_y))
	{
		RetrieveData();
		return true;
	}

	return ret;
}



bool CMenuSubLoadgame::MouseDown(int32 norm_x, int32 norm_y)
{
	if (load.MouseDown(norm_x,norm_y)) return true;
	if (files.MouseDown(norm_x,norm_y))
	{
		RetrieveData();
		return true;
	}
	return false;
}


bool CMenuSubLoadgame::MouseUp(int32 norm_x, int32 norm_y)
{
	if (load.MouseUp(norm_x,norm_y))
	{
		DoLoad();
		return true;
	}

	if (files.MouseUp(norm_x,norm_y))
	{
		RetrieveData();
		return true;
	}

	return false;
}

bool CMenuSubLoadgame::RetrieveData()
{
	int32 sel = files.Selection();
	if (sel != curSelection)
	{
		if (files.Selection(cur_file,NULL,NULL,NULL))
		{
			FillGameInfo();
			curSelection = sel;
			return true;
		}
	}

	return false;
}

void CMenuSubLoadgame::FillGameInfo()
{
	// set up the game info structure
	memset(&game_info,0,4 * sizeof(game_info_t));
	memset(&title,0,sizeof(title));
	strcpy(game_info[0].name,tongue_menu[T_MENU_MODELNAME]);	// actually says 'character'

	save_header_t *header;
	save_header_t	_header;
	memset(&_header,0,sizeof(_header));
	header = &_header;

	char name[MAX_OSPATH];

	Com_sprintf(name,sizeof(name),"%s/save/%s/info.hdr",FS_SaveGameDir(),cur_file);
	FILE	*f;
	f = fopen (name, "rb");

	bool success = false;
	if (f)
	{
		if (1 == fread(&_header,sizeof(_header),1,f))
			success = true;
		fclose(f);
	}
	
	if (!success)//!buffer)// || length != sizeof(save_header_t))
	{
		monsters = -1;
		tmonsters = -1;
		secrets = -1;
		tsecrets = -1;
		time = -1;
		total = -1;
		episode = -1;
		memset( title, 0, MAX_TITLE );
		picDrawStruct.pImage = re.RegisterPic( "pics/noscreen_avail.tga", NULL, NULL, RESOURCE_INTERFACE );
		return;
	}
	
	episode = header->episode;
	bCoop = header->bCoop;

	// get the title
	Com_sprintf(title,sizeof(title),header->caption);
	// fill in hiro's game info
	game_info_t *curInfo = &game_info[GAME_INFO_HIRO];
	strcpy(curInfo->name,"Hiro");
//	Com_sprintf(curInfo->stats[GAME_STAT_HEALTH],MAX_STAT_LEN,"%d",(int)header->hiroHealth);
//	Com_sprintf(curInfo->stats[GAME_STAT_ARMOR],MAX_STAT_LEN,"%d",(int)header->hiroArmor);
//	Com_sprintf(curInfo->stats[GAME_STAT_LEVEL],MAX_STAT_LEN,"%d",(int)header->hiroLevel);

	long health;
	if (header->hiroHealth)
	{
		Com_sprintf(curInfo->stats[GAME_STAT_HEALTH],MAX_STAT_LEN,"%d",(int)header->hiroHealth);
		Com_sprintf(curInfo->stats[GAME_STAT_ARMOR],MAX_STAT_LEN,"%d",(int)header->hiroArmor);
		Com_sprintf(curInfo->stats[GAME_STAT_LEVEL],MAX_STAT_LEN,"%d",(int)header->hiroLevel);
	}
	else
	{
		strcpy(curInfo->stats[GAME_STAT_HEALTH],"--");
		strcpy(curInfo->stats[GAME_STAT_ARMOR],"--");
		strcpy(curInfo->stats[GAME_STAT_LEVEL],"--");
	}


	// fill in superfly's game info
	curInfo = &game_info[GAME_INFO_SUPERFLY];
	strcpy(curInfo->name,"Superfly");
	health = header->sfHealth;
	if (health)
	{
		Com_sprintf(curInfo->stats[GAME_STAT_HEALTH],MAX_STAT_LEN,"%d",health);
		Com_sprintf(curInfo->stats[GAME_STAT_ARMOR],MAX_STAT_LEN,"%d",(int)header->sfArmor);
	}
	else
	{
		strcpy(curInfo->stats[GAME_STAT_HEALTH],"--");
		strcpy(curInfo->stats[GAME_STAT_ARMOR],"--");
	}
	strcpy(curInfo->stats[GAME_STAT_LEVEL],"--");

	// fill in mikiko's game info
	curInfo = &game_info[GAME_INFO_MIKIKO];
	strcpy(curInfo->name,"Mikiko");
	health = header->mHealth;
	if (health)
	{
		Com_sprintf(curInfo->stats[GAME_STAT_HEALTH],MAX_STAT_LEN,"%d",health);
		Com_sprintf(curInfo->stats[GAME_STAT_ARMOR],MAX_STAT_LEN,"%d",(int)header->mArmor);
	}
	else
	{
		strcpy(curInfo->stats[GAME_STAT_HEALTH],"--");
		strcpy(curInfo->stats[GAME_STAT_ARMOR],"--");
	}
	strcpy(curInfo->stats[GAME_STAT_LEVEL],"--");

	// get the monster kill count
	monsters = header->monsters;
	tmonsters = header->tMonsters;

	// get the secret count
	secrets = header->secrets;
	tsecrets = header->tSecrets;

	time = header->time;
	total = header->total;
/*
	char	string[128];
	Com_sprintf(string,sizeof(string),"%s/save/%s/",FS_SaveGameDir(),cur_file);

	if (FileExists(string,"shot.tga"))
	{
		Com_sprintf(string,sizeof(string),"save/%s/shot.tga",cur_file);
		picDrawStruct.pImage = re.RegisterPic( string, NULL, NULL, RESOURCE_INTERFACE );
	}
	else
		picDrawStruct.pImage = re.RegisterPic( "pics/noscreen_avail.tga", NULL, NULL, RESOURCE_INTERFACE );
*/
}

void CMenuSubLoadgame::GetPic()
{
	char	string[128];
	Com_sprintf(string,sizeof(string),"%s/save/%s/",FS_SaveGameDir(),cur_file);

	if (FileExists(string,"shot.tga"))
	{
		Com_sprintf(string,sizeof(string),"save/%s/shot.tga",cur_file);
		picDrawStruct.pImage = re.RegisterPic( string, NULL, NULL, RESOURCE_INTERFACE );
	}
	else
		picDrawStruct.pImage = re.RegisterPic( "pics/noscreen_avail.tga", NULL, NULL, RESOURCE_INTERFACE );
}

void CMenuSubLoadgame::DoLoad()
{
	char loadGame[MAX_OSPATH];
	if (files.Selection(loadGame,NULL,NULL,NULL))
	{
//       loading\n DKM_ShutdownString("deathmatch 0\ncoop 0\nloading\nkillserver\nwait\nwait\nwait\nwait\nwait\nwait\nwait\nwait\nwait\nwait\nwait\nwait\nwait\nwait\nwait\nwait\nload %s\n",loadGame);
		DKM_ShutdownString("wait\nwait\nload %s\n",loadGame);
        DKM_Shutdown();
	}
}

