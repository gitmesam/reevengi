/*
	Load EMD model
	Resident Evil

	Copyright (C) 2008-2010	Patrice Mandin

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <assert.h>
#include <SDL.h>

#include "../filesystem.h"

#include "../r_common/render.h"
#include "../r_common/render_skel.h"

/*--- Defines ---*/

#define EMD_SKELETON 0
#define EMD_ANIM_FRAMES 1
#define EMD_MESHES 2
#define EMD_TIM 3

/*--- Types ---*/

typedef struct {
	Uint32 offset;
	Uint32 length;
} emd_header_t;

typedef struct {
	Uint16	count;
	Uint16	offset;
} emd_anim_header_t;

typedef struct {
	Sint16	x,y,z;
} emd_skel_relpos_t;

typedef struct {
	Uint16	num_mesh;
	Uint16	offset;
} emd_skel_data_t;

typedef struct {
	Uint16	relpos_offset;
	Uint16	anim_offset;
	Uint16	count;
	Uint16	size;
} emd_skel_header_t;

typedef struct {
	Sint16	pos[3];
	Sint16	speed[3];

	/* 16 bits values for angles following */
} emd_skel_anim_t;

typedef struct {
	Sint16 x,y,z,w;
} emd_vertex_t;

typedef struct {
	Uint32 id;

	unsigned char tu0,tv0;
	Uint16 page;
	unsigned char tu1,tv1;
	Uint16 clutid;
	unsigned char tu2,tv2;
	Uint16 dummy;

	Uint16	n0,v0;
	Uint16	n1,v1;
	Uint16	n2,v2;
} emd_triangle_t;

typedef struct {
	Uint32	vtx_offset;
	Uint32	vtx_count;
	Uint32	nor_offset;
	Uint32	nor_count;
	Uint32	mesh_offset;
	Uint32	mesh_count;
	Uint32	dummy;
} emd_mesh_t;

typedef struct {
	Uint32 length;
	Uint32 dummy;
	Uint32 num_objects;
} emd_mesh_header_t;

typedef struct {
	emd_mesh_t triangles;
} emd_mesh_object_t;

/*--- Functions prototypes ---*/

static render_skel_t *emd_load_render_skel(void *emd, Uint32 emd_length, render_texture_t *texture);

static int getChild(render_skel_t *this, int num_parent, int num_child);
static int getNumAnims(render_skel_t *this);
static int setAnimFrame(render_skel_t *this, int num_anim, int num_frame);
static void getAnimPosition(render_skel_t *this, Sint16 *x, Sint16 *y, Sint16 *z);
static void getAnimAngles(render_skel_t *this, int num_mesh, int *x, int *y, int *z);

/*--- Functions ---*/

render_skel_t *model_emd_load(void *emd, Uint32 emd_length)
{
	Uint32 *hdr_offsets, tim_offset;
	render_texture_t *texture;
	render_skel_t *skel;

	/* TIM file embedded */
	hdr_offsets = (Uint32 *)
		(&((char *) emd)[emd_length-16]);

	tim_offset = SDL_SwapLE32(hdr_offsets[EMD_TIM]);

	texture = render.createTexture(RENDER_TEXTURE_MUST_POT);
	if (!texture) {
		return NULL;
	}
	texture->load_from_tim(texture, (&((char *) emd)[tim_offset]));

	skel = emd_load_render_skel(emd, emd_length, texture);
	if (!skel) {
		texture->shutdown(texture);
		return NULL;
	}

	skel->getChild = getChild;
	skel->getNumAnims = getNumAnims;
	skel->setAnimFrame = setAnimFrame;
	skel->getAnimPosition = getAnimPosition;
	skel->getAnimAngles = getAnimAngles;

	return skel;
}

