#include "dk_shared.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_menu_controls.h"

#include "keys.h"

#include "l__language.h"

#include "client.h"
#include "server.h"


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

#define SAVE_TOP											(SECRETS_TOP)//(CENTERED(SCREEN_SHOT_BOTTOM,(SCREEN_BOTTOM-SCREEN_SHOT_BOTTOM),BUTTON_HEIGHT))
#define SAVE_LEFT											(10 + SCREEN_SHOT_RIGHT - BUTTON_MED_WIDTH)//(SCREEN_SHOT_LEFT)

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

#define SAVEGEM_COUNT_TOP									(SCREEN_TOP)
#define SAVEGEM_COUNT_LEFT									(SCREEN_LEFT + 40)

#define MONSTERS_LEFT										(NAME_LEFT)
#define MONSTERS_TOP										(MIKIKO_STAT_TOP + 45)

#define SECRETS_LEFT										(NAME_LEFT)//(_STAT_COLUMN_LEFT(1) - 10)
#define SECRETS_TOP											(MONSTERS_TOP + NAME_COLUMN_HEIGHT)//(MONSTERS_TOP)

#define TIME_LEFT											(_STAT_COLUMN_LEFT(1) - 0)//(NAME_LEFT)
#define TIME_TOP											(MONSTERS_TOP)//(MONSTERS_TOP + NAME_COLUMN_HEIGHT)

#define TOTAL_TIME_LEFT										(TIME_LEFT)//(SECRETS_LEFT)
#define	TOTAL_TIME_TOP										(SECRETS_TOP)//(TIME_TOP)

// now the caption editor
#define FILE_NAME_LEFT										(MAP_LEFT)//(SCREEN_LEFT)
#define FILE_NAME_TOP										(CAPTION_TOP)//(SECRETS_TOP + 30)

#define SCREEN_SHOT_HEIGHT									(100)
#define SCREEN_SHOT_WIDTH									(120)
#define SCREEN_SHOT_TOP										(SEPARATOR_TOP + 10)
#define SCREEN_SHOT_RIGHT									(SCREEN_RIGHT - 2)
#define SCREEN_SHOT_LEFT									(SCREEN_SHOT_RIGHT - SCREEN_SHOT_WIDTH)
#define SCREEN_SHOT_BOTTOM									(SCREEN_SHOT_TOP + SCREEN_SHOT_HEIGHT)

#define SCREEN_SHOT_IMAGE_WIDTH	256.0
#define SCREEN_SHOT_IMAGE_HEIGHT 256.0

#define SAVEGEM_ROTATE_TIME									100
int32 savegem_rotation;
int32 savegem_rotate_time;
#define ROLLOVER(val,min,max)								((val <= min) ? (val = max) : ((val >= max) ? (val = min) : (0)))

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

#define STATUS_HEIGHT										(20)
#define STATUS_WIDTH										(FILE_LIST_WIDTH)
#define STATUS_LEFT											(FILE_LIST_LEFT)
#define STATUS_TOP											(FILE_LIST_BOTTOM)
#define STATUS_BOTTOM										(STATUS_TOP + STATUS_HEIGHT)
#define STATUS_RIGHT										(STATUS_LEFT + STATUS_WIDTH)
#define STATUS_DURATION										(5)

//static cvar_t *sv_savegamedir = Cvar_Get( "sv_savegamedir", 0, 0 );

int FileExists(char *path, char *file);
int validSaveGame(char *path);
void Sys_DateTime(char *path, char *date, int dateSize, char *time, int timeSize);
int save_LoadGameVerify(char *name, char *field1, char *field2, char *comment)
{
	if (!name)
		return 0;

	if( !strstr( name, "save" ) )
	{
		return 0;
	}

	if (strstr(name,"save0") || strstr(name,"save1"))
		return 0;

	char path[MAX_OSPATH];
	Com_sprintf(path,sizeof(path),"%s/save/%s/",FS_SaveGameDir(),name);

	// see if the files exist (game.ssv, server.ssv)
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
		Com_sprintf( path2,sizeof(path2), "save/%s/info.hdr", name );
		
		byte	*buffer;
		int length = FS_LoadFile (path2, (void **)&buffer);
		if (!buffer || length != sizeof(save_header_t))
		{
			return 1;
		}

		save_header_t *header = (save_header_t *)buffer;
		if (strlen(header->mapTitle))
			Com_sprintf(comment,MAX_FIELD,header->mapTitle);
		FS_FreeFile (buffer);
	}
	else
	{
		Com_sprintf(comment,MAX_FIELD,"-%s-",tongue_menu[T_MENU_SAVE]);
		Com_sprintf(field1,MAX_FIELD,"--");
		Com_sprintf(field2,MAX_FIELD,"--");
	}

	return 1;
}


