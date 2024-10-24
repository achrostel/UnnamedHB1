#include "collision.h"

#include "stdio.h"

extern unsigned char test_acf_data[];
CollisionMesh test_col;

#define abs(a) (a < 0) ? (-a) : (a)

void initCollisions() {
    colInitData(&test_col, test_acf_data);
}

void colInitData(CollisionMesh* col, unsigned char* data) {
    uint16_t* dt = (uint16_t*) data;
    col->poly_amount = dt[0];
    col->edge_amount = dt[1];
    col->colPolys = (ColPoly*)(data+4);
    col->colEdges = (ColEdge*)(data+4+(col->poly_amount * sizeof(ColPoly)));
    col->x = *(uint16_t*)(data+4+(col->poly_amount * sizeof(ColPoly))+(col->edge_amount * sizeof(ColEdge)));
    col->z = *(uint16_t*)(data+4+2+(col->poly_amount * sizeof(ColPoly))+(col->edge_amount * sizeof(ColEdge)));
    col->bounds = *(CollisionBounds*)(data+4+4+(col->poly_amount * sizeof(ColPoly))+(col->edge_amount * sizeof(ColEdge)));
    col->chunks = (CollisionChunk**)(data+4+4+(col->poly_amount * sizeof(ColPoly))+(col->edge_amount * sizeof(ColEdge))+sizeof(CollisionBounds));
    uint32_t* temp = (uint32_t*)(data+4+4+(col->poly_amount * sizeof(ColPoly))+(col->edge_amount * sizeof(ColEdge))+sizeof(CollisionBounds));
    uint16_t ltemp = col->x*col->z;
    for (uint16_t i = 0; i < col->x*col->z; ++i) {
        uint16_t ea = (*temp)>>16;
        uint16_t fa = (*temp)&(0xffff);
        // printf("%d INFO %d %d\n", i, fa, ea);
        col->chunks[i] = (CollisionChunk*)(temp+ltemp);
        // fa = col->chunks[i]->poly_amount;
        // ea = col->chunks[i]->edge_amount;
        col->chunks[i]->colPolys_indices = (uint16_t*)(col->chunks[i])+6;
        col->chunks[i]->colEdges_indices = (uint16_t*)(col->chunks[i])+6+col->chunks[i]->poly_amount;
        // printf("%d INF2 %d %d\n", i, fa, ea);
        ltemp += ((ea+fa)>>1) + 2;
        temp++;
    }
}

