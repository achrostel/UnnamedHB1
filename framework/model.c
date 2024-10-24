#include "model.h"

#include <stdio.h>

#include "timer.h"

#define SUBDIVIDE(RES, V1, V2) (RES)->vx = (((V1)->vx+(V2)->vx)>>1); (RES)->vy = (((V1)->vy+(V2)->vy)>>1); (RES)->vz = (((V1)->vz+(V2)->vz)>>1)
#define SUBDIVIDE_COL(RES, V1, V2) (RES)->r = (((V1)->r+(V2)->r)>>1); (RES)->g = (((V1)->g+(V2)->g)>>1); (RES)->b = (((V1)->b+(V2)->b)>>1)
#define SUBDIVIDE_UV(RES, V1, V2) (RES)->u = (((V1)->u+(V2)->u)>>1); (RES)->v = (((V1)->v+(V2)->v)>>1)
#define SUBDIVIDE_DEPTH(RES, V1, V2) (RES) = (((V1)+(V2))>>1)
#define max(a, b) (a) - (((a) - (b)) & (((a) - (b)) >> 31))
#define max3(z1, z2, z3) max((z1), max((z2), (z3)))
#define max4(z1, z2, z3, z4) max((z1), max((z2), max((z3), (z4))))
#define min(a, b) (b) + (((a) - (b)) & (((a) - (b)) >> 31))
#define min3(z1, z2, z3) min((z1), min((z2), (z3)))
#define min4(z1, z2, z3, z4) min((z1), min((z2), min((z3), (z4))))
#define max16(a, b) (a) - (((a) - (b)) & (((a) - (b)) >> 15))
#define max16_3(z1, z2, z3) max16((z1), max16((z2), (z3)))
#define max16_4(z1, z2, z3, z4) max16((z1), max16((z2), max16((z3), (z4))))
#define min16(a, b) (b) + (((a) - (b)) & (((a) - (b)) >> 15))
#define min16_3(z1, z2, z3) min16((z1), min16((z2), (z3)))
#define min16_4(z1, z2, z3, z4) min16((z1), min16((z2), min16((z3), (z4))))
// #define min_Z(z1, z2, z3) ((z1) < (z2) ? (z1) : ((z2) < (z3) ? (z2) : (z3))) 
#define abs(a) ((a) ^ ((a)>>31)) - ((a)>>31) 

#define AREA_SPLIT (1<<14)
#define AREA_SPLIT_INTERLACED (1<<16)

#define DEPTH_FLATTENING (1)

// models included from incbin
extern unsigned char cyl_amf_data[];
AMF cyl_amf;
extern unsigned char room_amf_data[];
AMF room_amf;

extern unsigned char enemy_amf_data[];
AMF enemy_amf;
extern unsigned char pellet_amf_data[];
AMF pellet_amf;

void initModels() {
    // amfInitData(&cyl_amf, cyl_amf_data);
    amfInitData(&enemy_amf, enemy_amf_data);
    amfInitData(&pellet_amf, pellet_amf_data);
    amfInitData(&room_amf, room_amf_data);

}

void amfInitData(AMF* amf, unsigned char* data) {
    uint16_t* d16 = (uint16_t*) data;
    uint32_t* d32 = (uint32_t*) data;
    amf->info.used_textures = *(d32);
    amf->info.x = *(d16+2);
    amf->info.z = *(d16+3);
    amf->info.bounds = *(WorldBounds*)(data+8);

    amf->names = (char*)(data+8+sizeof(WorldBounds));

    amf->chunks = (AMF_CHUNK**)(data+8+sizeof(WorldBounds)+amf->info.used_textures*8);
    uint32_t* temp = (uint32_t*)(data+8+sizeof(WorldBounds)+amf->info.used_textures*8);
    uint32_t ltemp = amf->info.x * amf->info.z;
    for (uint16_t i = 0; i < amf->info.x * amf->info.z; ++i) {
        uint16_t a4 = (*temp) & 0xffff;
        uint16_t a3 = (*temp)>>16;
        uint32_t offset = 8 * 3;
        // printf("%d INFO %d %d\n", i, a4, a3);
        amf->chunks[i] = (AMF_CHUNK*)(temp+ltemp);
        offset += sizeof(PF4) * amf->chunks[i]->F4_amount;
        offset += sizeof(PG4) * amf->chunks[i]->G4_amount;
        offset += sizeof(PFT4) * amf->chunks[i]->FT4_amount;
        offset += sizeof(PGT4) * amf->chunks[i]->GT4_amount;
        offset += sizeof(PF3) * amf->chunks[i]->F3_amount;
        offset += sizeof(PG3) * amf->chunks[i]->G3_amount;
        offset += sizeof(PFT3) * amf->chunks[i]->FT3_amount;
        offset += sizeof(PGT3) * amf->chunks[i]->GT3_amount;
        // printf("\n%d INF2 %d %d %d %d %d %d %d %d %d %d\n", i, amf->chunks[i]->F4_amount, amf->chunks[i]->G4_amount,
        //                                                         amf->chunks[i]->FT4_amount, amf->chunks[i]->GT4_amount,
        //                                                         amf->chunks[i]->F3_amount, amf->chunks[i]->G3_amount,
        //                                                         amf->chunks[i]->FT3_amount, amf->chunks[i]->GT3_amount, ltemp, ((offset>>2)+5)<<2);
        
        amf->chunks[i]->f4_polies = (PF4*)(((uint32_t*)amf->chunks[i])+12);
        amf->chunks[i]->g4_polies = (PG4*)(amf->chunks[i]->f4_polies+amf->chunks[i]->F4_amount);
        amf->chunks[i]->ft4_polies = (PFT4*)(amf->chunks[i]->g4_polies+amf->chunks[i]->G4_amount);
        amf->chunks[i]->gt4_polies = (PGT4*)(amf->chunks[i]->ft4_polies+amf->chunks[i]->FT4_amount);
        amf->chunks[i]->f3_polies = (PF3*)(amf->chunks[i]->gt4_polies+amf->chunks[i]->GT4_amount);
        amf->chunks[i]->g3_polies = (PG3*)(amf->chunks[i]->f3_polies+amf->chunks[i]->F3_amount);
        amf->chunks[i]->ft3_polies = (PFT3*)(amf->chunks[i]->g3_polies+amf->chunks[i]->G3_amount);
        amf->chunks[i]->gt3_polies = (PGT3*)(amf->chunks[i]->ft3_polies+amf->chunks[i]->FT3_amount);
        
        ltemp += (offset>>2) + 5;
        temp++;
    }
    
    // printf("%d\n", amf->info.used_textures);
    // for (int i = 0; i < amf->info.used_textures; ++i) {
    //     printf("%.8s\n", (amf->names+i*8));
    // } 

    // for (int i = 0; i < amf->info.quad_amount; ++i) {
    //     QuadPoly* pol = &(amf->quadPolys[i]);
    //     char* name = amf->names+((uint32_t)pol->tex)*8;
    //     pol->tex = findTex(name);
    // }
    AMF_CHUNK* chunk;
    char* names = amf->names;
    for (uint16_t i = 0; i < amf->info.x * amf->info.z; ++i) {
        chunk = amf->chunks[i];
        // printf("x %d\n", i);
        for (uint16_t j = 0; j < chunk->FT3_amount; ++j) {   
            char* name = names+((uint32_t)chunk->ft3_polies[j].tex)*8;
            // printf("%.8s\n", name);
            if (!(amf->info.used_textures & 1<<31)) chunk->ft3_polies[j].tex = findTex(name);
            chunk->ft3_polies[j].pol.tpage = chunk->ft3_polies[j].tex->tpage;
            chunk->ft3_polies[j].pol.clut = chunk->ft3_polies[j].tex->clut;
            chunk->ft3_polies[j].pol.u0 += chunk->ft3_polies[j].tex->ox;
            chunk->ft3_polies[j].pol.v0 += chunk->ft3_polies[j].tex->oy;
            chunk->ft3_polies[j].pol.u1 += chunk->ft3_polies[j].tex->ox;
            chunk->ft3_polies[j].pol.v1 += chunk->ft3_polies[j].tex->oy;
            chunk->ft3_polies[j].pol.u2 += chunk->ft3_polies[j].tex->ox;
            chunk->ft3_polies[j].pol.v2 += chunk->ft3_polies[j].tex->oy;
        }
        for (uint16_t j = 0; j < chunk->FT4_amount; ++j) {
            char* name = names+((uint32_t)chunk->ft4_polies[j].tex)*8;
            // printf("%.8s\n", name);
            if (!(amf->info.used_textures & 1<<31)) chunk->ft4_polies[j].tex = findTex(name);
            chunk->ft4_polies[j].pol.tpage = chunk->ft4_polies[j].tex->tpage;
            chunk->ft4_polies[j].pol.clut = chunk->ft4_polies[j].tex->clut;
            chunk->ft4_polies[j].pol.u0 += chunk->ft4_polies[j].tex->ox;
            chunk->ft4_polies[j].pol.v0 += chunk->ft4_polies[j].tex->oy;
            chunk->ft4_polies[j].pol.u1 += chunk->ft4_polies[j].tex->ox;
            chunk->ft4_polies[j].pol.v1 += chunk->ft4_polies[j].tex->oy;
            chunk->ft4_polies[j].pol.u2 += chunk->ft4_polies[j].tex->ox;
            chunk->ft4_polies[j].pol.v2 += chunk->ft4_polies[j].tex->oy;
            chunk->ft4_polies[j].pol.u3 += chunk->ft4_polies[j].tex->ox;
            chunk->ft4_polies[j].pol.v3 += chunk->ft4_polies[j].tex->oy;
        }
        for (uint16_t j = 0; j < chunk->GT3_amount; ++j) {
            char* name = names+((uint32_t)chunk->gt3_polies[j].tex)*8;
            // printf("%.8s\n", name);
            if (!(amf->info.used_textures & 1<<31)) chunk->gt3_polies[j].tex = findTex(name);
            chunk->gt3_polies[j].pol.tpage = chunk->gt3_polies[j].tex->tpage;
            chunk->gt3_polies[j].pol.clut = chunk->gt3_polies[j].tex->clut;
            chunk->gt3_polies[j].pol.u0 += chunk->gt3_polies[j].tex->ox;
            chunk->gt3_polies[j].pol.v0 += chunk->gt3_polies[j].tex->oy;
            chunk->gt3_polies[j].pol.u1 += chunk->gt3_polies[j].tex->ox;
            chunk->gt3_polies[j].pol.v1 += chunk->gt3_polies[j].tex->oy;
            chunk->gt3_polies[j].pol.u2 += chunk->gt3_polies[j].tex->ox;
            chunk->gt3_polies[j].pol.v2 += chunk->gt3_polies[j].tex->oy;
        }
        for (uint16_t j = 0; j < chunk->GT4_amount; ++j) {
            char* name = names+((uint32_t)chunk->gt4_polies[j].tex)*8;
            if (!(amf->info.used_textures & (1<<31))) {
                chunk->gt4_polies[j].tex = findTex(name);
                // printf("c %08x\n", chunk->gt4_polies[j].tex);
            }
            chunk->gt4_polies[j].pol.tpage = chunk->gt4_polies[j].tex->tpage;
            chunk->gt4_polies[j].pol.clut = chunk->gt4_polies[j].tex->clut;
            chunk->gt4_polies[j].pol.u0 += chunk->gt4_polies[j].tex->ox;
            chunk->gt4_polies[j].pol.v0 += chunk->gt4_polies[j].tex->oy;
            chunk->gt4_polies[j].pol.u1 += chunk->gt4_polies[j].tex->ox;
            chunk->gt4_polies[j].pol.v1 += chunk->gt4_polies[j].tex->oy;
            chunk->gt4_polies[j].pol.u2 += chunk->gt4_polies[j].tex->ox;
            chunk->gt4_polies[j].pol.v2 += chunk->gt4_polies[j].tex->oy;
            chunk->gt4_polies[j].pol.u3 += chunk->gt4_polies[j].tex->ox;
            chunk->gt4_polies[j].pol.v3 += chunk->gt4_polies[j].tex->oy;
        }
    }

    // set highest bit to not relocate textures
    // printf("ut %d\n", amf->info.used_textures);
    d32[0] |= 1<<31;
}

