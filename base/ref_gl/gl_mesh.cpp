// gl_mesh.c: triangle model functions

#include "gl_local.h"
#include <new>
#include <vector>
#include <deque>
//#include "gl_protex.h"
#include "..\shared\alias_quant.h"
#include "gl_surfsprite.h"

//	prototype.  defined in gl_rmain.c
//int	R_GetProjectedVert (const CVector &org, int *x, int *y);
void GL_DrawAliasFrameLerpProcShell(entity_t* ent, dmdl_t* paliashdr);
void R_BuildHeadMatrix(entity_t* e, float* pMatrix);
static qboolean R_CullAliasModel(CVector bbox[8], entity_t* e);

/*
=============================================================

  ALIAS MODELS


=============================================================
*/

typedef float vec4_t[4];

static vec4_t	s_lerped[MAX_VERTS];

//static CVector	lerped[MAX_VERTS];

CVector	shadevector;
CVector	shadelight;
CVector	oldshadelight;
qboolean	shademodified = false;
image_t* pEnvMapSkin;
image_t* r_shadowtexture;
msurface_t pointSurface;
CVector model_cam_pos;

static int aliasCullState = -1;

static void GL_AliasCullReset(void)
{
	aliasCullState = -1;
}

static void GL_AliasSetCull(qboolean enable)
{
	int state = enable ? 1 : 0;

	if (aliasCullState == state)
		return;

	if (enable)
		qglEnable(GL_CULL_FACE);
	else
		qglDisable(GL_CULL_FACE);

	aliasCullState = state;
}

dmdl_t* s_pmdl;
dsurface_t* s_psurface;

#include "vertnormals.h"
static CVertexNormals avertexnormals;

// precalculated dot products
float	avertexnormaldots[NUMVERTEXNORMALS];

CVector avertexcolors[NUMVERTEXNORMALS];
static CVector s_vertexcolors[MAX_VERTS];
static float s_envmapuv[MAX_VERTS][2];
static qboolean s_preparedEnvMapUV = FALSE;

extern float r_turbsin[];
extern float fWarpTime;
extern msurface_t* r_surfs_alpha;
extern CVector lightspot;

#define RENDER_START_ALPHA 0.25f
#define RENDER_END_ALPHA 0.05f
#define RENDER_ONE_16TH_PI_SQR 0.3926990816986f
#define TWO_THIRDS 0.6666666666666f
#define RENDER_TURBSCALE (256.0f / (2.0f * M_PI))
#define RENDER_FLOWING_SPEED ( ( fWarpTime * 0.5f ) - ( int )( fWarpTime * 0.5f ) )

struct alias_render_lighting_t
{
	CVector shadelight;
	CVector shadevector;
	CVector fixedshadevector;
	CVector model_cam_pos;
	CVector dloc;
	CVector lightspot;
	msurface_t pointSurface;
	CVector modelForward;
	CVector modelRight;
	CVector modelUp;
	CVector scaleOfs;
	float sx, sy, sz;
	int dynamicLightIndices[MAX_DLIGHTS];
	int numDynamicLightIndices;
	qboolean smoothDynamic;
	qboolean fDLight;
	qboolean drawShadow;
};

typedef struct
{
	float x, y, z;
	float s, t;
	float r, g, b;
} render_prep_alias_vertex_t;

typedef struct
{
	float x, y, z;
	float s, t;
} render_prep_surface_vertex_t;

typedef struct
{
	int type;
	int ready;
	int failed;
	const entity_t* entity;
	CVector origin;
	CVector angles;
	CVector renderScale;
	int flags;
	const msurface_t* surface;
	const dmdl_t* pmdl;
	int version;
	float backlerp;
	int frame;
	int oldframe;
	CVector move;
	CVector frontv;
	CVector backv;
	alias_render_lighting_t lighting;
	CVector entityColor;
	CVector boundsMins;
	CVector boundsMaxs;
	qboolean fullbright;
	qboolean prepareEnvmap;
	std::vector<render_prep_alias_vertex_t> aliasVertices;
	std::vector<float> envmapUV;
	CVector spotlightStart[2][17];
	CVector spotlightEnd[2][17];
	int alphaPrimType;
	std::vector<render_prep_surface_vertex_t> alphaVertices;
	std::vector<int> alphaOffsets;
	std::vector<int> alphaCounts;
} render_prep_job_t;

#define RENDER_PREP_ALIAS 1
#define RENDER_PREP_SPOTLIGHT 2
#define RENDER_PREP_ALPHA_SURFACE 3

static SDL_Thread* render_prep_worker_thread = NULL;
static SDL_Semaphore* render_prep_worker_start = NULL;
static SDL_Semaphore* render_prep_worker_done = NULL;
static SDL_Semaphore* render_prep_worker_all_done = NULL;
static SDL_Mutex* render_prep_worker_mutex = NULL;
static qboolean render_prep_worker_exit = false;
static int render_prep_worker_outstanding = 0;
static std::deque<render_prep_job_t> render_prep_worker_jobs;

static void RenderPrepAliasInterpolate(render_prep_job_t& job)
{
	int i;
	int nverts = job.pmdl->num_xyz;

	job.aliasVertices.resize(nverts);

	if (job.version == ALIAS_VERSION2)
	{
		const dtrivertx2_t* v = (const dtrivertx2_t*)((const byte*)job.pmdl + job.pmdl->ofs_frames + job.frame * job.pmdl->framesize);
		const dtrivertx2_t* ov = (const dtrivertx2_t*)((const byte*)job.pmdl + job.pmdl->ofs_frames + job.oldframe * job.pmdl->framesize);

		if (job.backlerp == 0.0f)
		{
			for (i = 0; i < nverts; i++)
			{
				job.aliasVertices[i].x = job.move.x + (v[i].v >> X_BIT_SHIFT) * job.frontv.x;
				job.aliasVertices[i].y = job.move.y + ((v[i].v & Y_BIT_MASK) >> Y_BIT_SHIFT) * job.frontv.y;
				job.aliasVertices[i].z = job.move.z + (v[i].v & Z_BIT_MASK) * job.frontv.z;
			}
		}
		else if (job.backlerp == 1.0f)
		{
			for (i = 0; i < nverts; i++)
			{
				job.aliasVertices[i].x = job.move.x + (ov[i].v >> X_BIT_SHIFT) * job.backv.x;
				job.aliasVertices[i].y = job.move.y + ((ov[i].v & Y_BIT_MASK) >> Y_BIT_SHIFT) * job.backv.y;
				job.aliasVertices[i].z = job.move.z + (ov[i].v & Z_BIT_MASK) * job.backv.z;
			}
		}
		else
		{
			for (i = 0; i < nverts; i++)
			{
				job.aliasVertices[i].x = job.move.x + (ov[i].v >> X_BIT_SHIFT) * job.backv.x + (v[i].v >> X_BIT_SHIFT) * job.frontv.x;
				job.aliasVertices[i].y = job.move.y + ((ov[i].v & Y_BIT_MASK) >> Y_BIT_SHIFT) * job.backv.y + ((v[i].v & Y_BIT_MASK) >> Y_BIT_SHIFT) * job.frontv.y;
				job.aliasVertices[i].z = job.move.z + (ov[i].v & Z_BIT_MASK) * job.backv.z + (v[i].v & Z_BIT_MASK) * job.frontv.z;
			}
		}
	}
	else
	{
		const dtrivertx_t* v = (const dtrivertx_t*)((const byte*)job.pmdl + job.pmdl->ofs_frames + job.frame * job.pmdl->framesize);
		const dtrivertx_t* ov = (const dtrivertx_t*)((const byte*)job.pmdl + job.pmdl->ofs_frames + job.oldframe * job.pmdl->framesize);

		if (job.backlerp == 0.0f)
		{
			for (i = 0; i < nverts; i++)
			{
				job.aliasVertices[i].x = job.move.x + v[i].v[0] * job.frontv.x;
				job.aliasVertices[i].y = job.move.y + v[i].v[1] * job.frontv.y;
				job.aliasVertices[i].z = job.move.z + v[i].v[2] * job.frontv.z;
			}
		}
		else if (job.backlerp == 1.0f)
		{
			for (i = 0; i < nverts; i++)
			{
				job.aliasVertices[i].x = job.move.x + ov[i].v[0] * job.backv.x;
				job.aliasVertices[i].y = job.move.y + ov[i].v[1] * job.backv.y;
				job.aliasVertices[i].z = job.move.z + ov[i].v[2] * job.backv.z;
			}
		}
		else
		{
			for (i = 0; i < nverts; i++)
			{
				job.aliasVertices[i].x = job.move.x + ov[i].v[0] * job.backv.x + v[i].v[0] * job.frontv.x;
				job.aliasVertices[i].y = job.move.y + ov[i].v[1] * job.backv.y + v[i].v[1] * job.frontv.y;
				job.aliasVertices[i].z = job.move.z + ov[i].v[2] * job.backv.z + v[i].v[2] * job.frontv.z;
			}
		}
	}
}

