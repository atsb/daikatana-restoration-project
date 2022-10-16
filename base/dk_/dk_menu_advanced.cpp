#include "dk_shared.h"

//#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
//#include "dk_menu_controls.h"	// SCG[1/21/00]: already included in "dk_menup.h"

#include "keys.h"
#include "client.h"

#include "l__language.h"


void CL_WriteConfiguration (char *name);
void CL_SaveLastConfiguration(void);

///////////////////////////////////////////////////////////////////////////////////
//
//  Advanced Menu
//
///////////////////////////////////////////////////////////////////////////////////
#define CENTERED(left,width,cwidth)							(left + (0.5*width) - (0.5*cwidth))

#define CONFIG_LEFT											110
#define CONFIG_TOP											115
#define CONFIG_WIDTH										375
#define CONFIG_HEIGHT										300

// the buttons
#define BUTTON_HEIGHT										(25)
#define BUTTON_SHORT_WIDTH									(63)
#define BUTTON_MED_WIDTH									(115)
//#define BUTTON_LONG_WIDTH									(200)

// some cool stuff for centering buttons
#define BUTTON_SPACE(width,eawidth,max)						((width - (max * eawidth))/(max + 1))
#define BUTTON_L(left,width,num,max,eawidth)				(left + (num * BUTTON_SPACE(width,eawidth,max)) + ((num-1)*eawidth))
#define BUTTON_MED_LEFT(num,max)							(BUTTON_L(CONFIG_LEFT,FILE_LIST_WIDTH,num,max,BUTTON_MED_WIDTH))


										
#define BUTTON_CONTROL_TOP									(CONFIG_TOP + 10 + BUTTON_HEIGHT)

//#define BUTTON_SAVEAS_TOP									(CONFIG_TOP)
//#define BUTTON_SAVEAS_LEFT									(FILE_LIST_LEFT + (0.5 * FILE_LIST_WIDTH) - (0.5 * BUTTON_LONG_WIDTH))

#define HEADER_LEFT											(200)
#define HEADER_TOP											(CONFIG_TOP)

#define FILE_LIST_LENGTH									12
#define FILE_LIST_SPACING									14
#define FILE_LIST_WIDTH										365

#define config_line_y(i)									(FILE_FIELD_TOP + BUTTON_HEIGHT + (i) * FILE_LIST_SPACING)

#define file_list_line_y(i)									(config_line_y(i + 1))
#define FILE_LIST_LEFT										(CONFIG_LEFT - 5)
#define FILE_LIST_TOP										(file_list_line_y(-0.5) - 3)
#define FILE_LIST_RIGHT										(FILE_LIST_LEFT + FILE_LIST_WIDTH)
#define FILE_LIST_BOTTOM									(file_list_line_y( FILE_LIST_LENGTH ) + 4)

#define STATUS_HEIGHT										(20)
#define STATUS_WIDTH										(FILE_LIST_WIDTH)
#define STATUS_LEFT											(FILE_LIST_LEFT)
#define STATUS_TOP											(CONFIG_TOP + CONFIG_HEIGHT - 15 - STATUS_HEIGHT)
#define STATUS_BOTTOM										(STATUS_TOP + STATUS_HEIGHT)
#define STATUS_RIGHT										(STATUS_LEFT + STATUS_WIDTH)
#define STATUS_DURATION										(5)

#define FILE_NAME_CHARS										22
#define FILE_NAME_START										0
#define FILE_DATE_CHARS										9
#define FILE_DATE_START										22
#define FILE_TIME_CHARS										9
#define FILE_TIME_START										33

#define SEPARATOR_TOP										(CONFIG_TOP + BUTTON_HEIGHT)

// the input box
#define FILE_FIELD_TOP										(BUTTON_CONTROL_TOP + BUTTON_HEIGHT)
#define FILE_FIELD_LEFT										(FILE_LIST_LEFT)


#define CONFIRM_WIDTH										(200)
#define CONFIRM_HEIGHT										(65)

#define CONFIRM_LEFT										(CENTERED(CONFIG_LEFT,CONFIG_WIDTH,CONFIRM_WIDTH))
#define CONFIRM_RIGHT										(CONFIRM_LEFT + CONFIRM_WIDTH)
#define CONFIRM_TOP											(CENTERED(CONFIG_TOP,CONFIG_HEIGHT,CONFIRM_HEIGHT))
#define CONFIRM_BOTTOM										(CONFIRM_TOP + CONFIRM_HEIGHT)

