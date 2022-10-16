#include "world.h"
#include "ai_utils.h"


// export
DllExport void misc_healthtree(userEntity_t *self);
DllExport void misc_drugbox(userEntity_t *self);


// local
void misc_healthtree_think(userEntity_t *self);

// defines
#define HT_IGNORE_RATE (1.0)
#define HT_MAXFRUIT    (5)
#define HT_MINFRUIT    (0)

#define	IS_INSTAGIB				(deathmatch->value && dm_instagib->value)

// SCG[11/19/99]: Save game stuff
#define	HEALTHTREEHOOK(x) (int)&(((healthtreeHook_t *)0)->x)
field_t healthtree_hook_fields[] = 
{
	{"max_fruit",		HEALTHTREEHOOK(max_fruit),		F_INT},
	{"current_fruit",	HEALTHTREEHOOK(current_fruit),	F_INT},
	{"fruit_value",		HEALTHTREEHOOK(fruit_value),	F_INT},
	{"recharge_rate",	HEALTHTREEHOOK(recharge_rate),	F_FLOAT},
	{"ignore_rate",		HEALTHTREEHOOK(ignore_rate),	F_FLOAT},
	{"ignore_timer",	HEALTHTREEHOOK(ignore_timer),	F_FLOAT},
	{"sound_use1",		HEALTHTREEHOOK(sound_use1),		F_INT},
	{"sound_use2",		HEALTHTREEHOOK(sound_use2),		F_INT},
	{"sound_regen",		HEALTHTREEHOOK(sound_regen),	F_INT},
	{NULL, 0, F_INT}
};

void health_tree_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, healthtree_hook_fields, sizeof( healthtreeHook_t ) );
}

void health_tree_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, healthtree_hook_fields, sizeof( healthtreeHook_t ) );
	healthtreeHook_t *hook = (healthtreeHook_t *) ent->userHook;
	ent->s.frame = abs(hook->current_fruit - HT_MAXFRUIT);
	ent->s.frameInfo.frameFlags |= FRAME_FORCEINDEX; // force stay on this frame, no anims
}

//---------------------------------------------------------------------------
// entAnimate()
//---------------------------------------------------------------------------
void entAnimate(userEntity_t *ent,short first,short last,unsigned short frameflags,float frametime)
{
    ent->s.frame=first;
    ent->s.frameInfo.startFrame=first;
    ent->s.frameInfo.endFrame=last;
    ent->s.frameInfo.frameFlags=frameflags | FRAME_FORCEINDEX;
    ent->s.frameInfo.frameInc=1;
    ent->s.frameInfo.frameTime=frametime;
    ent->s.frameInfo.frameState=FRSTATE_STARTANIM;
    ent->s.frameInfo.next_frameTime=0;
}

///////////////////////
//	misc_healthtree_use
///////////////////////