static render_skel_t *emd_load_render_skel(void *emd_file, Uint32 emd_length, render_texture_t *texture)
{
	Uint32 *hdr_offsets, skel_offset, mesh_offset;
	int i,j;
	/*emd_skel_header_t *emd_skel_header;*/
	emd_skel_relpos_t *emd_skel_relpos;
	/*emd_skel_data_t *emd_skel_data;*/
	emd_mesh_header_t *emd_mesh_header;
	emd_mesh_object_t *emd_mesh_object;

	render_skel_t *skeleton;

	/* Directory offsets */
	hdr_offsets = (Uint32 *)
		(&((char *) emd_file)[emd_length-16]);

	/* Offset 0: Skeleton */
	skel_offset = SDL_SwapLE32(hdr_offsets[EMD_SKELETON]);

	/*emd_skel_header = (emd_skel_header_t *)
		(&((char *) emd_file)[skel_offset]);*/
	emd_skel_relpos = (emd_skel_relpos_t *)
		(&((char *) emd_file)[skel_offset+sizeof(emd_skel_header_t)]);
	/*emd_skel_data = (emd_skel_data_t *)
		(&((char *) emd_file)[skel_offset+SDL_SwapLE16(emd_skel_header->relpos_offset)]);*/

	skeleton = render.createSkel(emd_file, emd_length, texture);
	if (!skeleton) {
		fprintf(stderr, "Can not create skeleton\n");
		return NULL;
	}

	/* Offset 2: Mesh data */
	mesh_offset = SDL_SwapLE32(hdr_offsets[EMD_MESHES]);
	emd_mesh_header = (emd_mesh_header_t *)
		(&((char *) emd_file)[mesh_offset]);

	mesh_offset += sizeof(emd_mesh_header_t);
	emd_mesh_object = (emd_mesh_object_t *)
		(&((char *) emd_file)[mesh_offset]);

	for (i=0; i<SDL_SwapLE32(emd_mesh_header->num_objects); i++) {
		emd_vertex_t *emd_vtx;
		emd_triangle_t *emd_tri_idx;
		Uint16 *txcoordPtr, *txcoords;

		render_mesh_t *mesh = render.createMesh(texture);
		if (!mesh) {
			fprintf(stderr, "Can not create mesh\n");
			break;
		}

		/* Vertex array */
		emd_vtx = (emd_vertex_t *)
			(&((char *) emd_file)[mesh_offset+SDL_SwapLE32(emd_mesh_object->triangles.vtx_offset)]);

		mesh->setArray(mesh, RENDER_ARRAY_VERTEX, 3, RENDER_ARRAY_SHORT,
			SDL_SwapLE32(emd_mesh_object->triangles.vtx_count), sizeof(emd_vertex_t),
			emd_vtx,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			1
#else
			0
#endif
			);

		/* Normal array */
		emd_vtx = (emd_vertex_t *)
			(&((char *) emd_file)[mesh_offset+SDL_SwapLE32(emd_mesh_object->triangles.nor_offset)]);

		mesh->setArray(mesh, RENDER_ARRAY_NORMAL, 3, RENDER_ARRAY_SHORT,
			SDL_SwapLE32(emd_mesh_object->triangles.nor_count), sizeof(emd_vertex_t),
			emd_vtx,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			1
#else
			0
#endif
			);

		/* Texcoord array */
		txcoordPtr = (Uint16 *) malloc(6*sizeof(Uint16)*SDL_SwapLE32(emd_mesh_object->triangles.mesh_count));
		if (!txcoordPtr) {
			fprintf(stderr, "Can not allocate memory for txcoords\n");
			mesh->shutdown(mesh);
			break;
		}

		emd_tri_idx = (emd_triangle_t *)
			(&((char *) emd_file)[mesh_offset+SDL_SwapLE32(emd_mesh_object->triangles.mesh_offset)]);

		txcoords = txcoordPtr;
		for (j=0; j<SDL_SwapLE32(emd_mesh_object->triangles.mesh_count); j++) {
			int page = (SDL_SwapLE16(emd_tri_idx[j].page)<<1) & 0xff;

			*txcoords++ = emd_tri_idx[j].tu0 + page;
			*txcoords++ = emd_tri_idx[j].tv0;
			*txcoords++ = emd_tri_idx[j].tu1 + page;
			*txcoords++ = emd_tri_idx[j].tv1;
			*txcoords++ = emd_tri_idx[j].tu2 + page;
			*txcoords++ = emd_tri_idx[j].tv2;
		}

		mesh->setArray(mesh, RENDER_ARRAY_TEXCOORD, 2, RENDER_ARRAY_SHORT,
			3*SDL_SwapLE32(emd_mesh_object->triangles.mesh_count), sizeof(Uint16)*2,
			txcoordPtr, 0);

		free(txcoordPtr);

		/* Triangles */
		for (j=0; j<SDL_SwapLE32(emd_mesh_object->triangles.mesh_count); j++) {
			render_mesh_tri_t	mesh_tri;
			
			mesh_tri.v[0] = SDL_SwapLE16(emd_tri_idx[j].v0);
			mesh_tri.v[1] = SDL_SwapLE16(emd_tri_idx[j].v1);
			mesh_tri.v[2] = SDL_SwapLE16(emd_tri_idx[j].v2);

			mesh_tri.n[0] = SDL_SwapLE16(emd_tri_idx[j].n0);
			mesh_tri.n[1] = SDL_SwapLE16(emd_tri_idx[j].n1);
			mesh_tri.n[2] = SDL_SwapLE16(emd_tri_idx[j].n2);

			mesh_tri.tx[0] = j*3;
			mesh_tri.tx[1] = j*3+1;
			mesh_tri.tx[2] = j*3+2;

			mesh_tri.txpal = SDL_SwapLE16(emd_tri_idx[j].clutid) & 3;

			mesh->addTriangle(mesh, &mesh_tri);
		}

		/* Add mesh to skeleton */
		skeleton->addMesh(skeleton, mesh,
			SDL_SwapLE16(emd_skel_relpos[i].x),
			SDL_SwapLE16(emd_skel_relpos[i].y),
			SDL_SwapLE16(emd_skel_relpos[i].z));

		emd_mesh_object++;
	}

	return skeleton;
}