static void RenderPrepAliasLighting(render_prep_job_t& job)
{
	int i;
	int normalIndex;
	float overbright = 1.0f;
	CVector modelForward, modelRight, modelUp;
	int numDynamicLightIndices = 0;
	float sx, sy, sz;
	const dmdl_t* pmdl = job.pmdl;

	job.lighting.shadelight = vec3_origin;
	job.lighting.shadevector = vec3_origin;
	job.lighting.fixedshadevector = vec3_origin;
	job.lighting.model_cam_pos = vec3_origin;
	job.lighting.dloc = vec3_origin;
	job.lighting.lightspot = vec3_origin;
	memset(&job.lighting.pointSurface, 0, sizeof(job.lighting.pointSurface));
	job.lighting.numDynamicLightIndices = 0;
	job.lighting.smoothDynamic = false;
	job.lighting.fDLight = false;
	job.lighting.drawShadow = false;

	if (gl_overbrightbits != NULL && gl_overbrightbits->value >= 2.0f)
		overbright = 2.0f;

	if (job.fullbright)
	{
		if (gl_shadows->value && r_worldmodel != NULL && r_worldmodel->lightdata &&
			!(job.flags & (RF_TRANSLUCENT | RF_WEAPONMODEL)) &&
			LittleLong(pmdl->version) != ALIAS_VERSION2)
		{
			CVector lightColor;
			job.lighting.fDLight = R_LightPointThreadSafe(job.origin, lightColor,
				&job.lighting.dloc, &job.lighting.pointSurface, &job.lighting.lightspot);
		}
		job.lighting.shadelight.Set(1.0, 1.0, 1.0);
	}
	else
	{
		job.lighting.fDLight = R_LightPointThreadSafe(job.origin,
			job.lighting.shadelight, &job.lighting.dloc, &job.lighting.pointSurface, &job.lighting.lightspot);

		if (gl_dynamic != NULL && gl_dynamic->value && r_worldmodel != NULL &&
			r_worldmodel->lightdata && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		{
			CVector dynamicAtOrigin;
			R_DynamicLightPoint(job.origin, dynamicAtOrigin);
			job.lighting.shadelight -= dynamicAtOrigin;
			if (job.lighting.shadelight.x < 0.0f) job.lighting.shadelight.x = 0.0f;
			if (job.lighting.shadelight.y < 0.0f) job.lighting.shadelight.y = 0.0f;
			if (job.lighting.shadelight.z < 0.0f) job.lighting.shadelight.z = 0.0f;
		}

		if (gl_monolightmap->string[0] != '0')
		{
			float s = job.lighting.shadelight.x;
			if (s < job.lighting.shadelight.y) s = job.lighting.shadelight.y;
			if (s < job.lighting.shadelight.z) s = job.lighting.shadelight.z;
			job.lighting.shadelight.Set(s, s, s);
		}
	}

	if (job.flags & RF_MINLIGHT)
	{
		for (i = 0; i < 3; i++)
		{
			if (job.lighting.shadelight[i] > 0.3f)
				break;
		}
		if (i == 3)
			job.lighting.shadelight.Set(0.3, 0.3, 0.3);
	}

	if (job.flags & RF_GLOW)
	{
		float scale = 0.0f;
		float minValue;
		scale = sin(r_newrefdef.time * 2);
		minValue = job.lighting.shadelight.x;
		job.lighting.shadelight.x += scale;
		if (job.lighting.shadelight.x < minValue) job.lighting.shadelight.x = minValue;
		minValue = job.lighting.shadelight.y;
		job.lighting.shadelight.y += scale;
		if (job.lighting.shadelight.y < minValue) job.lighting.shadelight.y = minValue;
		minValue = job.lighting.shadelight.z;
		job.lighting.shadelight.z += scale;
		if (job.lighting.shadelight.z < minValue) job.lighting.shadelight.z = minValue;
	}

	job.lighting.fixedshadevector.Set(-1, -1, -1);
	{
		CVector fixedshadeangles;
		job.lighting.fixedshadevector.VectorToAngles(fixedshadeangles);
		fixedshadeangles.yaw -= job.angles.yaw;
		fixedshadeangles.AngleToForwardVector(job.lighting.fixedshadevector);
	}

	if (job.lighting.fDLight && !(gl_dynamic != NULL && gl_dynamic->value))
	{
		CVector shadeAngles;
		job.lighting.shadevector = job.lighting.dloc - job.origin;
		job.lighting.shadevector.Normalize();
		job.lighting.shadevector.VectorToAngles(shadeAngles);
		shadeAngles.yaw -= job.angles.yaw;
		shadeAngles.AngleToForwardVector(job.lighting.shadevector);
	}
	else
	{
		job.lighting.shadevector = job.lighting.fixedshadevector;
	}

	job.lighting.model_cam_pos = r_newrefdef.vieworg - job.origin;
	{
		float mag = job.lighting.model_cam_pos.Normalize();
		CVector angles;
		job.lighting.model_cam_pos.VectorToAngles(angles);
		angles.yaw -= job.angles.yaw;
		angles.AngleToForwardVector(job.lighting.model_cam_pos);
		job.lighting.model_cam_pos = mag * job.lighting.model_cam_pos;
	}

	if (gl_dynamic != NULL && gl_dynamic->value &&
		r_worldmodel != NULL && r_worldmodel->lightdata &&
		!(r_newrefdef.rdflags & RDF_NOWORLDMODEL) &&
		!job.fullbright &&
		r_newrefdef.num_dlights > 0)
	{
		job.lighting.smoothDynamic = true;
		AngleToVectors(job.angles, modelForward, modelRight, modelUp);
		job.lighting.modelForward = modelForward;
		job.lighting.modelRight = modelRight;
		job.lighting.modelUp = modelUp;

		sx = job.renderScale.x;
		sy = job.renderScale.y;
		sz = job.renderScale.z;
		if (sx == 0.0f) sx = 1.0f;
		if (sy == 0.0f) sy = 1.0f;
		if (sz == 0.0f) sz = 1.0f;
		job.lighting.sx = sx;
		job.lighting.sy = sy;
		job.lighting.sz = sz;

		job.lighting.scaleOfs.x = (sx * pmdl->org.x) - pmdl->org.x;
		job.lighting.scaleOfs.y = (sy * pmdl->org.y) - pmdl->org.y;
		job.lighting.scaleOfs.z = (sz * pmdl->org.z) - pmdl->org.z;

		if (job.flags & RF_WEAPONMODEL)
		{
			int lnum;
			numDynamicLightIndices = r_newrefdef.num_dlights;
			if (numDynamicLightIndices > MAX_DLIGHTS)
				numDynamicLightIndices = MAX_DLIGHTS;
			for (lnum = 0; lnum < numDynamicLightIndices; lnum++)
				job.lighting.dynamicLightIndices[lnum] = lnum;
		}
		else
		{
			CVector localCenter;
			CVector dynamicCenter;
			float extentX, extentY, extentZ, radius;
			int lnum;

			localCenter.x = 0.5f * (job.boundsMins.x + job.boundsMaxs.x);
			localCenter.y = 0.5f * (job.boundsMins.y + job.boundsMaxs.y);
			localCenter.z = 0.5f * (job.boundsMins.z + job.boundsMaxs.z);

			dynamicCenter = job.origin - job.lighting.scaleOfs;
			dynamicCenter += modelForward * localCenter.x;
			dynamicCenter -= modelRight * localCenter.y;
			dynamicCenter += modelUp * localCenter.z;

			extentX = 0.5f * (job.boundsMaxs.x - job.boundsMins.x);
			extentY = 0.5f * (job.boundsMaxs.y - job.boundsMins.y);
			extentZ = 0.5f * (job.boundsMaxs.z - job.boundsMins.z);
			radius = sqrt(extentX * extentX + extentY * extentY + extentZ * extentZ);

			for (lnum = 0; lnum < r_newrefdef.num_dlights && numDynamicLightIndices < MAX_DLIGHTS; lnum++)
			{
				const dlight_t* dl = &r_newrefdef.dlights[lnum];
				float dx = dl->origin.x - dynamicCenter.x;
				float dy = dl->origin.y - dynamicCenter.y;
				float dz = dl->origin.z - dynamicCenter.z;
				float reach = radius + dl->intensity;
				if (dl->intensity > 0.0f && dx * dx + dy * dy + dz * dz < reach * reach)
					job.lighting.dynamicLightIndices[numDynamicLightIndices++] = lnum;
			}
		}
	}
	else
	{
		job.lighting.smoothDynamic = false;
		job.lighting.sx = job.lighting.sy = job.lighting.sz = 1.0f;
		job.lighting.scaleOfs.Zero();
	}

	job.lighting.numDynamicLightIndices = numDynamicLightIndices;
	job.lighting.drawShadow = (gl_shadows->value && r_worldmodel != NULL &&
		r_worldmodel->lightdata && !(job.flags & (RF_TRANSLUCENT | RF_WEAPONMODEL)) &&
		LittleLong(pmdl->version) != ALIAS_VERSION2);
	(void)overbright;
}

static void RenderPrepAliasBuildColors(render_prep_job_t& job)
{
	int i;
	int normalIndex;
	float overbright = 1.0f;
	const dmdl_t* pmdl = job.pmdl;
	const dlight_t* dl;
	float normalDots[NUMVERTEXNORMALS];
	int normalCount;

	job.aliasVertices.resize(pmdl->num_xyz);
	for (normalCount = 0; normalCount < NUMVERTEXNORMALS; normalCount++)
	{
		normalDots[normalCount] = max(job.lighting.shadevector.DotProduct(avertexnormals.GetNormal(normalCount)), 0.0f);
	}
	if (gl_overbrightbits != NULL && gl_overbrightbits->value >= 2.0f)
		overbright = 2.0f;

	if (job.fullbright)
	{
		for (i = 0; i < pmdl->num_xyz; i++)
		{
			job.aliasVertices[i].r = 1.0f;
			job.aliasVertices[i].g = 1.0f;
			job.aliasVertices[i].b = 1.0f;
		}
		return;
	}

	if (job.entityColor.x || job.entityColor.y || job.entityColor.z)
	{
		for (i = 0; i < pmdl->num_xyz; i++)
		{
			job.aliasVertices[i].r = job.entityColor.x;
			job.aliasVertices[i].g = job.entityColor.y;
			job.aliasVertices[i].b = job.entityColor.z;
		}
		return;
	}

	const daliasframe_t* oldframe = NULL;
	const daliasframe2_t* oldframe2 = NULL;
	if (job.version == ALIAS_VERSION2)
		oldframe2 = (const daliasframe2_t*)((const byte*)pmdl + pmdl->ofs_frames + job.oldframe * pmdl->framesize);
	else
		oldframe = (const daliasframe_t*)((const byte*)pmdl + pmdl->ofs_frames + job.oldframe * pmdl->framesize);

	for (i = 0; i < pmdl->num_xyz; i++)
	{
		CVector baseColor;
		CVector dynamicColor;
		if (job.version == ALIAS_VERSION2)
			normalIndex = oldframe2->verts[i].lightnormalindex;
		else
			normalIndex = oldframe->verts[i].lightnormalindex;

		baseColor.x = min((job.lighting.shadelight.x + normalDots[normalIndex]) * overbright, 1.0f);
		baseColor.y = min((job.lighting.shadelight.y + normalDots[normalIndex]) * overbright, 1.0f);
		baseColor.z = min((job.lighting.shadelight.z + normalDots[normalIndex]) * overbright, 1.0f);

		if (job.lighting.smoothDynamic)
		{
			CVector worldPos;
			CVector localNormal;
			CVector worldNormal;
			int lnum;

			worldPos = job.origin - job.lighting.scaleOfs;
			worldPos += job.lighting.modelForward * (job.aliasVertices[i].x * job.lighting.sx);
			worldPos -= job.lighting.modelRight * (job.aliasVertices[i].y * job.lighting.sy);
			worldPos += job.lighting.modelUp * (job.aliasVertices[i].z * job.lighting.sz);

			localNormal = avertexnormals.GetNormal(normalIndex);
			localNormal.x /= job.lighting.sx;
			localNormal.y /= job.lighting.sy;
			localNormal.z /= job.lighting.sz;
			localNormal.Normalize();

			worldNormal = job.lighting.modelForward * localNormal.x;
			worldNormal -= job.lighting.modelRight * localNormal.y;
			worldNormal += job.lighting.modelUp * localNormal.z;

			dynamicColor.Zero();
			for (lnum = 0; lnum < job.lighting.numDynamicLightIndices; lnum++)
			{
				dl = &r_newrefdef.dlights[job.lighting.dynamicLightIndices[lnum]];
				float dx = dl->origin.x - worldPos.x;
				float dy = dl->origin.y - worldPos.y;
				float dz = dl->origin.z - worldPos.z;
				float distanceSq = dx * dx + dy * dy + dz * dz;
				float distance;
				float invDistance;
				float add;
				float ndotl;
				float lightFactor;
				CVector toLight;

				if (distanceSq <= 0.000001f || distanceSq >= dl->intensity * dl->intensity)
					continue;

				distance = sqrt(distanceSq);
				invDistance = 1.0f / distance;
				toLight.x = dx * invDistance;
				toLight.y = dy * invDistance;
				toLight.z = dz * invDistance;
				ndotl = worldNormal.DotProduct(toLight);
				if (ndotl < 0.0f) ndotl = 0.0f;
				lightFactor = 0.18f + 0.82f * ndotl;

				add = 1.0f - distance / dl->intensity;
				add = add * add * (dl->intensity * (1.0f / 256.0f));
				add *= lightFactor;
				dynamicColor.x += dl->color.x * add;
				dynamicColor.y += dl->color.y * add;
				dynamicColor.z += dl->color.z * add;
			}

			baseColor += dynamicColor;
		}

		if (baseColor.x > 1.0f) baseColor.x = 1.0f;
		if (baseColor.y > 1.0f) baseColor.y = 1.0f;
		if (baseColor.z > 1.0f) baseColor.z = 1.0f;
		if (baseColor.x < 0.0f) baseColor.x = 0.0f;
		if (baseColor.y < 0.0f) baseColor.y = 0.0f;
		if (baseColor.z < 0.0f) baseColor.z = 0.0f;

		job.aliasVertices[i].r = baseColor.x;
		job.aliasVertices[i].g = baseColor.y;
		job.aliasVertices[i].b = baseColor.z;
	}
}

static void RenderPrepAliasBuildEnvmap(render_prep_job_t& job)
{
	int i;
	const daliasframe_t* oldframe = NULL;
	const daliasframe2_t* oldframe2 = NULL;

	if (!job.prepareEnvmap)
		return;

	job.envmapUV.resize(job.pmdl->num_xyz * 2);
	if (job.version == ALIAS_VERSION2)
		oldframe2 = (const daliasframe2_t*)((const byte*)job.pmdl + job.pmdl->ofs_frames + job.oldframe * job.pmdl->framesize);
	else
		oldframe = (const daliasframe_t*)((const byte*)job.pmdl + job.pmdl->ofs_frames + job.oldframe * job.pmdl->framesize);

	for (i = 0; i < job.pmdl->num_xyz; i++)
	{
		int normalIndex = (job.version == ALIAS_VERSION2) ? oldframe2->verts[i].lightnormalindex : oldframe->verts[i].lightnormalindex;
		CVector v(job.aliasVertices[i].x, job.aliasVertices[i].y, job.aliasVertices[i].z);
		CVector normal = avertexnormals.GetNormal(normalIndex);
		CVector incident = job.lighting.model_cam_pos - v;
		CVector bounce;
		CVector angles;
		float invK;

		incident.Normalize();
		bounce = incident - (2.0f * incident.DotProduct(normal)) * normal;
		bounce.VectorToAngles(angles);
		angles += job.angles;
		angles.AngleToForwardVector(bounce);
		bounce.z += 1.0f;
		invK = 0.5f / bounce.Length();

		job.envmapUV[i * 2 + 0] = 0.5f + bounce.x * invK;
		job.envmapUV[i * 2 + 1] = 0.5f + bounce.y * invK;
	}
}

static void RenderPrepSpotlight(render_prep_job_t& job)
{
	int i;
	float a, sina, cosa;
	float radius1 = job.frame;
	CVector length = job.renderScale - job.origin;
	CVector forward, right, up;
	AngleToVectors(job.angles, forward, right, up);

	for (int light = 0; light < 2; light++)
	{
		float radius2 = radius1 + length.Length() * 0.2f;
		for (i = 16; i >= 0; i--)
		{
			a = i * RENDER_ONE_16TH_PI_SQR;
			sina = sin(a);
			cosa = cos(a);
			job.spotlightStart[light][i].x = job.origin.x + forward.x * radius1 + right.x * cosa * radius1 + up.x * sina * radius1;
			job.spotlightStart[light][i].y = job.origin.y + forward.y * radius1 + right.y * cosa * radius1 + up.y * sina * radius1;
			job.spotlightStart[light][i].z = job.origin.z + forward.z * radius1 + right.z * cosa * radius1 + up.z * sina * radius1;
			job.spotlightEnd[light][i].x = job.renderScale.x + forward.x * radius2 + right.x * cosa * radius2 + up.x * sina * radius2;
			job.spotlightEnd[light][i].y = job.renderScale.y + forward.y * radius2 + right.y * cosa * radius2 + up.y * sina * radius2;
			job.spotlightEnd[light][i].z = job.renderScale.z + forward.z * radius2 + right.z * cosa * radius2 + up.z * sina * radius2;
		}
		radius1 *= TWO_THIRDS;
		length = length * TWO_THIRDS;
	}
}

static void RenderPrepAlphaSurface(render_prep_job_t& job)
{
	const msurface_t* surf = job.surface;
	glpoly_t* poly;
	if (!surf || !surf->texinfo || !surf->texinfo->image || !surf->polys)
		return;
	int i;
	float fScroll = (surf->texinfo->flags & SURF_FLOWING) ? (64 * RENDER_FLOWING_SPEED) : 0;
	int nWidth = surf->texinfo->image->width;
	int nHeight = surf->texinfo->image->height;
	float fWaveModifier = 0.0f;
	int fWaveMultiplier = 0;
	qboolean bSurge = FALSE;

	if (surf->texinfo->flags & SURF_FOGPLANE)
		job.alphaPrimType = GL_POLYGON;
	else
		job.alphaPrimType = GL_TRIANGLE_FAN;

	if (surf->texinfo->flags & (SURF_SURGE | SURF_BIGSURGE))
	{
		bSurge = TRUE;
		if (surf->texinfo->flags & SURF_SURGE)
		{
			fWaveModifier = 0.250f;
			fWaveMultiplier = 1;
		}
		else
		{
			fWaveModifier = 0.500f;
			fWaveMultiplier = 4;
		}
	}

	{
		int totalVerts = 0;
		int totalPolys = 0;
		for (poly = surf->polys; poly; poly = poly->next)
		{
			totalVerts += poly->numverts;
			totalPolys++;
		}
		job.alphaVertices.resize(totalVerts);
		job.alphaOffsets.reserve(totalPolys);
		job.alphaCounts.reserve(totalPolys);
	}

	{
		int vertexIndex = 0;
		for (poly = surf->polys; poly; poly = poly->next)
		{
			job.alphaOffsets.push_back(vertexIndex);
			job.alphaCounts.push_back(poly->numverts);
			for (i = 0; i < poly->numverts; i++, vertexIndex++)
			{
				render_prep_surface_vertex_t& v = job.alphaVertices[vertexIndex];
				CVector vertex = poly->verts[i];
				float fOrigS, fOrigT, fWarpS, fWarpT, fS, fT, zWave;

				if (surf->flags & SURF_DRAWTURB)
				{
					fOrigS = poly->s1[i];
					fOrigT = poly->t1[i];
#if !id386
					fWarpS = r_turbsin[(int)((fOrigT * 0.125f + fWarpTime) * RENDER_TURBSCALE) & 255];
					fWarpT = r_turbsin[(int)((fOrigS * 0.125f + fWarpTime) * RENDER_TURBSCALE) & 255];
#else
					fWarpS = r_turbsin[Q_ftol(((fOrigT * 0.125f + fWarpTime) * RENDER_TURBSCALE)) & 255];
					fWarpT = r_turbsin[Q_ftol(((fOrigS * 0.125f + fWarpTime) * RENDER_TURBSCALE)) & 255];
#endif
					fS = fOrigS + fWarpS - fScroll;
					fS /= nWidth;
					fT = (fOrigT + fWarpT) / nHeight;
					if (bSurge)
					{
						zWave = r_turbsin[Q_ftol((((fS + fT) * fWaveModifier + fWarpTime) * RENDER_TURBSCALE)) & 255];
						vertex.z += zWave * fWaveMultiplier;
					}
					v.s = fS;
					v.t = fT;
				}
				else
				{
					v.s = (poly->s1[i] - fScroll) / nWidth;
					v.t = poly->t1[i] / nHeight;
				}

				v.x = vertex.x;
				v.y = vertex.y;
				v.z = vertex.z;
			}
		}
	}
}

static int SDLCALL R_RenderPrepWorkerMain(void* unused)
{
	(void)unused;
	for (;;)
	{
		SDL_WaitSemaphore(render_prep_worker_start);
		if (render_prep_worker_exit)
			break;

		for (;;)
		{
			render_prep_job_t* job = NULL;
			SDL_LockMutex(render_prep_worker_mutex);
			for (std::deque<render_prep_job_t>::iterator it = render_prep_worker_jobs.begin(); it != render_prep_worker_jobs.end(); ++it)
			{
				if (it->ready == 0 && it->failed == 0)
				{
					job = &(*it);
					it->failed = 2;
					break;
				}
			}
			SDL_UnlockMutex(render_prep_worker_mutex);

			if (!job)
				break;

			try
			{
				switch (job->type)
				{
				case RENDER_PREP_ALIAS:
					RenderPrepAliasLighting(*job);
					RenderPrepAliasInterpolate(*job);
					RenderPrepAliasBuildColors(*job);
					RenderPrepAliasBuildEnvmap(*job);
					break;
				case RENDER_PREP_SPOTLIGHT:
					RenderPrepSpotlight(*job);
					break;
				case RENDER_PREP_ALPHA_SURFACE:
					RenderPrepAlphaSurface(*job);
					break;
				default:
					job->failed = 1;
					break;
				}
			}
			catch (...)
			{
				job->failed = 1;
			}

			SDL_LockMutex(render_prep_worker_mutex);
			if (job->failed != 1)
				job->ready = 1;
			job->failed = (job->failed == 1) ? 1 : 0;
			if (render_prep_worker_outstanding > 0)
				render_prep_worker_outstanding--;
			if (render_prep_worker_outstanding == 0)
				SDL_SignalSemaphore(render_prep_worker_all_done);
			SDL_UnlockMutex(render_prep_worker_mutex);
			SDL_SignalSemaphore(render_prep_worker_done);
		}
	}
	return 0;
}

static void R_RenderPrepWorkerQueueJob(render_prep_job_t& input)
{
	if (!render_prep_worker_thread || !render_prep_worker_mutex || !render_prep_worker_start)
		return;

	SDL_LockMutex(render_prep_worker_mutex);
	try
	{
		render_prep_worker_jobs.push_back(input);
		render_prep_worker_outstanding++;
	}
	catch (...)
	{
		SDL_UnlockMutex(render_prep_worker_mutex);
		return;
	}
	SDL_UnlockMutex(render_prep_worker_mutex);
	SDL_SignalSemaphore(render_prep_worker_start);
}

static render_prep_job_t* R_RenderPrepWorkerFind(const entity_t* ent, int type)
{
	render_prep_job_t* result = NULL;
	std::deque<render_prep_job_t>::iterator it;
	if (!render_prep_worker_mutex)
		return NULL;
	SDL_LockMutex(render_prep_worker_mutex);
	for (it = render_prep_worker_jobs.begin(); it != render_prep_worker_jobs.end(); ++it)
	{
		if (it->type == type && it->entity == ent)
		{
			result = &(*it);
			break;
		}
	}
	SDL_UnlockMutex(render_prep_worker_mutex);
	return result;
}

static render_prep_job_t* R_RenderPrepWorkerFindSurface(const msurface_t* surf)
{
	render_prep_job_t* result = NULL;
	std::deque<render_prep_job_t>::iterator it;
	if (!render_prep_worker_mutex)
		return NULL;
	SDL_LockMutex(render_prep_worker_mutex);
	for (it = render_prep_worker_jobs.begin(); it != render_prep_worker_jobs.end(); ++it)
	{
		if (it->type == RENDER_PREP_ALPHA_SURFACE && it->surface == surf)
		{
			result = &(*it);
			break;
		}
	}
	SDL_UnlockMutex(render_prep_worker_mutex);
	return result;
}

static void R_RenderPrepWorkerWaitJob(render_prep_job_t* job)
{
	if (!job || !render_prep_worker_mutex || !render_prep_worker_done)
		return;

	for (;;)
	{
		qboolean complete;
		SDL_LockMutex(render_prep_worker_mutex);
		complete = (job->ready != 0 || job->failed == 1);
		SDL_UnlockMutex(render_prep_worker_mutex);

		if (complete)
			return;

		SDL_WaitSemaphore(render_prep_worker_done);
	}
}

void R_RenderPrepWorkerInit(void)
{
	if (render_prep_worker_thread != NULL)
		return;

	render_prep_worker_start = SDL_CreateSemaphore(0);
	render_prep_worker_done = SDL_CreateSemaphore(0);
	render_prep_worker_all_done = SDL_CreateSemaphore(0);
	render_prep_worker_mutex = SDL_CreateMutex();
	if (!render_prep_worker_start || !render_prep_worker_done || !render_prep_worker_all_done || !render_prep_worker_mutex)
	{
		if (render_prep_worker_start) SDL_DestroySemaphore(render_prep_worker_start);
		if (render_prep_worker_done) SDL_DestroySemaphore(render_prep_worker_done);
		if (render_prep_worker_all_done) SDL_DestroySemaphore(render_prep_worker_all_done);
		if (render_prep_worker_mutex) SDL_DestroyMutex(render_prep_worker_mutex);
		render_prep_worker_start = NULL;
		render_prep_worker_done = NULL;
		render_prep_worker_all_done = NULL;
		render_prep_worker_mutex = NULL;
		return;
	}

	render_prep_worker_exit = false;
	render_prep_worker_outstanding = 0;
	render_prep_worker_jobs.clear();
	render_prep_worker_thread = SDL_CreateThread(R_RenderPrepWorkerMain, "DaikatanaRenderPrepWorker", NULL);
	if (!render_prep_worker_thread)
	{
		SDL_DestroySemaphore(render_prep_worker_start);
		SDL_DestroySemaphore(render_prep_worker_done);
		SDL_DestroySemaphore(render_prep_worker_all_done);
		SDL_DestroyMutex(render_prep_worker_mutex);
		render_prep_worker_start = NULL;
		render_prep_worker_done = NULL;
		render_prep_worker_all_done = NULL;
		render_prep_worker_mutex = NULL;
	}
}

void R_RenderPrepWorkerWaitAll(void)
{
	if (!render_prep_worker_thread || !render_prep_worker_mutex || !render_prep_worker_all_done)
		return;

	for (;;)
	{
		SDL_LockMutex(render_prep_worker_mutex);
		if (render_prep_worker_outstanding == 0)
		{
			SDL_UnlockMutex(render_prep_worker_mutex);
			return;
		}
		SDL_UnlockMutex(render_prep_worker_mutex);
		SDL_WaitSemaphore(render_prep_worker_all_done);
	}
}

void R_RenderPrepWorkerShutdown(void)
{
	if (!render_prep_worker_thread)
		return;

	R_RenderPrepWorkerWaitAll();
	render_prep_worker_exit = true;
	SDL_SignalSemaphore(render_prep_worker_start);
	SDL_WaitThread(render_prep_worker_thread, NULL);
	render_prep_worker_thread = NULL;
	SDL_DestroySemaphore(render_prep_worker_start);
	SDL_DestroySemaphore(render_prep_worker_done);
	SDL_DestroySemaphore(render_prep_worker_all_done);
	SDL_DestroyMutex(render_prep_worker_mutex);
	render_prep_worker_start = NULL;
	render_prep_worker_done = NULL;
	render_prep_worker_all_done = NULL;
	render_prep_worker_mutex = NULL;
	render_prep_worker_jobs.clear();
}

static qboolean R_RenderPrepQueueAlias(const entity_t* ent)
{
	model_t* model;
	dmdl_t* pmdl;
	render_prep_job_t job;
	entity_t safeEnt;
	CVector lightingBefore = shadelight;
	CVector vectorBefore = shadevector;
	CVector camBefore = model_cam_pos;
	model_t* modelBefore = currentmodel;
	dmdl_t* pmdlBefore = s_pmdl;
	qboolean culled;

	if (!ent || !ent->model || !r_drawalias || !r_drawalias->value)
		return false;
	model = (model_t*)ent->model;
	if (model->type != mod_alias || model->registration_sequence != registration_sequence ||
		model->extradata == NULL)
		return false;
	if ((ent->flags & RF_WEAPONMODEL) && weapon_visible && weapon_visible->value == 0)
		return false;

	safeEnt = *ent;
	currentmodel = model;
	s_pmdl = (dmdl_t*)model->extradata;
	pmdl = s_pmdl;
	if (pmdl->num_xyz <= 0 || pmdl->num_xyz > MAX_VERTS)
	{
		currentmodel = modelBefore;
		s_pmdl = pmdlBefore;
		shadelight = lightingBefore;
		shadevector = vectorBefore;
		model_cam_pos = camBefore;
		return false;
	}
	if (!(safeEnt.flags & RF_WEAPONMODEL))
	{
		CVector bbox[8];
		culled = R_CullAliasModel(bbox, &safeEnt);
		if (culled)
		{
			currentmodel = modelBefore;
			s_pmdl = pmdlBefore;
			shadelight = lightingBefore;
			shadevector = vectorBefore;
			model_cam_pos = camBefore;
			return false;
		}
	}

	job.type = RENDER_PREP_ALIAS;
	job.ready = 0;
	job.failed = 0;
	job.entity = ent;
	job.origin = safeEnt.origin;
	job.angles = safeEnt.angles;
	job.renderScale = safeEnt.render_scale;
	job.flags = safeEnt.flags;
	job.surface = NULL;
	job.pmdl = pmdl;
	job.version = LittleLong(pmdl->version);

	if (safeEnt.frame < 0 || safeEnt.frame >= pmdl->num_frames)
		safeEnt.frame = 0;
	if (safeEnt.oldframe < 0 || safeEnt.oldframe >= pmdl->num_frames)
		safeEnt.oldframe = 0;

	job.frame = safeEnt.frame;
	job.oldframe = safeEnt.oldframe;
	job.entityColor = safeEnt.color;
	job.boundsMins = safeEnt.mins;
	job.boundsMaxs = safeEnt.maxs;
	job.fullbright = (ent->flags & RF_FULLBRIGHT) || r_fullbright->value;
	job.prepareEnvmap = FALSE;
	job.backlerp = r_lerpmodels->value ? safeEnt.backlerp : 0.0f;
	job.entity = ent;

	job.move = job.backlerp * ((job.version == ALIAS_VERSION2) ? ((daliasframe2_t*)((byte*)pmdl + pmdl->ofs_frames + safeEnt.oldframe * pmdl->framesize))->translate : ((daliasframe_t*)((byte*)pmdl + pmdl->ofs_frames + safeEnt.oldframe * pmdl->framesize))->translate) +
		(1.0f - job.backlerp) * ((job.version == ALIAS_VERSION2) ? ((daliasframe2_t*)((byte*)pmdl + pmdl->ofs_frames + safeEnt.frame * pmdl->framesize))->translate : ((daliasframe_t*)((byte*)pmdl + pmdl->ofs_frames + safeEnt.frame * pmdl->framesize))->translate);
	job.frontv = (1.0f - job.backlerp) * ((job.version == ALIAS_VERSION2) ? ((daliasframe2_t*)((byte*)pmdl + pmdl->ofs_frames + safeEnt.frame * pmdl->framesize))->scale : ((daliasframe_t*)((byte*)pmdl + pmdl->ofs_frames + safeEnt.frame * pmdl->framesize))->scale);
	job.backv = job.backlerp * ((job.version == ALIAS_VERSION2) ? ((daliasframe2_t*)((byte*)pmdl + pmdl->ofs_frames + safeEnt.oldframe * pmdl->framesize))->scale : ((daliasframe_t*)((byte*)pmdl + pmdl->ofs_frames + safeEnt.oldframe * pmdl->framesize))->scale);

	R_RenderPrepWorkerQueueJob(job);

	currentmodel = modelBefore;
	s_pmdl = pmdlBefore;
	shadelight = lightingBefore;
	shadevector = vectorBefore;
	model_cam_pos = camBefore;
	return true;
}

static void R_RenderPrepQueueSpotlight(const entity_t* ent)
{
	render_prep_job_t job;
	job.type = RENDER_PREP_SPOTLIGHT;
	job.ready = 0;
	job.failed = 0;
	job.entity = ent;
	job.origin = ent->origin;
	job.angles = ent->angles;
	job.renderScale = ent->render_scale;
	job.flags = ent->flags;
	job.entityColor = ent->color;
	job.frame = ent->frame;
	job.oldframe = ent->oldframe;
	job.surface = NULL;
	job.pmdl = NULL;
	job.version = 0;
	R_RenderPrepWorkerQueueJob(job);
}

void R_RenderPrepWorkerBeginFrame(void)
{
	int i;
	R_RenderPrepWorkerWaitAll();
	render_prep_worker_jobs.clear();

	if (!render_prep_worker_thread || !r_drawentities || !r_drawentities->value)
		return;

	for (i = 0; i < r_newrefdef.num_entities; i++)
	{
		entity_t* ent = &r_newrefdef.entities[i];
		if (ent->flags & RF_SPOTLIGHT)
			R_RenderPrepQueueSpotlight(ent);
		else
			R_RenderPrepQueueAlias(ent);
	}

	for (i = 0; i < r_newrefdef.num_alphaentities; i++)
	{
		entity_t* ent = &r_newrefdef.alphaentities[i].ent;
		if (ent->flags & RF_SPOTLIGHT)
			R_RenderPrepQueueSpotlight(ent);
		else
			R_RenderPrepQueueAlias(ent);
	}
}

void R_RenderPrepWorkerQueueAlphaSurfaces(void)
{
	msurface_t* surf;
	if (!render_prep_worker_thread || !r_surfs_alpha || !gl_alphasurfaces->value)
		return;
	for (surf = r_surfs_alpha; surf; surf = surf->texturechain)
	{
		if (!(surf->flags & (SURF_DRAWTURB | SURF_DRAWFOG)))
			continue;
		render_prep_job_t job;
		job.type = RENDER_PREP_ALPHA_SURFACE;
		job.ready = 0;
		job.failed = 0;
		job.entity = NULL;
		job.surface = surf;
		job.pmdl = NULL;
		R_RenderPrepWorkerQueueJob(job);
	}
}

qboolean R_RenderPrepUseAlias(const entity_t* ent, float* positions, CVector* colors, float* envUV, int maxVerts)
{
	render_prep_job_t* job;
	int i;
	if (!render_prep_worker_thread)
		return false;
	job = R_RenderPrepWorkerFind(ent, RENDER_PREP_ALIAS);
	if (!job)
		return false;
	R_RenderPrepWorkerWaitJob(job);
	if (job->failed || !job->ready || job->aliasVertices.size() > (size_t)maxVerts)
		return false;
	(void)positions;
	(void)envUV;
	for (i = 0; i < (int)job->aliasVertices.size(); i++)
	{
		colors[i] = CVector(job->aliasVertices[i].r, job->aliasVertices[i].g, job->aliasVertices[i].b);
	}
	shadelight = job->lighting.shadelight;
	shadevector = job->lighting.shadevector;
	model_cam_pos = job->lighting.model_cam_pos;
	pointSurface = job->lighting.pointSurface;
	lightspot = job->lighting.lightspot;
	return true;
}

qboolean R_RenderPrepGetAliasLighting(const entity_t* ent, CVector& outShadeLight, CVector& outShadeVector,
	CVector& outModelCamPos, CVector& outFixedShadeVector, CVector& outDLoc, msurface_t& outPointSurface, bool& outFDLight)
{
	render_prep_job_t* job;
	if (!render_prep_worker_thread)
		return false;
	job = R_RenderPrepWorkerFind(ent, RENDER_PREP_ALIAS);
	if (!job)
		return false;
	R_RenderPrepWorkerWaitJob(job);
	if (job->failed || !job->ready)
		return false;
	outShadeLight = job->lighting.shadelight;
	outShadeVector = job->lighting.shadevector;
	outModelCamPos = job->lighting.model_cam_pos;
	lightspot = job->lighting.lightspot;
	outFixedShadeVector = job->lighting.fixedshadevector;
	outDLoc = job->lighting.dloc;
	outPointSurface = job->lighting.pointSurface;
	outFDLight = job->lighting.fDLight;
	return true;
}

qboolean R_RenderPrepDrawSpotlight(entity_t* ent)
{
	render_prep_job_t* job;
	int i;
	if (!render_prep_worker_thread)
		return false;
	job = R_RenderPrepWorkerFind(ent, RENDER_PREP_SPOTLIGHT);
	if (!job)
		return false;
	R_RenderPrepWorkerWaitJob(job);
	if (job->failed || !job->ready)
		return false;

	qglShadeModel(GL_SMOOTH);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE);
	GL_SetState(GLSTATE_BLEND | GLSTATE_DEPTH_TEST);
	for (int light = 0; light < 2; light++)
	{
		qglBegin(GL_TRIANGLE_STRIP);
		for (i = 0; i < 16; i++)
		{
			qglColor4f(job->entityColor.x, job->entityColor.y, job->entityColor.z, RENDER_START_ALPHA);
			qglVertex3fv(job->spotlightStart[light][i]);
			qglVertex3fv(job->spotlightStart[light][(i + 1) % 16]);
			qglColor4f(0, 0, 0, RENDER_END_ALPHA);
			qglVertex3fv(job->spotlightEnd[light][i]);
			qglVertex3fv(job->spotlightEnd[light][(i + 1) % 16]);
		}
		qglColor4f(job->entityColor.x, job->entityColor.y, job->entityColor.z, RENDER_START_ALPHA);
		qglVertex3fv(job->spotlightStart[light][0]);
		qglColor4f(0, 0, 0, RENDER_END_ALPHA);
		qglVertex3fv(job->spotlightEnd[light][0]);
		qglColor4f(job->entityColor.x, job->entityColor.y, job->entityColor.z, RENDER_START_ALPHA);
		qglVertex3fv(job->spotlightStart[light][1]);
		qglEnd();
	}
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	return true;
}