CMenuSubSavegame::CMenuSubSavegame()
{
	char path[MAX_OSPATH];
	char path1[MAX_OSPATH];

	Com_sprintf(path,sizeof(path),"%s/save/save",FS_SaveGameDir());

	for (char c = 'a'; c <= 'a'+20; c++)
	{
		Com_sprintf(path1,sizeof(path1),"%s%c/",path,c);
		FS_CreatePath (path1);
	}
}

void CMenuSubSavegame::Exit()
{
	// yank the save game temp thingy
	char shot[MAX_OSPATH];
	Com_sprintf(shot,sizeof(shot),"%s/shot.tga", FS_SaveGameDir());
	remove(shot);
}

void CMenuSubSavegame::Enter()
{
	status.Init(STATUS_LEFT,STATUS_TOP,STATUS_RIGHT,STATUS_BOTTOM,0);
	memset(&save_file,0,sizeof(save_file));
 	// SCG[11/9/99]: Initialize the screen shot box
	screen_shot_box.Init( SCREEN_SHOT_LEFT - 2, SCREEN_SHOT_TOP - 2, SCREEN_SHOT_RIGHT + 2, SCREEN_SHOT_BOTTOM + 2, 2 );

	// SCG[11/10/99]: Initialize the seperator
    separator.Init( SCREEN_LEFT, SEPARATOR_TOP, SCREEN_RIGHT, 2 );
    separator2.Init( NAME_LEFT, INFO_SEPARATOR_TOP, INFO_SEPARATOR_RIGHT, 1 );

    save.Init(SAVE_LEFT, SAVE_TOP, menu_font, menu_font_bright, button_font);
    save.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
    save.SetText(tongue_menu[T_MENU_SAVE], true);

	FillGameInfo();

	re.SaveSavegameScreenShot( FS_SaveGameDir(), "shot" );
	savegem_rotation = 0;
	
	// the user input field
    file_field.Init( FILE_NAME_LEFT, FILE_NAME_TOP, menu_font, menu_font_bright, button_font, INPUTFIELD_WIDE_CHAR );
	file_field.SetText(CM_KeyValue( map_epairs, "mapname" ));

	// set up the file list
	char file_filter[MAX_OSPATH];
	Com_sprintf(file_filter,sizeof(file_filter),"%s/save/",FS_SaveGameDir());

	files.Init(FILE_LIST_LEFT,FILE_LIST_TOP,FILE_LIST_RIGHT,FILE_LIST_BOTTOM,2,file_filter,"*.",NULL,FILE_LIST_SPACING,FILE_LIST_LENGTH);
	files.SetFields(FILE_NAME_START,FILE_NAME_CHARS,FILE_DATE_START,FILE_DATE_CHARS,FILE_TIME_START,FILE_TIME_CHARS);
	files.Strip(true,true);
//	files.SortBy(CInterfaceFileBox::SORT_NAME_UP,CInterfaceFileBox::SORT_NONE,CInterfaceFileBox::SORT_NONE);
	files.SortBy(CInterfaceFileBox::SORT_FIELD1_UP,CInterfaceFileBox::SORT_NAME_UP,CInterfaceFileBox::SORT_NONE);//,CInterfaceFileBox::SORT_NONE);
	files.Verify(save_LoadGameVerify);
	files.Refresh();

	// try to find an empty slot
	char path[MAX_OSPATH];
	char path1[MAX_OSPATH];
	Com_sprintf(path,sizeof(path),"%s/save/save",FS_SaveGameDir());

	for (char c = 'a'; c <= 'a'+20; c++)
	{
		Com_sprintf(path1,sizeof(path1),"%s%c/",path,c);
		if (!validSaveGame(path1))
		{
			Com_sprintf(path1,sizeof(path1),"save%c",c);
			files.Select(path1,true);
			curSelection = -1;
			RetrieveData();
			return;
		}
	}
	files.Select(0,false);

	curSelection = -1;
	RetrieveData();
}

