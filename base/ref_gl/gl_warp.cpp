// gl_warp.c -- sky and water polygons

#include "gl_local.h"

extern	model_t	*loadmodel;

char	skyname[MAX_QPATH];
image_t	*sky_images[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
image_t *cloud_image = NULL;
image_t *moon_image = NULL;
int		sky_texures [6];	//	index into gltextures for each sky image

msurface_t	*warpface;

float   gl_cloud1tile_value,
        gl_cloudxdir_value,
        gl_cloudydir_value,
        gl_lightningfreq_value,
        gl_cloud1speed_value,
        gl_cloud2alpha_value,
        gl_cloud2tile_value,
        gl_cloud2speed_value;
        

// sky dome is a rectangular grid with this many verts to the edge
//const float SKY_BOX_SIZE=6000.0f;
const float SKY_BOX_SIZE=8192.0f;
#define SKY_GRID_SIZE  20
#define SKY_NUM_STARS  500

typedef struct
{
    float u, v;
} TexCoord;

typedef struct
{
    int ci, cj;     // center
    float startrad;  
    float currad;  
    float startbrt; // brightness
    float curbrt;   // brightness
    float decay;
    int   nFlashes;        
    float lastTime; // last update time
} lightning_t;

typedef struct
{
    int ci, cj;     // center
    float width;
    float height;
} moon_t;

class CSkyBox
{
protected:
    CVector  mVerts[SKY_GRID_SIZE*SKY_GRID_SIZE];
    TexCoord mBaseTexCoords[SKY_GRID_SIZE*SKY_GRID_SIZE];
    TexCoord mCurTexCoords[SKY_GRID_SIZE*SKY_GRID_SIZE];
    unsigned int mDrawArray[(SKY_GRID_SIZE-1)*(SKY_GRID_SIZE)*2];
    float mCurSkyScale;
    CVector mStarPos[SKY_NUM_STARS*4];
    CVector mStarColor[SKY_NUM_STARS*4];

public:
    
    CSkyBox();
    ~CSkyBox();

    void CalcTexCoords(float skyHeight);
    void Draw(float, float, float, float, float, float);
    void DrawMoon(moon_t&);
    void UpdateLightning(lightning_t&);
    void DrawLightning(lightning_t&);
    void DrawStars();
    void DrawWireframe();
    inline unsigned int getIndex(int i, int j)   { return i*SKY_GRID_SIZE+j; }
};

CSkyBox::CSkyBox() :
    mCurSkyScale(0)
{
    float delta, ikScale, jkScale;
    float heightScaleConst;
    int i, j;
    CVector bp(-SKY_BOX_SIZE, -SKY_BOX_SIZE, 0);

    delta = 2*SKY_BOX_SIZE/(SKY_GRID_SIZE-1);    // here's the distance between two grid verts
    heightScaleConst = ((float)M_PI)/((float)(SKY_GRID_SIZE-1));
    
    // setup geometry of sky dome
    for (i=0;i<SKY_GRID_SIZE;i++)
    {
        ikScale = sin((float)i*heightScaleConst);
        for (j=0;j<SKY_GRID_SIZE;j++)
        {   
            jkScale = sin((float)j*heightScaleConst);
            mVerts[getIndex(i, j)] = bp + CVector(delta*i, delta*j, 0.25f*SKY_BOX_SIZE*ikScale*jkScale);
        }
    }

    // build draw list
    for (i=0;i<SKY_GRID_SIZE-1;i++)
    {
        int pi, pj;

        for (j=0;j<SKY_GRID_SIZE;j++)
        {
            pi = i; pj = j;
            mDrawArray[i*SKY_GRID_SIZE*2 + 2*j] = getIndex(pi, pj);
            
            pi = i+1; pj = j;                                               
            mDrawArray[i*SKY_GRID_SIZE*2 + 2*j+1] = getIndex(pi, pj);
        }
    
    }

    // init stars
    for (i=0;i<SKY_NUM_STARS;i++)
    {
        CVector r, up, right;

        // get a point in the right range
        r.x = SKY_BOX_SIZE*(2*(float)rand()/RAND_MAX - 1);  // -SKY_BOX_SIZE to SKY_BOX_SIZE
        r.y = SKY_BOX_SIZE*(2*(float)rand()/RAND_MAX - 1);  // -SKY_BOX_SIZE to SKY_BOX_SIZE
        r.z = SKY_BOX_SIZE*(float)rand()/RAND_MAX;          // 0 to SKY_BOX_SIZE
        r.Normalize();


        right.x = -r.y;
        right.y = r.x;
        right.z = 0;
        right.Normalize();

        up.CrossProduct(right, r);

        mStarPos[4*i]   = SKY_BOX_SIZE*r;
        mStarPos[4*i+1] = SKY_BOX_SIZE*r + 15*up;
        mStarPos[4*i+2] = SKY_BOX_SIZE*r + 15*(up + right);
        mStarPos[4*i+3] = SKY_BOX_SIZE*r + 15*right;
        
        // get color
        float inten;
        
        int val = rand()&0xf;
        
        if (val==15)
            inten = 1.0f;
        else if (val>=13)
            inten = 0.5f;
        else if (val>=10)
            inten = 0.25f;
        else if (val>=8)
            inten = 0.125f;
        else 
            inten = 0.1f;

        mStarColor[4*i].Set(inten, inten, inten);
        mStarColor[4*i+1].Set(inten, inten, inten);
        mStarColor[4*i+2].Set(inten, inten, inten);
        mStarColor[4*i+3].Set(inten, inten, inten);
    }


}

CSkyBox::~CSkyBox()
{
    mCurSkyScale = 0;
}

static lightning_t gLightning;
//static moon_t gMoon;

void CSkyBox::CalcTexCoords(float skyScale)
{
    int i, j;   // grid coords
    float texScale;

    mCurSkyScale = skyScale;

    texScale = mCurSkyScale/(2*SKY_BOX_SIZE);

    for (i=0;i<SKY_GRID_SIZE;i++)
    {
        for (j=0;j<SKY_GRID_SIZE;j++)
        {
            mBaseTexCoords[getIndex(i, j)].u = mVerts[getIndex(i, j)].x * texScale;
            mBaseTexCoords[getIndex(i, j)].v = mVerts[getIndex(i, j)].y * texScale;
        }
    }
}

void CSkyBox::Draw(float texscale, float scrollval, float r, float g, float b, float a)
{
    int i, j, pi, pj;
    float du, dv;

    if (gl_cloud1tile_value!=mCurSkyScale)
        CalcTexCoords(gl_cloud1tile_value);
    
    du = gl_cloudxdir_value*scrollval*gl_cloud1tile_value;
    dv = gl_cloudydir_value*scrollval*gl_cloud1tile_value;

    // build tex coord array
    for (i=0;i<SKY_GRID_SIZE;i++)
    {
        for (j=0;j<SKY_GRID_SIZE;j++)
        {
            mCurTexCoords[getIndex(i, j)].u = texscale*mBaseTexCoords[getIndex(i, j)].u+du;
            mCurTexCoords[getIndex(i, j)].v = texscale*mBaseTexCoords[getIndex(i, j)].v+dv;
        }
    }
    
    qglColor4f(r, g, b, a);

    if (gl_vertex_arrays->value)
    {
        qglEnableClientState(GL_VERTEX_ARRAY);
        qglVertexPointer(3, GL_FLOAT, 0, mVerts);
        
        qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
        qglTexCoordPointer(2, GL_FLOAT, 0, mCurTexCoords);

		if ( qglLockArraysEXT != 0 )
			qglLockArraysEXT(0, SKY_GRID_SIZE*SKY_GRID_SIZE);

        for (i=0;i<SKY_GRID_SIZE-1;i++)
            qglDrawElements(GL_QUAD_STRIP, SKY_GRID_SIZE*2, GL_UNSIGNED_INT, mDrawArray+i*SKY_GRID_SIZE*2);
        
		if ( qglLockArraysEXT != 0 )
			qglUnlockArraysEXT();

        qglDisableClientState(GL_VERTEX_ARRAY);
        qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    else
    {
        for (i=0;i<SKY_GRID_SIZE-1;i++)
        {
            qglBegin (GL_QUAD_STRIP);
        
            for (j=0;j<SKY_GRID_SIZE;j++)
            {
                pi = i; pj = j;
                qglTexCoord2fv((const float *)&mCurTexCoords[getIndex(pi, pj)]);
                qglVertex3fv (mVerts[getIndex(pi, pj)]);
                
                pi = i+1; pj = j;                                               
                qglTexCoord2fv((const float *)&mCurTexCoords[getIndex(pi, pj)]);
                qglVertex3fv (mVerts[getIndex(pi, pj)]);
            }
        
            qglEnd();
        }
    }
}

void CSkyBox::UpdateLightning(lightning_t& l)
{
    float deltaTime = r_newrefdef.time-l.lastTime;
    float decay;

    if (deltaTime<0)
    {
        l.lastTime = r_newrefdef.time;
        return;
    }

    if ((l.curbrt>0||l.currad>0))
    {
        if (l.nFlashes==1)
            decay = l.decay;
        else
            decay = 20*l.decay;
        
        if (l.curbrt>0)
        {
            l.curbrt -= deltaTime * decay;
            
            if (l.curbrt<0)
                l.curbrt = 0;
        }
    
        if (l.currad>0)
        {
            l.currad -= deltaTime * SKY_GRID_SIZE * decay;
            
            if (l.currad<0)
                l.currad = 0;
        }

        l.lastTime = r_newrefdef.time;    // last time we did something with l
    }
    else if (l.nFlashes>1)
    {
        l.nFlashes--;
        l.currad = l.startrad;
        l.curbrt = l.startbrt;

        l.lastTime = r_newrefdef.time;
    }
    else
    {
        // try to generate a new flash once per second
        if (deltaTime>gl_lightningfreq_value)
        {
            if ((rand()&0x3)==0)
            {
                // choose new values
                if ((rand()&0x3)==0)
                    l.decay = 9.0f;
                else if ((rand()&0x1)==0)
                    l.decay = 4.0f;
                else
                    l.decay = 1.0f;

                if ((rand()&0x3)==0)
                    l.nFlashes = 2;
                else
                    l.nFlashes = 1;
                
                // no centers directly overhead...
                if (rand()&1)
                {
                    l.ci = rand()%SKY_GRID_SIZE; // 0 to SKY_GRID_SIZE-1
                    
                    if (rand()&1)
                        l.cj = rand()%(SKY_GRID_SIZE/3);
                    else
                        l.cj = SKY_GRID_SIZE-1 - rand()%(SKY_GRID_SIZE/3);
                }
                else
                {
                    l.cj = rand()%SKY_GRID_SIZE; // 0 to SKY_GRID_SIZE-1
                    
                    if (rand()&1)
                        l.ci = rand()%(SKY_GRID_SIZE/3);
                    else
                        l.ci = SKY_GRID_SIZE-1 - rand()%(SKY_GRID_SIZE/3);
                }

                l.startrad = (SKY_GRID_SIZE-1)*(float)rand()/RAND_MAX;
                l.startbrt = 2;

                l.currad = l.startrad;
                l.curbrt = l.startbrt;
            }
                     
            l.lastTime = r_newrefdef.time;
        }
    }
    
    
    
}

inline void CSkyBox::DrawLightning(lightning_t& l)
{
    float c, scale;
    int rCeil;
    int imin, imax, jmin, jmax;
    int i, j, pi, pj;
    
    rCeil = ceil(l.currad);
    scale = min(l.curbrt, 1)/(rCeil*rCeil);

    imin = max(l.ci-rCeil, 0);                
    jmin = max(l.cj-rCeil, 0);                
    
    imax = min(l.ci+rCeil, SKY_GRID_SIZE-1);                
    jmax = min(l.cj+rCeil, SKY_GRID_SIZE-1);                

    for (i=imin;i<imax;i++)
    {
        qglBegin (GL_QUAD_STRIP);

        for (j=jmin;j<=jmax;j++)
        {
            pi = i; pj = j;
            c = scale*max(l.currad-abs(l.ci-pi), 0)*max(l.currad-abs(l.cj-pj), 0);
            qglColor4f(c, c, c, c);
            qglVertex3fv (mVerts[getIndex(pi, pj)]);
            
            pi = i+1; pj = j;                                               
            c = scale*max(l.currad-abs(l.ci-pi), 0)*max(l.currad-abs(l.cj-pj), 0);
            qglColor4f(c, c, c, c);
            qglVertex3fv (mVerts[getIndex(pi, pj)]);
        }

        qglEnd();
    }

}

void CSkyBox::DrawMoon(moon_t& moon)
{
    float deltaI, deltaJ;
    int rCeilI, rCeilJ;
    int imin, imax, jmin, jmax;
    int i, j, pi, pj;
    
    rCeilI = ceil(moon.width);
    rCeilJ = ceil(moon.height);

    imin = max(moon.ci-rCeilI, 0);                
    jmin = max(moon.cj-rCeilJ, 0);                
    
    imax = min(moon.ci+rCeilI, SKY_GRID_SIZE-1);                
    jmax = min(moon.cj+rCeilJ, SKY_GRID_SIZE-1);                

    deltaI = 1.0f/moon.width;
    deltaJ = 1.0f/moon.height;

    qglColor4f(1, 1, 1, 1);

    for (i=imin;i<imax;i++)
    {
        qglBegin (GL_QUAD_STRIP);

        for (j=jmin;j<=jmax;j++)
        {
            pi = i; pj = j;
            qglTexCoord2f((pi-moon.ci)*deltaI + 0.5f, (pj-moon.cj)*deltaJ + 0.5f);
            qglVertex3fv (mVerts[getIndex(pi, pj)]);
            
            pi = i+1; pj = j;                                               
            qglTexCoord2f((pi-moon.ci)*deltaI + 0.5f, (pj-moon.cj)*deltaJ + 0.5f);
            qglVertex3fv (mVerts[getIndex(pi, pj)]);
        }

        qglEnd();
    }

}

void CSkyBox::DrawStars()
{
    if (gl_vertex_arrays->value)
    {
        qglEnableClientState(GL_VERTEX_ARRAY);
        qglVertexPointer(3, GL_FLOAT, 0, mStarPos);
        
        qglEnableClientState(GL_COLOR_ARRAY);
        qglColorPointer(3, GL_FLOAT, 0, mStarColor);

		if ( qglLockArraysEXT != 0 )
			qglLockArraysEXT(0, 4*SKY_NUM_STARS);

        qglDrawArrays(GL_QUADS, 0, 4*SKY_NUM_STARS);
        
		if ( qglLockArraysEXT != 0 )
			qglUnlockArraysEXT();

        qglDisableClientState(GL_VERTEX_ARRAY);
        qglDisableClientState(GL_COLOR_ARRAY);
    }
    else
    {
        int i;

        qglBegin(GL_QUADS);

        for (i=0;i<4*SKY_NUM_STARS;i++)
        {
            qglColor3fv(mStarColor[i]);
            qglVertex3fv(mStarPos[i]);
        }
        
        qglEnd();
    }
}


void CSkyBox::DrawWireframe()
{
    int i, j;

    qglColor4f(1, 1, 1, 1);

    for (i=0;i<SKY_GRID_SIZE-1;i++)
    {
        qglBegin (GL_LINE_STRIP);

        for (j=0;j<SKY_GRID_SIZE-1;j++)
        {
            qglVertex3fv (mVerts[getIndex(i,   j  )]);
            qglVertex3fv (mVerts[getIndex(i+1, j  )]);
            qglVertex3fv (mVerts[getIndex(i+1, j+1)]);
            qglVertex3fv (mVerts[getIndex(i  , j+1)]);
        }

        qglEnd();
    }
}

static CSkyBox gSkyBox;


//#define	SUBDIVIDE_SIZE	64
//#define	SUBDIVIDE_SIZE	1024

glpoly_t *AllocGLPoly( int numverts ){
	glpoly_t	*poly;
	int			allocSize;
	byte*		ptr;

	// allocate once.. assign pointers..
	allocSize = sizeof(glpoly_t) + (numverts * sizeof(CVector)) + (numverts * sizeof(float) * 4);
		
	ptr = (byte*)ri.X_Malloc( allocSize, MEM_TAG_GLPOLY );
//	ptr = (byte*)Hunk_Alloc( allocSize );

	memset( ptr, 0, sizeof( glpoly_t ) );

	poly = (glpoly_t *)	ptr;							   // 36 + (12*12) + (12*16) == 372

	ptr += sizeof(glpoly_t);
	poly->verts = (CVector *)ptr;

	ptr += (numverts * sizeof(CVector));
	poly->s1 = (float *)ptr;
	
	ptr += (numverts * sizeof(float));
	poly->s2 = (float *)ptr;
	
	ptr += (numverts * sizeof(float));
	poly->t1 = (float *)ptr;											   
	
	ptr += (numverts * sizeof(float));
	poly->t2 = (float *)ptr;					

	return poly;
}

void BoundPoly (int numverts, CVector *verts, CVector &mins, CVector &maxs)
{
	mins.x = mins.y = mins.z = 9999;
	maxs.x = maxs.y = maxs.z = -9999;

	for ( int i = 0 ; i < numverts ; i++ )
	{
		if( verts[i].x < mins.x )
			mins.x = verts[i].x;
		if( verts[i].x > maxs.x )
			maxs.x = verts[i].x;

		if( verts[i].y < mins.y )
			mins.y = verts[i].y;
		if( verts[i].y > maxs.y )
			maxs.y = verts[i].y;

		if( verts[i].z < mins.z )
			mins.z = verts[i].z;
		if( verts[i].z > maxs.z )
			maxs.z = verts[i].z;

	}
}

void BoundSubdividedPoly ( msurface_t *pSurf, CVector &mins, CVector &maxs)
{
	glpoly_t	*pPoly;
	CVector		*verts;
	int			numverts;

	mins.x = mins.y = mins.z = 9999;
	maxs.x = maxs.y = maxs.z = -9999;

	for( pPoly = pSurf->polys; pPoly; pPoly = pPoly->next )
	{
		verts = pPoly->verts;
		numverts = pPoly->numverts;

		for ( int i = 0 ; i < numverts ; i++ )
		{
			if( verts[i].x < mins.x )
				mins.x = verts[i].x;
			if( verts[i].x > maxs.x )
				maxs.x = verts[i].x;

			if( verts[i].y < mins.y )
				mins.y = verts[i].y;
			if( verts[i].y > maxs.y )
				maxs.y = verts[i].y;

			if( verts[i].z < mins.z )
				mins.z = verts[i].z;
			if( verts[i].z > maxs.z )
				maxs.z = verts[i].z;

		}
	}
}

void SubdividePolygon( int numverts, CVector *verts )
{
	int			i, j, k;
	CVector		mins, maxs;
	float		m;
	CVector		front[64], back[64];
	int			f, b;
	float		dist[64];
	float		frac;
	glpoly_t	*poly;
	float		s, t;
	CVector		total;
	float		total_s, total_t;
	float		lightmap_total_s, lightmap_total_t;

	if (numverts > 60)
		ri.Sys_Error (ERR_DROP, "numverts = %i", numverts);

	BoundPoly( numverts, verts, mins, maxs );

	for ( i = 0 ; i < 3 ; i++ )
	{
		m = (mins[i] + maxs[i]) * 0.5;
		m = gl_subdivide_size->value * floor (m / gl_subdivide_size->value + 0.5);

		if (maxs[i] - m < 8)
		{
			continue;
		}
		if (m - mins[i] < 8)
		{
			continue;
		}

		// cut it
		for ( j = 0 ; j < numverts ; j++ )
		{
			dist[j] = verts[j][i] - m;
		}

		// wrap cases
		dist[j] = dist[0];
		verts[numverts] = verts[0];

		f = 0;
		b = 0;
		for ( j = 0 ; j < numverts ; j++ )
		{
			if (dist[j] >= 0)
			{
				front[f] = verts[j];
				f++;
			}
			if (dist[j] <= 0)
			{
				back[b] = verts[j];
				b++;
			}
			
			if (dist[j] == 0 || dist[j+1] == 0)
			{
				continue;
			}

			if ( (dist[j] > 0) != (dist[j+1] > 0) )
			{
				// clip point
				frac = dist[j] / (dist[j] - dist[j+1]);
				
				for (k=0 ; k<3 ; k++)
					front[f][k] = back[b][k] = verts[j][k] + frac*(verts[j+1][k] - verts[j][k]);

				f++;
				b++;
			}
		}

		SubdividePolygon( f, front );
		SubdividePolygon( b, back );
		return;
	}										

	int lnumverts;

	// add a point in the center to help keep warp valid
	if( warpface->flags & SURF_DRAWFOG )
	{
		lnumverts = numverts;

		poly = AllocGLPoly( lnumverts );

		poly->next		= warpface->polys;
		warpface->polys = poly;
		poly->numverts	= lnumverts;

		for ( i = 0 ; i < numverts ; i++ )
		{
			poly->verts[i] = verts[i];

			s = DotProduct(verts[i], warpface->texinfo->s) + warpface->texinfo->s_offset;
			t = DotProduct(verts[i], warpface->texinfo->t) + warpface->texinfo->t_offset;

			poly->s1[i] = s;
			poly->t1[i] = t;

			s = DotProduct( verts[i], warpface->texinfo->s ) + warpface->texinfo->s_offset;
			s -= ( warpface->texturemins[0] );
			s += warpface->light_s * 16;
			s += 8;
			s /= 2048;

			t = DotProduct( verts[i], warpface->texinfo->t ) + warpface->texinfo->t_offset;
			t -= ( warpface->texturemins[1] );
			t += warpface->light_t * 16;
			t += 8;
			t /= 2048;

			poly->s2[i] = s;
			poly->t2[i] = t;
		}
	}
	else
	{
		lnumverts = numverts + 2;

		poly = AllocGLPoly( lnumverts );

		poly->next		= warpface->polys;
		warpface->polys = poly;
		poly->numverts	= lnumverts;

		total.Zero();
		total_s = 0;
		total_t = 0;
		lightmap_total_s = 0;
		lightmap_total_t = 0;

		for ( i = 0 ; i < numverts ; i++ )
		{
			poly->verts[i + 1] = verts[i];

			s = DotProduct(verts[i], warpface->texinfo->s) + warpface->texinfo->s_offset;
			t = DotProduct(verts[i], warpface->texinfo->t) + warpface->texinfo->t_offset;

			poly->s1[i + 1] = s;
			poly->t1[i + 1] = t;

			total_s += s;
			total_t += t;
			total = total + verts[i];

			s = DotProduct( verts[i], warpface->texinfo->s ) + warpface->texinfo->s_offset;
			s -= ( warpface->texturemins[0] );
			s += warpface->light_s * 16;
			s += 8;
			s /= 2048;

			t = DotProduct( verts[i], warpface->texinfo->t ) + warpface->texinfo->t_offset;
			t -= ( warpface->texturemins[1] );
			t += warpface->light_t * 16;
			t += 8;
			t /= 2048;

			poly->s2[i + 1] = s;
			poly->t2[i + 1] = t;

			lightmap_total_s += s;
			lightmap_total_t += t;
		}

		poly->verts[0] = total * ( 1.0 / numverts );

		poly->s1[0] = ( total_s/numverts );
		poly->t1[0] = ( total_t/numverts );
		poly->s2[0] = lightmap_total_s/numverts;
		poly->t2[0] = lightmap_total_t/numverts;

		// copy first vertex to last
		poly->verts[i + 1] = poly->verts[1];
		poly->s1[i + 1] = poly->s1[1];
		poly->t1[i + 1] = poly->t1[1];
		poly->s2[i + 1] = poly->s2[1];
		poly->t2[i + 1] = poly->t2[1];
	}
}

/*
================
GL_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent and sky warps
can be done reasonably.
================
*/
void GL_SubdivideSurface (msurface_t *fa)
{
	CVector		verts[64];
	int			numverts;
	int			i;
	int			lindex;
	CVector		vec;

	warpface = fa;

	//
	// convert edges back to a normal polygon
	//
	numverts = 0;
	for (i=0 ; i<fa->numedges ; i++)
	{
		lindex = loadmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
			vec = loadmodel->vertexes[loadmodel->edges[lindex].v[0]].position;
		else
			vec = loadmodel->vertexes[loadmodel->edges[-lindex].v[1]].position;
		verts[numverts] = vec;
		numverts++;
	}

	SubdividePolygon(numverts, verts);
}

//===================================================================

CVector	skyclip[6] = {
	CVector(1,1,0),
	CVector(1,-1,0),
	CVector(0,-1,1),
	CVector(0,1,1),
	CVector(1,0,1),
	CVector(-1,0,1) 
};
int	c_sky;

// 1 = s, 2 = t, 3 = 2048
int	st_to_vec[6][3] =
{
	{3,-1,2},
	{-3,1,2},

	{1,3,2},
	{-1,-3,2},

	{-2,-1,3},		// 0 degrees yaw, look straight up
	{2,-1,-3}		// look straight down

//	{-1,2,3},
//	{1,2,-3}
};

// s = [0]/[2], t = [1]/[2]
int	vec_to_st[6][3] =
{
	{-2,3,1},
	{2,3,-1},

	{1,3,2},
	{-1,3,-2},

	{-2,-1,3},
	{-2,1,-3}

//	{-1,2,3},
//	{1,2,-3}
};

float	skymins[2][6], skymaxs[2][6];
float	sky_min, sky_max;

void DrawSkyPolygon (int nump, const CVector *vecs)
{
	int		i,j;
	CVector	v, av;
	float	s, t, dv;
	int		axis;

	c_sky++;
	// decide which face it maps to
	v = vec3_origin;
	for ( i = 0; i < nump ; i++ )
	{
		for( j = 0; j < 3; j++ )
			v[j] = v[j] + vecs[i][j];
	}

	av.x = fabs(v.x);
	av.y = fabs(v.y);
	av.z = fabs(v.z);

	if (av.x > av.y && av.x > av.z)
	{
		if (v.x < 0)
			axis = 1;
		else
			axis = 0;
	}
	else 
	if (av.y > av.z && av.y > av.x)
	{
		if (v.y < 0)
			axis = 3;
		else
			axis = 2;
	}
	else
	{
		if (v.z < 0)
			axis = 5;
		else
			axis = 4;
	}

	// project new texture coords
	for (i=0 ; i<nump ; i++)
	{
		j = vec_to_st[axis][2];

		if (j > 0)
			dv = vecs[i][j - 1];
		else
			dv = -vecs[i][-j - 1];

		if (dv < 0.001)
			continue;	// don't divide by zero

		j = vec_to_st[axis][0];
		if (j < 0)
			s = -vecs[i][-j -1] / dv;
		else
			s = vecs[i][j-1] / dv;

		j = vec_to_st[axis][1];
		if (j < 0)
			t = -vecs[i][-j -1] / dv;
		else
			t = vecs[i][j-1] / dv;

		if (s < skymins[0][axis])
			skymins[0][axis] = s;
		if (t < skymins[1][axis])
			skymins[1][axis] = t;
		if (s > skymaxs[0][axis])
			skymaxs[0][axis] = s;
		if (t > skymaxs[1][axis])
			skymaxs[1][axis] = t;
	}
}
 
#define	ON_EPSILON		0.1			// point on plane side epsilon
#define	MAX_CLIP_VERTS	64
void ClipSkyPolygon (int nump, CVector *vecs, int stage)
{
	CVector	norm;
	qboolean	front, back;
	float	d, e;
	float	dists[MAX_CLIP_VERTS];
	int		sides[MAX_CLIP_VERTS];
	CVector	newv[2][MAX_CLIP_VERTS];
	int		newc[2];
	int		i, j;

	if (nump > MAX_CLIP_VERTS-2)
		ri.Sys_Error (ERR_DROP, "ClipSkyPolygon: MAX_CLIP_VERTS");

	if (stage == 6)
	{	// fully clipped, so draw it
		DrawSkyPolygon (nump, vecs);
		return;
	}

	front = back = false;
	norm = skyclip[stage];
	for ( i = 0; i < nump ; i++ )
	{
		d = DotProduct( vecs[i], norm );
		if (d > ON_EPSILON)
		{
			front = true;
			sides[i] = SIDE_FRONT;
		}
		else 
		if (d < -ON_EPSILON)
		{
			back = true;
			sides[i] = SIDE_BACK;
		}
		else
		{
			sides[i] = SIDE_ON;
		}
		dists[i] = d;
	}

	if (!front || !back)
	{	// not clipped
		ClipSkyPolygon( nump, vecs, stage+1 );
		return;
	}

	// clip it
	sides[i] = sides[0];
	dists[i] = dists[0];
	vecs[i] = vecs[0];
	//VectorCopy (vecs, (vecs+(i*3)) );
	newc[0] = newc[1] = 0;

	for ( i = 0; i < nump ; i++ )
	{
		switch (sides[i])
		{
		case SIDE_FRONT:
			newv[0][newc[0]] = vecs[i];
			newc[0]++;
			break;
		case SIDE_BACK:
			newv[1][newc[1]] = vecs[i];
			newc[1]++;
			break;
		case SIDE_ON:
			newv[0][newc[0]] = vecs[i];
			newc[0]++;

			newv[1][newc[1]] = vecs[i];
			newc[1]++;
			break;
		}

		if (sides[i] == SIDE_ON || sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
		{
			continue;
		}

		d = dists[i] / (dists[i] - dists[i+1]);

		for( j = 0; j < 3; j++ )
		{
			e = vecs[i][j] + d*(vecs[i+1][j] - vecs[i][j]);
			newv[0][newc[0]][j] = e;
			newv[1][newc[1]][j] = e;
		}

		newc[0]++;
		newc[1]++;
	}

	// continue
	ClipSkyPolygon(newc[0], newv[0], stage+1);
	ClipSkyPolygon(newc[1], newv[1], stage+1);
}

/*
=================
R_AddSkySurface
=================
*/
void R_AddSkySurface (msurface_t *fa)
{
	int			i;
	CVector		verts[MAX_CLIP_VERTS];
	glpoly_t	*p;

	// calculate vertex values for sky box
	for (p=fa->polys ; p ; p=p->next)
	{
		for (i=0 ; i<p->numverts ; i++)
		{
			verts[i] = p->verts[i] - r_origin;
		}
		ClipSkyPolygon (p->numverts, verts, 0);
	}
}


/*
==============
R_ClearSkyBox
==============
*/
void R_ClearSkyBox (void)
{
	int		i;

	for (i=0 ; i<6 ; i++)
	{
		skymins[0][i] = skymins[1][i] = 9999;
		skymaxs[0][i] = skymaxs[1][i] = -9999;
	}
}

void MakeSkyVec (float s, float t, int axis)
{
	CVector		v, b;
	int			j, k;

	b.x = s*4096;//2300
	b.y = t*4096;
	b.z = 4096;

	for( j = 0; j < 3; j++ )
	{
		k = st_to_vec[axis][j];
		if (k < 0)
			v[j] = -b[-k - 1];
		else
			v[j] = b[k - 1];
	}

	// avoid bilerp seam
	s = (s+1)*0.5;
	t = (t+1)*0.5;

	if (s < sky_min)
		s = sky_min;
	else if (s > sky_max)
		s = sky_max;
	if (t < sky_min)
		t = sky_min;
	else if (t > sky_max)
		t = sky_max;

	t = 1.0 - t;

	qglTexCoord2f (s, t);
	qglVertex3fv (v);
}

/*
==============
R_DrawSkyBox
==============
*/
int	skytexorder[6] = {0,2,1,3,4,5};
void R_DrawSkyBox (void)
{
	int		i;
    bool    fDrawClouds;
    unsigned int nStateFlags = GLSTATE_PRESET1;
    float   timeval;

	if( r_testfog->value )
	{
		qglFogf( GL_FOG_END, r_skyfogfar->value );
	}
	else if( r_newrefdef.foginfo.active )
	{
		qglFogf( GL_FOG_END, r_newrefdef.foginfo.skyend );
	}


	if( qglColorTableEXT )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

    qglPushMatrix ();
    qglTranslatef (r_origin.x, r_origin.y, r_origin.z);
    qglDepthMask(GL_FALSE);
    
    fDrawClouds = false;
    
    if (cloud_image)
    {
        for (i=0;i<6;i++)
        {   
            if (skymins[0][i] < skymaxs[0][i] && skymins[1][i] < skymaxs[1][i])
                fDrawClouds = true;;
        }

        if (fDrawClouds)
        {
            GL_Bind (cloud_image->texnum);
            GL_SetState(GLSTATE_DEPTH_TEST | GLSTATE_CULL_FACE | GLSTATE_TEXTURE_2D | GLSTATE_CULL_FACE_FRONT | GLSTATE_FOG);
			qglShadeModel(GL_SMOOTH);
			if( qglColorTableEXT )
			{
				GL_SetTexturePalette( cloud_image, FALSE );
			}

            // draw cloud layer 1
            double dummy;
            timeval = r_newrefdef.time * 0.002f;
            timeval = modf(timeval, &dummy);
    
            gSkyBox.Draw(1, gl_cloud1speed_value*timeval, 1, 1, 1, 1);
            
            /*
            // draw stars
            GL_SetState(GLSTATE_DEPTH_TEST);
            gSkyBox.DrawStars();

            // draw moon
            GL_Bind (moon_image->texnum);
            GL_SetState(GLSTATE_DEPTH_TEST | GLSTATE_CULL_FACE | GLSTATE_TEXTURE_2D | GLSTATE_BLEND | GLSTATE_CULL_FACE_FRONT);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            gSkyBox.DrawMoon(gMoon);
            */

            // draw lightning
            if (gl_lightningfreq_value>0)
            {
                gSkyBox.UpdateLightning(gLightning);
    
                if (gLightning.curbrt>0&&gLightning.currad>0)
                {
    				GL_SetState(GLSTATE_DEPTH_TEST | GLSTATE_CULL_FACE | GLSTATE_BLEND | GLSTATE_CULL_FACE_FRONT | GLSTATE_FOG);
                    gSkyBox.DrawLightning(gLightning);
                }
            }

            GL_SetState(GLSTATE_DEPTH_TEST | GLSTATE_CULL_FACE | GLSTATE_TEXTURE_2D | GLSTATE_BLEND | GLSTATE_CULL_FACE_FRONT);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            GL_TexEnv(GL_MODULATE);
            
            // draw cloud layer 2
            if (gl_cloud2alpha_value>0)
            {
                float alpha = min(gl_cloud2alpha_value, 1);
                GL_Bind(cloud_image->texnum);
                gSkyBox.Draw(gl_cloud2tile_value/gl_cloud1tile_value, gl_cloud2speed_value*timeval, 1, 1, 1, alpha);
            }

            if (gl_showtris->value)
            {
                GL_SetState(GLSTATE_DEPTH_TEST | GLSTATE_CULL_FACE | GLSTATE_CULL_FACE_FRONT);
                gSkyBox.DrawWireframe();
            }
        }
    
        GL_SetState( nStateFlags | GLSTATE_BLEND);
        GL_TexEnv(GL_MODULATE);
    }
    else
    {
        GL_SetState( nStateFlags);
        GL_TexEnv(GL_REPLACE);
    }

	if( qglColorTableEXT )
	{
		GL_SetTexturePalette( sky_images[0], FALSE );
	}

    for (i=0 ; i<6 ; i++)
    {
        if (skymins[0][i] >= skymaxs[0][i]
        || skymins[1][i] >= skymaxs[1][i])
            continue;

        GL_Bind (sky_images[skytexorder[i]]->texnum);

        qglBegin (GL_QUADS);
        MakeSkyVec (skymins[0][i], skymins[1][i], i);
        MakeSkyVec (skymins[0][i], skymaxs[1][i], i);
        MakeSkyVec (skymaxs[0][i], skymaxs[1][i], i);
        MakeSkyVec (skymaxs[0][i], skymins[1][i], i);
        qglEnd ();
    }

    qglDepthMask(GL_TRUE);
    qglPopMatrix ();

	if( r_testfog->value )
	{
		qglFogf( GL_FOG_END, r_fogfar->value );
	}
	else if( r_newrefdef.foginfo.active )
	{
		qglFogf( GL_FOG_END, r_newrefdef.foginfo.end );
	}
}


/*
============
SetSky
============
*/
// 3dstudio environment map names
char	*suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};


// if skynum is passed in, then skies are set entirely by number
// otherwise the named sky and cloud are used with the cvars for sky 1
void SetSky (const char *name, const char *cloudname, int skynum=0)
{
	int		i;
	char	pathname[MAX_QPATH], 
            subdir[MAX_QPATH];

	if( name == NULL )
	{
		return;
	}

	switch (skynum)
	{
	case 0:
		gl_cloud1tile_value = gl_cloud1tile->value;
		gl_cloudxdir_value = gl_cloudxdir->value;
		gl_cloudydir_value = gl_cloudydir->value;
		gl_lightningfreq_value = gl_lightningfreq->value;
		gl_cloud1speed_value = gl_cloud1speed->value;
		gl_cloud2alpha_value = gl_cloud2alpha->value;
		gl_cloud2tile_value = gl_cloud2tile->value;
		gl_cloud2speed_value = gl_cloud2speed->value;
		break;

    case 1:
        name = gl_skyname->string;
        cloudname = gl_cloudname->string;
		gl_cloud1tile_value = gl_cloud1tile->value;
		gl_cloudxdir_value = gl_cloudxdir->value;
		gl_cloudydir_value = gl_cloudydir->value;
		gl_lightningfreq_value = gl_lightningfreq->value;
		gl_cloud1speed_value = gl_cloud1speed->value;
		gl_cloud2alpha_value = gl_cloud2alpha->value;
		gl_cloud2tile_value = gl_cloud2tile->value;
		gl_cloud2speed_value = gl_cloud2speed->value;
		break;

	case 2:
        name = gl_skyname_2->string;
        cloudname = gl_cloudname_2->string;
		gl_cloud1tile_value = gl_cloud1tile_2->value;
		gl_cloudxdir_value = gl_cloudxdir_2->value;
		gl_cloudydir_value = gl_cloudydir_2->value;
		gl_lightningfreq_value = gl_lightningfreq_2->value;
		gl_cloud1speed_value = gl_cloud1speed_2->value;
		gl_cloud2alpha_value = gl_cloud2alpha_2->value;
		gl_cloud2tile_value = gl_cloud2tile_2->value;
		gl_cloud2speed_value = gl_cloud2speed_2->value;
		break;

	case 3:
        name = gl_skyname_3->string;
        cloudname = gl_cloudname_3->string;
		gl_cloud1tile_value = gl_cloud1tile_3->value;
		gl_cloudxdir_value = gl_cloudxdir_3->value;
		gl_cloudydir_value = gl_cloudydir_3->value;
		gl_lightningfreq_value = gl_lightningfreq_3->value;
		gl_cloud1speed_value = gl_cloud1speed_3->value;
		gl_cloud2alpha_value = gl_cloud2alpha_3->value;
		gl_cloud2tile_value = gl_cloud2tile_3->value;
		gl_cloud2speed_value = gl_cloud2speed_3->value;
		break;

	case 4:
        name = gl_skyname_4->string;
        cloudname = gl_cloudname_4->string;
		gl_cloud1tile_value = gl_cloud1tile_4->value;
		gl_cloudxdir_value = gl_cloudxdir_4->value;
		gl_cloudydir_value = gl_cloudydir_4->value;
		gl_lightningfreq_value = gl_lightningfreq_4->value;
		gl_cloud1speed_value = gl_cloud1speed_4->value;
		gl_cloud2alpha_value = gl_cloud2alpha_4->value;
		gl_cloud2tile_value = gl_cloud2tile_4->value;
		gl_cloud2speed_value = gl_cloud2speed_4->value;
		break;

    case 5:
        name = gl_skyname_5->string;
        cloudname = gl_cloudname_5->string;
        gl_cloud1tile_value = gl_cloud1tile_5->value;
		gl_cloudxdir_value = gl_cloudxdir_5->value;
		gl_cloudydir_value = gl_cloudydir_5->value;
		gl_lightningfreq_value = gl_lightningfreq_5->value;
		gl_cloud1speed_value = gl_cloud1speed_5->value;
		gl_cloud2alpha_value = gl_cloud2alpha_5->value;
		gl_cloud2tile_value = gl_cloud2tile_5->value;
		gl_cloud2speed_value = gl_cloud2speed_5->value;
		break;

    default:
		ri.Sys_Error (ERR_DROP, "SetSky: Bad sky number\n");
        break;
        
	}

	strncpy (skyname, name, sizeof(skyname)-1);

//    if (16==gl_state.nColorBits)
//        strcpy(subdir, "16bit");
//    else
        strcpy(subdir, "32bit");

    // skybox
	for (i=0 ; i<6 ; i++)
	{
		Com_sprintf (pathname, sizeof(pathname), "env/%s/%s%s.tga", subdir, skyname, suf[i]);
        sky_images[i] = R_FindImage (pathname, it_sky, RESOURCE_LEVEL);

        // fall back to the old stuff
        if (!sky_images[i])
        {   
            Com_sprintf (pathname, sizeof(pathname), "env/%s%s.pcx", skyname, suf[i]);
            sky_images[i] = R_FindImage (pathname, it_sky, RESOURCE_LEVEL);
        }

        if (!sky_images[i])
			sky_images[i] = r_notexture;

        sky_min = 1.0/512;
		sky_max = 511.0/512;

	}

    // cloud texture
    cloud_image = NULL;
    if (cloudname)
    {
        Com_sprintf (pathname, sizeof(pathname), "env/%s/%s%s", subdir, cloudname, ".tga");
        cloud_image = R_FindImage (pathname, it_sky, RESOURCE_LEVEL);
    }

    // moon texture
//    moon_image = R_FindImage ("env/moon.tga", it_sky, RESOURCE_LEVEL);

//    gMoon.ci = SKY_GRID_SIZE/2;
//    gMoon.cj = SKY_GRID_SIZE/4;
//    gMoon.width = 0.75;
//    gMoon.height = 1.5;

}

