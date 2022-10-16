#include "gl_local.h"

extern cvar_t *gl_drawfog;

/*
===============
GL_EnableMultitexture
===============
*/
void GL_EnableMultitexture( qboolean enable )
{
	if ( !qglSelectTextureSGIS )
		return;

	if ( enable )
	{
		GL_SelectTexture( GL_TEXTURE1_SGIS );
		qglEnable( GL_TEXTURE_2D );
		GL_TexEnv( GL_REPLACE );
	}
	else
	{
		GL_SelectTexture( GL_TEXTURE1_SGIS );
		qglDisable( GL_TEXTURE_2D );
		GL_TexEnv( GL_REPLACE );
	}
	GL_SelectTexture( GL_TEXTURE0_SGIS );
	qglEnable( GL_TEXTURE_2D );
	gl_state.nStateFlags |= GLSTATE_TEXTURE_2D;
	GL_TexEnv( GL_REPLACE );
#ifdef	_DEBUG
	RImp_CheckError();
#endif
}

/*
===============
GL_SelectTexture
===============
*/
void GL_SelectTexture( GLenum texture )
{
	int tmu;

	if ( !qglSelectTextureSGIS )
		return;

	if ( texture == GL_TEXTURE0_SGIS )
		tmu = 0;
	else
		tmu = 1;

	if ( tmu == gl_state.nTMU )
		return;

	gl_state.nTMU = tmu;

	if ( tmu == 0 )
		qglSelectTextureSGIS( GL_TEXTURE0_SGIS );
	else
		qglSelectTextureSGIS( GL_TEXTURE1_SGIS );
#ifdef	_DEBUG
	RImp_CheckError();
#endif
}

/*
===============
GL_TexEnv
===============
*/
void GL_TexEnv( GLenum mode )
{
	if ( mode != gl_state.nTexEnv[gl_state.nTMU] )
	{
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode );
		gl_state.nTexEnv[gl_state.nTMU] = mode;
	}
#ifdef	_DEBUG
	RImp_CheckError();
#endif
}

/*
===============
GL_TexEnv
===============
*/
void GL_MTexEnv( GLenum target, GLenum mode )
{
	GL_SelectTexture( target );
	if ( target == GL_TEXTURE0_SGIS )
	{
		if ( gl_state.nTexEnv[0] == target )
			return;
		gl_state.nTexEnv[0] = target;
	}
	else
	{
		if ( gl_state.nTexEnv[1] == target )
			return;
		gl_state.nTexEnv[1] = target;
	}
	GL_TexEnv( mode );
#ifdef	_DEBUG
	RImp_CheckError();
#endif
}

/*
===============
GL_Bind
===============
*/
void GL_Bind (int texnum)
{
	extern	image_t	*draw_chars;

	if (gl_nobind->value && draw_chars)		// performance evaluation option
	{
		texnum = draw_chars->texnum;
	}

	if ( gl_state.nTexnum[gl_state.nTMU] == texnum)
		return;

	gl_state.nTexnum[gl_state.nTMU] = texnum;
	qglBindTexture (GL_TEXTURE_2D, texnum);
#ifdef	_DEBUG
	RImp_CheckError();
#endif
}

/*
===============
GL_MBind
===============
*/
void GL_MBind( GLenum target, int texnum )
{
	GL_SelectTexture( target );
	if ( target == GL_TEXTURE0_SGIS )
	{
		if ( gl_state.nTexnum[0] == texnum )
			return;
	}
	else
	{
		if ( gl_state.nTexnum[1] == texnum )
			return;
	}
	GL_Bind( texnum );
#ifdef	_DEBUG
	RImp_CheckError();
#endif
}

/*
===============
GL_InitState
===============
*/
void GL_InitState()
{
	gl_state.nStateFlags = 0;
	gl_state.eAlphaFunc = -1;
	gl_state.eDepthFunc = -1;
	gl_state.nTMU = 0;
	gl_state.nTexEnv[0] = -1;
	gl_state.nTexEnv[1] = -1;
	gl_state.nTexnum[0] = -1;
	gl_state.nTexnum[1] = -1;
}

/*
===============
GL_GetState
===============
*/
unsigned int GL_GetState()
{
	return gl_state.nStateFlags;
}