qboolean R_RenderPrepDrawAlphaSurface(msurface_t* surf)
{
	render_prep_job_t* job;
	int p;
	if (!render_prep_worker_thread)
		return false;
	job = R_RenderPrepWorkerFindSurface(surf);
	if (!job)
		return false;
	R_RenderPrepWorkerWaitJob(job);
	if (job->failed || !job->ready)
		return false;
	for (p = 0; p < (int)job->alphaOffsets.size(); p++)
	{
		int j;
		const render_prep_surface_vertex_t* v = &job->alphaVertices[job->alphaOffsets[p]];
		c_brush_polys++;
		qglBegin(job->alphaPrimType);
		for (j = 0; j < job->alphaCounts[p]; j++)
		{
			qglTexCoord2f(v[j].s, v[j].t);
			qglVertex3fv(&v[j].x);
		}
		qglEnd();
	}
	return true;
}

float   gHeadMatrix[16];
bool    gfHaveHeadMatrix;

//
// Ash -- precalc all the vertnormal dots
// 
void GL_PrecalcShadevectorDots()
{
	int i;

	for (i = 0;i < NUMVERTEXNORMALS;i++)
		avertexnormaldots[i] = max(shadevector.DotProduct(avertexnormals.GetNormal(i)), 0.0f);
}