#define CONFIRM_HEADER_TOP									(CONFIRM_TOP + 10)
#define CONFIRM_BUTTON_TOP									(CONFIRM_HEADER_TOP + BUTTON_HEIGHT)
#define CONFIRM_BUTTON_LEFT(idx)							( BUTTON_L(CONFIRM_LEFT,(CONFIRM_RIGHT-CONFIRM_LEFT),idx,2,BUTTON_SHORT_WIDTH) )

#define SPECIAL_CONFIG(config)								(strlen(config) && (stricmp(config,"daikatana")) && (stricmp(config,"curcfg")) && (stricmp(config,"default_keys"))  && (stricmp(config,"autoexec")))


CMenuSubAdvanced::CMenuSubAdvanced()
{
	cl_curconfig = Cvar_Get("currentconfig","",0);
	separator.Init(FILE_LIST_LEFT, SEPARATOR_TOP, FILE_LIST_RIGHT, 3);
}

void CMenuSubAdvanced::Enter()
{
	curSelection = -1;
	// buttons
    load.Init(BUTTON_MED_LEFT(1,3), BUTTON_CONTROL_TOP, menu_font, menu_font_bright, button_font);
    load.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
    load.SetText(tongue_menu[T_MENU_LOAD_CONFIG], true);

    save.Init(BUTTON_MED_LEFT(2,3), BUTTON_CONTROL_TOP, menu_font, menu_font_bright, button_font);
    save.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
    save.SetText(tongue_menu[T_MENU_SAVE_CONFIG], true);

    delBtn.Init(BUTTON_MED_LEFT(3,3), BUTTON_CONTROL_TOP, menu_font, menu_font_bright, button_font);
    delBtn.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
    delBtn.SetText(tongue_menu[T_MENU_DELETE_CONFIG], true);

	// the file list
	char file_filter[MAX_OSPATH];
	Com_sprintf(file_filter,sizeof(file_filter),"%s/",FS_ConfigDir());

	files.Init(FILE_LIST_LEFT,FILE_LIST_TOP,FILE_LIST_RIGHT,FILE_LIST_BOTTOM,2,file_filter,"*.cfg","curcfg",FILE_LIST_SPACING,FILE_LIST_LENGTH);
	files.SetFields(FILE_NAME_START,FILE_NAME_CHARS,FILE_DATE_START,FILE_DATE_CHARS,FILE_TIME_START,FILE_TIME_CHARS);
	files.Strip(true,true);
	files.SortBy(CInterfaceFileBox::SORT_NAME_UP,CInterfaceFileBox::SORT_FIELD1_DN,CInterfaceFileBox::SORT_FIELD2_DN);
	files.Refresh();
	files.Select(cl_curconfig->string,true);

	// the user input field
    file_field.Init( FILE_FIELD_LEFT, FILE_FIELD_TOP, menu_font, menu_font_bright, button_font, INPUTFIELD_WIDE_CHAR );
//    file_field.SetText( dk_userName );
	RetrieveData();

	confirm_dlg.Init(CONFIRM_LEFT,CONFIRM_TOP,CONFIRM_RIGHT,CONFIRM_BOTTOM,2,CONFIRM_BUTTON_TOP,CONFIRM_BUTTON_LEFT(1),CONFIRM_BUTTON_LEFT(2));
	status.Init(STATUS_LEFT,STATUS_TOP,STATUS_RIGHT,STATUS_BOTTOM,0);
}

void CMenuSubAdvanced::Animate(int32 elapsed_time)
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.Animate(elapsed_time);
	}
	else
		files.Animate(elapsed_time);

    //animate our buttons.
    load.Animate(elapsed_time);
    save.Animate(elapsed_time);
    delBtn.Animate(elapsed_time);
	status.Animate(elapsed_time);
}

void CMenuSubAdvanced::PlaceEntities()
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.PlaceEntities();
	}
	else
		files.PlaceEntities();

    separator.PlaceEntities();  //draw the separator.
	load.PlaceEntities();
	save.PlaceEntities();
	delBtn.PlaceEntities();
	status.PlaceEntities();
}


