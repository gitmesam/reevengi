/*
	Load EMD model
	Resident Evil

	Copyright (C) 2008	Patrice Mandin

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

#include <SDL.h>

#include "filesystem.h"
#include "video.h"
#include "render.h"
#include "model.h"
#include "model_emd.h"
#include "render_mesh.h"
#include "render_skel.h"

/*--- Defines ---*/

#define EMD_SKELETON 0
#define EMD_MESHES 2
#define EMD_TIM 3

/*--- Types ---*/

typedef struct {
	Uint32 offset;
	Uint32 length;
} emd_header_t;

typedef struct {
	Sint16	x,y,z;
} emd_skel_relpos_t;

typedef struct {
	Uint16	num_mesh;
	Uint16	offset;
} emd_skel_data_t;

typedef struct {
	Uint16	relpos_offset;
	Uint16	unk_offset;
	Uint16	count;
	Uint16	size;
} emd_skel_header_t;

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

static void model_emd_shutdown(model_t *this);
static void model_emd_draw(model_t *this);

static void emd_convert_endianness(model_t *this);
static void emd_convert_endianness_skel(
	model_t *this, int num_skel,
	emd_skel_relpos_t *emd_skel_relpos,
	emd_skel_data_t *emd_skel_data);

static void emd_draw_skel(model_t *this, int num_skel,
	emd_skel_relpos_t *emd_skel_relpos,
	emd_skel_data_t *emd_skel_data);
static void emd_draw_mesh(model_t *this, int num_mesh);

static void emd_add_mesh(model_t *this, int num_skel, int *num_idx, int *idx, vertex_t *vtx,
	int x, int y, int z,
	emd_skel_relpos_t *emd_skel_relpos,
	emd_skel_data_t *emd_skel_data);

static void emd_load_render_mesh(model_t *this);

/*--- Functions ---*/

model_t *model_emd_load(void *emd, Uint32 emd_length)
{
	model_t	*model;
	Uint32 *hdr_offsets, tim_offset;
	
	model = (model_t *) calloc(1, sizeof(model_t));
	if (!model) {
		fprintf(stderr, "Can not allocate memory for model\n");
		return NULL;
	}

	model->emd_file = emd;
	model->emd_length = emd_length;

	/* TIM file embedded */
	hdr_offsets = (Uint32 *)
		(&((char *) model->emd_file)[model->emd_length-16]);

	tim_offset = SDL_SwapLE32(hdr_offsets[EMD_TIM]);
	model->tim_file = (&((char *) model->emd_file)[tim_offset]);
	model->tim_length = model->emd_length - tim_offset;

	emd_convert_endianness(model);

	model->texture = render.createTexture(RENDER_TEXTURE_MUST_POT);
	if (model->texture) {
		model->texture->load_from_tim(model->texture, model->tim_file);
	}

	model->shutdown = model_emd_shutdown;
	model->draw = model_emd_draw;

	return model;
}

static void model_emd_shutdown(model_t *this)
{
	if (this) {
		if (this->emd_file) {
			free(this->emd_file);
		}
		if (this->texture) {
			this->texture->shutdown(this->texture);
		}
		free(this);
	}
}