// 
// Ash -- do all the lighting at once
//
void GL_LightVerts()
{
	int i;
	int normalIndex;
	float overbright = 1.0f;
	qboolean smoothDynamic = false;
	CVector modelForward, modelRight, modelUp;
	CVector scaleOfs;
	int dynamicLightIndices[MAX_DLIGHTS];
	int numDynamicLightIndices = 0;
	float sx, sy, sz;

	if (gl_overbrightbits != NULL && gl_overbrightbits->value >= 2.0f)
		overbright = 2.0f;

	if (currententity != NULL && gl_dynamic != NULL && gl_dynamic->value &&
		r_worldmodel != NULL && r_worldmodel->lightdata &&
		!(r_newrefdef.rdflags & RDF_NOWORLDMODEL) &&
		!(currententity->flags & RF_FULLBRIGHT) &&
		!(r_fullbright->value) &&
		r_newrefdef.num_dlights > 0)
	{
		smoothDynamic = true;
		AngleToVectors(currententity->angles, modelForward, modelRight, modelUp);

		sx = currententity->render_scale.x;
		sy = currententity->render_scale.y;
		sz = currententity->render_scale.z;
		if (sx == 0.0f) sx = 1.0f;
		if (sy == 0.0f) sy = 1.0f;
		if (sz == 0.0f) sz = 1.0f;

		scaleOfs.x = (sx * s_pmdl->org.x) - s_pmdl->org.x;
		scaleOfs.y = (sy * s_pmdl->org.y) - s_pmdl->org.y;
		scaleOfs.z = (sz * s_pmdl->org.z) - s_pmdl->org.z;

		if (currententity->flags & RF_WEAPONMODEL)
		{
			int lnum;

			numDynamicLightIndices = r_newrefdef.num_dlights;
			if (numDynamicLightIndices > MAX_DLIGHTS)
				numDynamicLightIndices = MAX_DLIGHTS;
			for (lnum = 0; lnum < numDynamicLightIndices; lnum++)
				dynamicLightIndices[lnum] = lnum;
		}
		else
		{
			CVector localCenter;
			CVector dynamicCenter;
			float extentX;
			float extentY;
			float extentZ;
			float radius;
			int lnum;

			localCenter.x = 0.5f * (currententity->mins.x + currententity->maxs.x);
			localCenter.y = 0.5f * (currententity->mins.y + currententity->maxs.y);
			localCenter.z = 0.5f * (currententity->mins.z + currententity->maxs.z);

			dynamicCenter = currententity->origin - scaleOfs;
			dynamicCenter += modelForward * localCenter.x;
			dynamicCenter -= modelRight * localCenter.y;
			dynamicCenter += modelUp * localCenter.z;

			extentX = 0.5f * (currententity->maxs.x - currententity->mins.x);
			extentY = 0.5f * (currententity->maxs.y - currententity->mins.y);
			extentZ = 0.5f * (currententity->maxs.z - currententity->mins.z);
			radius = sqrt(extentX * extentX + extentY * extentY + extentZ * extentZ);

			for (lnum = 0; lnum < r_newrefdef.num_dlights && numDynamicLightIndices < MAX_DLIGHTS; lnum++)
			{
				const dlight_t* dl = &r_newrefdef.dlights[lnum];
				float dx = dl->origin.x - dynamicCenter.x;
				float dy = dl->origin.y - dynamicCenter.y;
				float dz = dl->origin.z - dynamicCenter.z;
				float reach = radius + dl->intensity;

				if (dl->intensity > 0.0f &&
					dx * dx + dy * dy + dz * dz < reach * reach)
				{
					dynamicLightIndices[numDynamicLightIndices++] = lnum;
				}
			}
		}
	}
	else
	{
		scaleOfs.Zero();
		sx = sy = sz = 1.0f;
	}

	if (r_fullbright->value)
	{
		for (i = 0; i < s_pmdl->num_xyz; i++)
		{
			s_vertexcolors[i].x = 1.0;
			s_vertexcolors[i].y = 1.0;
			s_vertexcolors[i].z = 1.0;
		}
		return;
	}

	if (currententity->color.x || currententity->color.y || currententity->color.z)
	{
		for (i = 0; i < s_pmdl->num_xyz; i++)
		{
			s_vertexcolors[i].x = currententity->color.x;
			s_vertexcolors[i].y = currententity->color.y;
			s_vertexcolors[i].z = currententity->color.z;
		}
		return;
	}

	{
		daliasframe_t* oldframe = NULL;
		daliasframe2_t* oldframe2 = NULL;
		int version = LittleLong(s_pmdl->version);

		if (version == ALIAS_VERSION2)
		{
			oldframe2 = (daliasframe2_t*)((byte*)s_pmdl + s_pmdl->ofs_frames + currententity->oldframe * s_pmdl->framesize);
		}
		else
		{
			oldframe = (daliasframe_t*)((byte*)s_pmdl + s_pmdl->ofs_frames + currententity->oldframe * s_pmdl->framesize);
		}

		for (i = 0; i < s_pmdl->num_xyz; i++)
		{
			CVector baseColor;
			CVector dynamicColor;

			if (version == ALIAS_VERSION2)
				normalIndex = oldframe2->verts[i].lightnormalindex;
			else
				normalIndex = oldframe->verts[i].lightnormalindex;

			baseColor.x = min((shadelight.x + avertexnormaldots[normalIndex]) * overbright, 1.0f);
			baseColor.y = min((shadelight.y + avertexnormaldots[normalIndex]) * overbright, 1.0f);
			baseColor.z = min((shadelight.z + avertexnormaldots[normalIndex]) * overbright, 1.0f);

			if (smoothDynamic)
			{
				CVector worldPos;
				CVector localNormal;
				CVector worldNormal;
				dlight_t* dl;
				int lnum;

				worldPos = currententity->origin - scaleOfs;
				worldPos += modelForward * (s_lerped[i][0] * sx);
				worldPos -= modelRight * (s_lerped[i][1] * sy);
				worldPos += modelUp * (s_lerped[i][2] * sz);

				localNormal = avertexnormals.GetNormal(normalIndex);
				localNormal.x /= sx;
				localNormal.y /= sy;
				localNormal.z /= sz;
				localNormal.Normalize();

				worldNormal = modelForward * localNormal.x;
				worldNormal -= modelRight * localNormal.y;
				worldNormal += modelUp * localNormal.z;

				dynamicColor.Zero();
				for (lnum = 0; lnum < numDynamicLightIndices; lnum++)
				{
					dl = &r_newrefdef.dlights[dynamicLightIndices[lnum]];
					float dx = dl->origin.x - worldPos.x;
					float dy = dl->origin.y - worldPos.y;
					float dz = dl->origin.z - worldPos.z;
					float distanceSq = dx * dx + dy * dy + dz * dz;
					float distance;
					float invDistance;
					float add;
					float ndotl;
					float lightFactor;
					CVector toLight;

					if (distanceSq <= 0.000001f || distanceSq >= dl->intensity * dl->intensity)
						continue;

					distance = sqrt(distanceSq);
					invDistance = 1.0f / distance;
					toLight.x = dx * invDistance;
					toLight.y = dy * invDistance;
					toLight.z = dz * invDistance;
					ndotl = worldNormal.DotProduct(toLight);
					if (ndotl < 0.0f) ndotl = 0.0f;
					lightFactor = 0.18f + 0.82f * ndotl;

					add = 1.0f - distance / dl->intensity;
					add = add * add * (dl->intensity * (1.0f / 256.0f));
					add *= lightFactor;

					VectorMA(dynamicColor, dl->color, add, dynamicColor);
				}

				baseColor += dynamicColor;
			}

			if (baseColor.x > 1.0f) baseColor.x = 1.0f;
			if (baseColor.y > 1.0f) baseColor.y = 1.0f;
			if (baseColor.z > 1.0f) baseColor.z = 1.0f;
			if (baseColor.x < 0.0f) baseColor.x = 0.0f;
			if (baseColor.y < 0.0f) baseColor.y = 0.0f;
			if (baseColor.z < 0.0f) baseColor.z = 0.0f;

			s_vertexcolors[i] = baseColor;
			avertexcolors[normalIndex] = baseColor;
		}
	}
}


inline void GL_EnvMapTexCoords(int vert_index, int normal_index, float* uv, CVector& entangles)
{
	if (s_preparedEnvMapUV)
	{
		uv[0] = s_envmapuv[vert_index][0];
		uv[1] = s_envmapuv[vert_index][1];
		return;
	}

	CVector	v(s_lerped[vert_index][0], s_lerped[vert_index][1], s_lerped[vert_index][2]);
	//    CVector& vert_pos = lerped[vert_index];
	CVector& vert_pos = v;
	CVector normal = avertexnormals.GetNormal(normal_index);
	CVector incident = model_cam_pos - vert_pos;

	incident.Normalize();

	CVector bounce = incident - (2.0f * incident.DotProduct(normal)) * normal;
	CVector angles;

	bounce.VectorToAngles(angles);
	angles += entangles;
	angles.AngleToForwardVector(bounce);

	// do what gl does for envmapping:
	bounce.z += 1.0f;

	float invK = 0.5f / bounce.Length();

	uv[0] = 0.5f + bounce.x * invK;
	uv[1] = 0.5f + bounce.y * invK;
}

void GL_LerpVerts(int nverts, dtrivertx_t* v, dtrivertx_t* ov, const CVector& move, const CVector& frontv, const CVector& backv, float backlerp)
{
	int i;

	/*
		if (0.0f == backlerp)
		{
			for ( i = 0 ; i < nverts; i++, v++, ov++ )
			{
				lerped[i].x = move.x + v->v[0] * frontv.x;
				lerped[i].y = move.y + v->v[1] * frontv.y;
				lerped[i].z = move.z + v->v[2] * frontv.z;
			}
		}
		else if (1.0f == backlerp)
		{
			for ( i = 0 ; i < nverts; i++, v++, ov++ )
			{
				lerped[i].x = move.x + ov->v[0] * backv.x;
				lerped[i].y = move.y + ov->v[1] * backv.y;
				lerped[i].z = move.z + ov->v[2] * backv.z;
			}
		}
		else
		{
			for ( i = 0 ; i < nverts; i++, v++, ov++ )
			{
				lerped[i].x = move.x + ov->v[0] * backv.x + v->v[0] * frontv.x;
				lerped[i].y = move.y + ov->v[1] * backv.y + v->v[1] * frontv.y;
				lerped[i].z = move.z + ov->v[2] * backv.z + v->v[2] * frontv.z;
			}
		}
	*/
	if (0.0f == backlerp)
	{
		for (i = 0; i < nverts; i++, v++, ov++)
		{
			s_lerped[i][0] = move.x + v->v[0] * frontv.x;
			s_lerped[i][1] = move.y + v->v[1] * frontv.y;
			s_lerped[i][2] = move.z + v->v[2] * frontv.z;
		}
	}
	else if (1.0f == backlerp)
	{
		for (i = 0; i < nverts; i++, v++, ov++)
		{
			s_lerped[i][0] = move.x + ov->v[0] * backv.x;
			s_lerped[i][1] = move.y + ov->v[1] * backv.y;
			s_lerped[i][2] = move.z + ov->v[2] * backv.z;
		}
	}
	else
	{
		for (i = 0; i < nverts; i++, v++, ov++)
		{
			s_lerped[i][0] = move.x + ov->v[0] * backv.x + v->v[0] * frontv.x;
			s_lerped[i][1] = move.y + ov->v[1] * backv.y + v->v[1] * frontv.y;
			s_lerped[i][2] = move.z + ov->v[2] * backv.z + v->v[2] * frontv.z;
		}
	}
}

