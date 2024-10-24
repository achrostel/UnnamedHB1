#ifndef MODEL_H
#define MODEL_H

#include <psxgte.h>
#include <stdlib.h>
#include <inline_c.h>

#include "disp.h"
#include "clip.h"
#include "texture_manager.h"
#include "player.h"

typedef struct {
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR n; // padding hold depth check type
    POLY_F3 pol;
} PF3;

typedef struct {
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR v3;
    SVECTOR n; // padding hold depth check type
    POLY_F4 pol;
} PF4;

typedef struct {
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR n; // padding hold depth check type
    Texture* tex;
    POLY_FT3 pol;
} PFT3;

typedef struct {
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR v3;
    SVECTOR n; // padding hold depth check type
    Texture* tex;
    POLY_FT4 pol;
} PFT4;

typedef struct {
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR n0; // padding hold depth check type
    SVECTOR n1; // padding hold depth check type
    SVECTOR n2; // padding hold depth check type
    POLY_G3 pol;
} PG3;

typedef struct {
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR v3;
    SVECTOR n0; // padding hold depth check type
    SVECTOR n1; // padding hold depth check type
    SVECTOR n2; // padding hold depth check type
    SVECTOR n3; // padding hold depth check type
    POLY_G4 pol;
} PG4;

typedef struct {
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR n0; // padding hold depth check type
    SVECTOR n1; // padding hold depth check type
    SVECTOR n2; // padding hold depth check type
    Texture* tex;
    POLY_GT3 pol;
} PGT3;

typedef struct {
    SVECTOR v0; // padding holds max subdiv depth
    SVECTOR v1; // padding holds backface culling mode
    SVECTOR v2;
    SVECTOR v3;
    SVECTOR n0; // padding hold depth check type
    SVECTOR n1; // padding hold depth check type
    SVECTOR n2; // padding hold depth check type
    SVECTOR n3; // padding hold depth check type
    CVECTOR c0;
    CVECTOR c1;
    CVECTOR c2;
    CVECTOR c3;
    Texture* tex;
    POLY_GT4 pol;
} PGT4;

typedef struct {
    int32_t xv;
    int32_t yv;
    int32_t zv;
    uint16_t u, v;
} SUBDIV_V;

typedef struct {
    uint16_t tpage;
    uint16_t clut;
} TINF;


typedef struct {
    int32_t minX;
    int32_t minZ;
    int32_t maxX;
    int32_t maxZ;
} WorldBounds;

typedef struct {
    uint32_t used_textures; // highest bit for Texture* already set
    uint16_t x;
    uint16_t z;
    WorldBounds bounds;
} AMF_HEADER;

typedef struct {
    uint16_t F4_amount;
    uint16_t G4_amount;
    uint16_t FT4_amount;
    uint16_t GT4_amount;
    uint16_t F3_amount;
    uint16_t G3_amount;
    uint16_t FT3_amount;
    uint16_t GT3_amount;
    PF4* f4_polies;
    PG4* g4_polies;
    PFT4* ft4_polies;
    PGT4* gt4_polies;
    PF3* f3_polies;
    PG3* g3_polies;
    PFT3* ft3_polies;
    PGT3* gt3_polies;
} AMF_CHUNK;

typedef struct {
    AMF_HEADER info;
    char* names; // one always 8 long
    AMF_CHUNK** chunks;
} AMF;


extern RECT screen_clip;

void initModels();

void amfInitData(AMF* amf, unsigned char* data);

void setTransformMatrix(MATRIX* transform);

void drawModel(AMF* model, RenderContext* ctx);
void drawWorld(AMF* model, RenderContext* ctx, Player* player);

void subdivideFT3(POLY_FT3* pol, SVECTOR* verts, uint8_t level, RenderContext* ctx, int depth);
void subdivideGT3(POLY_GT3* pol, SVECTOR* verts, uint8_t level, RenderContext* ctx, int depth);
void subdivideFT4(POLY_FT4* pol, SVECTOR* verts, uint8_t level, RenderContext* ctx, int depth);
void subdivideGT4(POLY_GT4* pol, SVECTOR* verts, uint8_t level, RenderContext* ctx, int depth);
// void subdivideQuad(QuadPoly* pol, uint8_t level, RenderContext* ctx, int depth);
// void subdivideTris(TrisPoly* pol, uint8_t level, RenderContext* ctx, int depth);

void subdivideGT42D(POLY_GT4* pol, int32_t zs[4], uint8_t level, RenderContext* ctx, int depth);

#endif // _MODEL_H