unsigned char test_collide(CollisionMesh* col, VECTOR* pos, VECTOR* velocity, int32_t radius) {
    unsigned char is_floor = 0;
    VECTOR* lpos = (VECTOR*)(0x1F800008);
    VECTOR* pclosest = (VECTOR*)(0x1F800020);
    SVECTOR* cnorm = (SVECTOR*)(0x1F800032);
    int16_t* pamount = (int16_t*)(0x1F800040);
    *lpos = *pos;
    pclosest->vx = 0; pclosest->vy = 0; pclosest->vz = 0;
    cnorm->vx = 0; cnorm->vy = 0; cnorm->vz = 0;
    *pamount = 0;
    uint16_t cx = ((pos->vx-col->bounds.minX)>>10);
    uint16_t cz = ((pos->vz-col->bounds.minZ)>>10);
    if (cx > col->x || cz > col->z) return 0;
    uint16_t chunkid = cx + (cz*col->x);
    // printf("Chunk: %d x %d z %d\n", chunkid, ((pos->vx-col->bounds.minX)>>10), (((pos->vz-col->bounds.minZ)>>10)));
    CollisionChunk* chunk = col->chunks[chunkid];
    for (uint16_t i = 0; i < chunk->poly_amount; ++i) {
        ColPoly* curr = &col->colPolys[chunk->colPolys_indices[i]];
        VECTOR p;
        p.vx = -(lpos->vx) + (curr->point.vx);
        p.vy = -(lpos->vy) + (curr->point.vy);
        p.vz = -(lpos->vz) + (curr->point.vz);
        // printf("POINT: %d %d %d\n", curr->point.vx, curr->point.vy, curr->point.vz);
        // printf("POS  : %d %d %d\n", lpos->vx, lpos->vy, lpos->vz);
        // printf("P    : %d %d %d\n", p.vx, p.vy, p.vz);
        int32_t r = (((curr->RC.vx * p.vx
                    + curr->RC.vy * p.vy
                    + curr->RC.vz * p.vz)>>8)
                    * curr->inv_help)>>8;

        // printf("ID: %d R: %d\n", i, r);

        if (r<0) r = -r;
        if (r > radius) continue;

        // printf("ID: %d R: %d\n", i, r);
        
        int32_t u = (((curr->UC.vx * p.vx
                    + curr->UC.vy * p.vy
                    + curr->UC.vz * p.vz)>>8)
                    * curr->inv_help)>>8;

        // printf("U %d\n", u);

        if (0 > u) continue;
        int32_t v = (((curr->VC.vx * p.vx
                    + curr->VC.vy * p.vy
                    + curr->VC.vz * p.vz)>>8)
                    * curr->inv_help)>>8;

        
        // printf("V %d\n", v);

        if (0 > v || u+v > ONE>>4) continue;

        // printf("ID: %d U: %d V: %d\n", i, u, v);

        // printf("id: %d r: %d u: %d v: %d\n", i, r, u, v);
        // printf("p: %d %d %d\n", p.vx, p.vy, p.vz);

        if (curr->is_floor) {
            is_floor = 1;

            // printf("POINT: %d %d %d\n", curr->point.vx, curr->point.vy, curr->point.vz);
            // printf("POS  : %d %d %d\n", lpos->vx, lpos->vy, lpos->vz);
            // printf("P    : %d %d %d\n", p.vx, p.vy, p.vz);
            
            lpos->vy += ((curr->norm.vy*(radius-r))>>12);
            velocity->vy = 0;
        } else {
            
            // printf("id: %d radius: %d r: %d u: %d v: %d\n", i, radius, r, u, v);

            lpos->vx += ((curr->norm.vx*(radius-r))>>12);
            lpos->vy += ((curr->norm.vy*(radius-r))>>12);
            lpos->vz += ((curr->norm.vz*(radius-r))>>12);
            // printf("coll: %d\n", i);
        }

    }

    for (uint16_t i = 0; i < chunk->edge_amount; ++i) {
        ColEdge* curr = &col->colEdges[chunk->colEdges_indices[i]];
        VECTOR p;
        p.vx = -(lpos->vx) + curr->origin.vx;
        p.vy = -(lpos->vy) + curr->origin.vy;
        p.vz = -(lpos->vz) + curr->origin.vz;

        int32_t r = ((curr->dir.vx * p.vx
                      + curr->dir.vy * p.vy
                      + curr->dir.vz * p.vz)>>8);
        int32_t r1 = (r*r)>>8;

        int32_t r2 = ((p.vx * p.vx
                     + p.vy * p.vy
                     + p.vz * p.vz)>>8);
        r2 = (r2) - ((radius*radius)>>8);

        int32_t res = r1-r2;

        if (res < 0) continue;

        int32_t sq = SquareRoot12(res<<4)>>4;
        int32_t a = -r+sq; int32_t b = -r-sq;
        // printf("ab %d %d\n", a, b);
        // if (!((a > 0 && a < curr->len) || (b > 0 && b < curr->len))) continue;
        
        int32_t dist = INT32_MAX;
        unsigned char a_hit = 0, b_hit = 0;
        
        VECTOR pclosesta = {0, 0, 0};
        VECTOR pclosestb = {0, 0, 0};
        // printf("POS  : %d %d %d\n", lpos->vx, lpos->vy, lpos->vz);
        if (a > 0 && a < curr->len) {
            pclosesta.vx = curr->origin.vx + ((curr->dir.vx*a)>>8);
            pclosesta.vy = curr->origin.vy + ((curr->dir.vy*a)>>8);
            pclosesta.vz = curr->origin.vz + ((curr->dir.vz*a)>>8);
            // printf("pcola %d %d %d\n", pclosesta.vx, pclosesta.vy, pclosesta.vz);
            a_hit = 1;
            // dista = (((pclosest->vx-(lpos->vx>>12))*(pclosest->vx-(lpos->vx>>12)))
            //         +((pclosest->vy-(lpos->vy>>12))*(pclosest->vy-(lpos->vy>>12)))
            //         +((pclosest->vz-(lpos->vz>>12))*(pclosest->vz-(lpos->vz>>12))))>>8;
        }
        if (b > 0 && b < curr->len) {
            pclosestb.vx = curr->origin.vx + ((curr->dir.vx*b)>>8);
            pclosestb.vy = curr->origin.vy + ((curr->dir.vy*b)>>8);
            pclosestb.vz = curr->origin.vz + ((curr->dir.vz*b)>>8);
            // printf("pcolb %d %d %d\n", pclosestb.vx, pclosestb.vy, pclosestb.vz);
            b_hit = 1;
            // distb = ((pclosest->vx-(lpos->vx>>12))*(pclosest->vx-(lpos->vx>>12)))
            //        +((pclosest->vy-(lpos->vy>>12))*(pclosest->vy-(lpos->vy>>12)))
            //        +((pclosest->vz-(lpos->vz>>12))*(pclosest->vz-(lpos->vz>>12)))>>8;
        }
        if (a_hit && b_hit) {
            pclosest->vx += (pclosesta.vx + pclosestb.vx)/2;
            pclosest->vy += (pclosesta.vy + pclosestb.vy)/2;
            pclosest->vz += (pclosesta.vz + pclosestb.vz)/2;
        } else if (a_hit) {
            // pclosest = pclosesta;
            pclosest->vx += pclosesta.vx;
            pclosest->vy += pclosesta.vy;
            pclosest->vz += pclosesta.vz;
        } else if (b_hit) {
            // pclosest = pclosestb;
            pclosest->vx += pclosestb.vx;
            pclosest->vy += pclosestb.vy;
            pclosest->vz += pclosestb.vz;
        } else {
            continue;
        }
        cnorm->vx += curr->norm.vx;
        cnorm->vy += curr->norm.vy;
        cnorm->vz += curr->norm.vz;
        *pamount += 1;
    }

    if (*pamount > 0) {
        pclosest->vx /= *pamount;
        pclosest->vy /= *pamount;
        pclosest->vz /= *pamount;
        cnorm->vx /= *pamount;
        cnorm->vy /= *pamount;
        cnorm->vz /= *pamount;
        int32_t x_dist = pclosest->vx-lpos->vx;
        int32_t y_dist = pclosest->vy-lpos->vy;
        int32_t z_dist = pclosest->vz-lpos->vz;
        // x_dist = abs(x_dist);
        // y_dist = abs(y_dist);
        // z_dist = abs(z_dist);
        int32_t dist = ((x_dist*x_dist)+(y_dist*y_dist)+(z_dist*z_dist))>>8;
        int32_t diff = SquareRoot12(dist<<4)>>4;
        // printf("%d coll %d  |  %d %d %d\n", *pamount, radius-diff, cnorm->vx, cnorm->vy, cnorm->vz);
        // printf("coll %d %d %d  |  %d %d %d\n", radius - x_dist, radius - y_dist, radius - z_dist, curr->norm.vx, curr->norm.vy, curr->norm.vz);
        // printf("POS: %d %d %d | CLOSE: %d %d %d\n", lpos->vx, lpos->vy, lpos->vz, pclosest->vx, pclosest->vy, pclosest->vz);
        lpos->vx += ((cnorm->vx*(radius - diff))>>12);
        lpos->vy += ((cnorm->vy*(radius - diff))>>12);
        lpos->vz += ((cnorm->vz*(radius - diff))>>12);
    }
    // lpos->vx = lpos->vx;
    // lpos->vy = lpos->vy;
    // lpos->vz = lpos->vz;
    *pos = *lpos;
    
    return is_floor;
}