static void model_emd_draw(model_t *this)
{
	emd_skel_header_t *emd_skel_header;
	emd_skel_relpos_t *emd_skel_relpos;
	emd_skel_data_t *emd_skel_data;
	Uint32 *hdr_offsets;
	void *emd_file;

#if 0
	int idx_mesh[32];
	vertex_t pos_mesh[32];
	int count, i;
#endif

	if (!this) {
		return;
	}
	if (!this->emd_file) {
		return;
	}
	emd_file = this->emd_file;

	hdr_offsets = (Uint32 *)
		(&((char *) emd_file)[this->emd_length-16]);

	emd_skel_header = (emd_skel_header_t *)
		(&((char *) emd_file)[hdr_offsets[EMD_SKELETON]]);
	emd_skel_relpos = (emd_skel_relpos_t *)
		(&((char *) emd_file)[hdr_offsets[EMD_SKELETON]+sizeof(emd_skel_header_t)]);
	emd_skel_data = (emd_skel_data_t *)
		(&((char *) emd_file)[hdr_offsets[EMD_SKELETON]+emd_skel_header->relpos_offset]);

#if 1
	emd_draw_skel(this, 0, emd_skel_relpos, emd_skel_data);
#else
	count = 0;
	emd_add_mesh(this, 0, &count, idx_mesh,pos_mesh, 0,0,0, emd_skel_relpos, emd_skel_data);
	render.sortBackToFront(count, idx_mesh,pos_mesh);

	for (i=0; i<count; i++) {
		render.push_matrix();
		render.translate(
			pos_mesh[i].x,
			pos_mesh[i].y,
			pos_mesh[i].z
		);

		emd_draw_mesh(this, idx_mesh[i]);

		render.pop_matrix();
	}
#endif
}

static void emd_add_mesh(model_t *this, int num_skel, int *num_idx, int *idx, vertex_t *vtx,
	int x, int y, int z,
	emd_skel_relpos_t *emd_skel_relpos,
	emd_skel_data_t *emd_skel_data)
{
	int i, count = *num_idx;
	Uint8 *emd_skel_mesh = (Uint8 *) emd_skel_data;

	if (count>=32) {
		return;
	}

	x += emd_skel_relpos[num_skel].x;
	y += emd_skel_relpos[num_skel].y;
	z += emd_skel_relpos[num_skel].z;

	/* Add current mesh */
	idx[count] = num_skel;
	vtx[count].x = x;
	vtx[count].y = y;
	vtx[count].z = z;

	*num_idx = ++count;

	/* Add children meshes */
	for (i=0; i<emd_skel_data[num_skel].num_mesh; i++) {
		int num_mesh = emd_skel_mesh[emd_skel_data[num_skel].offset+i];
		emd_add_mesh(this, num_mesh, num_idx, idx, vtx, x,y,z, emd_skel_relpos, emd_skel_data);
	}
}

static void emd_draw_skel(model_t *this, int num_skel,
	emd_skel_relpos_t *emd_skel_relpos,
	emd_skel_data_t *emd_skel_data)
{
	Uint8 *emd_skel_mesh = (Uint8 *) emd_skel_data;
	int i;

	render.push_matrix();
	render.translate(
		emd_skel_relpos[num_skel].x,
		emd_skel_relpos[num_skel].y,
		emd_skel_relpos[num_skel].z
	);

	/* Draw current mesh */
	emd_draw_mesh(this, num_skel);

	/* Draw children meshes */
	for (i=0; i<emd_skel_data[num_skel].num_mesh; i++) {
		int num_mesh = emd_skel_mesh[emd_skel_data[num_skel].offset+i];
		emd_draw_skel(this, num_mesh, emd_skel_relpos, emd_skel_data);
	}

	render.pop_matrix();
}