void setTransformMatrix(MATRIX* transform) {
    gte_SetRotMatrix(transform);
    gte_SetTransMatrix(transform);
}

void drawModel(AMF* model, RenderContext* ctx) {
    AMF_CHUNK* chunk = model->chunks[0];
    int p;
    uint16_t i;
    // z0 z1 z2 z3 zfar depth
    int32_t* zs = (int32_t*)(0x1F800028); // 6 * 4 bytes
    int16_t* xys = (int16_t*)(0x1F800008); // 8 * 2 bytes
    int32_t area;
    uint8_t* cols = (uint8_t*)(0x1F800028); // 3 * 4 bytes

    for (i = 0; i < chunk->GT4_amount; ++i) {
        PGT4* pol = &chunk->gt4_polies[i];
        
        if (ctx->render_mode && pol->tex->aif.header.transparency & 1) continue;

        gte_ldv3c(&pol->v0);
        gte_rtpt();
        
        // if (!pol->v1.pad) {
        gte_nclip();
        gte_stopz(&p);
        // add_debug_count(0);
        if (p < 0) continue;
        // }

        gte_avsz3();
        gte_stotz(&zs[4]);

        zs[5] = zs[4]>>DEPTH_FLATTENING;
        // printf("1 z0 %d z1 %d z2 %d z3 %d\n", z0, z1, z2, z3);

        // add_debug_count(1);
        if((zs[5] <= 0) || (zs[5] >= OT_LENGTH))
            continue;

        gte_stsxy3c(xys);

        gte_ldv0(&pol->v3);
        gte_rtps();
        gte_stsxy(&xys[6]);

        // printf("POLY: %d %d %d %d %d %d %d %d\n", polGT4->x0, polGT4->y0, polGT4->x1, polGT4->y1, polGT4->x2, polGT4->y2, polGT4->x3, polGT4->y3);

        gte_avsz4();
        gte_stotz(&zs[4]);
        zs[5] = zs[4]>>DEPTH_FLATTENING;

        // add_debug_count(3);

        // add_debug_count(2);
        // if( quad_clip( ctx->screen_clip,
        //     (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
        //     (DVECTOR*)&xys[4], (DVECTOR*)&xys[6] ) ) {
        //     continue;
        // }
        POLY_GT4* pol4 = (POLY_GT4*)new_primitive(ctx);

        setPolyGT4(pol4);
        setSemiTrans(pol4, pol->tex->aif.header.transparency&1);
        setXY4(pol4, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5], xys[6], xys[7]);
        setUV4(pol4, pol->pol.u0, pol->pol.v0, pol->pol.u1, pol->pol.v1, pol->pol.u2, pol->pol.v2, pol->pol.u3, pol->pol.v3);
        pol4->tpage = pol->pol.tpage;
        pol4->clut = pol->pol.clut;
        gte_lddp(zs[4]*4);
        gte_ldrgb3(&pol->pol.r0, &pol->pol.r1, &pol->pol.r2);
        gte_dpct();
        gte_strgb3(&cols[0], &cols[4], &cols[8]);
        gte_ldrgb(&pol->pol.r3);
        gte_dpcs();
        setRGB0(pol4, cols[0], cols[1], cols[2]);
        setRGB1(pol4, cols[4], cols[5], cols[6]);
        setRGB2(pol4, cols[8], cols[9], cols[10]);
        gte_strgb(&cols[12]);

        setRGB3(pol4, cols[12], cols[13], cols[14]);
        // setRGB0(pol4, pol->pol.r0, pol->pol.g0, pol->pol.b0);
        // setRGB1(pol4, pol->pol.r1, pol->pol.g1, pol->pol.b1);
        // setRGB2(pol4, pol->pol.r2, pol->pol.g2, pol->pol.b2);
        // setRGB3(pol4, pol->pol.r3, pol->pol.g3, pol->pol.b3);
        
        push_primitive(ctx, zs[5], sizeof(POLY_GT4), (uint8_t*)pol4);
    }
    for (i = 0; i < chunk->G4_amount; ++i) {
        PG4* pol = &chunk->g4_polies[i];

        gte_ldv3c(&pol->v0);
        gte_rtpt();
        
        // if (!pol->v1.pad) {
        gte_nclip();
        gte_stopz(&p);
        // add_debug_count(0);
        if (p < 0) continue;
        // }

        gte_avsz3();
        gte_stotz(&zs[4]);

        zs[5] = zs[4]>>DEPTH_FLATTENING;
        // printf("1 z0 %d z1 %d z2 %d z3 %d\n", z0, z1, z2, z3);

        // add_debug_count(1);
        if((zs[5] <= 0) || (zs[5] >= OT_LENGTH))
            continue;

        gte_stsxy3c(xys);

        gte_ldv0(&pol->v3);
        gte_rtps();
        gte_stsxy(&xys[6]);

        // printf("POLY: %d %d %d %d %d %d %d %d\n", polGT4->x0, polGT4->y0, polGT4->x1, polGT4->y1, polGT4->x2, polGT4->y2, polGT4->x3, polGT4->y3);

        gte_avsz4();
        gte_stotz(&zs[4]);
        zs[5] = zs[4]>>DEPTH_FLATTENING;

        // add_debug_count(3);

        // add_debug_count(2);
        // if( quad_clip( ctx->screen_clip,
        //     (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
        //     (DVECTOR*)&xys[4], (DVECTOR*)&xys[6] ) ) {
        //     continue;
        // }
        POLY_G4* pol4 = (POLY_G4*)new_primitive(ctx);

        setPolyG4(pol4);
        setXY4(pol4, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5], xys[6], xys[7]);
        gte_lddp(zs[4]*4);
        gte_ldrgb3(&pol->pol.r0, &pol->pol.r1, &pol->pol.r2);
        gte_dpct();
        gte_strgb3(&cols[0], &cols[4], &cols[8]);
        gte_ldrgb(&pol->pol.r3);
        gte_dpcs();
        setRGB0(pol4, cols[0], cols[1], cols[2]);
        setRGB1(pol4, cols[4], cols[5], cols[6]);
        setRGB2(pol4, cols[8], cols[9], cols[10]);
        gte_strgb(&cols[12]);

        setRGB3(pol4, cols[12], cols[13], cols[14]);
        // setRGB0(pol4, pol->pol.r0, pol->pol.g0, pol->pol.b0);
        // setRGB1(pol4, pol->pol.r1, pol->pol.g1, pol->pol.b1);
        // setRGB2(pol4, pol->pol.r2, pol->pol.g2, pol->pol.b2);
        // setRGB3(pol4, pol->pol.r3, pol->pol.g3, pol->pol.b3);
        
        push_primitive(ctx, zs[5], sizeof(POLY_G4), (uint8_t*)pol4);
    }
}