bool SV_CanSave (client_t *cl, bool msg);
bool enable_save()
{
	return ( in_main_game() && SV_CanSave(NULL, false) );
/*
	return ( (cl_savegem_count > 0) &&																					// have save gem
			 !(cl.refdef.rdflags & RDF_LETTERBOX) &&																	// no cines
			 (in_main_game() && svs.initialized && svs.clients[0].edict && svs.clients[0].edict->client->ps.stats[STAT_HEALTH] > 0) &&	// in game and alive
			 (cls.disable_screen == 0));																				// um, yeah
*/
}
void CMenuSubSavegame::Animate(int32 elapsed_time)
{
	status.Animate(elapsed_time);
	files.Animate(elapsed_time);
	save.Animate(elapsed_time);

	savegem_rotate_time -= elapsed_time;
	if (savegem_rotate_time <= 0)
	{
		savegem_rotate_time = SAVEGEM_ROTATE_TIME;
		savegem_rotation -= 10;
		ROLLOVER(savegem_rotation,0,360);
	}
}

void CMenuSubSavegame::DrawBackground()
{
	status.DrawBackground();
	files.DrawBackground();
	save.Enable( (strlen(save_file) > 0) && (enable_save()) );
	save.DrawBackground();

	// SCG[11/9/99]: Draw the box containing the screenshot
	screen_shot_box.DrawBackground();

	// SCG[11/10/99]: Draw the seperator
	separator.DrawBackground();
	separator2.DrawBackground();
	file_field.DrawBackground();
}


void CMenuSubSavegame::DrawForeground()
{
	status.DrawForeground();
	status.PlaceEntities();
	files.DrawForeground();
	save.DrawForeground();

	char		string[128];
	DRAWSTRUCT	drawStruct;

// SCG[1/16/00]: 	sprintf(string,": %d",cl_savegem_count);
	Com_sprintf(string,sizeof(string),": %d",cl_savegem_count);
	DKM_DrawString( SAVEGEM_COUNT_LEFT, SAVEGEM_COUNT_TOP, string, NULL, false, false );
	
	// SCG[11/9/99]: Draw the box containing the screenshot
	screen_shot_box.DrawForeground();
	// SCG[12/8/99]: Draw the screen
//	cvar_t *savegamedir = Cvar_Get( "sv_savegamedir", 0, 0 );
// SCG[1/16/00]: 	sprintf( string, "%s.tga", "shot" );
//	Com_sprintf( string,sizeof(string), "%s.tga", "shot" );


	float xScale = (viddef.width / 640.0);
	float yScale = (viddef.height / 480.0);
	drawStruct.nFlags = DSFLAG_SCALE;
	drawStruct.fScaleX = (SCREEN_SHOT_WIDTH * xScale) / SCREEN_SHOT_IMAGE_WIDTH;
	drawStruct.fScaleY = (SCREEN_SHOT_HEIGHT * yScale) / SCREEN_SHOT_IMAGE_HEIGHT;
	drawStruct.nXPos = SCREEN_SHOT_LEFT * xScale;
	drawStruct.nYPos = SCREEN_SHOT_TOP * yScale;
//	drawStruct.pImage = re.RegisterPic( "shot.tga", NULL, NULL, RESOURCE_INTERFACE );
	Com_sprintf(string,sizeof(string),"%s/",FS_SaveGameDir());

	if (FileExists(string,"shot.tga"))
	{
		drawStruct.pImage = re.RegisterPic( "shot.tga", NULL, NULL, RESOURCE_INTERFACE );
	}
	else
		drawStruct.pImage = re.RegisterPic( "pics/noscreen_avail.tga", NULL, NULL, RESOURCE_INTERFACE );

	re.DrawPic( drawStruct );

	// SCG[11/10/99]: Draw the seperator
	separator.DrawForeground();
	separator2.DrawForeground();

	// SCG[11/9/99]: Draw the level stats
// SCG[1/16/00]: 	sprintf(string,"%s %d:",tongue_menu[T_MENU_EPISODE],episode);
	Com_sprintf(string,sizeof(string),"%s %d:",tongue_menu[T_MENU_EPISODE],episode);
	DKM_DrawString( EPISODE_LEFT, CAPTION_TOP, string, NULL, false, false );
//	DKM_DrawString( EPISODE_LEFT, CAPTION_TOP, "Episode 2:", NULL, false, false );

	// SCG[11/9/99]: Draw the level name
//	DKM_DrawString( MAP_LEFT, CAPTION_TOP, title, NULL, false, false );

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
// SCG[1/16/00]: 	sprintf(string, "%s %d/%d",tongue_menu[T_MENU_MONSTERS],monsters,tmonsters);
	Com_sprintf(string,sizeof(string),  "%s %d/%d",tongue_menu[T_MENU_MONSTERS],monsters,tmonsters);
	DKM_DrawString( MONSTERS_LEFT, MONSTERS_TOP, string, NULL, false, false );

	// SCG[11/9/99]: Secrets
// SCG[1/16/00]: 	sprintf(string, "%s %d/%d",tongue_menu[T_MENU_SECRETS],secrets,tsecrets);
	Com_sprintf(string,sizeof(string), "%s %d/%d",tongue_menu[T_MENU_SECRETS],secrets,tsecrets);
	DKM_DrawString( SECRETS_LEFT, SECRETS_TOP, string, NULL, false, false );

/*	// SCG[11/9/99]: Monsters
	sprintf(string, "%s 100/100",tongue_menu[T_MENU_MONSTERS]);//,monsters);
	DKM_DrawString( MONSTERS_LEFT, MONSTERS_TOP, string, NULL, false, false );

	// SCG[11/9/99]: Secrets
	sprintf(string, "%s %d",tongue_menu[T_MENU_SECRETS],secrets);
	DKM_DrawString( SECRETS_LEFT, SECRETS_TOP, string, NULL, false, false );
*/

	long *stats = (long*)&cl.frame.playerstate.stats;
	long time = *(stats + STAT_TIME);
	long tTime = *(stats + STAT_TOTAL_TIME);

	int nMinutes	= ( time / 60 );
	int nHours		= ( ( time / 60 ) / 60 );
	int nSeconds	= ( time % 60 );

	int nTMinutes	= ( tTime / 60 );
	int nTHours		= ( ( tTime / 60 ) / 60 );
	int nTSeconds	= ( tTime % 60 );

	// SCG[11/9/99]: Draw the Time
	Com_sprintf(string,sizeof(string), "%s %02d:%02d:%02d",tongue_menu[T_MENU_TIME],nHours,nMinutes,nSeconds);
	DKM_DrawString( TIME_LEFT, TIME_TOP, string, NULL, false, false );

	// SCG[11/9/99]: Draw the Total Time
	Com_sprintf(string,sizeof(string), "%s %02d:%02d:%02d",tongue_menu[T_MENU_TOTAL_TIME],nTHours,nTMinutes,nTSeconds);
	DKM_DrawString( TOTAL_TIME_LEFT, TOTAL_TIME_TOP, string, NULL, false, false );

	file_field.DrawForeground();
}