/*
===============
GL_SetState
===============
*/
void GL_SetState( unsigned int nStateFlags )
{
	qglColor4f (1,1,1,1);

	if( nStateFlags == 0 )
	{
		GL_EnableMultitexture( false );
		qglDepthMask( GL_FALSE );
		qglDisable( GL_DEPTH_TEST );
		qglDisable( GL_ALPHA_TEST );
		qglDisable( GL_BLEND );
		qglDisable( GL_CULL_FACE );
		qglDisable( GL_FOG );
		qglDisable( GL_TEXTURE_2D );
		gl_state.nStateFlags = 0;
		return;
	}

	if( gl_drawfog->value == 0 )
	{
		nStateFlags &= ~GLSTATE_FOG;
	}

	if( ( nStateFlags & GLSTATE_MULTITEXTURE ) )
	{
		GL_EnableMultitexture( true );
	}
	else if( gl_state.nStateFlags & GLSTATE_MULTITEXTURE )
	{
		GL_EnableMultitexture( false );
	}

	if( nStateFlags & GLSTATE_CULL_FACE_FRONT )
	{
		qglCullFace( GL_FRONT );
	}
	else
	{
		qglCullFace( GL_BACK );
	}

	if( nStateFlags & GLSTATE_DEPTH_MASK )
	{
		if( !( gl_state.nStateFlags & GLSTATE_DEPTH_MASK ) )
		{
			qglDepthMask( GL_TRUE );
		}
	}
	else if( gl_state.nStateFlags & GLSTATE_DEPTH_MASK )
	{
		qglDepthMask( GL_FALSE );
	}

	if( nStateFlags & GLSTATE_DEPTH_TEST )
	{
		if( !( gl_state.nStateFlags & GLSTATE_DEPTH_TEST ) )
		{
			qglEnable( GL_DEPTH_TEST );
		}
	}
	else if( gl_state.nStateFlags & GLSTATE_DEPTH_TEST )
	{
		qglDisable( GL_DEPTH_TEST );
	}

	if( nStateFlags & GLSTATE_ALPHA_TEST )
	{
		if( !( gl_state.nStateFlags & GLSTATE_ALPHA_TEST ) )
		{
			qglEnable( GL_ALPHA_TEST );
		}
	}
	else if( gl_state.nStateFlags & GLSTATE_ALPHA_TEST )
	{
		qglDisable( GL_ALPHA_TEST );
	}
	
	if( nStateFlags & GLSTATE_BLEND )
	{
		if( !( gl_state.nStateFlags & GLSTATE_BLEND ) )
		{
			qglEnable( GL_BLEND );
		}
	}
	else if( gl_state.nStateFlags & GLSTATE_BLEND )
	{
		qglDisable( GL_BLEND );
	}

	if( nStateFlags & GLSTATE_CULL_FACE && ( gl_cull->value ) )
	{
//		if( !( gl_state.nStateFlags & GLSTATE_CULL_FACE ) )
		{
			qglEnable( GL_CULL_FACE );
		}
	}
	else // if( gl_state.nStateFlags & GLSTATE_CULL_FACE )
	{
		qglDisable( GL_CULL_FACE );
	}
	
	if( nStateFlags & GLSTATE_FOG && ( ( r_newrefdef.foginfo.active ) || ( r_testfog->value ) ) )
	{
		if( !( gl_state.nStateFlags & GLSTATE_FOG ) )
		{
			qglEnable( GL_FOG );
		}
	}
	else if( gl_state.nStateFlags & GLSTATE_FOG )
	{
		qglDisable( GL_FOG );
	}

	if( nStateFlags & GLSTATE_SCISSOR_TEST )
	{
		if( !( gl_state.nStateFlags & GLSTATE_SCISSOR_TEST ) )
		{
			qglEnable( GL_SCISSOR_TEST );
		}
	}
	else if( gl_state.nStateFlags & GLSTATE_SCISSOR_TEST )
	{
		qglDisable( GL_SCISSOR_TEST );
	}

	if( ( nStateFlags & GLSTATE_TEXTURE_2D ) )
	{
		if( !( gl_state.nStateFlags & GLSTATE_TEXTURE_2D ) )
		{
			qglEnable( GL_TEXTURE_2D );
		}
	}
	else if( gl_state.nStateFlags & GLSTATE_TEXTURE_2D )
	{
		qglDisable( GL_TEXTURE_2D );
	}

	if( qglColorTableEXT )
	{
		if( nStateFlags & GLSTATE_SHARED_PALETTE )
		{
			if( !( gl_state.nStateFlags & GLSTATE_SHARED_PALETTE ) )
			{
				qglEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
			}
		}
	}
	
/*
	if( nStateFlags & GLSTATE_TEXTURE_GEN_Q )
	{
		if( !( gl_state.nStateFlags & GLSTATE_TEXTURE_GEN_Q ) )
		{
		}
	}
	
	if( nStateFlags & GLSTATE_TEXTURE_GEN_R )
	{
		if( !( gl_state.nStateFlags & GLSTATE_TEXTURE_GEN_R ) )
		{
		}
	}
	
	if( nStateFlags & GLSTATE_TEXTURE_GEN_S )
	{
		if( !( gl_state.nStateFlags & GLSTATE_TEXTURE_GEN_S ) )
		{
		}
	}
	
	if( nStateFlags & GLSTATE_TEXTURE_GEN_T )
	{
		if( !( gl_state.nStateFlags & GLSTATE_TEXTURE_GEN_T ) )
		{
		}
	}
*/
	gl_state.nStateFlags = nStateFlags;

#ifdef	_DEBUG
	RImp_CheckError();
#endif

}

/*
===============
GL_SetFunc
===============
*/
void GL_SetFunc( unsigned int nStateFlags, GLenum eUser, float fUser )
{
	if( nStateFlags & GLSTATE_DEPTH_FUNC )
	{
//		if( gl_state.eDepthFunc != eUser )
		{
			qglDepthFunc( eUser );
			gl_state.eDepthFunc = eUser;
		}
	}
	else if( nStateFlags & GLSTATE_ALPHA_FUNC )
	{
//		if( ( gl_state.fAlphaFunc != fUser ) || ( gl_state.eAlphaFunc != eUser ) )
		{
			qglAlphaFunc( eUser, fUser );
			gl_state.fAlphaFunc = fUser;
			gl_state.eAlphaFunc = eUser;
		}
	}
	else if( nStateFlags & GLSTATE_BLEND_FUNC )
	{
		GLenum	eSource, eDest;

		eSource = eUser;
		eDest = ( GLenum ) fUser;
		
		qglBlendFunc( eSource, eDest );
	}

#ifdef	_DEBUG
	RImp_CheckError();
#endif
}