void CMenuSubAdvanced::DrawForeground()
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.DrawForeground();
	}
	else
	{
		file_field.DrawForeground();
		files.DrawForeground();
	}

	// don't allow certain things.  :)
	char newCfg[MAX_QPATH];
	strcpy(newCfg,file_field.GetText());
	bool bEnable = SPECIAL_CONFIG(newCfg);//(strlen(newCfg) && (stricmp(newCfg,"daikatana")) && (stricmp(newCfg,"curcfg")) && (stricmp(newCfg,"default_keys"))  && (stricmp(newCfg,"autoexec")));
	delBtn.Enable(bEnable);
	save.Enable(bEnable);
	load.Enable(strlen(newCfg) > 0);

	load.DrawForeground();
	save.DrawForeground();
	delBtn.DrawForeground();
	DKM_DrawString(HEADER_LEFT, HEADER_TOP, tongue_menu[T_MENU_SELECT_CONFIG],NULL, false,true);
	status.DrawForeground();
}

void CMenuSubAdvanced::DrawBackground()
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.DrawBackground();
	}
	else
	{
		file_field.DrawBackground();
		files.DrawBackground();
	}
	load.DrawBackground();
	save.DrawBackground();
	delBtn.DrawBackground();
	status.DrawBackground();
}

bool CMenuSubAdvanced::Keydown(int32 key)
{
	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.Keydown(key))
		{
			ConfirmFinish();
			return true;
		}
/*
		switch(key)
		{
		case K_ESCAPE:
		case 'n':
			DoDelete(DEL_ABORT);
			return true;
		case 'y':
			DoDelete(DEL_DEL);
			return true;
		}
*/
	}

	if (file_field.Keydown(key)) return true;
	if (load.Keydown(key)) return true;
	if (save.Keydown(key)) return true;

	if (files.Keydown(key))
	{
		RetrieveData();
		return true;
	}

	switch (key)
	{
    case '`' :
    case K_ESCAPE:
		//leave the menu.
		//DKM_EnterMain();
		if( Cvar_VariableValue( "console" ) == 0 )
		{
			return false;
		}
		DKM_Shutdown();       // exit entire interface  1.10 dsn
		return true;
/*	case 'l':
		DoLoad();
		return true;
	case 's':
		DoSave();
		return true;
	case 'd':
		DoDelete();
		return true;*/
	}


	return false;
}

bool CMenuSubAdvanced::RetrieveData()
{
	int32 sel = files.Selection();
	if (sel != curSelection)
	{
		if (files.Selection(cfg_file,NULL,NULL,NULL))
		{
			file_field.SetText( cfg_file );
			curSelection = sel;
			return true;
		}
	}

	return false;
}

bool CMenuSubAdvanced::MousePos(int32 norm_x, int32 norm_y)
{
	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MousePos(norm_x,norm_y)) return true;
	}
	else
	{
		if (files.MousePos(norm_x,norm_y))
		{
			RetrieveData();
			return true;
		}

		if (load.MousePos(norm_x,norm_y)) return true;
		if (save.MousePos(norm_x,norm_y)) return true;
		if (delBtn.MousePos(norm_x,norm_y)) return true;
		if (file_field.MousePos(norm_x,norm_y)) return true;
	}
	return false;
}


bool CMenuSubAdvanced::MouseDown(int32 norm_x, int32 norm_y)
{
	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MouseDown(norm_x,norm_y)) return true;
	}
	else
	{
		if (files.MouseDown(norm_x,norm_y))
		{
			RetrieveData();
			return true;
		}

		if (load.MouseDown(norm_x,norm_y)) return true;
		if (save.MouseDown(norm_x,norm_y)) return true;
		if (file_field.MouseDown(norm_x,norm_y)) return true;
	}
	return false;
}

bool CMenuSubAdvanced::ConfirmFinish()
{
	if (confirm_dlg.Showing())
	{
		bool yes = false;
		if (confirm_dlg.Result(yes))
		{
			if (yes) DoDelete(DEL_DEL);
			else DoDelete(DEL_ABORT);
			return true;
		}
	}

	return false;
}


bool CMenuSubAdvanced::MouseUp(int32 norm_x, int32 norm_y)
{
	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MouseUp(norm_x,norm_y))
		{
			ConfirmFinish();
/*
			bool yes = false;
			if (confirm_dlg.Result(yes))
			{
				if (yes) DoDelete(DEL_DEL);
				else DoDelete(DEL_ABORT);
			}*/
			return true;
		}
	}
	else
	{
		if (files.MouseUp(norm_x,norm_y))
		{
			RetrieveData();
			return true;
		}

		if (load.MouseUp(norm_x,norm_y))
		{
			DoLoad();
			return true;
		}

		if (save.MouseUp(norm_x,norm_y))
		{	
			DoSave();
			return true;
		}

		if (delBtn.MousePos(norm_x,norm_y))
		{
			DoDelete();
			return true;
		}

		if (file_field.MouseUp(norm_x,norm_y)) return true;
	}
	return false;
}
void DKM_LoadVideoSettings(bool setDefaults);
void DKM_UpdateVideoSettings();

