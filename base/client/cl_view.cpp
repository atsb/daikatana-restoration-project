// cl_view.c -- player rendering positioning

#include "client.h"
#include "daikatana.h"
#include "dk_gce_main.h"
#include "dk_beams.h"
#include "cl_inventory.h"
#include "cl_icons.h"
#include "dk_screen_coord.h"
#include "csv.h"

#ifdef _DEBUG
#include "windows.h"
#endif

//=============
//
// development tools for weapons
//
int gun_frame;
void *gun_model;

//=============

extern	bspModel_t	bspModel;
extern int scr_draw_loading;
extern int scr_draw_progress;

autoAim_t autoAim={0,0,NULL,0};

///////////////////////////////////////////////////////////////////////////////
//	for hierarchical entities
///////////////////////////////////////////////////////////////////////////////

//#define	MAX_HR_ENTITIES	128

//typedef	struct	hr_EntList_s
//{
//	entity_t			*ent;
//	centity_t			*cl_ent;
//	entity_state_t		*state;
//	unsigned	short	flags;
//} hr_EntList_t;

//hr_EntList_t	hr_EntList	[MAX_HR_ENTITIES];
//static	int		hr_EntCount;


// NSS[10/2/00]:Added for level delay OEM and DEMO
typedef struct Level_Delay_s
{
	float	delay;				//How long... in mili-seconds	
	int		type;				//What kind of Level Delay (Demo/OEM/Etc)
	bool	explored;			//True/False no delay if True
	char	level_name[32];		//map name
}Level_Delay_t;

#define No_Level_Delay		0
#define Demo_Level_Delay	1
#define OEM_Level_Delay		2

Level_Delay_t Level_Delay[]=
{
	{20000,Demo_Level_Delay,FALSE,"e1m1a"},
	{20000,Demo_Level_Delay,FALSE,"e2m2a"},
	{20000,Demo_Level_Delay,FALSE,"e3m2a"},
	{20000,OEM_Level_Delay,FALSE,"e1m2a"},
	{20000,OEM_Level_Delay,FALSE,"e2m1a"},
	{20000,OEM_Level_Delay,FALSE,"e3m1a"},
	{20000,OEM_Level_Delay,FALSE,"e4m1a"},
	{0,0,FALSE,"ENDITNOEL"}
};


///////////////////////////////////////////////////////////////////////////////

cvar_t      *cv_autoaim;
cvar_t		*cv_crosshair;
cvar_t		*cv_crosshair_enabled;
cvar_t		*cl_testparticles;
cvar_t		*cl_testentities;
cvar_t		*cl_numtestentities;
cvar_t		*cl_testentitymodel;
cvar_t		*cl_testlights;
cvar_t		*cl_testblend;

cvar_t		*cl_stats;

cvar_t		*cl_framefix;	//	Nelno:	for testing client-side animation

//	Nelno:	for keeping track of the number of flares
cvar_t		*cl_flarecount;
cvar_t		*cl_showflares;
cvar_t		*cl_particlevol;

cvar_t      *cl_bob;

int			r_numdlights;
dlight_t	r_dlights[MAX_DLIGHTS];

int			r_numentities;
entity_t	r_entities[MAX_ENTITIES];

int			r_numparticles;
particle_t	r_particles[MAX_PARTICLES];

lightstyle_t	r_lightstyles[MAX_LIGHTSTYLES];

int			r_numflares;					//  Shawn:  Added for Daikatana
flare_t		r_flares[MAX_FLARES];			//  Shawn:  Added for Daikatana

int			r_numcomparticles;				//  Shawn:  Added for Daikatana
particle_t	r_comparticles[MAX_PARTICLES];	//  Shawn:  Added for Daikatana

int         r_numdhcomparticles;
particle_t  r_dhcomparticles[MAX_DHPARTICLES];

int					r_numparticleVolumes;						// number of particle volumes
particleVolume_t	r_particleVolumes[MAX_PARTICLEVOLUMES];		// array of particle volume records

foginfo_t	r_foginfo;						//  Shawn:  Added for Daikatana

int				r_numsortentities;
sortentity_t	r_sortentities[MAX_ENTITIES];

/*
====================
V_ClearScene

Specifies the model that will be used as the world
====================
*/
void V_ClearScene (void)
{
	r_numdlights = 0;
	r_numentities = 0;
	r_numparticles = 0;
	r_numflares = 0;		//  Shawn:  Added for Daikatana
	r_numcomparticles = 0;	//  Shawn:  Added for Daikatana
	r_numdhcomparticles = 0;
//	hr_EntCount = 0;
	r_numsortentities = 0;	

	//	clear all beam structures for this frame
	beam_InitFrame ();
}

