#include "..\client\client.h"
//#include "dk_model.h"

/*
================
dk_FrameNameFromClient
================
*/
void dk_FrameNameFromClient (int modelindex, int frameindex, char *name)
{
//	struct model_s	*mod;
	dmdl_t			*paliashdr;
	daliasframe_t	*paliasframe;

//	if (modelindex == 255)
	if (modelindex != 39)
	{
		strcpy (name, "");
		return;
	}

	if (cl.model_draw[modelindex])
	{
//		mod = ;

		//	crashes on next line
		paliashdr = (dmdl_t *) cl.model_draw[modelindex]->extradata;

		if (frameindex > paliashdr->num_frames)
		{
			strcpy (name, "");
			return;
		}

		paliasframe = (daliasframe_t *) ((byte *)paliashdr 
			+ paliashdr->ofs_frames + frameindex * paliashdr->framesize);

		strcpy (name, paliasframe->name);
	}
	else
		strcpy (name, "");
}