static int getChild(render_skel_t *this, int num_parent, int num_child)
{
	Uint32 *hdr_offsets, skel_offset;
	/*emd_header_t *emd_header;*/
	emd_skel_header_t *emd_skel_header;
	emd_skel_data_t *emd_skel_data;
	int num_meshes;
	Uint8 *mesh_numbers;

	assert(this);
	assert(this->emd_file);
	assert(num_parent>=0);
	assert(num_child>=0);

	/*emd_header = (emd_header_t *) this->emd_file;*/

	hdr_offsets = (Uint32 *)
		(&((char *) (this->emd_file))[(this->emd_length)-16]);

	/* Offset 0: Skeleton */
	skel_offset = SDL_SwapLE32(hdr_offsets[EMD_SKELETON]);

	emd_skel_header = (emd_skel_header_t *)
		(&((char *) (this->emd_file))[skel_offset]);
	emd_skel_data = (emd_skel_data_t *)
		(&((char *) (this->emd_file))[skel_offset+SDL_SwapLE16(emd_skel_header->relpos_offset)]);

	num_meshes = SDL_SwapLE16(emd_skel_data[num_parent].num_mesh);
	if (num_child>=num_meshes) {
		return -1;
	}

	mesh_numbers = (Uint8 *) emd_skel_data;
	return mesh_numbers[SDL_SwapLE16(emd_skel_data[num_parent].offset)+num_child];
}

static int getNumAnims(render_skel_t *this)
{
	Uint32 *hdr_offsets, anim_offset;
	/*emd_header_t *emd_header;*/
	emd_anim_header_t *emd_anim_header;

	assert(this);
	assert(this->emd_file);

	/*emd_header = (emd_header_t *) this->emd_file;*/

	hdr_offsets = (Uint32 *)
		(&((char *) (this->emd_file))[(this->emd_length)-16]);

	/* Offset 1: Animation frames */
	anim_offset = SDL_SwapLE32(hdr_offsets[EMD_ANIM_FRAMES]);

	emd_anim_header = (emd_anim_header_t *)
		(&((char *) (this->emd_file))[anim_offset]);

	return (SDL_SwapLE16(emd_anim_header->offset) / sizeof(emd_anim_header_t));
}

static int setAnimFrame(render_skel_t *this, int num_anim, int num_frame)
{
	Uint32 *hdr_offsets, anim_offset;
	/*emd_header_t *emd_header;*/
	emd_anim_header_t *emd_anim_header;
	int num_anims;

	assert(this);
	assert(this->emd_file);
	assert(num_anim>=0);
	assert(num_frame>=0);

	/*emd_header = (emd_header_t *) this->emd_file;*/

	hdr_offsets = (Uint32 *)
		(&((char *) (this->emd_file))[(this->emd_length)-16]);

	/* Offset 1: Animation frames */
	anim_offset = SDL_SwapLE32(hdr_offsets[EMD_ANIM_FRAMES]);

	emd_anim_header = (emd_anim_header_t *)
		(&((char *) (this->emd_file))[anim_offset]);

	num_anims = SDL_SwapLE16(emd_anim_header->offset) / sizeof(emd_anim_header_t);
	if (num_anim>=num_anims) {
		return 0;
	}

	this->num_anim = num_anim;
	this->num_frame = num_frame % SDL_SwapLE16(emd_anim_header[num_anim].count);
	return 1;
}