void CMenuSubSavegame::PlaceEntities()
{
	files.PlaceEntities();
	save.PlaceEntities();

	screen_shot_box.PlaceEntities();

	separator.PlaceEntities();
	separator2.PlaceEntities();


	entity_t *ent;
	
	ent = NewEntity();  //get an entity structure we can fill in.
	ent->model = re.RegisterModel("models/global/a_savegem.dkm", RESOURCE_GLOBAL);
	ent->skin = re.RegisterSkin("skins/a_savegem.wal",RESOURCE_GLOBAL);
	ent->flags = RF_FULLBRIGHT;   // make the button fullbright.
	
	ent->origin.Set(120, 28, 16); // set origin
	ent->angles.Set(0,0,0); // show your good side, baby
	
	ent->angles.y = savegem_rotation;
	ent->render_scale.Set(.2,.2,.2); 
	ent->frame = 0;
}


bool CMenuSubSavegame::Keydown(int32 key)
{
	if (files.Keydown(key))
	{
//		RetrieveData();
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
	}

	if (file_field.Keydown(key)) return true;

	return false;
}




bool CMenuSubSavegame::MousePos(int32 norm_x, int32 norm_y)
{
	if (files.MousePos(norm_x,norm_y))
	{
//		RetrieveData();
		return true;
	}

	if (save.MousePos(norm_x,norm_y)) return true;
	if (file_field.MousePos(norm_x,norm_y)) return true;

	bool ret = false;

	return ret;
}



bool CMenuSubSavegame::MouseDown(int32 norm_x, int32 norm_y)
{
	if (files.MouseDown(norm_x,norm_y))
	{
		RetrieveData();
		return true;
	}
	if (save.MouseDown(norm_x,norm_y)) return true;
	if (file_field.MouseDown(norm_x,norm_y)) return true;
	return false;
}