static void emd_draw_mesh(model_t *this, int num_mesh)
{
	emd_mesh_header_t *emd_mesh_header;
	emd_mesh_object_t *emd_mesh_object;
	Uint32 *hdr_offsets, mesh_offset;
	int num_objects, i;
	emd_vertex_t *emd_tri_vtx;
	emd_triangle_t *emd_tri_idx;
	void *emd_file = this->emd_file;
	vertex_t v[3];

	hdr_offsets = (Uint32 *)
		(&((char *) emd_file)[this->emd_length-16]);

	emd_mesh_header = (emd_mesh_header_t *)
		(&((char *) emd_file)[hdr_offsets[EMD_MESHES]]);
	num_objects = emd_mesh_header->num_objects;

	if ((num_mesh<0) || (num_mesh>=num_objects)) {
		fprintf(stderr, "Invalid mesh %d/%d\n", num_mesh, num_objects);
		return;
	}

	mesh_offset = hdr_offsets[EMD_MESHES]+sizeof(emd_mesh_header_t);

	emd_mesh_object = (emd_mesh_object_t *)
		(&((char *) emd_file)[mesh_offset]);
	emd_mesh_object = &emd_mesh_object[num_mesh];

	/* Draw triangles */
	emd_tri_vtx = (emd_vertex_t *)
		(&((char *) emd_file)[mesh_offset+emd_mesh_object->triangles.vtx_offset]);
	emd_tri_idx = (emd_triangle_t *)
		(&((char *) emd_file)[mesh_offset+emd_mesh_object->triangles.mesh_offset]);

	for (i=0; i<emd_mesh_object->triangles.mesh_count; i++) {
		int page = (emd_tri_idx[i].page<<1) & 0xff;
		/*printf("page: 0x%04x, palette: 0x%04x\n", emd_tri_idx[i].page, emd_tri_idx[i].clutid);*/

		v[0].x = emd_tri_vtx[emd_tri_idx[i].v0].x;
		v[0].y = emd_tri_vtx[emd_tri_idx[i].v0].y;
		v[0].z = emd_tri_vtx[emd_tri_idx[i].v0].z;
		v[0].u = emd_tri_idx[i].tu0 + page;
		v[0].v = emd_tri_idx[i].tv0;

		v[1].x = emd_tri_vtx[emd_tri_idx[i].v1].x;
		v[1].y = emd_tri_vtx[emd_tri_idx[i].v1].y;
		v[1].z = emd_tri_vtx[emd_tri_idx[i].v1].z;
		v[1].u = emd_tri_idx[i].tu1 + page;
		v[1].v = emd_tri_idx[i].tv1;

		v[2].x = emd_tri_vtx[emd_tri_idx[i].v2].x;
		v[2].y = emd_tri_vtx[emd_tri_idx[i].v2].y;
		v[2].z = emd_tri_vtx[emd_tri_idx[i].v2].z;
		v[2].u = emd_tri_idx[i].tu2 + page;
		v[2].v = emd_tri_idx[i].tv2;

		render.set_texture(emd_tri_idx[i].clutid & 3, this->texture);
		render.triangle(&v[0], &v[1], &v[2]);
	}
}

static void emd_load_render_skel(model_t *this)
{
	Uint32 *hdr_offsets, skel_offset, mesh_offset;
	int i,j;
	emd_skel_header_t *emd_skel_header;
	emd_skel_relpos_t *emd_skel_relpos;
	emd_skel_data_t *emd_skel_data;
	emd_mesh_header_t *emd_mesh_header;
	emd_mesh_object_t *emd_mesh_object;
	void *emd_file = this->emd_file;

	render_skel_t *skeleton;

	/* Directory offsets */
	hdr_offsets = (Uint32 *)
		(&((char *) emd_file)[this->emd_length-16]);

	/* Offset 0: Skeleton */
	skel_offset = SDL_SwapLE32(hdr_offsets[EMD_SKELETON]);

	emd_skel_header = (emd_skel_header_t *)
		(&((char *) emd_file)[skel_offset]);
	emd_skel_relpos = (emd_skel_relpos_t *)
		(&((char *) emd_file)[skel_offset+sizeof(emd_skel_header_t)]);
	emd_skel_data = (emd_skel_data_t *)
		(&((char *) emd_file)[skel_offset+SDL_SwapLE16(emd_skel_header->relpos_offset)]);

	skeleton = render_skel_create(this->texture);
	if (!skeleton) {
		fprintf(stderr, "Can not create skeleton\n");
		return;
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

		render_mesh_t *mesh = render_mesh_create(this->texture);
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
			3*SDL_SwapLE32(emd_mesh_object->triangles.mesh_count), sizeof(Uint16)*6,
			txcoordPtr, 0);

		free(txcoordPtr);

		/* Triangles */


		/* Add mesh to skeleton */
		skeleton->addMesh(skeleton, mesh, 0,0,0);

		emd_mesh_object++;
	}

	skeleton->shutdown(skeleton);
}