static void getAnimPosition(render_skel_t *this, Sint16 *x, Sint16 *y, Sint16 *z)
{
	Uint32 *hdr_offsets, skel_offset, anim_offset, *ptr_skel_frame;
	/*emd_header_t *emd_header;*/
	emd_skel_header_t *emd_skel_header;
	emd_skel_anim_t	*emd_skel_anim;
	emd_anim_header_t *emd_anim_header;
	int num_anims, num_skel_frame;

	assert(this);
	assert(this->emd_file);

	/*emd_header = (emd_header_t *) this->emd_file;*/

	hdr_offsets = (Uint32 *)
		(&((char *) (this->emd_file))[(this->emd_length)-16]);

	/* Offset 1: Animation frames */
	anim_offset = SDL_SwapLE32(hdr_offsets[EMD_ANIM_FRAMES]);

	emd_anim_header = (emd_anim_header_t *)
		(&((char *) (this->emd_file))[anim_offset]);

	num_anims = SDL_SwapLE16(emd_anim_header->offset) / sizeof(emd_anim_header_t);
	assert(this->num_anim < num_anims);
	assert(this->num_frame < SDL_SwapLE16(emd_anim_header[this->num_anim].count));

	/* Go to start of current animation */
	anim_offset += SDL_SwapLE16(emd_anim_header[this->num_anim].offset);

	ptr_skel_frame = (Uint32 *)
		(&((char *) (this->emd_file))[anim_offset]);
	num_skel_frame = SDL_SwapLE32(ptr_skel_frame[this->num_frame]) & ((1<<16)-1);

	/* Offset 2: Skeleton */
	skel_offset = SDL_SwapLE32(hdr_offsets[EMD_SKELETON]);

	emd_skel_header = (emd_skel_header_t *)
		(&((char *) (this->emd_file))[skel_offset]);
	emd_skel_anim = (emd_skel_anim_t *)
		(&((char *) (this->emd_file))[
			skel_offset
			+SDL_SwapLE16(emd_skel_header->anim_offset)
			+num_skel_frame*SDL_SwapLE16(emd_skel_header->size)
		]);

	/*printf(" pos %d: 0x%08x: offset 0x%08x\n",
		this->num_frame, num_skel_frame, skel_offset
			+SDL_SwapLE16(emd_skel_header->anim_offset)
			+num_skel_frame*SDL_SwapLE16(emd_skel_header->size)
	);*/

	*x = SDL_SwapLE16(emd_skel_anim->pos[0]);
	*y = SDL_SwapLE16(emd_skel_anim->pos[1]);
	*z = SDL_SwapLE16(emd_skel_anim->pos[2]);
}

static void getAnimAngles(render_skel_t *this, int num_mesh, int *x, int *y, int *z)
{
	Uint32 *hdr_offsets, skel_offset, anim_offset, *ptr_skel_frame;
	/*emd_header_t *emd_header;*/
	emd_skel_header_t *emd_skel_header;
	Sint16 *ptr_angles;
	emd_anim_header_t *emd_anim_header;
	int num_anims, num_skel_frame;

	assert(this);
	assert(this->emd_file);

	/*emd_header = (emd_header_t *) this->emd_file;*/

	hdr_offsets = (Uint32 *)
		(&((char *) (this->emd_file))[(this->emd_length)-16]);

	/* Offset 1: Animation frames */
	anim_offset = SDL_SwapLE32(hdr_offsets[EMD_ANIM_FRAMES]);

	emd_anim_header = (emd_anim_header_t *)
		(&((char *) (this->emd_file))[anim_offset]);

	num_anims = SDL_SwapLE16(emd_anim_header->offset) / sizeof(emd_anim_header_t);
	assert(this->num_anim < num_anims);
	assert(this->num_frame < SDL_SwapLE16(emd_anim_header[this->num_anim].count));

	/* Go to start of current animation */
	anim_offset += SDL_SwapLE16(emd_anim_header[this->num_anim].offset);

	ptr_skel_frame = (Uint32 *)
		(&((char *) (this->emd_file))[anim_offset]);
	num_skel_frame = SDL_SwapLE32(ptr_skel_frame[this->num_frame]) & ((1<<16)-1);

	/* Offset 2: Skeleton */
	skel_offset = SDL_SwapLE32(hdr_offsets[EMD_SKELETON]);

	emd_skel_header = (emd_skel_header_t *)
		(&((char *) (this->emd_file))[skel_offset]);
	ptr_angles = (Sint16 *)
		(&((char *) (this->emd_file))[
			skel_offset
			+SDL_SwapLE16(emd_skel_header->anim_offset)
			+num_skel_frame*SDL_SwapLE16(emd_skel_header->size)
			+sizeof(emd_skel_anim_t)
		]);

	/*printf(" ang %d: mesh %d: 0x%08x: offset 0x%08x\n",
		this->num_frame, num_mesh, num_skel_frame, skel_offset
			+SDL_SwapLE16(emd_skel_header->anim_offset)
			+num_skel_frame*SDL_SwapLE16(emd_skel_header->size)
			+sizeof(emd_skel_anim_t)
	);*/

	assert(num_mesh >= 0);
	assert(num_mesh < SDL_SwapLE16(emd_skel_header->count));
	/*--num_mesh;*/

	*x = SDL_SwapLE16(ptr_angles[num_mesh*3]) & 4095;
	*y = SDL_SwapLE16(ptr_angles[num_mesh*3+1]) & 4095;
	*z = SDL_SwapLE16(ptr_angles[num_mesh*3+2]) & 4095;
}