void CMenuSubAdvanced::DoLoad()
{
	// exec the selected config.
	char newCfg[MAX_QPATH];
	Com_sprintf (newCfg, sizeof(newCfg),"exec \"%s.cfg\"\n", cfg_file);
	Com_Printf ("Executed %s.cfg.\n", cfg_file);

	// set the new current config.  (special stuff for 'special configs')
	bool bAllow = SPECIAL_CONFIG(cfg_file);

	if (!bAllow)
	{
		Cvar_FullSet("currentconfig","current",0);
		CL_SaveLastConfiguration();
		Cbuf_AddText (newCfg);
		Cbuf_AddText ("cl_writeconfig current\n");
	}
	else
	{
		Cvar_FullSet("currentconfig",cfg_file,0);
		CL_SaveLastConfiguration();
		Cbuf_AddText (newCfg);
		Com_sprintf(newCfg,sizeof(newCfg),"cl_writeconfig %s\n",cfg_file);
		Cbuf_AddText (newCfg);
	}

	if (in_main_game())
	{
	    S_StartLocalSound(DKM_sounds[DKMS_BUTTON7]);
		DKM_Shutdown();
		return;
	}
	else
	{
		Cbuf_Execute();
	}
	
	DKM_LoadVideoSettings(false);
	DKM_UpdateVideoSettings();
	Com_sprintf (newCfg, sizeof(newCfg),"%s %s",tongue_menu[T_MENU_LOADED],cfg_file);
	status.SetText(newCfg,STATUS_DURATION);
    S_StartLocalSound(DKM_sounds[DKMS_BUTTON7]);
}

void CMenuSubAdvanced::DoSave()
{
	char newCfg[MAX_QPATH];
	strcpy(newCfg,file_field.GetText());

	// make sure it's not curcfg!
	bool bAllow = SPECIAL_CONFIG(newCfg);
	if (!bAllow)
		return;

    S_StartLocalSound(DKM_sounds[DKMS_BUTTON7]);
	Cvar_FullSet("currentconfig",newCfg,0);
	CL_WriteConfiguration(newCfg);
	CL_SaveLastConfiguration();
	files.Refresh();
	files.Select(cl_curconfig->string,true);
	RetrieveData();

	Com_sprintf (newCfg, sizeof(newCfg),"%s %s",tongue_menu[T_MENU_SAVED],cl_curconfig->string);
	status.SetText(newCfg,STATUS_DURATION);
}

void Sys_Delete(char *path);

void CMenuSubAdvanced::DoDelete(short type)
{
	switch(type)
	{
	case DEL_START:
		{
			// can't delete daikatana.cfg from here...
			bool bAllow = SPECIAL_CONFIG(cfg_file);//((stricmp(cfg_file,"daikatana")) && (stricmp(cfg_file,"curcfg")) && (stricmp(cfg_file,"default_keys"))  && (stricmp(cfg_file,"autoexec")));
			if (!bAllow)
				return;

			S_StartLocalSound(DKM_sounds[DKMS_BUTTON5]);
			confirm_dlg.Show(tongue_menu[T_MENU_DELETE_SELECTION],NULL);
		}
		return;
	case DEL_DEL:
		{
			// we're deleting our current config.  Set current to daikatana.
			if (!stricmp(cfg_file,cl_curconfig->string))
			{
				Cvar_FullSet("currentconfig","daikatana",0);
			}
			char newCfg[MAX_OSPATH];
			char delCfg[64];
			Com_sprintf(delCfg,sizeof(newCfg),"%s %s",tongue_menu[T_MENU_DELETED],cfg_file);

			Com_sprintf(newCfg,sizeof(newCfg),"%s/%s.cfg",FS_ConfigDir(),cfg_file);
			Sys_Delete(newCfg);
			files.Refresh();
			files.Select(cl_curconfig->string,true);
			RetrieveData();
			status.SetText(delCfg,STATUS_DURATION);
		    S_StartLocalSound(DKM_sounds[DKMS_BUTTON7]);
		}

	case DEL_ABORT:			// yes, I mean this to happen
	default:
		return;
	}
}