/*--- Convert EMD file (little endian) to big endian ---*/

static void emd_convert_endianness(model_t *this)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	Uint32 *hdr_offsets, mesh_offset;
	int i;
	emd_skel_header_t *emd_skel_header;
	emd_skel_relpos_t *emd_skel_relpos;
	emd_skel_data_t *emd_skel_data;
	emd_mesh_header_t *emd_mesh_header;
	emd_mesh_object_t *emd_mesh_object;
	void *emd_file = this->emd_file;

	/* Directory offsets */
	hdr_offsets = (Uint32 *)
		(&((char *) emd_file)[this->emd_length-16]);
	for (i=0; i<4; i++) {
		hdr_offsets[i] = SDL_SwapLE32(hdr_offsets[i]);
	}

	/* Offset 0: Skeleton */
	emd_skel_header = (emd_skel_header_t *)
		(&((char *) emd_file)[hdr_offsets[EMD_SKELETON]]);
	emd_skel_header->relpos_offset = SDL_SwapLE16(emd_skel_header->relpos_offset);
	emd_skel_header->unk_offset = SDL_SwapLE16(emd_skel_header->unk_offset);
	emd_skel_header->count = SDL_SwapLE16(emd_skel_header->count);
	emd_skel_header->size = SDL_SwapLE16(emd_skel_header->size);

	emd_skel_relpos = (emd_skel_relpos_t *)
		(&((char *) emd_file)[hdr_offsets[EMD_SKELETON]+sizeof(emd_skel_header_t)]);
	emd_skel_data = (emd_skel_data_t *)
		(&((char *) emd_file)[hdr_offsets[EMD_SKELETON]+emd_skel_header->relpos_offset]);

	emd_convert_endianness_skel(this, 0, emd_skel_relpos, emd_skel_data);

	/* Offset 2: Mesh data */
	emd_mesh_header = (emd_mesh_header_t *)
		(&((char *) emd_file)[hdr_offsets[EMD_MESHES]]);
	emd_mesh_header->length = SDL_SwapLE32(emd_mesh_header->length);
	emd_mesh_header->dummy = SDL_SwapLE32(emd_mesh_header->dummy);
	emd_mesh_header->num_objects = SDL_SwapLE32(emd_mesh_header->num_objects);

	mesh_offset = hdr_offsets[EMD_MESHES]+sizeof(emd_mesh_header_t);
	emd_mesh_object = (emd_mesh_object_t *)
		(&((char *) emd_file)[mesh_offset]);
	for (i=0; i<emd_mesh_header->num_objects; i++) {
		int j;
		emd_vertex_t *emd_vtx;
		emd_triangle_t *emd_tri_idx;
		void **list_done;

		/* Triangles */
		emd_mesh_object->triangles.vtx_offset = SDL_SwapLE32(emd_mesh_object->triangles.vtx_offset);
		emd_mesh_object->triangles.vtx_count = SDL_SwapLE32(emd_mesh_object->triangles.vtx_count);
		emd_mesh_object->triangles.nor_offset = SDL_SwapLE32(emd_mesh_object->triangles.nor_offset);
		emd_mesh_object->triangles.nor_count = SDL_SwapLE32(emd_mesh_object->triangles.nor_count);
		emd_mesh_object->triangles.mesh_offset = SDL_SwapLE32(emd_mesh_object->triangles.mesh_offset);
		emd_mesh_object->triangles.mesh_count = SDL_SwapLE32(emd_mesh_object->triangles.mesh_count);

		list_done = malloc(sizeof(void *)*
			(emd_mesh_object->triangles.vtx_count+
			emd_mesh_object->triangles.nor_count));
		if (!list_done) {
			fprintf(stderr, "Can not allocate mem for vtx/nor list conversion\n");
			break;
		}

		emd_vtx = (emd_vertex_t *)
			(&((char *) emd_file)[mesh_offset+emd_mesh_object->triangles.vtx_offset]);
		for (j=0; j<emd_mesh_object->triangles.vtx_count; j++) {
			emd_vtx[j].x = SDL_SwapLE16(emd_vtx[j].x);
			emd_vtx[j].y = SDL_SwapLE16(emd_vtx[j].y);
			emd_vtx[j].z = SDL_SwapLE16(emd_vtx[j].z);
			emd_vtx[j].w = SDL_SwapLE16(emd_vtx[j].w);
			list_done[j] = &emd_vtx[j];
		}

		emd_vtx = (emd_vertex_t *)
			(&((char *) emd_file)[mesh_offset+emd_mesh_object->triangles.nor_offset]);
		for (j=0; j<emd_mesh_object->triangles.nor_count; j++) {
			emd_vtx[j].x = SDL_SwapLE16(emd_vtx[j].x);
			emd_vtx[j].y = SDL_SwapLE16(emd_vtx[j].y);
			emd_vtx[j].z = SDL_SwapLE16(emd_vtx[j].z);
			emd_vtx[j].w = SDL_SwapLE16(emd_vtx[j].w);
			list_done[emd_mesh_object->triangles.vtx_count+j] = &emd_vtx[j];
		}

		emd_tri_idx = (emd_triangle_t *)
			(&((char *) emd_file)[mesh_offset+emd_mesh_object->triangles.mesh_offset]);
		for (j=0; j<emd_mesh_object->triangles.mesh_count; j++) {
			emd_tri_idx[j].n0 = SDL_SwapLE16(emd_tri_idx[j].n0);
			emd_tri_idx[j].v0 = SDL_SwapLE16(emd_tri_idx[j].v0);
			emd_tri_idx[j].n1 = SDL_SwapLE16(emd_tri_idx[j].n1);
			emd_tri_idx[j].v1 = SDL_SwapLE16(emd_tri_idx[j].v1);
			emd_tri_idx[j].n2 = SDL_SwapLE16(emd_tri_idx[j].n2);
			emd_tri_idx[j].v2 = SDL_SwapLE16(emd_tri_idx[j].v2);

			emd_tri_idx[j].clutid = SDL_SwapLE16(emd_tri_idx[j].clutid);
			emd_tri_idx[j].page = SDL_SwapLE16(emd_tri_idx[j].page);
		}

		free(list_done);

		emd_mesh_object++;
	}
#endif
}

static void emd_convert_endianness_skel(model_t *this, int num_skel,
	emd_skel_relpos_t *emd_skel_relpos,
	emd_skel_data_t *emd_skel_data)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	/* FIXME: mark already converted skel parts, to avoid multiple conversion if needed*/
	int i;
	Uint8 *emd_skel_mesh = (Uint8 *) emd_skel_data;

	emd_skel_relpos[num_skel].x = SDL_SwapLE16(emd_skel_relpos[num_skel].x);
	emd_skel_relpos[num_skel].y = SDL_SwapLE16(emd_skel_relpos[num_skel].y);
	emd_skel_relpos[num_skel].z = SDL_SwapLE16(emd_skel_relpos[num_skel].z);

	emd_skel_data[num_skel].num_mesh = SDL_SwapLE16(emd_skel_data[num_skel].num_mesh);
	emd_skel_data[num_skel].offset = SDL_SwapLE16(emd_skel_data[num_skel].offset);
	for (i=0; i<emd_skel_data[num_skel].num_mesh; i++) {
		int num_mesh = emd_skel_mesh[emd_skel_data[num_skel].offset+i];
		emd_convert_endianness_skel(this, num_mesh, emd_skel_relpos, emd_skel_data);
	}
#endif
}