void GL_LerpVerts(int nverts, dtrivertx2_t* v, dtrivertx2_t* ov, const CVector& move, const CVector& frontv, const CVector& backv, float backlerp)
{
	int i;

	/*
		if (0.0f == backlerp)
		{
			for (i=0 ; i < nverts; i++, v++, ov++)
			{
				lerped[i].x = move.x + (v->v >> X_BIT_SHIFT) * frontv.x;
				lerped[i].y = move.y + ((v->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * frontv.y;
				lerped[i].z = move.z + (v->v & Z_BIT_MASK) * frontv.z;
			}
		}
		else if (1.0f == backlerp)
		{
			for (i=0 ; i < nverts; i++, v++, ov++)
			{
				lerped[i].x = move.x + (ov->v >> X_BIT_SHIFT) * backv.x;
				lerped[i].y = move.y + ((ov->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * backv.y;
				lerped[i].z = move.z + (ov->v & Z_BIT_MASK) * backv.z;
			}
		}
		else
		{
			for (i=0 ; i < nverts; i++, v++, ov++)
			{
				lerped[i].x = move.x + (ov->v >> X_BIT_SHIFT) * backv.x + (v->v >> X_BIT_SHIFT) * frontv.x;
				lerped[i].y = move.y + ((ov->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * backv.y + ((v->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * frontv.y;
				lerped[i].z = move.z + (ov->v & Z_BIT_MASK) * backv.z + (v->v & Z_BIT_MASK) * frontv.z;
			}
		}
	*/
	if (0.0f == backlerp)
	{
		for (i = 0; i < nverts; i++, v++, ov++)
		{
			s_lerped[i][0] = move.x + (v->v >> X_BIT_SHIFT) * frontv.x;
			s_lerped[i][1] = move.y + ((v->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * frontv.y;
			s_lerped[i][2] = move.z + (v->v & Z_BIT_MASK) * frontv.z;
		}
	}
	else if (1.0f == backlerp)
	{
		for (i = 0; i < nverts; i++, v++, ov++)
		{
			s_lerped[i][0] = move.x + (ov->v >> X_BIT_SHIFT) * backv.x;
			s_lerped[i][1] = move.y + ((ov->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * backv.y;
			s_lerped[i][2] = move.z + (ov->v & Z_BIT_MASK) * backv.z;
		}
	}
	else
	{
		for (i = 0; i < nverts; i++, v++, ov++)
		{
			s_lerped[i][0] = move.x + (ov->v >> X_BIT_SHIFT) * backv.x + (v->v >> X_BIT_SHIFT) * frontv.x;
			s_lerped[i][1] = move.y + ((ov->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * backv.y + ((v->v & Y_BIT_MASK) >> Y_BIT_SHIFT) * frontv.y;
			s_lerped[i][2] = move.z + (ov->v & Z_BIT_MASK) * backv.z + (v->v & Z_BIT_MASK) * frontv.z;
		}
	}
}

/*
=============
GL_AddAliasGlow
=============
*/
void GL_AddAliasGlow(void)
{
	float	scale;
	float	min;

	oldshadelight = shadelight;

	scale = 0.05 * sin(r_newrefdef.time * 7);

	min = shadelight.x * 0.8;
	shadelight.x += scale;
	if (shadelight.x < min)
		shadelight.x = min;
	min = shadelight.y * 0.8;
	shadelight.y += scale;
	if (shadelight.y < min)
		shadelight.y = min;
	min = shadelight.z * 0.8;
	shadelight.z += scale;
	if (shadelight.z < min)
		shadelight.z = min;

	shademodified = true;
}

#define SURFANIM_LOOP	0x01

/*
=============
GL_DrawAliasFrameLerp

interpolates between two frames and origins

=============
*/
void GL_DrawAliasFrameLerp(entity_t* ent, dmdl_t* paliashdr, float backlerp)
{
	daliasframe_t* frame, * oldframe;
	dtrivertx_t* v, * ov;
	int* order;
	int				count/*, stripFanCount = 0*/;
	float			frontlerp;
	float			alpha;
	CVector			move, frontv, backv;
	int				index_xyz;
	image_t* skin;
	GLenum			primtype;
	int				surface;
	//	CVector			skColor = CVector( 0.0f, 0.0f, 0.0f );		// skin color
	//	int				sCount[4] = {0,0,0,0};
	//	int				lastSurf = -1;
	CVector         vColor;

	frame = (daliasframe_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->frame * s_pmdl->framesize);
	v = frame->verts;

	oldframe = (daliasframe_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->oldframe * s_pmdl->framesize);
	ov = oldframe->verts;

	order = (int*)((byte*)s_pmdl + s_pmdl->ofs_glcmds);

	frontlerp = 1.0 - backlerp;

	//adam: this is the replacement for the old block....
	move = backlerp * oldframe->translate + frontlerp * frame->translate;

	frontv = frontlerp * frame->scale;
	backv = backlerp * oldframe->scale;

	GL_LerpVerts(s_pmdl->num_xyz, v, ov, move, frontv, backv, backlerp);
	if (!R_RenderPrepUseAlias(ent, NULL, s_vertexcolors, NULL, MAX_VERTS))
		GL_LightVerts();

	s_preparedEnvMapUV = FALSE;

	// Ash -- now that we're lerped, build the head matrix
	R_BuildHeadMatrix(ent, gHeadMatrix);

	if (ent->flags & RF_TRANSLUCENT)
	{
		alpha = ent->alpha;
		qglEnable(GL_BLEND);
	}
	else
	{
		alpha = 1.0;
	}

	GL_AliasCullReset();

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
		{
			skin = (image_t*)ent->skin;	// assign skin
		}
		else
		{
			if (ent->skinnum >= numgltextures)
				skin = currentmodel->skins[0];
			else if (ent->skinnum > 0)
			{
				skin = &gltextures[ent->skinnum];
			}
			else
			{
				skin = currentmodel->skins[*order];
				if (!skin)
					skin = currentmodel->skins[0];
			}
		}

		order++;
		surface = *order;
		order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{
			order += 3 * count;
			continue;
		}

		if (!skin)
		{
			skin = r_notexture;
		}

		if (skin->has_alpha)
		{
			GL_AliasSetCull(false);
		}
		else
		{
			GL_AliasSetCull(true);
		}

		GL_Bind(skin->texnum);
		R_TextureFilterBeginBase(skin);

		qglBegin(primtype);

		do
		{
			index_xyz = *order++;

			// texture coordinates come from the draw list
			qglTexCoord2f(((float*)order)[0], ((float*)order)[1]);

			qglColor4f(s_vertexcolors[index_xyz].x,
				s_vertexcolors[index_xyz].y,
				s_vertexcolors[index_xyz].z, alpha);

			//            qglVertex3fv (lerped[index_xyz]);
			qglVertex3fv(s_lerped[index_xyz]);

			order += 2;

		} while (--count);

		qglEnd();


	}

	R_TextureFilterEnd();
	qglDisable(GL_BLEND);
	qglDisable(GL_ALPHA_TEST);

	qglEnable(GL_TEXTURE_2D);
	GL_AliasSetCull(true);

}

void GL_DrawAliasFrameLerpEnv(entity_t* ent, dmdl_t* paliashdr, float backlerp)
{
	daliasframe_t* frame, * oldframe;
	dtrivertx_t* v, * ov;
	int* order;
	int				count/*, stripFanCount = 0*/;
	float			frontlerp;
	float			alpha;
	CVector			move, frontv, backv;
	int				index_xyz;
	image_t* skin;
	GLenum			primtype;
	int				surface;
	//	CVector			skColor = CVector( 0.0f, 0.0f, 0.0f );		// skin color
	//	int				sCount[4] = {0,0,0,0};
	//	int				lastSurf = -1;
	CVector         vColor;

	frame = (daliasframe_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->frame * s_pmdl->framesize);
	v = frame->verts;

	oldframe = (daliasframe_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->oldframe * s_pmdl->framesize);
	ov = oldframe->verts;

	order = (int*)((byte*)s_pmdl + s_pmdl->ofs_glcmds);

	frontlerp = 1.0 - backlerp;

	//adam: this is the replacement for the old block....
	move = backlerp * oldframe->translate + frontlerp * frame->translate;

	frontv = frontlerp * frame->scale;
	backv = backlerp * oldframe->scale;

	GL_LerpVerts(s_pmdl->num_xyz, v, ov, move, frontv, backv, backlerp);
	if (!R_RenderPrepUseAlias(ent, NULL, s_vertexcolors, NULL, MAX_VERTS))
		GL_LightVerts();
	s_preparedEnvMapUV = FALSE;

	// Ash -- now that we're lerped, build the head matrix
	R_BuildHeadMatrix(ent, gHeadMatrix);

	if (ent->flags & RF_TRANSLUCENT)
	{
		alpha = ent->alpha;
		qglEnable(GL_BLEND);
	}
	else
	{
		alpha = 1.0;
	}

	GL_AliasCullReset();

	float uv[2];
	float texEnvColor[4];

	GL_EnableMultitexture(true);

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
		{
			skin = (image_t*)ent->skin;	// assign skin
		}
		else
		{
			if (ent->skinnum >= numgltextures)
				skin = currentmodel->skins[0];
			else if (ent->skinnum > 0)
			{
				skin = &gltextures[ent->skinnum];
			}
			else
			{
				skin = currentmodel->skins[*order];
				if (!skin)
					skin = currentmodel->skins[0];
			}
		}

		order++;
		surface = *order;
		order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{
			order += 3 * count;
			continue;
		}

		if (!skin)
		{
			skin = r_notexture;
		}

		if (skin->has_alpha)
		{
			GL_AliasSetCull(false);
		}
		else
		{
			GL_AliasSetCull(true);
		}

		if (qglMTexCoord2fSGIS)
		{
			GL_MBind(GL_TEXTURE0_SGIS, skin->texnum);
			GL_MBind(GL_TEXTURE1_SGIS, pEnvMapSkin->texnum);

			R_TextureFilterBeginEnvmap(skin, pEnvMapSkin, gl_texenvcolor->value);

			GL_MTexEnv(GL_TEXTURE0_SGIS, GL_REPLACE);
			GL_MTexEnv(GL_TEXTURE1_SGIS, GL_BLEND);

			texEnvColor[0] = texEnvColor[1] = texEnvColor[2] = texEnvColor[3] = gl_texenvcolor->value;
			GL_SelectTexture(GL_TEXTURE1_SGIS);
			qglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texEnvColor);

			qglBegin(primtype);

			do
			{
				index_xyz = *order++;

				// texture coordinates come from the draw list
				qglMTexCoord2fSGIS(GL_TEXTURE0_SGIS, ((float*)order)[0], ((float*)order)[1]);

				GL_EnvMapTexCoords(index_xyz, ov[index_xyz].lightnormalindex, uv, ent->angles);

				qglMTexCoord2fSGIS(GL_TEXTURE1_SGIS, uv[0], uv[1]);

				qglColor4f(s_vertexcolors[index_xyz].x,
					s_vertexcolors[index_xyz].y,
					s_vertexcolors[index_xyz].z, alpha);

				//				qglVertex3fv (lerped[index_xyz]);
				qglVertex3fv(s_lerped[index_xyz]);

				order += 2;

			} while (--count);

			qglEnd();


			texEnvColor[0] = texEnvColor[1] = texEnvColor[2] = texEnvColor[3] = 0;
			GL_SelectTexture(GL_TEXTURE1_SGIS);
			qglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texEnvColor);

		}
	}

	GL_EnableMultitexture(false);

	R_TextureFilterEnd();
	qglDisable(GL_BLEND);
	qglDisable(GL_ALPHA_TEST);

	qglEnable(GL_TEXTURE_2D);
	GL_AliasSetCull(true);
}

/*
=============
GL_DrawAliasFrameLerpFogShell

Ash:  Does what FrameLerp does,
except draws a fog shell instead
of the skin.


=============
*/

void GL_DrawAliasFrameLerpFogShell(entity_t* ent, dmdl_t* paliashdr)
{
	daliasframe_t* frame;
	int* order;
	int				count/*, stripFanCount = 0*/;
	float			alpha;
	CVector			move;
	int				index_xyz;
	GLenum			primtype;
	int				surface;
	float           zOffset;

	frame = (daliasframe_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->frame * s_pmdl->framesize);

	order = (int*)((byte*)s_pmdl + s_pmdl->ofs_glcmds);

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		order++;
		surface = *order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{
			order += ((2) + 1) * count;
			continue;
		}

		if ((!stricmp(s_psurface[surface].name, "s_head")) && gfHaveHeadMatrix)
		{
			qglPushMatrix();
			qglLoadMatrixf(gHeadMatrix);
		}

		zOffset = ent->origin.z - ((ent->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z);

		qglBegin(primtype);

		do
		{
			index_xyz = *order++;
			order += 2;

			//            alpha = GL_GetFogSampleAlphaAtZ(lerped[index_xyz].z+zOffset, gFogSample);
			alpha = GL_GetFogSampleAlphaAtZ(s_lerped[index_xyz][2] + zOffset, gFogSample);

#pragma message("TODO Ash: remove this 3dfx minidriver fix")
			if (alpha < 0.001f) alpha = 0.0f; // temp fix for 3dfx minidriver

			// normals and vertexes come from the frame list
			qglColor4f(gFogSample.fog_r, gFogSample.fog_g, gFogSample.fog_b, alpha);
			//            qglVertex3fv (lerped[index_xyz]);
			qglVertex3fv(s_lerped[index_xyz]);
		} while (--count);

		// increment the strip and fan counter
//		stripFanCount++;
		qglEnd();

		if ((!stricmp(s_psurface[surface].name, "s_head")) && gfHaveHeadMatrix)
		{
			qglPopMatrix();
		}
	}
}

/*
=============
GL_DrawAliasFrameLerp2
=============
*/
void GL_DrawAliasFrameLerp2(entity_t* ent, dmdl_t* paliashdr, float backlerp)
{
	daliasframe2_t* frame, * oldframe;
	dtrivertx2_t* v, * ov;
	int* order;
	int				count/*, stripFanCount = 0*/;
	float			frontlerp;
	float			alpha;
	CVector			move, frontv, backv;
	int				index_xyz;
	image_t* skin;
	GLenum			primtype;
	int				surface;
	//	CVector			skColor = CVector( 0.0f, 0.0f, 0.0f );		// skin color
	//	int				sCount[4] = {0,0,0,0};
	//	int				lastSurf = -1;
	CVector         vColor;
	dstvert_t* stvert;

	frame = (daliasframe2_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->frame * s_pmdl->framesize);
	v = frame->verts;

	oldframe = (daliasframe2_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->oldframe * s_pmdl->framesize);
	ov = oldframe->verts;

	stvert = (dstvert_t*)(byte*)s_pmdl + s_pmdl->ofs_st;

	order = (int*)((byte*)s_pmdl + s_pmdl->ofs_glcmds);

	frontlerp = 1.0 - backlerp;

	//adam: this is the replacement for the old block....
	move = backlerp * oldframe->translate + frontlerp * frame->translate;

	frontv = frontlerp * frame->scale;
	backv = backlerp * oldframe->scale;

	GL_LerpVerts(s_pmdl->num_xyz, v, ov, move, frontv, backv, backlerp);
	if (!R_RenderPrepUseAlias(ent, NULL, s_vertexcolors, NULL, MAX_VERTS))
		GL_LightVerts();
	s_preparedEnvMapUV = FALSE;

	if (ent->flags & RF_TRANSLUCENT)
	{
		alpha = ent->alpha;
		qglEnable(GL_BLEND);
	}
	else
	{
		alpha = 1.0;
	}

	GL_AliasCullReset();

	if (gl_vertex_arrays->value)
	{
		float colorArray[MAX_VERTS * 3];
		float normalArray[MAX_VERTS * 3];
		//		float texcoordArray[MAX_VERTS*2];
		/*
				qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
				qglTexCoordPointer( 2, GL_FLOAT, 8, texcoordArray );

				// get the texture coords
				index_xyz = 0;
				while (1)
				{
					count = *order++;
					if( !count )
						break;

					if( count < 0 )
						count = -count;

					order++;
					surface = *order++;

					do
					{
						order++;
						texcoordArray[index_xyz * 2 + 0] = ( ( float * )order )[0];
						texcoordArray[index_xyz * 2 + 1] = ( ( float * )order )[1];
						index_xyz++;
						order += 2;
					} while( --count );
				}
		*/
		//
		// pre light everything
		//
		for (index_xyz = 0; index_xyz < s_pmdl->num_xyz; index_xyz++)
		{
			colorArray[index_xyz * 3 + 0] = s_vertexcolors[index_xyz].x;
			colorArray[index_xyz * 3 + 1] = s_vertexcolors[index_xyz].y;
			colorArray[index_xyz * 3 + 2] = s_vertexcolors[index_xyz].z;

			normalArray[index_xyz * 3 + 0] = avertexnormals.GetNormal(ov[index_xyz].lightnormalindex)[0];
			normalArray[index_xyz * 3 + 1] = avertexnormals.GetNormal(ov[index_xyz].lightnormalindex)[1];
			normalArray[index_xyz * 3 + 2] = avertexnormals.GetNormal(ov[index_xyz].lightnormalindex)[2];
		}

		qglEnableClientState(GL_VERTEX_ARRAY);
		qglVertexPointer(3, GL_FLOAT, 16, s_lerped);

		qglEnableClientState(GL_COLOR_ARRAY);
		qglColorPointer(3, GL_FLOAT, 0, colorArray);

		qglEnableClientState(GL_NORMAL_ARRAY);
		qglNormalPointer(GL_FLOAT, 12, normalArray);

		order = (int*)((byte*)s_pmdl + s_pmdl->ofs_glcmds);

		if (qglLockArraysEXT != 0)
		{
			qglLockArraysEXT(0, s_pmdl->num_xyz);
		}


		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;
			if (!count)
				break;		// done
			if (count < 0)
			{
				count = -count;
				primtype = GL_TRIANGLE_FAN;
			}
			else
				primtype = GL_TRIANGLE_STRIP;

			if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
			{
				skin = (image_t*)ent->skin;	// assign skin
			}
			else
			{
				if (ent->skinnum >= numgltextures)
					skin = currentmodel->skins[0];
				else if (ent->skinnum > 0)
				{
					skin = &gltextures[ent->skinnum];
				}
				else
				{
					skin = currentmodel->skins[*order];
					if (!skin)
						skin = currentmodel->skins[0];
				}
			}

			order++;
			surface = *order;
			order++;

			// dont draw this surface
			if (s_psurface[surface].flags & SRF_NODRAW)
			{
				order += 3 * count;
				continue;
			}

			if (!skin)
			{
				skin = r_notexture;
			}

			if (skin->has_alpha)
			{
				GL_AliasSetCull(false);
			}
			else
			{
				GL_AliasSetCull(true);
			}

			GL_Bind(skin->texnum);
			R_TextureFilterBeginBase(skin);

			qglBegin(primtype);

			do
			{
				index_xyz = *order++;

				// texture coordinates come from the draw list
				qglTexCoord2f(((float*)order)[0], ((float*)order)[1]);

				//				qglColor4f(s_vertexcolors[index_xyz].x, 
				//						   s_vertexcolors[index_xyz].y, 
				//						   s_vertexcolors[index_xyz].z, alpha);

				//				qglVertex3fv( lerped[index_xyz] );
				qglArrayElement(index_xyz);

				order += 2;

			} while (--count);

			qglEnd();

		}

		if (qglUnlockArraysEXT != 0)
			qglUnlockArraysEXT();
	}
	else
	{
		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;
			if (!count)
				break;		// done
			if (count < 0)
			{
				count = -count;
				primtype = GL_TRIANGLE_FAN;
			}
			else
				primtype = GL_TRIANGLE_STRIP;

			if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
			{
				skin = (image_t*)ent->skin;	// assign skin
			}
			else
			{
				if (ent->skinnum >= numgltextures)
					skin = currentmodel->skins[0];
				else if (ent->skinnum > 0)
				{
					skin = &gltextures[ent->skinnum];
				}
				else
				{
					skin = currentmodel->skins[*order];
					if (!skin)
						skin = currentmodel->skins[0];
				}
			}

			order++;
			surface = *order;
			order++;

			// dont draw this surface
			if (s_psurface[surface].flags & SRF_NODRAW)
			{
				order += 3 * count;
				continue;
			}

			if (!skin)
			{
				skin = r_notexture;
			}

			if (skin->has_alpha)
			{
				GL_AliasSetCull(false);
			}
			else
			{
				GL_AliasSetCull(true);
			}

			GL_Bind(skin->texnum);
			R_TextureFilterBeginBase(skin);

			qglBegin(primtype);

			do
			{
				index_xyz = *order++;

				// texture coordinates come from the draw list
				qglTexCoord2f(((float*)order)[0], ((float*)order)[1]);

				qglColor4f(s_vertexcolors[index_xyz].x,
					s_vertexcolors[index_xyz].y,
					s_vertexcolors[index_xyz].z, alpha);

				//				qglVertex3fv( lerped[index_xyz] );
				qglVertex3fv(s_lerped[index_xyz]);

				order += 2;

			} while (--count);

			qglEnd();

		}
	}

	R_TextureFilterEnd();
	qglDisable(GL_BLEND);
	qglDisable(GL_ALPHA_TEST);

	qglEnable(GL_TEXTURE_2D);
	GL_AliasSetCull(true);
}