void drawWorld(AMF* model, RenderContext* ctx, Player* player) {
    // RenderBuffer *buffer = &(ctx->buffers[ctx->active_buffer]);

    int p;
    uint16_t i;
    // z0 z1 z2 z3 zfar depth
    int32_t* zs = (int32_t*)(0x1F800028); // 6 * 4 bytes
    int16_t* xys = (int16_t*)(0x1F800008); // 8 * 2 bytes
    int32_t area;
    uint8_t* cols = (uint8_t*)(0x1F800028); // 3 * 4 bytes
    int16_t angle = (player->cam.rot.vy>>12);
    int16_t rangle = (player->cam.rot.vy>>12) + 512;
    uint32_t res = sincos_t(rangle);
    int16_t rx = (-(int16_t)(res))>>4;
    int16_t rz = (-(int16_t)(res>>16))>>4;
    int16_t lx = -rz;
    int16_t lz = rx;

    res = sincos_t(angle);
    int16_t cx = ((player->pos.vx-model->info.bounds.minX+((int16_t)(res)>>1))>>10);
    int16_t cz = ((player->pos.vz-model->info.bounds.minZ+((int16_t)(res>>16)>>1))>>10);

    uint16_t chunks[96];
    int chunkamount = 0;

    for (int16_t z = -cz; z < model->info.z-cz; ++z) {
        for (int16_t x = -cx; x < model->info.x-cx; ++x) {
            int16_t lsign = (1 | (((z * lx) - (x * lz))>>15));
            int16_t rsign = (1 | (((z * rx) - (x * rz))>>15));
            if (!(lsign+rsign) && lsign < 0 && (x*x+z*z) < 40) {
                chunks[chunkamount] = x+cx + ((z+cz)*model->info.x);
                chunkamount++;
            }
        }
    }

    TIME_FUNCTION_ACC_P("SUBDIV", 11);
    TIME_FUNCTION_ACC_P("SUBDIV2D", 12);

    for (int c = 0; c < chunkamount; ++c) {
        AMF_CHUNK* chunk = model->chunks[chunks[c]];
        for (i = 0; i < chunk->F4_amount; ++i) {
            PF4* pol = &chunk->f4_polies[i];

            gte_ldv3c(&pol->v0);
            gte_rtpt();
            
            gte_nclip();
            gte_stopz(&p);
            // add_debug_count(0);
            if (p < 0) continue;

            if (pol->n.pad) {
                gte_stsz3c(&zs[0]);
                zs[4] = max3(zs[0], zs[1], zs[2])>>2;
            } else {
                gte_avsz3();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;
            // printf("1 z0 %d z1 %d z2 %d z3 %d\n", z0, z1, z2, z3);

            // add_debug_count(1);
            if((zs[5] <= 0) || (zs[5] >= OT_LENGTH))
                continue;

            gte_stsxy3c(xys);

            gte_ldv0(&pol->v3);
            gte_rtps();
            gte_stsxy(&xys[6]);

            // printf("POLY: %d %d %d %d %d %d %d %d\n", polGT4->x0, polGT4->y0, polGT4->x1, polGT4->y1, polGT4->x2, polGT4->y2, polGT4->x3, polGT4->y3);

            if (pol->n.pad) {
                gte_stsz(&zs[3]);
                zs[4] = max(zs[4], zs[3]>>2);
            } else {
                gte_avsz4();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;

            // add_debug_count(3);

            // add_debug_count(2);
            if( quad_clip( ctx->screen_clip,
                (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
                (DVECTOR*)&xys[4], (DVECTOR*)&xys[6] ) ) {
                continue;
            }

            setXY4(&pol->pol, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5], xys[6], xys[7]);
            push_primitive_pre(ctx, zs[5], (uint8_t*)&pol->pol);
        }

        for (i = 0; i < chunk->G4_amount; ++i) {
            PG4* pol = &chunk->g4_polies[i];

            gte_ldv3c(&pol->v0);
            gte_rtpt();
            
            gte_nclip();
            gte_stopz(&p);
            // add_debug_count(0);
            if (p < 0) continue;

            if (pol->n0.pad) {
                gte_stsz3c(&zs[0]);
                zs[4] = max3(zs[0], zs[1], zs[2])>>2;
            } else {
                gte_avsz3();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;
            // printf("1 z0 %d z1 %d z2 %d z3 %d\n", z0, z1, z2, z3);

            // add_debug_count(1);
            if((zs[5] <= 0) || (zs[5] >= OT_LENGTH))
                continue;

            gte_stsxy3c(xys);

            gte_ldv0(&pol->v3);
            gte_rtps();
            gte_stsxy(&xys[6]);

            // printf("POLY: %d %d %d %d %d %d %d %d\n", polGT4->x0, polGT4->y0, polGT4->x1, polGT4->y1, polGT4->x2, polGT4->y2, polGT4->x3, polGT4->y3);

            if (pol->n0.pad) {
                gte_stsz(&zs[3]);
                zs[4] = max(zs[4], zs[3]>>2);
            } else {
                gte_avsz4();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;

            // add_debug_count(3);

            // add_debug_count(2);
            if( quad_clip( ctx->screen_clip,
                (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
                (DVECTOR*)&xys[4], (DVECTOR*)&xys[6] ) ) {
                continue;
            }

            setXY4(&pol->pol, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5], xys[6], xys[7]);
            push_primitive_pre(ctx, zs[5], (uint8_t*)&pol->pol);
        }
        
        for (i = 0; i < chunk->FT4_amount; ++i) {
            PFT4* pol = &chunk->ft4_polies[i];

            gte_ldv3c(&pol->v0);
            gte_rtpt();
            
            gte_nclip();
            gte_stopz(&p);
            // add_debug_count(0);
            if (p < 0) continue;

            if (pol->n.pad) {
                gte_stsz3c(&zs[0]);
                zs[4] = max3(zs[0], zs[1], zs[2])>>2;
            } else {
                gte_avsz3();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;
            // printf("1 z0 %d z1 %d z2 %d z3 %d\n", z0, z1, z2, z3);

            // add_debug_count(1);
            if((zs[5] <= 0) || (zs[5] >= OT_LENGTH))
                continue;

            gte_stsxy3c(xys);

            gte_ldv0(&pol->v3);
            gte_rtps();
            gte_stsxy(&xys[6]);

            // add_debug_count(2);
            if( quad_clip( ctx->screen_clip,
                (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
                (DVECTOR*)&xys[4], (DVECTOR*)&xys[6] ) ) {
                continue;
            }

            // printf("POLY: %d %d %d %d %d %d %d %d\n", polGT4->x0, polGT4->y0, polGT4->x1, polGT4->y1, polGT4->x2, polGT4->y2, polGT4->x3, polGT4->y3);

            if (pol->n.pad) {
                gte_stsz(&zs[3]);
                zs[4] = max(zs[4], zs[3]>>2);
            } else {
                gte_avsz4();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;

            // add_debug_count(3);

            area = 0;
            area += xys[2] * (xys[7] - xys[1]);
            area += xys[6] * (xys[5] - xys[3]);
            area += xys[0] * (xys[3] - xys[5]);
            area += xys[4] * (xys[1] - xys[7]);

            // printf("AREA 0: %d\n", area);
            if (area > (ctx->render_mode ? AREA_SPLIT_INTERLACED : AREA_SPLIT)) {
                subdivideFT4(&pol->pol, &pol->v0, 1, ctx, zs[5]);
                continue;
            }

            setXY4(&pol->pol, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5], xys[6], xys[7]);
            push_primitive_pre(ctx, zs[5], (uint8_t*)&pol->pol);
        }

        for (i = 0; i < chunk->GT4_amount; ++i) {
            PGT4* pol = &chunk->gt4_polies[i];
            if (ctx->render_mode && pol->tex->aif.header.transparency & 1) continue;
            POLY_GT4* dpol = new_primitive_reserved(ctx, sizeof(POLY_GT4));
            gte_ldv3c(&pol->v0);
            gte_rtpt();
            
            if (!pol->v1.pad) {
                gte_nclip();
                gte_stopz(&p);
                // add_debug_count(0);
                if (p < 0) continue;
            }

            if (pol->n0.pad) {
                gte_stsz3c(&zs[0]);
                zs[4] = max3(zs[0], zs[1], zs[2])>>2;
            } else {
                gte_avsz3();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;
            // printf("1 z0 %d z1 %d z2 %d z3 %d\n", z0, z1, z2, z3);

            // add_debug_count(1);
            if((zs[5] <= 0) || (zs[5] >= OT_LENGTH))
                continue;

            gte_stsxy3c(xys);
            gte_ldv0(&pol->v3);
            gte_rtps();
            gte_stsxy(&xys[6]);

            // add_debug_count(2);
            if( quad_clip( ctx->screen_clip,
                (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
                (DVECTOR*)&xys[4], (DVECTOR*)&xys[6] ) ) {
                continue;
            }
            setXY4(dpol, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5], xys[6], xys[7]);
            
            // printf("POLY: %d %d %d %d %d %d %d %d\n", polGT4->x0, polGT4->y0, polGT4->x1, polGT4->y1, polGT4->x2, polGT4->y2, polGT4->x3, polGT4->y3);

            if (pol->n0.pad) {
                gte_stsz(&zs[3]);
                zs[4] = max(zs[4], zs[3]>>2);
            } else {
                gte_avsz4();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;

            // add_debug_count(3);
            area = 0;
            area += xys[2] * (xys[7] - xys[1]);
            area += xys[6] * (xys[5] - xys[3]);
            area += xys[0] * (xys[3] - xys[5]);
            area += xys[4] * (xys[1] - xys[7]);

            gte_lddp(zs[4]*4);
            gte_ldrgb3(&pol->c0, &pol->c1, &pol->c2);
            gte_dpct();
            gte_strgb3(&cols[0], &cols[4], &cols[8]);
            gte_ldrgb(&pol->c3);
            gte_dpcs();
            setRGB0(dpol, cols[0], cols[1], cols[2]);
            setRGB1(dpol, cols[4], cols[5], cols[6]);
            setRGB2(dpol, cols[8], cols[9], cols[10]);
            gte_strgb(&cols[12]);

            setRGB3(dpol, cols[12], cols[13], cols[14]);

            setUV4(dpol, pol->pol.u0, pol->pol.v0,
                         pol->pol.u1, pol->pol.v1,
                         pol->pol.u2, pol->pol.v2,
                         pol->pol.u3, pol->pol.v3);
            dpol->clut = pol->pol.clut;
            dpol->tpage = pol->pol.tpage;

            setPolyGT4(dpol);
            setSemiTrans(dpol, pol->tex->aif.header.transparency&1);

            // printf("AREA 0: %d\n", area);
            if (pol->v0.pad > 0 && area > (ctx->render_mode ? AREA_SPLIT_INTERLACED : AREA_SPLIT)) {
                // subdivideGT4(dpol, &pol->v0, pol->v0.pad, ctx, zs[5]);
                if (zs[4] < 255) {
                    TIME_FUNCTION_ACC_S(11);
                    subdivideGT4(dpol, &pol->v0, pol->v0.pad-1, ctx, zs[5]);
                    TIME_FUNCTION_ACC_E(11);
                } else {
                    TIME_FUNCTION_ACC_S(12);
                    subdivideGT42D(dpol, zs, pol->v0.pad-1, ctx, zs[5]);
                    TIME_FUNCTION_ACC_E(12);
                }
                continue;
            }
            
            if (area > (1<<16)) continue;

            push_primitive_pre(ctx, zs[5], (uint8_t*)dpol);
        }

        for (i = 0; i < chunk->F3_amount; ++i) {
            PF3* pol = &chunk->f3_polies[i];

            gte_ldv3c(&pol->v0);
            gte_rtpt();
            
            gte_nclip();
            gte_stopz(&p);
            // add_debug_count(0);
            if (p < 0) continue;

            if (pol->n.pad) {
                gte_stsz3c(&zs[0]);
                zs[4] = max3(zs[0], zs[1], zs[2])>>2;
            } else {
                gte_avsz3();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;
            // printf("1 z0 %d z1 %d z2 %d z3 %d\n", z0, z1, z2, z3);

            // add_debug_count(1);
            if((zs[5] <= 0) || (zs[5] >= OT_LENGTH))
                continue;

            gte_stsxy3c(xys);

            // add_debug_count(3);

            // add_debug_count(2);
            if( tri_clip( ctx->screen_clip,
                (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
                (DVECTOR*)&xys[4] ) ) {
                continue;
            }

            setXY3(&pol->pol, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5]);
            push_primitive_pre(ctx, zs[5], (uint8_t*)&pol->pol);
        }

        for (i = 0; i < chunk->G3_amount; ++i) {
            PG3* pol = &chunk->g3_polies[i];

            gte_ldv3c(&pol->v0);
            gte_rtpt();
            
            gte_nclip();
            gte_stopz(&p);
            // add_debug_count(0);
            if (p < 0) continue;

            if (pol->n0.pad) {
                gte_stsz3c(&zs[0]);
                zs[4] = max3(zs[0], zs[1], zs[2])>>2;
            } else {
                gte_avsz3();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;
            // printf("1 z0 %d z1 %d z2 %d z3 %d\n", z0, z1, z2, z3);

            // add_debug_count(1);
            if((zs[5] <= 0) || (zs[5] >= OT_LENGTH))
                continue;

            gte_stsxy3c(xys);

            // add_debug_count(3);

            // add_debug_count(2);
            if( tri_clip( ctx->screen_clip,
                (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
                (DVECTOR*)&xys[4] ) ) {
                continue;
            }

            setXY3(&pol->pol, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5]);
            push_primitive_pre(ctx, zs[5], (uint8_t*)&pol->pol);
        }
        
        for (i = 0; i < chunk->FT3_amount; ++i) {
            PFT3* pol = &chunk->ft3_polies[i];
            gte_ldv3c(&pol->v0);
            gte_rtpt();
            
            gte_nclip();
            gte_stopz(&p);
            // add_debug_count(0);
            if (p < 0) continue;

            if (pol->n.pad) {
                gte_stsz3c(&zs[0]);
                zs[4] = max3(zs[0], zs[1], zs[2])>>2;
            } else {
                gte_avsz3();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;
            // printf("1 z0 %d z1 %d z2 %d z3 %d\n", z0, z1, z2, z3);

            // add_debug_count(1);
            if((zs[5] <= 0) || (zs[5] >= OT_LENGTH))
                continue;

            gte_stsxy3c(xys);

            // add_debug_count(3);

            // add_debug_count(2);
            // if( tri_clip( ctx->screen_clip,
            //     (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
            //     (DVECTOR*)&xys[4] ) ) {
            //     continue;
            // }

            area = 0;
            area += xys[2] * (xys[5] - xys[1]);
            area += xys[4] * (xys[1] - xys[3]);
            area += xys[0] * (xys[3] - xys[5]);

            // printf("AREA 0: %d\n", area);
            if (area > (ctx->render_mode ? AREA_SPLIT_INTERLACED : AREA_SPLIT)) {
                subdivideFT3(&pol->pol, &pol->v0, 1, ctx, zs[5]);
                continue;
            }
        
            setXY3(&pol->pol, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5]);
            push_primitive_pre(ctx, zs[5], (uint8_t*)&pol->pol);
        }

        for (i = 0; i < chunk->GT3_amount; ++i) {
            PGT3* pol = &chunk->gt3_polies[i];

            gte_ldv3c(&pol->v0);
            gte_rtpt();
            
            gte_nclip();
            gte_stopz(&p);
            // add_debug_count(0);
            if (p < 0) continue;

            if (pol->n0.pad) {
                gte_stsz3c(&zs[0]);
                zs[4] = max3(zs[0], zs[1], zs[2])>>2;
            } else {
                gte_avsz3();
                gte_stotz(&zs[4]);
            }
            zs[5] = zs[4]>>DEPTH_FLATTENING;
            // printf("1 z0 %d z1 %d z2 %d z3 %d\n", z0, z1, z2, z3);

            // add_debug_count(1);
            if((zs[5] <= 0) || (zs[5] >= OT_LENGTH))
                continue;

            gte_stsxy3c(xys);

            // add_debug_count(3);

            // add_debug_count(2);
            // if( tri_clip( ctx->screen_clip,
            //     (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
            //     (DVECTOR*)&xys[4] ) ) {
            //     continue;
            // }

            setXY3(&pol->pol, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5]);
            push_primitive_pre(ctx, zs[5], (uint8_t*)&pol->pol);
        }
    }
}


/*
        *0
       / \
     0*   *1
     /     \
    *---*---*
    1   2   2
*/
/*
       0
   0*--*--*1
    |  4  |
   3*--*--*1
    |     |
   2*--*--*3
       2  
*/

void subdivideFT3(POLY_FT3* pol, SVECTOR* verts, uint8_t level, RenderContext* ctx, int depth) {                                               
    SVECTOR* vs = (SVECTOR*)(0x1F800068); // 3 * 8
    uint16_t* uxs = (uint16_t*)(0x1F800068+24); // 3 * 2
    uint16_t* xvs = (uint16_t*)(0x1F800068+24+6); // 3 * 2

    int16_t* xys = (int16_t*)(0x1F800068+24+6+6); // 8 * 2 bytes
    int32_t area;

    SVECTOR sub_verts[3];

    SUBDIVIDE(&vs[0], &verts[0], &verts[1]);
    SUBDIVIDE(&vs[1], &verts[0], &verts[2]);
    SUBDIVIDE(&vs[2], &verts[1], &verts[2]);

    uxs[0] = (pol->u0 + pol->u1) >> 1;
    uxs[1] = (pol->u0 + pol->u2) >> 1;
    uxs[2] = (pol->u1 + pol->u2) >> 1;
    xvs[0] = (pol->v0 + pol->v1) >> 1;
    xvs[1] = (pol->v0 + pol->v2) >> 1;
    xvs[2] = (pol->v1 + pol->v2) >> 1;

    for (uint8_t i = 0; i < 4; ++i) {
        POLY_FT3* spol = (POLY_FT3*)new_primitive(ctx);
        switch (i) {
        case 0: {
            sub_verts[0] = verts[0]; sub_verts[1] = vs[0]; sub_verts[2] = vs[1];
            spol->u0 = pol->u0; spol->u1 = uxs[0]; spol->u2 = uxs[1];
            spol->v0 = pol->v0; spol->v1 = xvs[0]; spol->v2 = xvs[1];
        }   break;
        case 1: {
            sub_verts[0] = vs[0]; sub_verts[1] = sub_verts[1]; sub_verts[2] = vs[2];
            spol->u0 = uxs[0]; spol->u1 = pol->u1; spol->u2 = uxs[2];
            spol->v0 = xvs[0]; spol->v1 = pol->v1; spol->v2 = xvs[2];
        }   break;
        case 2: {
            sub_verts[0] = vs[1]; sub_verts[1] = vs[2]; sub_verts[2] = verts[2];
            spol->u0 = uxs[1]; spol->u1 = uxs[2]; spol->u2 = pol->u2;
            spol->v0 = xvs[1]; spol->v1 = xvs[2]; spol->v2 = pol->v2;
        }   break;
        case 3: {
            sub_verts[0] = vs[0]; sub_verts[1] = vs[2]; sub_verts[2] = vs[1];
            spol->u0 = uxs[0]; spol->u1 = uxs[2]; spol->u2 = uxs[1];
            spol->v0 = xvs[0]; spol->v1 = xvs[2]; spol->v2 = xvs[1];
        }   break;
        }

        gte_ldv3c(sub_verts);
        gte_rtpt();

        gte_stsxy3c(xys);

        // // add_debug_count(2);
        if( tri_clip( ctx->screen_clip,
            (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
            (DVECTOR*)&xys[4]) ) {
            continue;
        }

        area = 0;
        area += xys[2] * (xys[5] - xys[1]);
        area += xys[4] * (xys[1] - xys[3]);
        area += xys[0] * (xys[3] - xys[5]);

        // printf("AREA 0: %d\n", area);
        if (level && area > (ctx->render_mode ? AREA_SPLIT_INTERLACED : AREA_SPLIT)) {
            subdivideFT3(spol, sub_verts, level-1, ctx, depth);
            continue;
        }

        setPolyF3(spol);
        setXY3(spol, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5]);
        setRGB0(spol, pol->r0, pol->g0, pol->b0);
        push_primitive(ctx, depth, sizeof(POLY_FT3), (uint8_t*)spol);
    }
}

void subdivideFT4(POLY_FT4* pol, SVECTOR* verts, uint8_t level, RenderContext* ctx, int depth) {                                               
    SVECTOR vs[5];
    uint8_t uxs[5];
    uint8_t xvs[5];

    int16_t* xys = (int16_t*)(0x1F800068); // 8 * 2 bytes
    int32_t area;

    SVECTOR sub_verts[4];

    SUBDIVIDE(&vs[0], &verts[0], &verts[1]);
    SUBDIVIDE(&vs[1], &verts[1], &verts[3]);
    SUBDIVIDE(&vs[2], &verts[2], &verts[3]);
    SUBDIVIDE(&vs[3], &verts[0], &verts[2]);
    SUBDIVIDE(&vs[4], &vs[0], &vs[2]);

    uxs[0] = ((pol->u0)>>1) + ((pol->u1)>>1);
    uxs[1] = ((pol->u1)>>1) + ((pol->u3)>>1);
    uxs[2] = ((pol->u2)>>1) + ((pol->u3)>>1);
    uxs[3] = ((pol->u0)>>1) + ((pol->u2)>>1);
    uxs[4] = ((uxs[0])>>1) + ((uxs[2])>>1);
    xvs[0] = ((pol->v0)>>1) + ((pol->v1)>>1);
    xvs[1] = ((pol->v1)>>1) + ((pol->v3)>>1);
    xvs[2] = ((pol->v2)>>1) + ((pol->v3)>>1);
    xvs[3] = ((pol->v0)>>1) + ((pol->v2)>>1);
    xvs[4] = ((xvs[0])>>1) + ((xvs[2])>>1);

    for (uint8_t i = 0; i < 4; ++i) {
        POLY_FT4 spol;
        switch (i) {
        case 3: {
            sub_verts[0] = verts[0]; sub_verts[1] = vs[0];    sub_verts[2] = vs[3];    sub_verts[3] = vs[4];
            spol.u0 = pol->u0;   spol.u1 = uxs[0];    spol.u2 = uxs[3];    spol.u3 = uxs[4];
            spol.v0 = pol->v0;   spol.v1 = xvs[0];    spol.v2 = xvs[3];    spol.v3 = xvs[4];
        }   break;
        case 2: {
            sub_verts[0] = vs[0];    sub_verts[1] = verts[1]; sub_verts[2] = vs[4];    sub_verts[3] = vs[1];
            spol.u0 = uxs[0];    spol.u1 = pol->u1;   spol.u2 = uxs[4];    spol.u3 = uxs[1];
            spol.v0 = xvs[0];    spol.v1 = pol->v1;   spol.v2 = xvs[4];    spol.v3 = xvs[1];
        }   break;
        case 1: {
            sub_verts[0] = vs[3];    sub_verts[1] = vs[4];    sub_verts[2] = verts[2]; sub_verts[3] = vs[2];
            spol.u0 = uxs[3];    spol.u1 = uxs[4];    spol.u2 = pol->u2;   spol.u3 = uxs[2];
            spol.v0 = xvs[3];    spol.v1 = xvs[4];    spol.v2 = pol->v2;   spol.v3 = xvs[2];
        }   break;
        case 0: {
            sub_verts[0] = vs[4];    sub_verts[1] = vs[1];    sub_verts[2] = vs[2];    sub_verts[3] = verts[3];
            spol.u0 = uxs[4];    spol.u1 = uxs[1];    spol.u2 = uxs[2];    spol.u3 = pol->u3;
            spol.v0 = xvs[4];    spol.v1 = xvs[1];    spol.v2 = xvs[2];    spol.v3 = pol->v3;
        }   break;
        }

        gte_ldv3c(sub_verts);
        gte_rtpt();
        gte_stsxy3c(xys);

        gte_ldv0(&sub_verts[3]);
        gte_rtps();
        gte_stsxy(&xys[6]);

        // // add_debug_count(2);
        if( quad_clip( ctx->screen_clip,
            (DVECTOR*)&xys[0], (DVECTOR*)&xys[2], 
            (DVECTOR*)&xys[4], (DVECTOR*)&xys[6]) ) {
            continue;
        }

        POLY_FT4* fpol = (POLY_FT4*)new_primitive(ctx);
        setUV4(fpol, spol.u0, spol.v0, spol.u1, spol.v1, spol.u2, spol.v2, spol.u3, spol.v3);
        
        spol.clut = pol->clut;
        spol.tpage = pol->tpage;
        setRGB0(&spol, pol->r0, pol->g0, pol->b0);

        area = 0;
        area += xys[2] * (xys[7] - xys[1]);
        area += xys[6] * (xys[5] - xys[3]);
        area += xys[0] * (xys[3] - xys[5]);
        area += xys[4] * (xys[1] - xys[7]);

        // printf("AREA 0: %d\n", area);
        if (level && area > (ctx->render_mode ? AREA_SPLIT_INTERLACED : AREA_SPLIT)) {
            subdivideFT4(&spol, sub_verts, level-1, ctx, depth);
            continue;
        }

        fpol->clut = pol->clut;
        fpol->tpage = pol->tpage;
        setPolyFT4(fpol);
        setXY4(fpol, xys[0], xys[1], xys[2], xys[3], xys[4], xys[5], xys[6], xys[7]);
        setRGB0(fpol, pol->r0, pol->g0, pol->b0);
        push_primitive(ctx, depth, sizeof(POLY_FT4), (uint8_t*)fpol);
    }
}

void subdivideGT4(POLY_GT4* pol, SVECTOR* verts, uint8_t level, RenderContext* ctx, int depth) {                                               
    SVECTOR vs[5];
    uint8_t uxs[5];
    uint8_t xvs[5];
    uint8_t rs[5];
    uint8_t gs[5];
    uint8_t bs[5];

    int16_t xys[12];
    int32_t area;

    SVECTOR cont_verts[4];

    SUBDIVIDE(&vs[0], &verts[0], &verts[1]);
    SUBDIVIDE(&vs[1], &verts[1], &verts[3]);
    SUBDIVIDE(&vs[2], &verts[2], &verts[3]);
    SUBDIVIDE(&vs[3], &verts[0], &verts[2]);
    SUBDIVIDE(&vs[4], &vs[0], &vs[2]);

    gte_ldv3c(vs);
    gte_rtpt();
    gte_stsxy3c(xys);

    gte_ldv3c(vs+3);
    gte_rtpt();
    gte_stsxy3c(xys+6);

    uxs[0] = ((pol->u0)>>1) + ((pol->u1)>>1);
    uxs[1] = ((pol->u1)>>1) + ((pol->u3)>>1);
    uxs[2] = ((pol->u2)>>1) + ((pol->u3)>>1);
    uxs[3] = ((pol->u0)>>1) + ((pol->u2)>>1);
    uxs[4] = ((uxs[0])>>1) + ((uxs[2])>>1);
    xvs[0] = ((pol->v0)>>1) + ((pol->v1)>>1);
    xvs[1] = ((pol->v1)>>1) + ((pol->v3)>>1);
    xvs[2] = ((pol->v2)>>1) + ((pol->v3)>>1);
    xvs[3] = ((pol->v0)>>1) + ((pol->v2)>>1);
    xvs[4] = ((xvs[0])>>1) + ((xvs[2])>>1);
    
    rs[0] = ((pol->r0)>>1) + ((pol->r1)>>1);
    rs[1] = ((pol->r1)>>1) + ((pol->r3)>>1);
    rs[2] = ((pol->r2)>>1) + ((pol->r3)>>1);
    rs[3] = ((pol->r0)>>1) + ((pol->r2)>>1);
    rs[4] = ((rs[0])>>1) + ((rs[2])>>1);
    gs[0] = ((pol->g0)>>1) + ((pol->g1)>>1);
    gs[1] = ((pol->g1)>>1) + ((pol->g3)>>1);
    gs[2] = ((pol->g2)>>1) + ((pol->g3)>>1);
    gs[3] = ((pol->g0)>>1) + ((pol->g2)>>1);
    gs[4] = ((gs[0])>>1) + ((gs[2])>>1);
    bs[0] = ((pol->b0)>>1) + ((pol->b1)>>1);
    bs[1] = ((pol->b1)>>1) + ((pol->b3)>>1);
    bs[2] = ((pol->b2)>>1) + ((pol->b3)>>1);
    bs[3] = ((pol->b0)>>1) + ((pol->b2)>>1);
    bs[4] = ((bs[0])>>1) + ((bs[2])>>1);

    for (uint8_t i = 0; i < 4; ++i) {
        POLY_GT4* spol = (POLY_GT4*)new_primitive_reserved(ctx, sizeof(POLY_GT4));;
        switch (i) {
        case 0: {
            setXY4(spol, pol->x0, pol->y0, xys[0], xys[1], xys[6], xys[7], xys[8], xys[9]);
            cont_verts[0] = verts[0]; cont_verts[1] = vs[0];    cont_verts[2] = vs[3];    cont_verts[3] = vs[4];
            spol->u0 = pol->u0;    spol->u1 = uxs[0];     spol->u2 = uxs[3];     spol->u3 = uxs[4];
            spol->v0 = pol->v0;    spol->v1 = xvs[0];     spol->v2 = xvs[3];     spol->v3 = xvs[4];
            spol->r0 = pol->r0;    spol->r1 =  rs[0];     spol->r2 =  rs[3];     spol->r3 =  rs[4];
            spol->g0 = pol->g0;    spol->g1 =  gs[0];     spol->g2 =  gs[3];     spol->g3 =  gs[4];
            spol->b0 = pol->b0;    spol->b1 =  bs[0];     spol->b2 =  bs[3];     spol->b3 =  bs[4];
        }   break;
        case 1: {
            setXY4(spol, xys[0], xys[1], pol->x1, pol->y1, xys[8], xys[9], xys[2], xys[3]);
            cont_verts[0] = vs[0];    cont_verts[1] = verts[1]; cont_verts[2] = vs[4];    cont_verts[3] = vs[1];
            spol->u0 = uxs[0];     spol->u1 = pol->u1;    spol->u2 = uxs[4];     spol->u3 = uxs[1];
            spol->v0 = xvs[0];     spol->v1 = pol->v1;    spol->v2 = xvs[4];     spol->v3 = xvs[1];
            spol->r0 =  rs[0];     spol->r1 = pol->r1;    spol->r2 =  rs[4];     spol->r3 =  rs[1];
            spol->g0 =  gs[0];     spol->g1 = pol->g1;    spol->g2 =  gs[4];     spol->g3 =  gs[1];
            spol->b0 =  bs[0];     spol->b1 = pol->b1;    spol->b2 =  bs[4];     spol->b3 =  bs[1];
        }   break;
        case 2: {
            setXY4(spol, xys[6], xys[7], xys[8], xys[9], pol->x2, pol->y2, xys[4], xys[5]);
            cont_verts[0] = vs[3];    cont_verts[1] = vs[4];    cont_verts[2] = verts[2]; cont_verts[3] = vs[2];
            spol->u0 = uxs[3];     spol->u1 = uxs[4];     spol->u2 = pol->u2;    spol->u3 = uxs[2];
            spol->v0 = xvs[3];     spol->v1 = xvs[4];     spol->v2 = pol->v2;    spol->v3 = xvs[2];
            spol->r0 =  rs[3];     spol->r1 =  rs[4];     spol->r2 = pol->r2;    spol->r3 =  rs[2];
            spol->g0 =  gs[3];     spol->g1 =  gs[4];     spol->g2 = pol->g2;    spol->g3 =  gs[2];
            spol->b0 =  bs[3];     spol->b1 =  bs[4];     spol->b2 = pol->b2;    spol->b3 =  bs[2];
        }   break;
        case 3: {
            setXY4(spol, xys[8], xys[9], xys[2], xys[3], xys[4], xys[5], pol->x3, pol->y3);
            cont_verts[0] = vs[4];    cont_verts[1] = vs[1];    cont_verts[2] = vs[2];    cont_verts[3] = verts[3];
            spol->u0 = uxs[4];     spol->u1 = uxs[1];     spol->u2 = uxs[2];     spol->u3 = pol->u3;
            spol->v0 = xvs[4];     spol->v1 = xvs[1];     spol->v2 = xvs[2];     spol->v3 = pol->v3;
            spol->r0 =  rs[4];     spol->r1 =  rs[1];     spol->r2 =  rs[2];     spol->r3 = pol->r3;
            spol->g0 =  gs[4];     spol->g1 =  gs[1];     spol->g2 =  gs[2];     spol->g3 = pol->g3;
            spol->b0 =  bs[4];     spol->b1 =  bs[1];     spol->b2 =  bs[2];     spol->b3 = pol->b3;
        }   break;
        }

        // add_debug_count(2);
        if( quad_clip( ctx->screen_clip,
            (DVECTOR*)&spol->x0, (DVECTOR*)&spol->x1, 
            (DVECTOR*)&spol->x2, (DVECTOR*)&spol->x3) ) {
            continue;
        }
        
        spol->clut = pol->clut;
        spol->tpage = pol->tpage;

        area = 0;
        area += spol->x1 * (spol->y3 - spol->y0);
        area += spol->x3 * (spol->y2 - spol->y1);
        area += spol->x0 * (spol->y1 - spol->y2);
        area += spol->x2 * (spol->y0 - spol->y3);

        // printf("AREA 0: %d\n", area);
        
        setPolyGT4(spol);
        spol->code = pol->code;

        if (level && area > (ctx->render_mode ? AREA_SPLIT_INTERLACED : AREA_SPLIT)) {
            subdivideGT4(spol, cont_verts, level-1, ctx, depth);
            continue;
        }

        // setPolyGT4(spol);
        // setSemiTrans(spol, 1);
        // spol->code = pol->code;
        push_primitive_pre(ctx, depth, (uint8_t*)spol);
    }
}

void subdivideGT42D(POLY_GT4* pol, int32_t zs[4], uint8_t level, RenderContext* ctx, int depth) {                                               
    uint8_t uxs[5];
    uint8_t xvs[5];
    uint8_t rs[5];
    uint8_t gs[5];
    uint8_t bs[5];

    int16_t xys[10];
    int32_t lzs[5];
    int32_t area;

    int32_t cont_zs[4];

    xys[0] = (pol->x0 + pol->x1)>>1;
    xys[1] = (pol->y0 + pol->y1)>>1;
    xys[2] = (pol->x1 + pol->x3)>>1;
    xys[3] = (pol->y1 + pol->y3)>>1;
    xys[4] = (pol->x2 + pol->x3)>>1;
    xys[5] = (pol->y2 + pol->y3)>>1;
    xys[6] = (pol->x0 + pol->x2)>>1;
    xys[7] = (pol->y0 + pol->y2)>>1;
    xys[8] = (xys[0] + xys[4])>>1;
    xys[9] = (xys[1] + xys[5])>>1;

    lzs[0] = (zs[0] + zs[1])>>1;
    lzs[1] = (zs[1] + zs[3])>>1;
    lzs[2] = (zs[2] + zs[3])>>1;
    lzs[3] = (zs[0] + zs[2])>>1;
    lzs[4] = (lzs[0] + lzs[2])>>1;

    uxs[0] = ((pol->u0)>>1) + ((pol->u1)>>1);
    uxs[1] = ((pol->u1)>>1) + ((pol->u3)>>1);
    uxs[2] = ((pol->u2)>>1) + ((pol->u3)>>1);
    uxs[3] = ((pol->u0)>>1) + ((pol->u2)>>1);
    uxs[4] = ((uxs[0])>>1) + ((uxs[2])>>1);
    xvs[0] = ((pol->v0)>>1) + ((pol->v1)>>1);
    xvs[1] = ((pol->v1)>>1) + ((pol->v3)>>1);
    xvs[2] = ((pol->v2)>>1) + ((pol->v3)>>1);
    xvs[3] = ((pol->v0)>>1) + ((pol->v2)>>1);
    xvs[4] = ((xvs[0])>>1) + ((xvs[2])>>1);
    
    rs[0] = ((pol->r0)>>1) + ((pol->r1)>>1);
    rs[1] = ((pol->r1)>>1) + ((pol->r3)>>1);
    rs[2] = ((pol->r2)>>1) + ((pol->r3)>>1);
    rs[3] = ((pol->r0)>>1) + ((pol->r2)>>1);
    rs[4] = ((rs[0])>>1) + ((rs[2])>>1);
    gs[0] = ((pol->g0)>>1) + ((pol->g1)>>1);
    gs[1] = ((pol->g1)>>1) + ((pol->g3)>>1);
    gs[2] = ((pol->g2)>>1) + ((pol->g3)>>1);
    gs[3] = ((pol->g0)>>1) + ((pol->g2)>>1);
    gs[4] = ((gs[0])>>1) + ((gs[2])>>1);
    bs[0] = ((pol->b0)>>1) + ((pol->b1)>>1);
    bs[1] = ((pol->b1)>>1) + ((pol->b3)>>1);
    bs[2] = ((pol->b2)>>1) + ((pol->b3)>>1);
    bs[3] = ((pol->b0)>>1) + ((pol->b2)>>1);
    bs[4] = ((bs[0])>>1) + ((bs[2])>>1);

    for (uint8_t i = 0; i < 4; ++i) {
        POLY_GT4* spol = (POLY_GT4*)new_primitive_reserved(ctx, sizeof(POLY_GT4));;
        switch (i) {
        case 0: {
            setXY4(spol, pol->x0, pol->y0, xys[0], xys[1], xys[6], xys[7], xys[8], xys[9]);
            cont_zs[0] = zs[0]; cont_zs[1] = lzs[0];    cont_zs[2] = lzs[3];    cont_zs[3] = lzs[4];
            spol->u0 = pol->u0;    spol->u1 = uxs[0];     spol->u2 = uxs[3];     spol->u3 = uxs[4];
            spol->v0 = pol->v0;    spol->v1 = xvs[0];     spol->v2 = xvs[3];     spol->v3 = xvs[4];
            spol->r0 = pol->r0;    spol->r1 =  rs[0];     spol->r2 =  rs[3];     spol->r3 =  rs[4];
            spol->g0 = pol->g0;    spol->g1 =  gs[0];     spol->g2 =  gs[3];     spol->g3 =  gs[4];
            spol->b0 = pol->b0;    spol->b1 =  bs[0];     spol->b2 =  bs[3];     spol->b3 =  bs[4];
        }   break;
        case 1: {
            setXY4(spol, xys[0], xys[1], pol->x1, pol->y1, xys[8], xys[9], xys[2], xys[3]);
            cont_zs[0] = lzs[0];    cont_zs[1] = zs[1]; cont_zs[2] = lzs[4];    cont_zs[3] = lzs[1];
            spol->u0 = uxs[0];     spol->u1 = pol->u1;    spol->u2 = uxs[4];     spol->u3 = uxs[1];
            spol->v0 = xvs[0];     spol->v1 = pol->v1;    spol->v2 = xvs[4];     spol->v3 = xvs[1];
            spol->r0 =  rs[0];     spol->r1 = pol->r1;    spol->r2 =  rs[4];     spol->r3 =  rs[1];
            spol->g0 =  gs[0];     spol->g1 = pol->g1;    spol->g2 =  gs[4];     spol->g3 =  gs[1];
            spol->b0 =  bs[0];     spol->b1 = pol->b1;    spol->b2 =  bs[4];     spol->b3 =  bs[1];
        }   break;
        case 2: {
            setXY4(spol, xys[6], xys[7], xys[8], xys[9], pol->x2, pol->y2, xys[4], xys[5]);
            cont_zs[0] = lzs[3];    cont_zs[1] = lzs[4];    cont_zs[2] = zs[2]; cont_zs[3] = lzs[2];
            spol->u0 = uxs[3];     spol->u1 = uxs[4];     spol->u2 = pol->u2;    spol->u3 = uxs[2];
            spol->v0 = xvs[3];     spol->v1 = xvs[4];     spol->v2 = pol->v2;    spol->v3 = xvs[2];
            spol->r0 =  rs[3];     spol->r1 =  rs[4];     spol->r2 = pol->r2;    spol->r3 =  rs[2];
            spol->g0 =  gs[3];     spol->g1 =  gs[4];     spol->g2 = pol->g2;    spol->g3 =  gs[2];
            spol->b0 =  bs[3];     spol->b1 =  bs[4];     spol->b2 = pol->b2;    spol->b3 =  bs[2];
        }   break;
        case 3: {
            setXY4(spol, xys[8], xys[9], xys[2], xys[3], xys[4], xys[5], pol->x3, pol->y3);
            cont_zs[0] = lzs[4];    cont_zs[1] = lzs[1];    cont_zs[2] = lzs[2];    cont_zs[3] = zs[3];
            spol->u0 = uxs[4];     spol->u1 = uxs[1];     spol->u2 = uxs[2];     spol->u3 = pol->u3;
            spol->v0 = xvs[4];     spol->v1 = xvs[1];     spol->v2 = xvs[2];     spol->v3 = pol->v3;
            spol->r0 =  rs[4];     spol->r1 =  rs[1];     spol->r2 =  rs[2];     spol->r3 = pol->r3;
            spol->g0 =  gs[4];     spol->g1 =  gs[1];     spol->g2 =  gs[2];     spol->g3 = pol->g3;
            spol->b0 =  bs[4];     spol->b1 =  bs[1];     spol->b2 =  bs[2];     spol->b3 = pol->b3;
        }   break;
        }

        // add_debug_count(2);
        if( quad_clip( ctx->screen_clip,
            (DVECTOR*)&spol->x0, (DVECTOR*)&spol->x1, 
            (DVECTOR*)&spol->x2, (DVECTOR*)&spol->x3) ) {
            continue;
        }
        
        spol->clut = pol->clut;
        spol->tpage = pol->tpage;

        area = 0;
        area += spol->x1 * (spol->y3 - spol->y0);
        area += spol->x3 * (spol->y2 - spol->y1);
        area += spol->x0 * (spol->y1 - spol->y2);
        area += spol->x2 * (spol->y0 - spol->y3);

        // printf("AREA %d: %d, zdiff: %d\n", level, area, maz-miz);
        
        setPolyGT4(spol);
        spol->code = pol->code;

        if (level && area > (ctx->render_mode ? AREA_SPLIT_INTERLACED : AREA_SPLIT)) {
            subdivideGT42D(spol, cont_zs, level-1, ctx, depth);
            continue;
        }

        // setPolyGT4(spol);
        // setSemiTrans(spol, 1);
        // spol->code = pol->code;
        push_primitive_pre(ctx, depth, (uint8_t*)spol);
    }
}