extern "C" int V_CompareDistance( const void *ent1, const void *ent2 );
int V_CompareDistance( const void *ent1, const void *ent2 )
{
	float dist1 = ( ( sortentity_t * ) ent1 )->distance;
	float dist2 = ( ( sortentity_t * ) ent2 )->distance;

	// SCG[3/29/99]: Objects furthest away from the camera are drawn first (back to front)
	if( dist1 > dist2 )
	{
		return -1;
	}
	if( dist1 < dist2 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
=====================
V_SortEntities
=====================
*/
void V_SortEntities()
{
	// SCG[3/29/99]: Get entity distance from viewpoint
	for( int count = 0; count < r_numsortentities; count++ )
	{
		r_sortentities[count].distance = 
			VectorDistance2( cl.refdef.vieworg, r_sortentities[count].ent.origin );
	}

	// SCG[3/29/99]: Sort the entity list
	qsort( r_sortentities, r_numsortentities, sizeof( sortentity_t ), V_CompareDistance ) ;

	// SCG[1/7/00]: Copy to the main entity list
	for( int i = 0; i < r_numsortentities; i++ )
	{
		r_entities[r_numentities++] = r_sortentities[i].ent;
	}
/*
	// SCG[3/29/99]: Get entity distance from viewpoint
	for( int count = 0; count < r_numentities; count++ )
	{
		r_entities[count].distance = 
			VectorDistance2( cl.refdef.vieworg, r_entities[count].origin );
	}

	// SCG[3/29/99]: Sort the entity list
	qsort( r_entities, r_numentities, sizeof( entity_t ), V_CompareDistance ) ;
*/

}

/*
=====================
V_AddEntity

=====================
*/
void V_AddEntity (entity_t *ent)
{
	if (r_numentities >= MAX_ENTITIES)
		return;

    if (CIN_Running() == 1 && (ent->flags & RF_DEPTHHACK) != 0) return;

//	if( ( ent->flags & RF_TRANSLUCENT ) && ( ent->alpha < 1.0 ) )
	{
		r_sortentities[r_numsortentities].ent = *ent;
		r_numsortentities++;
	}
/*
	else
	{
		r_entities[r_numentities] = *ent;
		r_numentities++;
	}
*/
}

entity_t *V_GetEntity() {
    if (r_numentities >= MAX_ENTITIES) return NULL;

    //increment our count.
    r_numentities++;

    //return the last entity.
    return &r_entities[r_numentities - 1];
}


/*
=====================
V_AddParticle

=====================
*/
void V_AddParticle ( CVector &org, int color, float alpha, particle_type_t type, float fScale)
{
	particle_t	*p;

	if (r_numparticles >= MAX_PARTICLES)
		return;
	p = &r_particles[r_numparticles++];
	p->origin = org;
	p->color = color;
	p->alpha = alpha;
	p->type = type;
    p->pscale = fScale;
}

void V_AddParticleRGB( CVector &org, int color, float alpha, particle_type_t type, CVector & color_rgb, float fScale)
{
	particle_t	*p;

	if (r_numparticles >= MAX_PARTICLES)
		return;
	p = &r_particles[r_numparticles++];
	p->origin = org;
	p->color = color;
	p->alpha = alpha;
	p->type = type;
    p->color_rgb = color_rgb;
    p->pscale = fScale;
}

/*
=====================
V_AddComParticle

=====================
*/
void V_AddComParticle ( CVector &org, int color, float alpha, particle_type_t type, 
			float pscale, short flags, CVector &color_rgb)
{
	particle_t	*p;

   if (flags & PF_DEPTHHACK)
   {
   	if (r_numdhcomparticles >= MAX_DHPARTICLES)
	   	return;
   	p = &r_dhcomparticles[r_numdhcomparticles++];
   }
   else
   {
   	if (r_numcomparticles >= MAX_PARTICLES)
	   	return;
   	p = &r_comparticles[r_numcomparticles++];
   }

	p->origin = org;
	p->color = color;
	p->alpha = alpha;
	p->type = type;
	p->pscale=pscale;
	p->flags=flags;
	p->color_rgb = color_rgb;
}

void V_AddComParticle2 (cparticle_t *cp)
{
	particle_t	*p;

	if (cp->flags & PF_DEPTHHACK)
	{
		if (r_numdhcomparticles >= MAX_DHPARTICLES)
			return;
		p = &r_dhcomparticles[r_numdhcomparticles++];
	}
	else
	{
		if (r_numcomparticles >= MAX_PARTICLES)
			return;
		p = &r_comparticles[r_numcomparticles++];
	}

	p->origin = cp->org;
//	p->color = cp->color;
	p->alpha = cp->alpha;
	p->type = (particle_type_t)cp->type;
	p->pscale= cp->pscale;
	p->flags= cp->flags;
	p->color_rgb = cp->color_rgb;
}


/*
=====================
V_AddLight

=====================
*/
void V_AddLight ( CVector &org, float intensity, float r, float g, float b)
{
	dlight_t	*dl;

	if (r_numdlights >= MAX_DLIGHTS)
		return;
	dl = &r_dlights[r_numdlights++];
	dl->origin = org;
	dl->intensity = intensity;
	dl->color.x = r;
	dl->color.y = g;
	dl->color.z = b;
}


/*
=====================
V_AddLightStyle

=====================
*/
void V_AddLightStyle (int style, float r, float g, float b)
{
	lightstyle_t	*ls;

	if (style < 0 || style > MAX_LIGHTSTYLES)
		Com_Error (ERR_DROP, "Bad light style %i", style);
	ls = &r_lightstyles[style];

	ls->white = r+g+b;
	ls->rgb.x = r;
	ls->rgb.y = g;
	ls->rgb.z = b;
}

/*
=====================
V_AddFlare
=====================
*/
void V_AddFlare ( CVector &org, CVector &ang, float intensity, float falloff, float r, float g, float b)
{
	flare_t	*f;

	if (r_numflares >= MAX_FLARES)
		return;
	f = &r_flares[r_numflares++];
	f->origin = org;
	f->angles = ang;
	f->intensity = intensity;
	f->falloff = falloff;
	f->color.x = r;
	f->color.y = g;
	f->color.z = b;
}

/*
================
V_TestParticles

If cl_testparticles is set, create 4096 particles in the view
================
*/
void V_TestParticles (void)
{
	particle_t	*p;
	int			i;
	float		d, r, u;

	r_numparticles = MAX_PARTICLES;
	for (i=0 ; i<r_numparticles ; i++)
	{
		d = i*0.25;
		r = 4*((i&7)-3.5);
		u = 4*(((i>>3)&7)-3.5);
		p = &r_particles[i];

		p->origin.x = cl.refdef.vieworg.x + cl.v_forward.x*d + cl.v_right.x*r + cl.v_up.x*u;
		p->origin.y = cl.refdef.vieworg.y + cl.v_forward.y*d + cl.v_right.y*r + cl.v_up.y*u;
		p->origin.z = cl.refdef.vieworg.z + cl.v_forward.z*d + cl.v_right.z*r + cl.v_up.z*u;

		p->color = 8;
		p->alpha = cl_testparticles->value;
	}
}

/*
================
V_TestEntities

If cl_testentities is set, create 32 player models
================
*/
void V_TestEntities (void)
{
	int			i;
	float		f, r;
	entity_t	*ent;
	void		*model;
	char		modelpath[128];

//	r_numentities = 32;
	r_numentities = (int)cl_numtestentities->value;
	memset (r_entities, 0, sizeof(r_entities));

//	sprintf( modelpath, "models/%s.dkm", cl_testentitymodel->string );
	Com_sprintf( modelpath, sizeof(modelpath), "models/%s.dkm", cl_testentitymodel->string );

	model = re.RegisterModel(modelpath, RESOURCE_GLOBAL);

	for (i=0 ; i<r_numentities ; i++)
	{
		ent = &r_entities[i];
		
		ent->render_scale[0] = ent->render_scale[1] = ent->render_scale[2] = 1;

		r = 64 * ( (i%4) - 1.5 );
		f = 64 * (i/4) + 128;

		ent->origin.x = cl.refdef.vieworg.x + cl.v_forward.x*f + cl.v_right.x*r;
		ent->origin.y = cl.refdef.vieworg.y + cl.v_forward.y*f + cl.v_right.y*r;
		ent->origin.z = cl.refdef.vieworg.z + cl.v_forward.z*f + cl.v_right.z*r;

		//ent->model = cl.baseclientinfo.model;
		ent->model = model;
		//ent->skin = cl.baseclientinfo.skin;
	}
}

/*
================
V_TestLights

If cl_testlights is set, create 32 lights models
================
*/
void V_TestLights (void)
{
	int			i;
	float		f, r;
	dlight_t	*dl;

	r_numdlights = 32;
	memset (r_dlights, 0, sizeof(r_dlights));

	for (i=0 ; i<r_numdlights ; i++)
	{
		dl = &r_dlights[i];

		r = 64 * ( (i%4) - 1.5 );
		f = 64 * (i/4) + 128;

		dl->origin.x = cl.refdef.vieworg.x + cl.v_forward.x*f + cl.v_right.x*r;
		dl->origin.y = cl.refdef.vieworg.y + cl.v_forward.y*f + cl.v_right.y*r;
		dl->origin.z = cl.refdef.vieworg.z + cl.v_forward.z*f + cl.v_right.z*r;

		dl->color.x = ((i%6)+1) & 1;
		dl->color.y = (((i%6)+1) & 2)>>1;
		dl->color.z = (((i%6)+1) & 4)>>2;
		dl->intensity = 200;
	}
}

//===================================================================

///////////////////////////////////////////////////////////////////////////////
//	CL_FindIndex
//
//	Find the index for a name in cl.configstrings
///////////////////////////////////////////////////////////////////////////////

int CL_FindIndex (char *name, int start, int max)
{
	int		i;
	
	if (!name || !name[0])
		return 0;
	
	for (i = 1 ; i < max && cl.configstrings [start + i][0] ; i++)
	{
		if (!stricmp (cl.configstrings [start + i], name))
			return	i;
	}
	
	return	false;
}

int CL_ModelIndex (char *name)
{
	return CL_FindIndex (name, CS_MODELS, MAX_MODELS);
}

/*
=================
CL_PrepRefresh

Call before entering a new level, or after changing dlls
=================
*/
qboolean bGotLoadscreenName;

void CL_InitWeaponIcons();
void SCR_SetConnecting(qboolean bON);
void SCR_ForceEndLoading();

#define MAPNAME_SIZE	32
void CL_PrepRefresh (void)
{
	char		mapname[MAPNAME_SIZE];
	int			i;
	char		name[MAX_QPATH];
	char		rpath [MAX_QPATH];
	int			bsptime, files;
	long		bytes;

	if (!cl.configstrings[CS_MODELS+1][0])
		return;		// no map loaded

	SCR_SetConnecting(FALSE);
	SCR_AddDirtyPoint (0, 0);
	SCR_AddDirtyPoint (viddef.width-1, viddef.height-1);

	// let the render dll load the map
	char *pString = cl.configstrings [CS_MODELS + 1];
	pString += 5;

//	strcpy (mapname, cl.configstrings [CS_MODELS + 1] + 5);	// skip "maps/"
	Com_sprintf( mapname, MAPNAME_SIZE, "%s", pString );
	mapname[strlen (mapname) - 4] = 0;		// cut off ".bsp"

	//if (cl_episode_num == cl_last_episode_num && stricmp (cl_mapname, mapname))
		//	same episode, so only purge resources if the map has changed!
		re.PurgeResources(RESOURCE_LEVEL);
//	else 
		if (cl_episode_num != cl_last_episode_num)
		re.PurgeResources(RESOURCE_EPISODE);
   
	Cvar_ForceSet( "cl_lastmapname", cl_mapname );

	strcpy (cl_mapname, mapname);

	bGotLoadscreenName = TRUE;

	//	Nelno:	set resource path for Daikatana
//	sprintf (rpath, "textures/%s/", mapname);
	Com_sprintf (rpath, sizeof(rpath), "textures/%s/", mapname);
	dk_SetResourcePath (rpath);

	// start bsp timing
	files = FS_GetFilesOpened();
	bytes = FS_GetBytesRead();
	bsptime = Sys_Milliseconds();

	// register models, pics, and skins
	Com_Printf ("Map: %s\r", mapname); 
	SCR_UpdateScreen ();
	re.BeginRegistration (mapname);
	Com_Printf ("                                     \r");

	// end bsp timing 
	load_info.bsp_time = Sys_Milliseconds() - bsptime;
	load_info.bsp_files = FS_GetFilesOpened() - files;
	load_info.bsp_bytes = FS_GetBytesRead() - bytes;

	// precache status bar pics
	Com_Printf ("pics\r"); 
	SCR_UpdateScreen ();
	SCR_RegisterCrosshair ();
	SCR_RegisterStatus ();
	Com_Printf ("                                     \r");

	// Set default fog info (default = off)
	CL_SetFogInfo (0, 0, 0, 0, 0, 0, 0, 0);

	// start model timing
	files = FS_GetFilesOpened();
	bytes = FS_GetBytesRead();
	bsptime = Sys_Milliseconds();	
	CL_RegisterTEntModels ();

// RE:[AJR]04/18/2000: Thanks Noel, except this happens every time you go from B back to A.  Thanks.
// NSS[10/2/00]:No problem, just re-wrote the code to be a little more modular and also a little more efficient
int Transition_Type = No_Level_Delay;

#ifdef DAIKATANA_DEMO
	Transition_Type = Demo_Level_Delay;
#else
	#ifdef DAIKATANA_OEM
		Transition_Type = OEM_Level_Delay;
	#endif	
#endif	
	
if(Transition_Type > No_Level_Delay)
{
	int Delay_Scan = 0;
	while(_stricmp("ENDITNOEL",Level_Delay[Delay_Scan].level_name))
	{
		if(Level_Delay[Delay_Scan].type == Transition_Type)
		{
			if(!_stricmp(mapname,Level_Delay[Delay_Scan].level_name) && !Level_Delay[Delay_Scan].explored )
			{
				int nStartReadTime = Sys_Milliseconds();
				int nCurrentReadTime;
				while( 1 )
				{
					nCurrentReadTime = Sys_Milliseconds();
					if( ( nCurrentReadTime - nStartReadTime ) >= Level_Delay[Delay_Scan].delay )
					{
						break;
					}
				}	
				Level_Delay[Delay_Scan].explored = TRUE;
				break;
			}
		}
		Delay_Scan++;
	}
}
	
/*
// SCG[4/14/00]: Romero wants a delay
#ifdef DAIKATANA_DEMO
	int nStartReadTime = Sys_Milliseconds();
	int nCurrentReadTime;
	static int e1m1a_visited = 0;
	static int e2m2a_visited = 0;
	static int e3m2a_visited = 0;

	//[NSS]04/18/2000: Only delay on the first map of each demo episode.
	//[AJR]04/18/2000: Thanks Noel, except this happens every time you go from B back to A.  Thanks.

	if ((!_stricmp(mapname, "e1m1a") && !e1m1a_visited) ||
		(!_stricmp(mapname, "e2m2a") && !e2m2a_visited) ||
		(!_stricmp(mapname, "e3m2a") && !e3m2a_visited))
//	if(_stricmp(mapname,"e1m1a") == 0 || _stricmp(mapname,"e2m2a") == 0 || _stricmp(mapname,"e3m2a") == 0)
	{
		while( 1 )
		{
			nCurrentReadTime = Sys_Milliseconds();
			if( ( nCurrentReadTime - nStartReadTime ) >= 20000 )
			{
				break;
			}
		}

		//[AJR]04/18/2000: Set value if we've already waited
		switch(mapname[1])
		{
			case '1': e1m1a_visited = 1; break;
			case '2': e2m2a_visited = 1; break;
			case '3': e3m2a_visited = 1; break;
		}
	}
#endif
*/


	progress_index = 0;
	for (i = 1; i < MAX_MODELS; i++)
	{
		progress_index++; // increment progress bar
		if (cl.configstrings[CS_MODELS+i][0]) // entry exists?
		{
			strcpy (name, cl.configstrings[CS_MODELS+i]);
			
			name[37] = 0;	// never go beyond one line
			if (name[0] != '*')
				Com_Printf ("%s\r", name); 
			
			cl.model_draw[i] = re.RegisterModel (cl.configstrings[CS_MODELS+i], RESOURCE_EPISODE);
			if (name[0] == '*')
				cl.model_clip[i] = CM_InlineModel (cl.configstrings[CS_MODELS+i]);
			else
				cl.model_clip[i] = NULL;
			
			if (name[0] != '*')
				Com_Printf ("                                     \r");
			
			bspModel.fixModelClip = false;
			
			SCR_UpdateScreen ();
			Sys_SendKeyEvents ();	// pump message loop
		}
		else
		{
// SCG[12/14/99]: Don't try to be smart, Dustin.  You're a fuckhead...
// SCG[12/14/99]: 			if (!(i % 4)) // don't update as often
			{
				SCR_UpdateScreen ();
				Sys_SendKeyEvents ();	// pump message loop
			}
		}
	}
	// SCG[11/8/99]: Init weapon model icons
	CL_InitWeaponIcons();

	load_info.models_time = Sys_Milliseconds() - bsptime;
	load_info.models_bytes = FS_GetBytesRead() - bytes;
	load_info.models_files = FS_GetFilesOpened() - files;

	files = FS_GetFilesOpened();
	bytes = FS_GetBytesRead(); 
	bsptime = Sys_Milliseconds();	
	Com_Printf ("images\r", i); 
	SCR_UpdateScreen ();

	for (i=1;i < MAX_IMAGES && cl.configstrings[CS_IMAGES+i][0]; i++)
	{
		cl.image_precache[i] = re.RegisterPic (cl.configstrings[CS_IMAGES+i], NULL, NULL, RESOURCE_EPISODE);
		
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();	// pump message loop
	}

	Com_Printf ("                                     \r");

	load_info.images_time = Sys_Milliseconds() - bsptime;
	load_info.images_files = FS_GetFilesOpened() - files;
	load_info.images_bytes = FS_GetBytesRead() - bytes;

	files = FS_GetFilesOpened();
	bytes = FS_GetBytesRead();
	bsptime = Sys_Milliseconds();	

	for (i=0 ; i<MAX_CLIENTS ; i++)
	{
		if (!cl.configstrings[CS_PLAYERSKINS+i][0])
			continue;

		Com_Printf ("client %i\r", i); 
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();	// pump message loop

		CL_ParseClientinfo (i);
		Com_Printf ("                                     \r");
	}

	load_info.clients_time = Sys_Milliseconds() - bsptime;
	load_info.clients_bytes = FS_GetBytesRead() - bytes;
	load_info.clients_files = FS_GetFilesOpened() - files;

	// set sky textures and speed
	Com_Printf ("sky\r", i); 
	SCR_UpdateScreen ();
	re.SetSky (cl.configstrings[CS_SKY], cl.configstrings[CS_CLOUDNAME]);
	Com_Printf ("                                     \r");

	// the renderer can now free unneeded stuff
	re.EndRegistration ();

	// clear any lines of console text
	Con_ClearNotify ();

	SCR_UpdateScreen ();
	cl.refresh_prepped = true;
	cl.force_refdef = true;	// make sure we have a valid refdef

	// start the cd track
// SCG[10/27/99]: We're not doing cd, do we need this?!?!?
//	CDAudio_Play (atoi(cl.configstrings[CS_CDTRACK]), true);

	//  start music for the level
	CCSVFile *pCsvFile = NULL;
	int nRetCode;
	char szLine[2048];
	char szElement[64];

	Com_Printf("Starting Music\n");

	SCR_UpdateScreen ();

	nRetCode = CSV_OpenFile("music/music.csv", &pCsvFile);
	if (!nRetCode)
	{
		while (CSV_GetNextLine(pCsvFile,szLine) != EOF)
		{
			// get level name
			CSV_GetFirstElement(pCsvFile, szLine, szElement);

			if (!_stricmp(szElement,mapname))
			{
				// get the music file name
				CSV_GetNextElement(pCsvFile, szLine, szElement);
				// play it
				S_StartMusic(szElement,CHAN_MUSIC_MAP);
				break;
			}
		}

		CSV_CloseFile( pCsvFile );
	}

//	SCR_EndLoadingPlaque ();
	SCR_ForceEndLoading();

}

/*
====================
CalcFov
====================
*/
float CalcFov (float fov_x, float width, float height)
{
	float	a;
	float	x;

	if (fov_x < 1 || fov_x > 179)
		fov_x = 90.0;
//		Com_Error (ERR_DROP, "Bad fov: %f", fov_x);

	x = width/tan(fov_x/360*M_PI);

	a = atan (height/x);

	a = a*360/M_PI;

	return a;
}

//============================================================================

// gun frame debugging functions
/*
void V_Gun_Next_f (void)
{
	gun_frame++;
	Com_Printf ("frame %i\n", gun_frame);
}

void V_Gun_Prev_f (void)
{
	gun_frame--;
	if (gun_frame < 0)
		gun_frame = 0;
	Com_Printf ("frame %i\n", gun_frame);
}

void V_Gun_Model_f (void)
{
	char	name[MAX_QPATH];

	if (GetArgc() != 2)
	{
		gun_model = NULL;
		return;
	}
	Com_sprintf (name, sizeof(name), "models/%s/tris.dkm", GetArgv(1));
	gun_model = re.RegisterModel (name, RESOURCE_LEVEL);
}
*/
//============================================================================

char display_name[64];
int display_name_frame_count;
int display_ent_number;
int display_name_x; // save extra calculations
int display_name_y;

void* aim_font = NULL;

//---------------------------------------------------------------------------
// V_CheckAutoAim()
//---------------------------------------------------------------------------
void V_CheckAutoAim()
{
   #define AIM_NUMRAYS  5

   entity_state_t *ent;
   trace_t trace;
   CVector start,end,angles,min,max,shootpos,newstart;
   CVector min1(4,4,4);
   CVector max1(-4,-4,-4);
//   short i,numrays,count;
   short numrays;
   float xofs[AIM_NUMRAYS]={0, 1, 0, -1,  0};
   float yofs[AIM_NUMRAYS]={0, 0, 1,  0, -1};

   // SCG[1/21/00]: lets initialize things!
   memset(&trace, 0, sizeof(trace_t));

   // do autoaim?
   if (cv_autoaim->value)
      numrays=AIM_NUMRAYS;
   else
      numrays=1;

   // set trace size
   int autoVal = (cv_autoaim->value) ? 4 : 0;
   min.x=min.y=min.z=(cv_autoaim->value) ? -10 : 0;
   max.x=max.y=max.z=(cv_autoaim->value) ? 10 : 0;

   angles = cl.refdef.viewangles;

   start = cl.refdef.vieworg;
   VectorMA(start, cl.v_forward, 2000, end);
   trace = CL_TraceBox(start,min,max,end,cl.playernum+1, MASK_SHOT);// SCG[1/25/00]: use mask shot, since it is for aiming a weapon...
   ent=(entity_state_t *)trace.ent;
   if (ent != (entity_state_t *)0x00000001 && ent && !trace.startsolid && (ent->flags & SFL_TARGETABLE) && trace.fraction < 1)
   {
		centity_t *cent;
		cent=&cl_entities[ent->number];
		shootpos = cent->lerp_origin;
		shootpos.z += cent->current.maxs.z * 0.25;
   }
   else
   {
		trace.ent = NULL;
   }
/*
   for (i=0; i<numrays; i++)
   {
      start = cl.refdef.vieworg;
      VectorMA(start, cl.v_right, xofs[i]*autoVal/2, start);
      VectorMA(start, cl.v_up, yofs[i]*autoVal/2, start);
      start.z += 12;
      VectorMA(start, cl.v_forward, 2000, end);

      count=0;
      newstart = start;
      do {
         // first trace is for monsters (ignores world)
//         trace = CL_TraceBox(newstart,min,max,end,cl.playernum+1,CONTENTS_MONSTERCLIP|CONTENTS_MONSTER|CONTENTS_PLAYERCLIP);
         trace = CL_TraceBox(newstart,min,max,end,cl.playernum+1, MASK_SHOT);// SCG[1/25/00]: use mask shot, since it is for aiming a weapon...
         ent=(entity_state_t *)trace.ent;
         VectorMA(trace.endpos, cl.v_forward, 1, newstart);
         count++;
      } while ((trace.fraction < 1) && (ent != (entity_state_t *)0x00000001) && (!(ent->flags & SFL_TARGETABLE)) && count<4);
//      } while ((trace.fraction < 1) && (ent != NULL) && (!(ent->flags & SFL_TARGETABLE)) && count<4);

      if (count > 1)
         trace.startsolid=0;

      if (ent != (entity_state_t *)0x00000001 && ent && !trace.startsolid && (ent->flags & SFL_TARGETABLE) && trace.fraction < 1)
//      if (ent != NULL && ent && !trace.startsolid && (ent->flags & SFL_TARGETABLE) && trace.fraction < 1)
      {
         centity_t *cent;
         entity_state_t *oldent;

         oldent=(entity_state_t *)trace.ent;
         cent=&cl_entities[oldent->number];
         shootpos = cent->lerp_origin;
         shootpos.z += 12;

         count=0;
         newstart = start;
         do {
            // second trace is for world, traces to monster with smaller bounding box
//            trace = CL_TraceBox(newstart,min1,max1,shootpos,cl.playernum+1,CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_MONSTER|CONTENTS_PLAYERCLIP);
            trace = CL_TraceBox(newstart,min1,max1,shootpos,cl.playernum+1,MASK_SHOT);// SCG[1/25/00]: use mask shot, since it is for aiming a weapon...
            ent=(entity_state_t *)trace.ent;
            VectorMA(trace.endpos, cl.v_forward, 1, newstart);
            count++;
         } while (trace.fraction < 1 && (ent != (entity_state_t *)0x00000001) && (ent != oldent) && (!(ent->flags & SFL_TARGETABLE)) && count<4);
//         } while (trace.fraction < 1 && (ent != NULL) && (ent != oldent) && (!(ent->flags & SFL_TARGETABLE)) && count<4);
         if (oldent==ent)
            break;
         else
            trace.ent=NULL;
      }
      else
         trace.ent=NULL;
   }
*/
   // for now, clear this
   autoAim.item=0;

   // set autoAim info
   if (trace.ent == (struct edict_s *)0x00000001 || !trace.ent || trace.startsolid)
//   if ( !trace.ent || trace.startsolid)
   {
      autoAim.xofs=0;
      autoAim.yofs=0;
      autoAim.ent=NULL;
   }
   else
   {
      int sx,sy;

      // only alter crosshair position if autoaim is active (non-zero)
      if (cv_autoaim->value)
      {
         ComputeScreenCoordinates(cl.refdef.width,cl.refdef.height,cl.refdef.x,cl.refdef.y,
                                 cl.refdef.fov_x,cl.refdef.fov_y,cl.refdef.vieworg,cl.refdef.viewangles,
                                 shootpos,&sx,&sy);

		 sx -= cl.refdef.x;sy -= cl.refdef.y;

         autoAim.xofs=sx - cl.refdef.width/2;
         autoAim.yofs=sy - cl.refdef.height/2;
      }

      autoAim.ent=(entity_state_t *)trace.ent;
   }
}

void V_PlayerID()
{
	// no player id in coop!
	if( cl.refdef.rdflags & RDF_COOP )
		return;

	// load the font up
	if (!aim_font)
	{
		aim_font = re.LoadFont ("scr_cent");
	}

	CVector start,end,min1,max1;
	trace_t trace;
	entity_state_t *ent;
	DRAWSTRUCT drawStruct;
	// cek[2-11-00]: make the detection box a bit bigger for this..(and make sure there is one..)
	
    start = cl.refdef.vieworg;
    VectorMA(start, cl.v_forward, 2000, end);
	min1.Set(-5,-5,-5);
	max1.Set(5,5,5);
	
	trace = CL_TraceBox(start,min1,max1,end,cl.playernum+1,MASK_SHOT);// SCG[1/25/00]: use mask shot, since it is for aiming a weapon...
	if(trace.fraction < 1.0)
	{
		ent = (entity_state_t *)trace.ent;
		if(trace.fraction < 1 && (ent != (entity_state_t *)0x00000001) && (ent->number < MAX_CLIENTS) && (display_ent_number != ent->number) && !(ent->renderfx & RF_PREDATOR))
		{           
			if(cl.clientinfo[ent->number-1].name)
			{
				display_ent_number = ent->number-1;
				display_name_frame_count=0;
				strncpy(display_name, cl.clientinfo[ent->number-1].name,sizeof(display_name));
				display_name_x = (cl.refdef.width*0.5)-(strlen(cl.clientinfo[ent->number-1].name)*4);
				display_name_y = (cl.refdef.height * 0.25);
				
				drawStruct.nFlags = DSFLAG_BLEND;
				drawStruct.szString = display_name;
				drawStruct.nXPos = display_name_x;
				drawStruct.nYPos = display_name_y;
				drawStruct.pImage = aim_font;
				re.DrawString( drawStruct );
			}
		} 
		else 
		{
			display_name_frame_count++;
			if( display_name_frame_count < 20 )
			{
				drawStruct.nFlags = DSFLAG_BLEND;
				drawStruct.szString = display_name;
				drawStruct.nXPos = display_name_x;
				drawStruct.nYPos = display_name_y;
				drawStruct.pImage = aim_font;
				re.DrawString( drawStruct );
			}
		}
		
	} 
	else 
	{
		display_name_frame_count++;
		if( display_name_frame_count < 20 )
		{
			drawStruct.nFlags = DSFLAG_BLEND;
			drawStruct.szString = display_name;
			drawStruct.nXPos = display_name_x;
			drawStruct.nYPos = display_name_y;
			drawStruct.pImage = aim_font;
			re.DrawString( drawStruct );
		}
	}
}

/*
=================
SCR_DrawCrosshair
=================
*/
void SCR_DrawCrosshair (void)
{
  crosshair_t *ch1,*ch2;
  float xofs,yofs;
  int cnum;

	if ((scr_draw_loading && scr_draw_progress) || (con.mode == CON_LOADING)) // loading level?  don't draw crosshair
	    return;

    // full screen cinematic or letterbox mode.. don't draw the crosshair
//	if ((CIN_Running() == 1) || Cvar_VariableValue ("scr_letterbox"))
	if ((CIN_Running() == 1) || (cl.frame.playerstate.rdflags & RDF_LETTERBOX))
		return;

	// cek[2-11-00]: so player id will work even if the weapon has no crosshair.
	V_PlayerID();
	// SCG[10/4/99]: cv_crosshair_enabled was added so only certain weapons will use the crosshair
	if( !cv_crosshair_enabled->value )
	{
		return;
	}

    if (!cv_crosshair->value)
        return;

	if( cl.frame.playerstate.stats[STAT_HEALTH] <= 0 )
	{
		return;
	}

	if (cv_crosshair->modified)
	{
		cv_crosshair->modified = false;
		SCR_RegisterCrosshair ();
	}

   V_CheckAutoAim();
   cnum=cv_crosshair->value-1;

   //
   xofs=autoAim.xofs;
   yofs=autoAim.yofs;

   ch2=NULL;
   if (autoAim.ent)
   {
      ch2=&crosshairList[cnum][CROSSHAIR_TARGET];
      ch1=&crosshairList[cnum][CROSSHAIR_CENTER];
   }
   else if (autoAim.item)
   {
      ch1=&crosshairList[cnum][CROSSHAIR_ITEM];
   }
   else
      ch1=&crosshairList[cnum][CROSSHAIR_NOTARGET];


	if (!ch1->pic[0])
		return;

//==================================
// consolidation change: SCG 3-11-99
		DRAWSTRUCT drawStruct;
		drawStruct.nFlags = DSFLAG_BLEND;
		drawStruct.pImage = re.RegisterPic( ch1->pic, NULL, NULL, RESOURCE_GLOBAL );
		drawStruct.nXPos = scr_vrect.x + ( ( scr_vrect.width - ch1->width ) >> 1 ) + xofs;
		drawStruct.nYPos = scr_vrect.y + ( ( scr_vrect.height - ch1->height ) >> 1 ) + yofs;
		re.DrawPic( drawStruct );

   if (ch2)
   {
//      re.DrawPic (scr_vrect.x + ((scr_vrect.width - ch2->width)>>1),
//                  scr_vrect.y + ((scr_vrect.height - ch2->height)>>1),
//                  ch2->pic, RESOURCE_GLOBAL);
		drawStruct.pImage = re.RegisterPic( ch2->pic, NULL, NULL, RESOURCE_GLOBAL );
		drawStruct.nXPos = scr_vrect.x + ((scr_vrect.width - ch2->width)>>1);
		drawStruct.nYPos = scr_vrect.y + ((scr_vrect.height - ch2->height)>>1);
		re.DrawPic( drawStruct );
// consolidation change: SCG 3-11-99
//==================================
   }

}

/*
==================
V_RenderView

==================
*/
void V_RenderView( float stereo_separation )
{
	extern int entitycmpfnc( const entity_t *, const entity_t * );
	extern qboolean bGotLoadscreenName;

	if (cls.state != ca_active)
		return;

	if (!cl.refresh_prepped)
		return;			// still loading

	if( cl.frame.playerstate.rdflags & RDF_HALTALLDRAWING )
	{
		return;
	}

	if (cl_timedemo->value)
	{
		if (!cl.timedemo_start)
			cl.timedemo_start = Sys_Milliseconds ();
		cl.timedemo_frames++;
	}

	// an invalid frame will just use the exact previous refdef
	// we can't use the old frame if the video mode has changed, though...

	if( CIN_Running() )
	{
		cl.frame.playerstate.rdflags |= RDF_LETTERBOX;
	}

	// SCG[1/31/00]: Blend cinematic colors with current colors
	if ( cl.frame.valid && (cl.force_refdef || !cl_paused->value) )
	{
		cl.force_refdef = false;

		V_ClearScene ();

		// build a refresh entity list and calc cl.sim*
		// this also calls CL_CalcViewValues which loads
		// v_forward, etc.
		CL_AddEntities ();

		if (cl_testparticles->value)
			V_TestParticles ();
		if (cl_testentities->value)
			V_TestEntities ();
		if (cl_testlights->value)
			V_TestLights ();
		if (cl_testblend->value)
		{
			cl.refdef.blend[0] = 1;
			cl.refdef.blend[1] = 0.5;
			cl.refdef.blend[2] = 0.25;
			cl.refdef.blend[3] = 0.5;
		}

		// offset vieworg appropriately if we're doing stereo separation
		if ( stereo_separation != 0 )
		{
			CVector tmp;

			tmp = cl.v_right * stereo_separation;
			cl.refdef.vieworg = cl.refdef.vieworg + tmp;
		}

		// never let it sit exactly on a node line, because a water plane can
		// dissapear when viewed with the eye exactly on it.
		// the server protocol only specifies to 1/8 pixel, so add 1/16 in each axis
        //of course, our lerped origin isn't constrained to 1/8 pixel, so adding 1/16 is a total waste of time.
		//cl.refdef.vieworg[0] += 1.0/16;
		//cl.refdef.vieworg[1] += 1.0/16;
		//cl.refdef.vieworg[2] += 1.0/16;


// SCG[10/30/99]: moved outside this loop so this gets calc'd when the game is paused.
/*
		cl.refdef.x = scr_vrect.x;
		cl.refdef.y = scr_vrect.y;
		cl.refdef.width = scr_vrect.width;
		cl.refdef.height = scr_vrect.height;
		cl.refdef.fov_y = CalcFov (cl.refdef.fov_x, cl.refdef.width, cl.refdef.height);
		cl.refdef.time = cl.time*0.001;
*/

		cl.refdef.areabits = cl.frame.areabits;

		if (!cl_add_entities->value)
			r_numentities = 0;
		if (!cl_add_particles->value)
			r_numparticles = 0;
		if (!cl_add_lights->value)
			r_numdlights = 0;
		if (!cl_add_blend->value)
		{
			cl.refdef.blend[0] = 0;
			cl.refdef.blend[1] = 0;
			cl.refdef.blend[2] = 0;
		}

		if (!cl_add_flares->value)				//  Shawn:  Added for Daikatna
			r_numflares = 0;					//  Shawn:  Added for Daikatna

		if (!cl_add_comparticles->value)		//  Shawn:  Added for Daikatna
			r_numcomparticles = 0;

		// sort entities for better cache locality
//		qsort( r_entities, r_numentities, sizeof( cl.refdef.entities[0] ), (int (*)(const void *, const void *))entitycmpfnc );

		// SCG[3/29/99]: Sort alpha entities
		V_SortEntities();

		cl.refdef.num_entities			= r_numentities;
		cl.refdef.entities				= r_entities;
//		cl.refdef.num_sortentities		= r_numsortentities;
//		cl.refdef.sortentities			= r_sortentities;

		cl.refdef.num_particleVolumes	= r_numparticleVolumes;
		cl.refdef.particleVolumes		= r_particleVolumes;
		cl.refdef.num_particles			= r_numparticles;
		cl.refdef.particles				= r_particles;
		cl.refdef.num_dlights			= r_numdlights;
		cl.refdef.dlights				= r_dlights;
		cl.refdef.lightstyles			= r_lightstyles;

		cl.refdef.num_flares			= r_numflares;				//  Shawn:  Added for Daikatna
		cl.refdef.flares				= r_flares;					//  Shawn:  Added for Daikatna
		cl.refdef.foginfo				= r_foginfo;					//  Shawn:  Added for Daikatana

		cl.refdef.beamList				= &beamList;

		cl.refdef.inventoryIcons		= inventoryIcons;
		cl.refdef.numInventoryIcons		= numInventoryIcons;
		cl.refdef.inventory_mode		= inventory_mode;
		cl.refdef.inventory_selected	= inventory_selected;
		cl.refdef.inventory_alpha		= inventory_alpha;

		cl.refdef.onscreen_icons_info	= onscreen_icons_info;           // 3.1 dsn

		//  Shawn:  Added for Daikatana
		cl.refdef.num_comparticles		= r_numcomparticles;
		cl.refdef.comparticles			= r_comparticles;

		cl.refdef.num_dhcomparticles	= r_numdhcomparticles;
		cl.refdef.dhcomparticles		= r_dhcomparticles;

		cl.refdef.rdflags				= cl.frame.playerstate.rdflags;

		cl.refdef.episodeNum			= cl_episode_num;

		// sort entities for better cache locality
//		qsort( cl.refdef.entities, cl.refdef.num_entities, sizeof( cl.refdef.entities[0] ), (int (*)(const void *, const void *))entitycmpfnc );

		//	Nelno:	do not draw world model if bsp has changed
		//			sometimes this happens right when a map is starting to
		//			load after another map and the rendere gets invalid bsp data
		//			because it sharing with the server\client.
		if (bspModel.fixModelClip)
			cl.refdef.rdflags |= RDF_NOWORLDMODEL;
	}

	cl.refdef.x = scr_vrect.x;
	cl.refdef.y = scr_vrect.y;
	cl.refdef.width = scr_vrect.width;
	cl.refdef.height = scr_vrect.height;
	cl.refdef.fov_y = CalcFov (cl.refdef.fov_x, cl.refdef.width, cl.refdef.height);

	// SCG[11/17/99]: Stop flowing surfaces, skies, etc.. from moving
	if( !cl_paused->value )
	{
		cl.refdef.time = cl.time * 0.001;
	}

	if( ( CIN_RunningWithView() == 1 ) && ( cl.refdef.rdflags & RDF_CINESCREENHACK ) )
	{
		DRAWSTRUCT	drawStruct;
		drawStruct.nFlags	= ( DSFLAG_FLAT | DSFLAG_COLOR | DSFLAG_ALPHA );
		drawStruct.rgbColor.Set( 0.0f, 0.0f, 0.0f );
		drawStruct.fAlpha	= 1.0;
		drawStruct.nTop		= 0;
		drawStruct.nLeft	= 0;
		drawStruct.nBottom	= viddef.height;
		drawStruct.nRight	= viddef.width;

		re.DrawPic( drawStruct );
	}
	else
	{
		re.RenderFrame (&cl.refdef);
	}

	if (cl_stats->value)
	Com_Printf ("ent:%i  lt:%i  part:%i  compart:%i  flares:%s\n", r_numentities, r_numdlights, r_numparticles, r_numcomparticles, r_numflares);
	if ( log_stats->value && ( log_stats_file != 0 ) )
	fprintf( log_stats_file, "%i,%i,%i,%i,%i,", r_numentities, r_numdlights, r_numparticles, r_numcomparticles, r_numflares);


	SCR_AddDirtyPoint (scr_vrect.x, scr_vrect.y);
	SCR_AddDirtyPoint (scr_vrect.x+scr_vrect.width-1,
	scr_vrect.y+scr_vrect.height-1);

	if(!bspModel.fixModelClip)
		SCR_DrawCrosshair ();

	if( !( cl.refdef.rdflags & RDF_LETTERBOX ) ) // don't update icons during cinemaics
	{
		CL_Boost_Icons_Think();
	}
}


/*
=============
V_Viewpos_f
=============
*/
void V_Viewpos_f (void)
{
	Com_Printf ("(%i %i %i) : %i\n", (int)cl.refdef.vieworg[0],
		(int)cl.refdef.vieworg[1], (int)cl.refdef.vieworg[2], 
		(int)cl.refdef.viewangles[YAW]);
}

/*
=============
V_Init
=============
*/
void V_Init (void)
{
//	Cmd_AddCommand ("gun_next", V_Gun_Next_f);
//	Cmd_AddCommand ("gun_prev", V_Gun_Prev_f);
//	Cmd_AddCommand ("gun_model", V_Gun_Model_f);
//	Cmd_AddCommand ("viewpos", V_Viewpos_f);

	cv_crosshair			= Cvar_Get ("crosshair", "0", CVAR_ARCHIVE);
	cv_crosshair_enabled	= Cvar_Get ("cv_crosshair_enabled", "1", 0);	// cek: this is an internally used cvar only...no archive.
	cv_autoaim				= Cvar_Get ("autoaim", "50", CVAR_ARCHIVE);
	cl_testblend			= Cvar_Get ("cl_testblend", "0", 0);
	cl_testparticles		= Cvar_Get ("cl_testparticles", "0", 0);
	cl_testentities			= Cvar_Get ("cl_testentities", "0", 0);
	cl_numtestentities		= Cvar_Get ("cl_numtestentities", "32", 0);
	cl_testentitymodel		= Cvar_Get ("cl_testentitymodel", "e1/m_ragemaster", 0);
	cl_testlights			= Cvar_Get ("cl_testlights", "0", 0);
	cl_flarecount			= Cvar_Get ("cl_flarecount", "0", 0);
	cl_showflares			= Cvar_Get ("cl_showflares", "1", 0);
	cl_particlevol			= Cvar_Get ("cl_particlevol", "1", CVAR_ARCHIVE);	// cek[1-11-00] archive this var
	cl_stats				= Cvar_Get ("cl_stats", "0", 0);
	cl_framefix				= Cvar_Get ("cl_framefix", "1", 1);
	cl_bob					= Cvar_Get("cl_bob", "1", 1);

}