void GL_DrawAliasFrameLerp2Env(entity_t* ent, dmdl_t* paliashdr, float backlerp)
{
	daliasframe2_t* frame, * oldframe;
	dtrivertx2_t* v, * ov;
	int* order;
	int				count/*, stripFanCount = 0*/;
	float			frontlerp;
	float			alpha;
	CVector			move, frontv, backv;
	int				index_xyz;
	image_t* skin;
	GLenum			primtype;
	int				surface;
	//	CVector			skColor = CVector( 0.0f, 0.0f, 0.0f );		// skin color
	//	int				sCount[4] = {0,0,0,0};
	//	int				lastSurf = -1;
	CVector         vColor;
	dstvert_t* stvert;

	frame = (daliasframe2_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->frame * s_pmdl->framesize);
	v = frame->verts;

	oldframe = (daliasframe2_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->oldframe * s_pmdl->framesize);
	ov = oldframe->verts;

	stvert = (dstvert_t*)(byte*)s_pmdl + s_pmdl->ofs_st;

	order = (int*)((byte*)s_pmdl + s_pmdl->ofs_glcmds);

	frontlerp = 1.0 - backlerp;

	//adam: this is the replacement for the old block....
	move = backlerp * oldframe->translate + frontlerp * frame->translate;

	frontv = frontlerp * frame->scale;
	backv = backlerp * oldframe->scale;

	GL_LerpVerts(s_pmdl->num_xyz, v, ov, move, frontv, backv, backlerp);
	if (!R_RenderPrepUseAlias(ent, NULL, s_vertexcolors, NULL, MAX_VERTS))
		GL_LightVerts();
	s_preparedEnvMapUV = FALSE;

	if (ent->flags & RF_TRANSLUCENT)
	{
		alpha = ent->alpha;
		qglEnable(GL_BLEND);
	}
	else
	{
		alpha = 1.0;
	}

	GL_AliasCullReset();

	float uv[2];
	float texEnvColor[4];

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
		{
			skin = (image_t*)ent->skin;	// assign skin
		}
		else
		{
			if (ent->skinnum >= numgltextures)
				skin = currentmodel->skins[0];
			else if (ent->skinnum > 0)
			{
				skin = &gltextures[ent->skinnum];
			}
			else
			{
				skin = currentmodel->skins[*order];
				if (!skin)
					skin = currentmodel->skins[0];
			}
		}

		order++;
		surface = *order;
		order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{
			order += 3 * count;
			continue;
		}

		if (!skin)
		{
			skin = r_notexture;
		}

		if (skin->has_alpha)
		{
			GL_AliasSetCull(false);
		}
		else
		{
			GL_AliasSetCull(true);
		}

		if (qglMTexCoord2fSGIS)
		{
			GL_EnableMultitexture(true);

			GL_MBind(GL_TEXTURE0_SGIS, skin->texnum);
			GL_MBind(GL_TEXTURE1_SGIS, pEnvMapSkin->texnum);

			R_TextureFilterBeginEnvmap(skin, pEnvMapSkin, gl_texenvcolor->value);

			GL_MTexEnv(GL_TEXTURE0_SGIS, GL_REPLACE);
			GL_MTexEnv(GL_TEXTURE1_SGIS, GL_BLEND);

			texEnvColor[0] = texEnvColor[1] = texEnvColor[2] = texEnvColor[3] = gl_texenvcolor->value;
			GL_SelectTexture(GL_TEXTURE1_SGIS);
			qglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texEnvColor);

			qglBegin(primtype);

			do
			{
				index_xyz = *order++;

				// texture coordinates come from the draw list
				qglMTexCoord2fSGIS(GL_TEXTURE0_SGIS, ((float*)order)[0], ((float*)order)[1]);

				GL_EnvMapTexCoords(index_xyz, ov[index_xyz].lightnormalindex, uv, ent->angles);

				qglMTexCoord2fSGIS(GL_TEXTURE1_SGIS, uv[0], uv[1]);

				qglColor4f(s_vertexcolors[index_xyz].x,
					s_vertexcolors[index_xyz].y,
					s_vertexcolors[index_xyz].z, alpha);

				qglVertex3fv(s_lerped[index_xyz]);

				order += 2;

			} while (--count);

			qglEnd();


			texEnvColor[0] = texEnvColor[1] = texEnvColor[2] = texEnvColor[3] = 0;
			GL_SelectTexture(GL_TEXTURE1_SGIS);
			qglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texEnvColor);

			GL_EnableMultitexture(false);
		}
		else
		{
			order += 3;
		}
	}

	R_TextureFilterEnd();
	qglDisable(GL_BLEND);
	qglDisable(GL_ALPHA_TEST);

	qglEnable(GL_TEXTURE_2D);
	GL_AliasSetCull(true);
}

/*
=============
GL_DrawAliasFrameLerp2FogShell

Ash:  Does what FrameLerp does,
except draws a fog shell instead
of the skin.


=============
*/

void GL_DrawAliasFrameLerp2FogShell(entity_t* ent, dmdl_t* paliashdr)
{
	daliasframe2_t* frame;
	int* order;
	int				count/*, stripFanCount = 0*/;
	float			alpha;
	CVector			move;
	int				index_xyz;
	GLenum			primtype;
	int				surface;
	float			r, g, b;
	float           zOffset;

	frame = (daliasframe2_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->frame * s_pmdl->framesize);

	order = (int*)((byte*)s_pmdl + s_pmdl->ofs_glcmds);

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		order++;
		surface = *order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{
			order += ((2) + 1) * count;
			continue;
		}

		if ((!stricmp(s_psurface[surface].name, "s_head")) && gfHaveHeadMatrix)
		{
			qglPushMatrix();
			qglLoadMatrixf(gHeadMatrix);
		}

		zOffset = ent->origin.z - ((ent->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z);

		qglBegin(primtype);

		r = gFogSample.fog_r;
		g = gFogSample.fog_g;
		b = gFogSample.fog_b;
		do
		{
			index_xyz = *order++;
			order += 2;

			//            alpha = GL_GetFogSampleAlphaAtZ(lerped[index_xyz].z+zOffset, gFogSample);
			alpha = GL_GetFogSampleAlphaAtZ(s_lerped[index_xyz][2] + zOffset, gFogSample);

#pragma message("TODO Ash: remove this 3dfx minidriver fix")
			if (alpha < 0.001f) alpha = 0.0f; // temp fix for 3dfx minidriver

			// normals and vertexes come from the frame list
			qglColor4f(r, g, b, alpha);
			//            qglVertex3fv (lerped[index_xyz]);
			qglVertex3fv(s_lerped[index_xyz]);
		} while (--count);

		// increment the strip and fan counter
//		stripFanCount++;
		qglEnd();

		if ((!stricmp(s_psurface[surface].name, "s_head")) && gfHaveHeadMatrix)
		{
			qglPopMatrix();
		}
	}
}

/*
=============
GL_DrawClusterFrameLerp

interpolates between two frames and origins

=============
*/




/*
=============
GL_DrawSpotShadow
=============
*/
extern	CVector			lightspot;
void CreateSurfaceSpritePoly(surfSprite_t& surfSprite, int nWidth, int nHeight);
void BoundPoly(int numverts, CVector* verts, CVector& mins, CVector& maxs);
qboolean PointInSurface(msurface_t* pSurface, CVector vPoint, float radius, CVector& vMins, CVector& vMaxs);

#define MAX_HIT_SURFACES	6

void GL_DrawSpotShadow(entity_t* ent, CVector& lightdir)
{
	surfSprite_t	surfSprite;
	msurface_t* pCurrentSurface;
	faceList_t* pFaceList;
	int				i, numHitSurfaces, nIndex;
	CVector			vMins, vMaxs;
	msurface_t* hitSurfaces[MAX_HIT_SURFACES];
	int				nWidth, nHeight;//, nLHeight;
	model_t* pCurrentModel;
	float			fHeight;

	fHeight = lightspot.z;

	if (r_worldmodel == NULL)
	{
		return;
	}

	if (ent->flags & RF_WEAPONMODEL)
	{
		return;
	}

	if (pointSurface.polys == NULL)
	{
		return;
	}

	if (pointSurface.polys->verts == NULL)
	{
		return;
	}

	if (pointSurface.texinfo->flags & SURF_MIDTEXTURE)
	{
		return;
	}

	for (i = 0; i < MAX_HIT_SURFACES; i++)
	{
		hitSurfaces[i] = NULL;
	}

	qglDepthMask(0);

	surfSprite.vOrigin = ent->origin;
	//	surfSprite.vOrigin.z	= pointSurface.polys->verts[0].z;
	//	surfSprite.vOrigin.z	= ent->origin.z + s_pmdl->org.z;
	surfSprite.vOrigin.z = fHeight;

	surfSprite.nFrame = 0;
	surfSprite.fScale = 1.0;
	surfSprite.nFlags = 0;
	surfSprite.fRoll = -ent->angles.y + 90;
	surfSprite.pImage = r_shadowtexture;
	surfSprite.pModel = NULL;
	surfSprite.vLightValue = shadelight;

	nWidth = (ent->maxs.x - ent->mins.x) * 0.5;
	nHeight = (ent->maxs.y - ent->mins.y) * 0.5;

	nIndex = pointSurface.plane->planeIndex;

	pFaceList = r_worldmodel->planePolys->planes[nIndex];

	if (pFaceList == NULL)
	{
		return;
	}

	numHitSurfaces = 0;

	pCurrentModel = r_worldmodel;

	for (i = 0; i < pFaceList->numFaces; i++)
	{
		pCurrentSurface = pCurrentModel->surfaces + pFaceList->faceNum[i];

		// SCG[5/10/99]: Get the mins maxs for this surface
		BoundPoly(pCurrentSurface->polys->numverts, pCurrentSurface->polys->verts, vMins, vMaxs);

		if (PointInSurface(pCurrentSurface, surfSprite.vOrigin, nWidth / 2, vMins, vMaxs) && numHitSurfaces < MAX_HIT_SURFACES)
			hitSurfaces[numHitSurfaces++] = pCurrentSurface;
	}

	GL_Bind(surfSprite.pImage->texnum);
	R_TextureFilterBeginBase(surfSprite.pImage);

	if (ent->flags & RF_TRANSLUCENT)
	{
		qglColor4f(surfSprite.vLightValue.x, surfSprite.vLightValue.y, surfSprite.vLightValue.z, ent->alpha);
	}
	else
	{
		qglColor4f(surfSprite.vLightValue.x, surfSprite.vLightValue.y, surfSprite.vLightValue.z, 1.0);
	}

	for (i = 0; i < numHitSurfaces; i++)
	{
		surfSprite.pSurf = hitSurfaces[i];
		surfSprite.vNormal = hitSurfaces[i]->plane->normal;
		surfSprite.numpoints = 0;

		R_LightPoint(surfSprite.vOrigin, surfSprite.vLightValue);

		CreateSurfaceSpritePoly(surfSprite, nWidth, nHeight);

		if (surfSprite.numpoints > 8)
		{
			R_TextureFilterEnd();
			return;
		}

		qglBegin(GL_POLYGON);
		for (int j = 0; j < surfSprite.numpoints; j++)
		{
			qglTexCoord2f(surfSprite.points[j].s, surfSprite.points[j].t);
			qglVertex3fv(surfSprite.points[j].pos);
		}
		qglEnd();
	}

	R_TextureFilterEnd();
	qglDepthMask(1);
}