void misc_healthtree_use(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	healthtreeHook_t *hook = (healthtreeHook_t *) self->userHook;
	playerHook_t *ohook = AI_GetPlayerHook( activator );


	// not ignoring the last use?
	if (gstate->time >= hook->ignore_timer)
	{
		if (activator->health < ohook->base_health &&  // does the player need the health?
		hook->current_fruit > 0)                   // does it have any fruit available?
		{
			// boost up health by fruit_value  (activator, give amount, maximum health)
			com->Health (activator, hook->fruit_value, ohook->base_health);

			int curFruit = hook->current_fruit;
			hook->current_fruit--;  // reduce tree's fruit

			// sound that fruit has been eaten
			if (rand() % 2)
				gstate->StartEntitySound(self, CHAN_AUTO, hook->sound_use1, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
			else
				gstate->StartEntitySound(self, CHAN_AUTO, hook->sound_use2, 0.85, ATTN_NORM_MIN, ATTN_NORM_MAX);

			if (hook->recharge_rate > 0.0f)  // does this tree regenerate fruit?
			{
				self->think     = misc_healthtree_think;               // set recharge func
				self->nextthink = gstate->time + hook->recharge_rate;  // set recharge time
			}

			// set ignore timer
			hook->ignore_timer = gstate->time + hook->ignore_rate; 

			// set new frame based on number of fruits on tree
			entAnimate(self,abs(curFruit - HT_MAXFRUIT),abs(hook->current_fruit - HT_MAXFRUIT),FRAME_ONCE,0.05);
		}
	}
}



/////////////////////////
//	misc_healthtree_think
/////////////////////////

void misc_healthtree_think(userEntity_t *self)
// think-func regenerates fruit on the tree
{
	if (!deathmatch->value || (deathmatch->value && !dm_item_respawn->value))
		return; // guaranteed no regeneration <sniff>

	healthtreeHook_t *hook = (healthtreeHook_t *) self->userHook;
	if (!hook)
		return;
	
	int curFruit = hook->current_fruit;
	if (hook->current_fruit < hook->max_fruit)     // any fruit depleted from tree?
	{
		hook->current_fruit++; // regenerate fruit 
		
		if (hook->current_fruit >= hook->max_fruit)    // replenished all fruits?
		{
			hook->current_fruit = hook->max_fruit;
			
			self->think     = NULL; // no more fruits to regenerate
			self->nextthink = 0.0f; // 
		}
		else
			self->nextthink = gstate->time + hook->recharge_rate; // reset recharge time
		
		// set new frame based on number of fruits on tree
		entAnimate(self,abs(curFruit - HT_MAXFRUIT),abs(hook->current_fruit - HT_MAXFRUIT),FRAME_ONCE,0.05);
		
		// play sound of fruit generating
		gstate->StartEntitySound(self, CHAN_AUTO, hook->sound_regen, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
}


///////////////////////
//	misc_healthtree
///////////////////////

void misc_healthtree(userEntity_t *self)
{
	if (IS_INSTAGIB || (deathmatch->value && !dm_allow_health->value))
	  return;

	healthtreeHook_t *hook;
	int				 i;

	self->userHook = gstate->X_Malloc(sizeof(healthtreeHook_t),MEM_TAG_HOOK);
	hook = (healthtreeHook_t *) self->userHook;

	self->save = health_tree_hook_save;
	self->load = health_tree_hook_load;

   	self->className	= "misc_healthtree";
	self->s.modelindex = gstate->ModelIndex( "models/e1/healthtree.dkm" );
    
	self->mass	  = 1.0;
	self->gravity = 1.0;

    // defaults
    self->health = 100;

    hook->recharge_rate = 0.0f;          // seconds before fruit recharges (0 = no regeneration)
    hook->max_fruit     = HT_MAXFRUIT;
    hook->current_fruit = HT_MAXFRUIT;
    hook->fruit_value   = 10;

    hook->sound_use1  = gstate->SoundIndex ("e1/t_use1.wav");
    hook->sound_use2  = gstate->SoundIndex ("e1/t_use2.wav");
    hook->sound_regen = gstate->SoundIndex ("e1/t_regen.wav");
        

    // pull variables from entity info
    i = 0;
	while (self->epair [i].key != NULL)
	{
//      if (stricmp (self->epair [i].key, "health") == 0)
//        self->health = atoi (self->epair[i].value);
//      else
//      {
        if (stricmp (self->epair [i].key, "max_fruit") == 0)
        {
	      hook->max_fruit = atoi (self->epair [i].value);

          // check min/max fruits
          if (hook->max_fruit > HT_MAXFRUIT)
            hook->max_fruit = HT_MAXFRUIT;
          else
            if (hook->max_fruit < HT_MINFRUIT)
              hook->max_fruit = HT_MINFRUIT;

          hook->current_fruit = hook->max_fruit; // set the current fruit to max fruit
        } 
//        else 
//        {
//          if (stricmp (self->epair [i].key, "fruit_value") == 0)
//	        hook->fruit_value = atoi(self->epair[i].value);
//          else
//          {
//            if (stricmp (self->epair [i].key, "recharge_rate") == 0)
//	  	      hook->recharge_rate = atof (self->epair [i].value);
//		    else 
//		      if (stricmp (self->epair [i].key, "sound_use1") == 0)
//		      {
//			    if (!(hook->sound_use1 = gstate->SoundIndex (self->epair [i].value)))
//			      hook->sound_use1 = gstate->SoundIndex ("e1/t_use1.wav");
//		      } 
//              else
//		        if (stricmp (self->epair [i].key, "sound_use2") == 0)
//		        {
//			      if (!(hook->sound_use2 = gstate->SoundIndex (self->epair [i].value)))
//			        hook->sound_use2 = gstate->SoundIndex ("e1/t_use2.wav");
//		        }
//                else
//		          if (stricmp (self->epair [i].key, "sound_regen") == 0)
//		          {
//			        if (!(hook->sound_regen = gstate->SoundIndex (self->epair [i].value)))
//			          hook->sound_regen = gstate->SoundIndex ("e1/t_regen.wav");
//		          }
//
//          }
//        }
//      }
	  i++;
	}

	if (deathmatch->value && dm_item_respawn->value)
	{
	    hook->recharge_rate = 30 / hook->max_fruit;
	}
                         
	entAnimate(self,0,abs(hook->max_fruit - HT_MAXFRUIT),FRAME_ONCE,0.05);

    gstate->SetSize(self, -8.0, -8.0, -24.0, 8.0, 8.0, 8.0);

	// if the model has been rotated, rotate the bounding box also
	if (self->s.angles[0] || self->s.angles[1] || self->s.angles[2])
	  com->RotateBoundingBox(self);
     
	self->movetype = MOVETYPE_TOSS;
	self->solid = SOLID_BBOX; // alias models (b-models use SOLID_BSP)
	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	
    if (!self->health)                        // indestructable?
      self->takedamage = DAMAGE_NO;
    else
      self->takedamage = DAMAGE_YES;

    hook->ignore_rate  = HT_IGNORE_RATE;     // seconds to ignore next use
    hook->ignore_timer = 0.0f;               // track ignore timer

    self->use       = misc_healthtree_use;     // "use" function
  
    self->think     = NULL;    // reset until first used
    self->nextthink = 0.0f;

    gstate->LinkEntity (self);   // let's exist!

}

// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:        misc_drugbox_fade
// Description: Fade function for the DrugBox
// Input:	userEntity_t *self
// Output:  NA
// ----------------------------------------------------------------------------
void misc_drugbox_fade(userEntity_t *self)
{
	self->s.alpha -= 0.05;
	if(self->s.alpha <= 0.05f)
	{
		gstate->RemoveEntity(self);
	}
	self->nextthink = gstate->time + 0.2f;
}


// ----------------------------------------------------------------------------
// NSS[1/14/00]:
// Name:        misc_drugbox_use
// Description: The use function for the drug box....drugs.
// Input:	userEntity_t *self, userEntity_t *other, userEntity_t *activator
// Output:  NA
// ----------------------------------------------------------------------------
void misc_drugbox_use(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	healthtreeHook_t *hook = (healthtreeHook_t *) self->userHook;
	playerHook_t *ohook = AI_GetPlayerHook( activator );


	// not ignoring the last use?
	if (gstate->time >= hook->ignore_timer)
	{
		if(hook->current_fruit == 0)
		{
			self->s.effects = 0;
			self->s.frame = 0;
			self->s.frameInfo.startFrame = 1;
			self->s.frameInfo.endFrame = 29;
			self->s.frameInfo.frameInc = 1;
			self->s.frameInfo.frameFlags = ( FRAME_FORCEINDEX | FRAME_ONCE );
			self->s.frameInfo.modelAnimSpeed = 1;

			hook->ignore_timer = gstate->time + 1.5f; 
			hook->current_fruit++;  // reduce tree's fruit
		}
		else if (activator->health < ohook->base_health && hook->current_fruit < hook->max_fruit)                   
		{
			// boost up health by fruit_value  (activator, give amount, maximum health)
			com->Health (activator, hook->fruit_value, ohook->base_health);


			if(hook->current_fruit == 0)
			{
				gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex ("global/e_doorsqk.wav"), 0.65f, ATTN_NORM_MIN, ATTN_NORM_MAX);
				// set ignore timer
				hook->ignore_timer = gstate->time + 2.25f; 
			}
			else if (hook->current_fruit == 1)
			{
				gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex ("e1/m_dspheresteama.wav"), 0.55f, ATTN_NORM_MIN, ATTN_NORM_MAX);
				hook->ignore_timer = gstate->time + 1.25f; 
			}
			else if (hook->current_fruit == 2)
			{
				gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex ("artifacts/antidoteuse.wav"), 0.65f, ATTN_NORM_MIN, ATTN_NORM_MAX);
				hook->ignore_timer = gstate->time + 2.25f; 
			}
			else if (hook->current_fruit == 3)
			{
				gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex ("e1/we_dgloveamba.wav"), 0.65f, ATTN_NORM_MIN, ATTN_NORM_MAX);
				hook->ignore_timer = gstate->time + 1.75f; 
			}


			// set new frame based on number of fruits on tree
			self->s.effects = 0;
			self->s.frame   = 29 + (hook->current_fruit-1);
			self->s.frameInfo.startFrame = self->s.frame;
			self->s.frameInfo.endFrame = self->s.frameInfo.startFrame + 1;
			self->s.frameInfo.frameInc = 1;
			self->s.frameInfo.frameFlags = ( FRAME_FORCEINDEX | FRAME_ONCE );
			self->s.frameInfo.modelAnimSpeed = 1;

			hook->current_fruit++;

			if (hook->current_fruit >= hook->max_fruit)
			{
				self->think = misc_drugbox_fade;
				self->nextthink = gstate->time + 0.1f;
			}

		}
	}
}




