#ifndef COLLISION_H
#define COLLISION_H

#include <psxgte.h>
#include <inline_c.h>

typedef struct {
    VECTOR point;
    SVECTOR norm;
    VECTOR RC;
    VECTOR UC;
    VECTOR VC;
    int32_t inv_help;
    unsigned char is_floor;
    unsigned char pad[3];
} ColPoly;

typedef struct {
    VECTOR origin;
    VECTOR dir;
    SVECTOR norm;
    uint32_t len;
} ColEdge;

typedef struct {
    uint16_t poly_amount;
    uint16_t edge_amount;
    uint16_t* colPolys_indices;
    uint16_t* colEdges_indices;
} CollisionChunk;

typedef struct {
    int32_t minX;
    int32_t minZ;
    int32_t maxX;
    int32_t maxZ;
} CollisionBounds;

typedef struct {
    uint16_t poly_amount;
    uint16_t edge_amount;
    ColPoly* colPolys;
    ColEdge* colEdges;
    CollisionBounds bounds;
    uint16_t x;
    uint16_t z;
    CollisionChunk** chunks;
} CollisionMesh;

void initCollisions();

void colInitData(CollisionMesh* col, unsigned char* data);

// returns is_floored
unsigned char test_collide(CollisionMesh* col, VECTOR* pos, VECTOR* velocity, int32_t radius);

#endif // _COLLISION_H