/*
** R_CullAliasModel
*/
static qboolean R_CullAliasModel(CVector bbox[8], entity_t* e)
{
	CVector         mins, maxs;
	int i;
	CVector			vectors[3];
	CVector			thismins, oldmins, thismaxs, oldmaxs;
	daliasframe_t* pframe, * poldframe;
	daliasframe2_t* pframe2, * poldframe2;
	CVector			angles;
	CVector			scaled_org;
	CVector			scale;
	CVector			frame_scale, frame_translate;
	CVector			oldframe_scale, oldframe_translate;
	int				version, same_frame = false;

	if ((e->frame >= s_pmdl->num_frames) || (e->frame < 0))
	{
		//		ri.Con_Printf (PRINT_ALL, "R_CullAliasModel: no such frame %d\n", e->frame);
		e->frame = 0;
	}
	if ((e->oldframe >= s_pmdl->num_frames) || (e->oldframe < 0))
	{
		//		ri.Con_Printf (PRINT_ALL, "R_CullAliasModel: no such oldframe %d\n", e->oldframe);
		e->oldframe = 0;
	}

	version = LittleLong(s_pmdl->version);

	if (version == ALIAS_VERSION2) {
		pframe2 = (daliasframe2_t*)((byte*)s_pmdl +
			s_pmdl->ofs_frames +
			e->frame * s_pmdl->framesize);

		poldframe2 = (daliasframe2_t*)((byte*)s_pmdl +
			s_pmdl->ofs_frames +
			e->oldframe * s_pmdl->framesize);
	}
	else {
		pframe = (daliasframe_t*)((byte*)s_pmdl +
			s_pmdl->ofs_frames +
			e->frame * s_pmdl->framesize);

		poldframe = (daliasframe_t*)((byte*)s_pmdl +
			s_pmdl->ofs_frames +
			e->oldframe * s_pmdl->framesize);
	}

	/*
	** compute axially aligned mins and maxs
	*/
	if (version == ALIAS_VERSION2) {
		if (pframe2 == poldframe2)
			same_frame = true;
		scale.x = X_BIT_RES;
		scale.y = Y_BIT_RES;
		scale.z = Z_BIT_RES;
		frame_scale = pframe2->scale;
		frame_translate = pframe2->translate;
		oldframe_scale = poldframe2->scale;
		oldframe_translate = poldframe2->translate;
	}
	else {
		if (pframe == poldframe)
			same_frame = true;
		scale.x = scale.y = scale.z = 255;
		frame_scale = pframe->scale;
		frame_translate = pframe->translate;
		oldframe_scale = poldframe->scale;
		oldframe_translate = poldframe->translate;
	}
	if (same_frame)
	{
		mins = frame_translate;
		maxs.x = mins.x + frame_scale.x * scale.x;
		maxs.y = mins.y + frame_scale.y * scale.y;
		maxs.z = mins.z + frame_scale.z * scale.z;
	}
	else
	{
		thismins = frame_translate;
		thismaxs.x = thismins.x + frame_scale.x * scale.x;
		thismaxs.y = thismins.y + frame_scale.y * scale.y;
		thismaxs.z = thismins.z + frame_scale.z * scale.z;

		oldmins = oldframe_translate;
		oldmaxs.x = oldmins.x + oldframe_scale.x * scale.x;
		oldmaxs.y = oldmins.y + oldframe_scale.y * scale.y;
		oldmaxs.z = oldmins.z + oldframe_scale.z * scale.z;

		if (thismins.x < oldmins.x)
			mins.x = thismins.x;
		else
			mins.x = oldmins.x;
		if (thismaxs.x > oldmaxs.x)
			maxs.x = thismaxs.x;
		else
			maxs.x = oldmaxs.x;

		if (thismins.y < oldmins.y)
			mins.y = thismins.y;
		else
			mins.y = oldmins.y;
		if (thismaxs.y > oldmaxs.y)
			maxs.y = thismaxs.y;
		else
			maxs.y = oldmaxs.y;

		if (thismins.z < oldmins.z)
			mins.z = thismins.z;
		else
			mins.z = oldmins.z;
		if (thismaxs.z > oldmaxs.z)
			maxs.z = thismaxs.z;
		else
			maxs.z = oldmaxs.z;
	}

	//	Nelno:	scale bounding box
	mins.x = mins.x * e->render_scale.x;
	mins.y = mins.y * e->render_scale.y;
	mins.z = mins.z * e->render_scale.z;
	maxs.x = maxs.x * e->render_scale.x;
	maxs.y = maxs.y * e->render_scale.y;
	maxs.z = maxs.z * e->render_scale.z;

	//	Nelno:	compute scaled center of model
	scaled_org.x = e->origin.x - ((e->render_scale.x * s_pmdl->org.x) - s_pmdl->org.x);
	scaled_org.y = e->origin.y - ((e->render_scale.y * s_pmdl->org.y) - s_pmdl->org.y);
	scaled_org.z = e->origin.z - ((e->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z);

	/*
	** compute a full bounding box
	*/
	for (i = 0; i < 8; i++)
	{
		CVector   tmp;

		if (i & 1)
			tmp.x = mins.x;
		else
			tmp.x = maxs.x;

		if (i & 2)
			tmp.y = mins.y;
		else
			tmp.y = maxs.y;

		if (i & 4)
			tmp.z = mins.z;
		else
			tmp.z = maxs.z;

		bbox[i] = tmp;
	}

	/*
	** rotate the bounding box
	*/
	angles = e->angles;
	angles.y = -angles.y;
	AngleToVectors(angles, vectors[0], vectors[1], vectors[2]);

	e->mins = mins;
	e->maxs = maxs;

	for (i = 0; i < 8; i++)
	{
		CVector tmp;

		tmp = bbox[i];

		bbox[i].x = DotProduct(vectors[0], tmp);
		bbox[i].y = -DotProduct(vectors[1], tmp);
		bbox[i].z = DotProduct(vectors[2], tmp);

		bbox[i] = bbox[i] + scaled_org;
	}

	{
		int p, f, aggregatemask = ~0;

		for (p = 0; p < 8; p++)
		{
			int mask = 0;

			for (f = 0; f < 4; f++)
			{
				float dp = DotProduct(frustum[f].normal, bbox[p]);

				if ((dp - frustum[f].dist) < 0)
				{
					mask |= (1 << f);
				}
			}

			aggregatemask &= mask;
		}

		if (aggregatemask)
		{
			return true;
		}

		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void R_RotateAndScaleEntity(entity_t* e)
{
	CVector		scale_ofs;

	if (e->render_scale.x == 0.0 && e->render_scale.y == 0.0 && e->render_scale.z == 0.0)
	{
		//		ri.Sys_Error (ERR_FATAL, "Entity with model %s has 0, 0, 0 render_scale.\n", currentmodel->name);
		ri.Con_Printf(PRINT_DEVELOPER, "R_DrawAliasModel: %s has 0 scale: %f %f %f\n", currentmodel->name, e->render_scale.x, e->render_scale.y, e->render_scale.z);
		e->render_scale.x = e->render_scale.y = e->render_scale.z = 1.0;

	}

	//	Nelno:	add x, y and z origin offsets here.
	scale_ofs.x = (e->render_scale.x * s_pmdl->org.x) - s_pmdl->org.x;
	scale_ofs.y = (e->render_scale.y * s_pmdl->org.y) - s_pmdl->org.y;
	scale_ofs.z = (e->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z;

	qglTranslatef(e->origin.x - scale_ofs.x, e->origin.y - scale_ofs.y, e->origin.z - scale_ofs.z);

	qglRotatef(e->angles.y, 0, 0, 1);
	qglRotatef(-e->angles.x, 0, 1, 0);
	qglRotatef(-e->angles.z, 1, 0, 0);

	qglScalef(e->render_scale.x, e->render_scale.y, e->render_scale.z);
}


void R_BuildHeadMatrix(entity_t* e, float* pMatrix)
{
	CVector		scale_ofs;
	CVector     hardPt, hardPt1, hardPt2;
	dsurface_t* pSurfaceList = (dsurface_t*)((byte*)s_pmdl + s_pmdl->ofs_surfaces);
	dsurface_t* pSurface = NULL;
	CVector     deltaAngles;

	gfHaveHeadMatrix = false;

	for (int i = 0; i < s_pmdl->num_surfaces; i++)
	{
		if (!stricmp(pSurfaceList[i].name, "s_head"))
		{
			pSurface = &pSurfaceList[i];
			break;
		}
	}

	if ((!pSurface) || (-1 == pSurface->num_uvframes))
		return;

	// check that we have cluster info (size in e->count)
	if (0 == e->count)
		return;

	// look angles stored in entry 0 in cluster array e->pDef
	deltaAngles = e->pDef[0].angles;

	dtriangle_t* pHPTri = (dtriangle_t*)((byte*)s_pmdl + s_pmdl->ofs_tris) + pSurface->num_uvframes;

	//    hardPt += lerped[pHPTri->index_xyz[0]];
	//    hardPt += lerped[pHPTri->index_xyz[1]];
	//    hardPt += lerped[pHPTri->index_xyz[2]];
	hardPt += s_lerped[pHPTri->index_xyz[0]];
	hardPt += s_lerped[pHPTri->index_xyz[1]];
	hardPt += s_lerped[pHPTri->index_xyz[2]];

	hardPt = (1.0f / 3.0f) * hardPt;

	qglPushMatrix();
	qglLoadMatrixf(gHeadMatrix);    // this was saved off before RotateAndScaleEntity

	if (e->render_scale.x == 0.0 && e->render_scale.y == 0.0 && e->render_scale.z == 0.0)
	{
		//		ri.Sys_Error (ERR_FATAL, "Entity with model %s has 0, 0, 0 render_scale.\n", currentmodel->name);
		ri.Con_Printf(PRINT_DEVELOPER, "R_DrawAliasModel: %s has 0 scale: %f %f %f\n", currentmodel->name, e->render_scale.x, e->render_scale.y, e->render_scale.z);
		e->render_scale.x = e->render_scale.y = e->render_scale.z = 1.0;
	}

	//	Nelno:	add x, y and z origin offsets here.
	scale_ofs.x = (e->render_scale.x * s_pmdl->org.x) - s_pmdl->org.x;
	scale_ofs.y = (e->render_scale.y * s_pmdl->org.y) - s_pmdl->org.y;
	scale_ofs.z = (e->render_scale.z * s_pmdl->org.z) - s_pmdl->org.z;

	qglTranslatef(e->origin.x - scale_ofs.x, e->origin.y - scale_ofs.y, e->origin.z - scale_ofs.z);

	qglRotatef(e->angles.y, 0, 0, 1);
	qglRotatef(-e->angles.x, 0, 1, 0);
	qglRotatef(-e->angles.z, 1, 0, 0);

	// rotate the head
	qglTranslatef(hardPt.x, hardPt.y, hardPt.z);
	qglRotatef(deltaAngles.yaw, 0, 0, 1);
	qglRotatef(deltaAngles.pitch, 0, 1, 0);
	qglTranslatef(-hardPt.x, -hardPt.y, -hardPt.z);

	qglScalef(e->render_scale.x, e->render_scale.y, e->render_scale.z);

	qglGetFloatv(GL_MODELVIEW_MATRIX, pMatrix);
	gfHaveHeadMatrix = true;

	qglPopMatrix();
}

/*
=================
R_DrawAliasModel

=================
*/

void R_DrawAliasModel(entity_t* ent)
{
	int			i;
	CVector		bbox[8];
	int			version;
	float		blend_colors[4] = { 0.5, 0.5, 0.5, 0.5 };
	bool        fDLight;
	CVector     dloc;
	bool        fDrawShadow;

	memset(&pointSurface, 0, sizeof(msurface_t));

	GL_SetState(GLSTATE_PRESET1 | GLSTATE_ALPHA_TEST);
	GL_SetFunc(GLSTATE_ALPHA_FUNC, GL_GREATER, 0.0);

	currentmodel = (model_t*)ent->model;

	s_pmdl = (dmdl_t*)currentmodel->extradata;
	s_psurface = (dsurface_t*)((byte*)s_pmdl + s_pmdl->ofs_surfaces);

	if (!(ent->flags & RF_WEAPONMODEL))
	{
		if (R_CullAliasModel(bbox, ent))
			return;
	}

	ent_draw_count++;

	version = LittleLong(s_pmdl->version);

	if (ent->flags & RF_WEAPONMODEL)
	{
		//if ( r_lefthand->value == 2 || r_drawweapon->value == 0)
		if (weapon_visible->value == 0)
			return;

		qglDisable(GL_FOG);
	}

	// resolve shadowing
	if (gl_shadows->value &&
		(r_worldmodel != NULL) &&
		r_worldmodel->lightdata &&
		!(ent->flags & (RF_TRANSLUCENT | RF_WEAPONMODEL)) &&
		version != ALIAS_VERSION2)
	{
		fDrawShadow = true;
	}
	else
		fDrawShadow = false;

	//
	// get lighting information
	//
	CVector fixedshadevector, fixedshadeangles, shadeangles;
	qboolean preparedLighting;

	preparedLighting = R_RenderPrepGetAliasLighting(ent, shadelight, shadevector,
		model_cam_pos, fixedshadevector, dloc, pointSurface, fDLight);

	if (!preparedLighting)
	{
		if (ent->flags & RF_FULLBRIGHT || r_fullbright->value)
		{
			// if shadowed, we need to compute the shadevector and lightspot
			if (fDrawShadow)
				R_LightPoint(ent->origin, shadelight, &dloc, &pointSurface);

			fDLight = false;
			shadelight.Set(1.0, 1.0, 1.0);
		}
		else
		{
			fDLight = R_LightPoint(ent->origin, shadelight, &dloc, &pointSurface);

			if (gl_dynamic != NULL && gl_dynamic->value && r_worldmodel != NULL &&
				r_worldmodel->lightdata && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
			{
				CVector dynamicAtOrigin;
				R_DynamicLightPoint(ent->origin, dynamicAtOrigin);
				shadelight -= dynamicAtOrigin;
				if (shadelight.x < 0.0f) shadelight.x = 0.0f;
				if (shadelight.y < 0.0f) shadelight.y = 0.0f;
				if (shadelight.z < 0.0f) shadelight.z = 0.0f;
			}

			if (gl_monolightmap->string[0] != '0')
			{
				float s = shadelight.x;
				if (s < shadelight.y) s = shadelight.y;
				if (s < shadelight.z) s = shadelight.z;
				shadelight.Set(s, s, s);
			}
		}

		if (ent->flags & RF_MINLIGHT)
		{
			for (i = 0; i < 3; i++)
			{
				if (shadelight[i] > 0.3)
					break;
			}
			if (i == 3)
				shadelight.Set(0.3, 0.3, 0.3);
		}

		if (ent->flags & RF_GLOW)
		{
			float scale = sin(r_newrefdef.time * 2);
			float min;
			min = shadelight.x;
			shadelight.x += scale;
			if (shadelight.x < min) shadelight.x = min;
			min = shadelight.y;
			shadelight.y += scale;
			if (shadelight.y < min) shadelight.y = min;
			min = shadelight.z;
			shadelight.z += scale;
			if (shadelight.z < min) shadelight.z = min;
		}

		fixedshadevector.Set(-1, -1, -1);
		fixedshadevector.VectorToAngles(fixedshadeangles);
		fixedshadeangles.yaw -= ent->angles.yaw;
		fixedshadeangles.AngleToForwardVector(fixedshadevector);

		if (fDLight && !(gl_dynamic != NULL && gl_dynamic->value))
		{
			shadevector = dloc - ent->origin;
			shadevector.Normalize();
			shadevector.VectorToAngles(shadeangles);
			shadeangles.yaw -= ent->angles.yaw;
			shadeangles.AngleToForwardVector(shadevector);
		}
		else
		{
			shadevector = fixedshadevector;
		}

		model_cam_pos = r_newrefdef.vieworg - ent->origin;
		{
			float mag = model_cam_pos.Normalize();
			CVector angles;
			model_cam_pos.VectorToAngles(angles);
			angles.yaw -= ent->angles.yaw;
			angles.AngleToForwardVector(model_cam_pos);
			model_cam_pos = mag * model_cam_pos;
		}
	}

	if (ent->flags & RF_WEAPONMODEL)
	{
		float lightLevel;
		if (shadelight.x > shadelight.y)
		{
			if (shadelight.x > shadelight.z) lightLevel = 150 * shadelight.x;
			else lightLevel = 150 * shadelight.z;
		}
		else
		{
			if (shadelight.y > shadelight.z) lightLevel = 150 * shadelight.y;
			else lightLevel = 150 * shadelight.z;
		}
		if (r_lightlevel) r_lightlevel->value = lightLevel;
	}

	// Now precalc the vert normals with the shadevector before rendering the model
	GL_PrecalcShadevectorDots();


	//
	// locate the proper data
	//

	c_alias_polys += s_pmdl->num_tris;

	//
	// draw all the triangles
	//

	qglPushMatrix();
	ent->angles.x = -ent->angles.pitch;	// sigh.

	memcpy(gHeadMatrix, r_world_matrix, sizeof(gHeadMatrix));  // cached frame view matrix
	R_RotateAndScaleEntity(ent);

	ent->angles.x = -ent->angles.pitch;	// sigh.

	//	show name of model for testing
//	R_GetProjectedVert (ent->origin, &x, &y);
//	ri.dk_printxy (x, y, ent->currentmodel->name);

	// draw it

	qglShadeModel(GL_SMOOTH);

	GL_TexEnv(GL_MODULATE);

	if ((ent->frame >= s_pmdl->num_frames)
		|| (ent->frame < 0))
	{
		//		ri.Con_Printf (PRINT_ALL, "R_DrawAliasModel: no such frame %d\n",
		//			ent->frame);
		ent->frame = 0;
		ent->oldframe = 0;
	}

	if ((ent->oldframe >= s_pmdl->num_frames)
		|| (ent->oldframe < 0))
	{
		//		ri.Con_Printf (PRINT_ALL, "R_DrawAliasModel: no such oldframe %d\n",
		//			ent->oldframe);
		ent->frame = 0;
		ent->oldframe = 0;
	}

	if (!r_lerpmodels->value)
		ent->backlerp = 0;

	if (qglColorTableEXT)
	{
		qglEnable(GL_SHARED_TEXTURE_PALETTE_EXT);

		image_t* skin = NULL;

		/*
				if( gl_envmap->value )
				{
		//			skin = (image_t *)RegisterSkin("skins/player_chrome1.wal", RESOURCE_GLOBAL);
					skin = pEnvMapSkin;
				}
				else
		*/ {
		// support custom and reassigned skins
			if (ent->skin)
			{
				skin = (image_t*)ent->skin;	// custom player skin
			}
			else
			{
				if (ent->skinnum >= numgltextures)
					skin = currentmodel->skins[0];
				else if (ent->skinnum > 0)
				{
					skin = &gltextures[ent->skinnum];
				}
				else
				{
					// default
					skin = currentmodel->skins[0];
				}
			}
		}

		// SCG[10/30/99]: This fuckin sucks!
		// SCG[10/30/99]: someone decided to make this a surface flags instead of an entity flag...
		int count, surface, * order = (int*)((byte*)s_pmdl + s_pmdl->ofs_glcmds);

		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;
			if (!count)	break;
			if (count < 0) count = -count;

			if (ent->skin && !(s_psurface[*order].flags & SRF_STATICSKIN))  // 2.23 dsn  only map skin if not a static surface
			{
				skin = (image_t*)ent->skin;	// assign skin
				break;
			}

			order++;
			surface = *order;
			order++;

			order += 3 * count;
		}

		GL_SetTexturePalette(skin, TRUE);
	}

	version = LittleLong(s_pmdl->version);

	if (version == ALIAS_VERSION2) {
		if (qglMTexCoord2fSGIS && ent->flags & RF_ENVMAP && gl_envmap->value) GL_DrawAliasFrameLerp2Env(ent, s_pmdl, ent->backlerp);
		else GL_DrawAliasFrameLerp2(ent, s_pmdl, ent->backlerp);
	}
	else {
		if (qglMTexCoord2fSGIS && ent->flags & RF_ENVMAP && gl_envmap->value) GL_DrawAliasFrameLerpEnv(ent, s_pmdl, ent->backlerp);
		else GL_DrawAliasFrameLerp(ent, s_pmdl, ent->backlerp);
	}

	//
	//  Ash -- no comment
	//

	if (gl_procshell->value)
	{
		qglDisable(GL_TEXTURE_2D);
		qglEnable(GL_BLEND);
		GL_TexEnv(GL_MODULATE);

		qglDepthFunc(GL_LEQUAL);
		qglDepthMask(0);

		if (version != ALIAS_VERSION2)
			GL_DrawAliasFrameLerpProcShell(ent, s_pmdl);

		qglDepthMask(1);
		qglDisable(GL_BLEND);
		qglEnable(GL_TEXTURE_2D);
	}


	//
	//  Ash - Draw the fog shell
	//
	if (!gl_drawflat->value)
	{
		int nFogSamples = R_MAX_FOG_SAMPLES;   // set this appropriately for the thing being fogged
		if ((gl_vfogents->value == 1) && (true == GL_FogBBox(bbox, 8, gFogSample, nFogSamples)))
		{
			qglDisable(GL_TEXTURE_2D);
			qglEnable(GL_BLEND);
			GL_TexEnv(GL_MODULATE);

			qglDepthFunc(GL_LEQUAL);
			qglDepthMask(0);

			if (version == ALIAS_VERSION2)
				GL_DrawAliasFrameLerp2FogShell(ent, s_pmdl);
			else
				GL_DrawAliasFrameLerpFogShell(ent, s_pmdl);

			qglDepthMask(1);
			qglDisable(GL_BLEND);
			qglEnable(GL_TEXTURE_2D);
		}
	}

	qglPopMatrix();

	//
	// Draw the shadow
	//
	if ((gl_shadows->value) &&
		!(r_newrefdef.rdflags & RDF_NOWORLDMODEL) &&
		(r_worldmodel != NULL) &&
		(r_worldmodel->lightdata != NULL))
	{
		qglPushMatrix();
		qglEnable(GL_BLEND);
		GL_TexEnv(GL_MODULATE);
		qglShadeModel(GL_SMOOTH);
		qglDepthFunc(GL_LESS);

		GL_DrawSpotShadow(ent, fixedshadevector);

		qglDepthFunc(GL_LEQUAL);
		qglDisable(GL_BLEND);
		qglPopMatrix();
	}

	GL_TexEnv(GL_REPLACE);
	qglShadeModel(GL_FLAT);
	//
	// Draw the bounding box
	//
	if (gl_bbox->value)
	{
		qglColor4f(1.0f, 1.0f, 1.0f, 0.3f);
		qglDisable(GL_CULL_FACE);
		qglDisable(GL_TEXTURE_2D);
		qglEnable(GL_BLEND);

		qglBegin(GL_QUADS);

		qglVertex3fv(bbox[0]);
		qglVertex3fv(bbox[1]);
		qglVertex3fv(bbox[3]);
		qglVertex3fv(bbox[2]);

		qglVertex3fv(bbox[0]);
		qglVertex3fv(bbox[1]);
		qglVertex3fv(bbox[5]);
		qglVertex3fv(bbox[4]);

		qglVertex3fv(bbox[1]);
		qglVertex3fv(bbox[3]);
		qglVertex3fv(bbox[7]);
		qglVertex3fv(bbox[5]);

		qglVertex3fv(bbox[2]);
		qglVertex3fv(bbox[3]);
		qglVertex3fv(bbox[7]);
		qglVertex3fv(bbox[6]);

		qglVertex3fv(bbox[0]);
		qglVertex3fv(bbox[2]);
		qglVertex3fv(bbox[6]);
		qglVertex3fv(bbox[4]);

		qglVertex3fv(bbox[4]);
		qglVertex3fv(bbox[5]);
		qglVertex3fv(bbox[7]);
		qglVertex3fv(bbox[6]);

		qglEnd();

		qglDisable(GL_BLEND);
		qglEnable(GL_TEXTURE_2D);
		qglEnable(GL_CULL_FACE);
	}

	if (ent->flags & RF_TRANSLUCENT)
	{
		qglDisable(GL_BLEND);
	}

	qglColor4f(1, 1, 1, 1);

	if (ent->flags & RF_WEAPONMODEL && (r_newrefdef.foginfo.active || r_testfog->value)) {
		qglEnable(GL_FOG);
	}

	GL_TexEnv(GL_REPLACE);

	GL_SetState(0);
}

///////////////////////////////////////////////////////////////////////////////
//	R_DrawAliasIcon
//
///////////////////////////////////////////////////////////////////////////////

void R_DrawAliasIcon(entity_t* ent)
{
	float		an;
	unsigned int	nStateFlags;
	int			version;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
	{
		return;
	}

	//	GL_SetFunc( GLSTATE_DEPTH_FUNC, GL_LESS, -1 );

	//	nStateFlags = GLSTATE_PRESET1;

	//	nStateFlags &= ~( GLSTATE_CULL_FACE_FRONT | GLSTATE_BLEND );

	currentmodel = (model_t*)ent->model;

	s_pmdl = (dmdl_t*)currentmodel->extradata;
	if (s_pmdl == NULL)
	{
		return;
	}

	s_psurface = (dsurface_t*)((byte*)s_pmdl + s_pmdl->ofs_surfaces);
	if (s_psurface == NULL)
	{
		return;
	}

	ent_draw_count++;

	if (ent->flags & RF_FULLBRIGHT)
	{
		shadelight.x = 1.0;
		shadelight.y = 1.0;
		shadelight.z = 1.0;
	}
	else
	{
		shadelight.x = 0.5;
		shadelight.y = 0.5;
		shadelight.z = 0.5;
	}

	an = ent->angles.y / 180 * M_PI;
	shadevector.x = cos(-an);
	shadevector.y = sin(-an);
	shadevector.z = 1;
	shadevector.Normalize();

	//
	// locate the proper data
	//

	c_alias_polys += s_pmdl->num_tris;

	//
	// draw all the triangles
	//

	qglPushMatrix();

	ent->angles.x = -ent->angles.x;	// sigh.
	R_RotateAndScaleEntity(ent);
	ent->angles.x = -ent->angles.x;	// sigh.

	if (ent->flags & RF_TRANSLUCENT)
	{
		nStateFlags |= GLSTATE_BLEND;
		GL_TexEnv(GL_MODULATE);
	}

	if ((ent->frame >= s_pmdl->num_frames)
		|| (ent->frame < 0))
	{
		ent->frame = 0;
		ent->oldframe = 0;
	}

	if ((ent->oldframe >= s_pmdl->num_frames)
		|| (ent->oldframe < 0))
	{
		ent->frame = 0;
		ent->oldframe = 0;
	}

	if (!r_lerpmodels->value)
		ent->backlerp = 0;


	if (qglColorTableEXT)
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;

		GL_SetTexturePalette(currentmodel->skins[0], FALSE);
	}

	GL_SetState(GLSTATE_TEXTURE_2D | GLSTATE_BLEND | GLSTATE_DEPTH_MASK | GLSTATE_DEPTH_TEST | GLSTATE_CULL_FACE);
	//	GL_SetState( nStateFlags );

	//	GL_DrawAliasFrameLerp (ent, s_pmdl, ent->backlerp);
	version = LittleLong(s_pmdl->version);

	if (version == ALIAS_VERSION2) {
		GL_DrawAliasFrameLerp2(ent, s_pmdl, ent->backlerp);
	}
	else {
		GL_DrawAliasFrameLerp(ent, s_pmdl, ent->backlerp);
	}

	GL_TexEnv(GL_REPLACE);

	qglPopMatrix();

	qglColor4f(1, 1, 1, 1);

	GL_SetState(0);
}

inline void R_ShellAgePatch(shell_t* pShell, int index)
{
	float val = 0.2f / gl_procd->value;
	pShell->pPatches[index].data++;
	pShell->pVData[pShell->pPatches[index].vindex[0]] += val;
	pShell->pVData[pShell->pPatches[index].vindex[1]] += val;
	pShell->pVData[pShell->pPatches[index].vindex[2]] += val;
}

inline void R_ShellKillPatch(shell_t* pShell, int index)
{
	float val = 0.2f * pShell->pPatches[index].data / gl_procd->value;
	pShell->pVData[pShell->pPatches[index].vindex[0]] -= val;
	pShell->pVData[pShell->pPatches[index].vindex[1]] -= val;
	pShell->pVData[pShell->pPatches[index].vindex[2]] -= val;
	pShell->pPatches[index].data = 0;
}

void R_ShellUpdate(shell_t* pShell)
{
	int index, nnum, nindex;
	shell_patch_t* pPatch;

	if (gl_procshell->value == 1)
	{
		if (!pShell->numActive)
		{
			// if none are active, turn one on at random
			index = rand() % pShell->numPatches;
			R_ShellAgePatch(pShell, index);
			pShell->numActive = 1;
		}
		else
		{
			// advance the ages of the active
			for (index = 0;index < pShell->numPatches;index++)
			{
				pPatch = pShell->pPatches + index;

				// if it's alive, make it older
				if (0 < pPatch->data)
				{
					R_ShellAgePatch(pShell, index);

					// if it's too old, kill it
					if (pPatch->data > gl_procd->value)
					{
						R_ShellKillPatch(pShell, index);
						pShell->numActive--;

						// if it's the last one, maybe start over here
						if ((1 == pShell->numActive) && (rand() & 1))
						{
							R_ShellAgePatch(pShell, index);
							pShell->numActive++;
						}
					}
					// if it's fertile, have some babies
					else if (pPatch->data >= gl_procp->value && pPatch->data <= gl_procm->value)
					{
						for (nnum = 0;nnum < 3;nnum++)
						{
							nindex = pPatch->nindex[nnum];

							if (0 == pShell->pPatches[nindex].data)
							{
								R_ShellAgePatch(pShell, nindex);
								pShell->numActive++;
							}
						}
					}
				}
			}
		}
	}
}

void GL_DrawAliasFrameLerpProcShell(entity_t* ent, dmdl_t* paliashdr)
{
	daliasframe_t* frame;
	int* order;
	int				count/*, stripFanCount = 0*/;
	float			color;
	CVector			move;
	int				index_xyz;
	GLenum			primtype;
	int				surface, i;
	shell_t* pShell;

	frame = (daliasframe_t*)((byte*)s_pmdl + s_pmdl->ofs_frames
		+ ent->frame * s_pmdl->framesize);

	order = (int*)((byte*)s_pmdl + s_pmdl->ofs_glcmds);

	// find the shell
	i = 0;
	while (i < gNumAliasShells && gAliasShells[i].pModel != currentmodel)
		i++;

	if (i >= gNumAliasShells)
		return;
	else
		pShell = gAliasShells + i;

	R_ShellUpdate(pShell);

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			primtype = GL_TRIANGLE_FAN;
		}
		else
			primtype = GL_TRIANGLE_STRIP;

		order++;
		surface = *order++;

		// dont draw this surface
		if (s_psurface[surface].flags & SRF_NODRAW)
		{
			order += ((2) + 1) * count;
			continue;
		}

		qglBegin(primtype);

		do
		{
			index_xyz = *order++;
			order += 2;

			color = min(pShell->pVData[index_xyz], 1.0f);

			qglColor4f(color, color, color, color);
			//            qglVertex3fv (lerped[index_xyz]);
			qglVertex3fv(s_lerped[index_xyz]);
		} while (--count);

		// increment the strip and fan counter
//		stripFanCount++;
		qglEnd();

	}

	qglDisable(GL_BLEND);
	qglDisable(GL_ALPHA_TEST);
	qglEnable(GL_TEXTURE_2D);
}