// ----------------------------------------------------------------------------
// NSS[1/14/00]:
// Name:        misc_drugbox
// Description: The drug box for episode 4
// Input:	userEntity_t *self
// Output:  NA
// Note:
// ----------------------------------------------------------------------------
void misc_drugbox(userEntity_t *self)
{
	healthtreeHook_t *hook;
	int i;

	self->userHook = gstate->X_Malloc(sizeof(healthtreeHook_t),MEM_TAG_HOOK);
	hook = (healthtreeHook_t *) self->userHook;

	self->save = health_tree_hook_save;
	self->load = health_tree_hook_load;

   	self->className	= "misc_healthtree";
	self->s.modelindex = gstate->ModelIndex( "models/e4/a4_dbox.dkm" );
    
	self->mass	  = 1.0;
	self->gravity = 1.0;

    // defaults
    self->health = 100;
	// NSS[1/14/00]:No bouncy bouncy
	self->s.iflags		|= IF_SV_SENDBBOX;
	
	self->s.renderfx	|= (SPR_ALPHACHANNEL);

    
    // pull variables from entity info
    i = 0;
	while (self->epair [i].key != NULL)
	{
        if (stricmp (self->epair [i].key, "angle") == 0)
        {
		  self->s.angles.yaw = atof (self->epair [i].value);
        } 
		i++;
	}
	
	hook->recharge_rate = 0.0f;          // seconds before fruit recharges (0 = no regeneration)
    hook->max_fruit     = HT_MAXFRUIT;
    hook->current_fruit = HT_MAXFRUIT;
    hook->fruit_value   = 10;

    hook->sound_use1  = gstate->SoundIndex ("e1/t_use1.wav");
    hook->sound_use2  = gstate->SoundIndex ("e1/t_use2.wav");
    hook->sound_regen = gstate->SoundIndex ("e1/t_regen.wav");
        

	// setup the value for the drugs(yes I know we are using the fruit tree hook but might as well instead of creating a whole new structure.                         
	// 0 -> 1	(open)
	// 1 -> 4	(give health)		
	hook->max_fruit		= 4;
	hook->current_fruit = 0;


	self->s.frame = 0;
    self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX; // force stay on this frame, no anims

    gstate->SetSize(self, -8.0, -8.0, -24.0, 8.0, 8.0, 8.0);

	// if the model has been rotated, rotate the bounding box also
	if (self->s.angles[0] || self->s.angles[1] || self->s.angles[2])
	  com->RotateBoundingBox(self);
     
	self->movetype = MOVETYPE_TOSS;
	self->solid = SOLID_BBOX; 
	gstate->SetOrigin (self, self->s.origin [0], self->s.origin [1], self->s.origin [2]);
	
    self->takedamage = DAMAGE_NO;

    hook->ignore_rate	= HT_IGNORE_RATE;		// seconds to ignore next use
    hook->ignore_timer	= 0.0f;					// track ignore timer

    self->use			= misc_drugbox_use;     // "use" function
  
    self->think			= NULL;					// reset until first used
    self->nextthink		= 0.0f;

    gstate->LinkEntity (self);					// link it.

	// register the sounds being used.
	gstate->SoundIndex ("global/e_doorsqk.wav");
	gstate->SoundIndex ("e1/m_dspheresteama.wav");
	gstate->SoundIndex ("artifacts/antidoteuse.wav");
	gstate->SoundIndex ("e1/we_dgloveamba.wav");
}



///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_healthtree_register_func()
{
	// NSS[1/15/00]:
	gstate->RegisterFunc("misc_drugbox_fade",misc_drugbox_fade);
	gstate->RegisterFunc("misc_drugbox_use",misc_drugbox_use);
	gstate->RegisterFunc("misc_drugbox_fade",misc_drugbox_fade);

	gstate->RegisterFunc("misc_healthtree_think",misc_healthtree_think);
	gstate->RegisterFunc("misc_healthtree_use",misc_healthtree_use);
	gstate->RegisterFunc("health_tree_hook_save",health_tree_hook_save);
	gstate->RegisterFunc("health_tree_hook_load",health_tree_hook_load);
}