short SV_DoSaveGame(char *dir, char *comment);
bool CMenuSubSavegame::MouseUp(int32 norm_x, int32 norm_y)
{
	if (file_field.MouseUp(norm_x,norm_y)) return true;
	if (files.MouseUp(norm_x,norm_y))
	{
//		RetrieveData();
		return true;
	}

	if (save.MouseUp(norm_x,norm_y))
	{
		if (strlen(save_file) == 0)
			return true;

		char comment[32];
		files.Selection(save_file,NULL,NULL,NULL);
		Com_sprintf(comment,sizeof(comment),"%s",file_field.GetText());
		StripInvalidChars(comment);

		if (SV_DoSaveGame(save_file,(char *)comment))
		{
			char txt[256];
			Com_sprintf (txt, sizeof(txt),"%s %s",tongue_menu[T_MENU_SAVED],comment);
			status.SetText(comment,STATUS_DURATION);
		}

		files.Refresh();
		files.Select(save_file);
		FillGameInfo();
		return true;
	}
	return false;
}


void CMenuSubSavegame::FillGameInfo()
{
	// set up the game info structure
	memset(&game_info,0,4 * sizeof(game_info_t));
	strcpy(game_info[0].name,tongue_menu[T_MENU_MODELNAME]);	// actually says 'character'

	episode = cl_episode_num;
//	strcpy(title,cl_mapname);
//	file_field.SetText( cl_mapname );

	// fill in the caption bar
	long *stats = (long*)&cl.frame.playerstate.stats;

	// get the monster kill count
	monsters = *(stats + STAT_KILLS);
	tmonsters = *(stats + STAT_MONSTERS);

	// get the secret count
	secrets = *(stats + STAT_FOUND_SECRETS);
	tsecrets = *(stats + STAT_SECRETS);

	// fill in hiro's game info
	game_info_t *curInfo = &game_info[GAME_INFO_HIRO];
	strcpy(curInfo->name,"Hiro");
	Com_sprintf(curInfo->stats[GAME_STAT_HEALTH],MAX_STAT_LEN,"%d",*(stats + STAT_HEALTH));
	Com_sprintf(curInfo->stats[GAME_STAT_ARMOR],MAX_STAT_LEN,"%d",*(stats + STAT_ARMOR));
	Com_sprintf(curInfo->stats[GAME_STAT_LEVEL],MAX_STAT_LEN,"%d",*(stats + STAT_LEVEL));

	long health;
	// fill in superfly's game info
	curInfo = &game_info[GAME_INFO_SUPERFLY];
	strcpy(curInfo->name,"Superfly");
	health = *(stats + STAT_SUPERFLY_HEALTH);
	if (health)
	{
		Com_sprintf(curInfo->stats[GAME_STAT_HEALTH],MAX_STAT_LEN,"%d",health);
		Com_sprintf(curInfo->stats[GAME_STAT_ARMOR],MAX_STAT_LEN,"%d",*(stats + STAT_SUPERFLY_ARMOR));
	}
	else
	{
		strcpy(curInfo->stats[GAME_STAT_HEALTH],"--");
		strcpy(curInfo->stats[GAME_STAT_ARMOR],"--");
	}
	strcpy(curInfo->stats[GAME_STAT_LEVEL],"--");

	// fill in superfly's game info
	curInfo = &game_info[GAME_INFO_MIKIKO];
	strcpy(curInfo->name,"Mikiko");
	health = *(stats + STAT_MIKIKO_HEALTH);

	if (health)
	{
		Com_sprintf(curInfo->stats[GAME_STAT_HEALTH],MAX_STAT_LEN,"%d",health);
		Com_sprintf(curInfo->stats[GAME_STAT_ARMOR],MAX_STAT_LEN,"%d",*(stats + STAT_MIKIKO_ARMOR));
	}
	else
	{
		strcpy(curInfo->stats[GAME_STAT_HEALTH],"--");
		strcpy(curInfo->stats[GAME_STAT_ARMOR],"--");
	}
	strcpy(curInfo->stats[GAME_STAT_LEVEL],"--");
}

bool CMenuSubSavegame::RetrieveData()
{
	int32 sel = files.Selection();
	if (sel != curSelection)
	{
		if (files.Selection(save_file,NULL,NULL,NULL))
		{
			curSelection = sel;
			return true;
		}
	}

	return false;
}
