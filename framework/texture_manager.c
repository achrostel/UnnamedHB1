#include "texture_manager.h"

#include <string.h>

#define block_16b256 (0b11111111111111111111111111111111ul)
#define block_8b256  (0b11110000111100001111000011110000ul) // >>4 x
#define block_4b256  (0b11000000110000001100000011000000ul) // >>2 x * 3
#define block_16b128 (0b11110000111100000000000000000000ul) // >>4 x + 12 y + 4 x
#define block_8b128  (0b11000000110000000000000000000000ul) // >>2 x * 3 + 10 y + 2 x * 3
#define block_4b128  (0b10000000100000000000000000000000ul) // >>1 x * 7 + 9 y + 1 x * 7
#define block_16b64  (0b11000000000000000000000000000000ul) // >>2 x * 15
#define block_8b64   (0b10000000000000000000000000000000ul) // >>1 x * 31

extern unsigned char transp_aif_data[];
extern unsigned char ceilBase_aif_data[];
extern unsigned char ceilLight_aif_data[];
extern unsigned char floor_aif_data[];
extern unsigned char wall_aif_data[];
extern unsigned char cam_aif_data[];
extern unsigned char buttons_idle_aif_data[];
extern unsigned char buttons_active_aif_data[];

Texture transp_tex;
Texture ceilBase_tex;
Texture ceilLight_tex;
Texture floor_tex;
Texture wall_tex;
Texture cam_tex;
Texture buttons_idle_tex;
Texture buttons_active_tex;

Texture_Manager texture_manager;

uint32_t get_tex_repr(Texture* tex, uint8_t* val) {
    uint8_t mode = tex->aif.header.mode;
    uint8_t size = tex->aif.header.size;
    switch (mode)
    {
    case 0:
        switch (size)
        {
        case 0: *val = 255; return 0;
        case 1: *val = 5; return block_4b128;
        case 2: *val = 2; return block_4b256;
        }
    case 1:
        switch (size)
        {
        case 0: *val = 7; return block_8b64;
        case 1: *val = 4; return block_8b128;
        case 2: *val = 1; return block_8b256;
        }
    case 2:
        switch (size)
        {
        case 0: *val = 6; return block_16b64;
        case 1: *val = 3; return block_16b128;
        case 2: *val = 0; return block_16b256;
        }
    }
    return 0;
}

void space_transform(uint32_t* space, uint8_t val, int8_t* step) {
    switch (val)
    {
    case 0: {
        *step = -1;
    } break;
    case 1: {
        *step = -1;
        *space = (*space)>>4;
    } break;
    case 2: {
        switch (*step)
        {
        case 0:
        case 1: {
            *step += 1;
            *space = (*space)>>2;
        } break;
        case 2: {
            *step = -1;
            *space = (*space)>>2;
        } break;
        }
    } break;
    case 3: {
        switch (*step)
        {
        case 0: {
            *step += 1;
            *space = (*space)>>4;
        } break;
        case 1: {
            *step += 1;
            *space = (*space)>>12;
        } break;
        case 2: {
            *step = -1;
            *space = (*space)>>4;
        } break;
        }
    } break;
    case 4: {
        switch (*step)
        {
        case 0:
        case 1:
        case 2: {
            *step += 1;
            *space = (*space)>>2;
        } break;
        case 3: {
            *step += 1;
            *space = (*space)>>10;
        } break;
        case 4:
        case 5: {
            *step += 1;
            *space = (*space)>>2;
        } break;
        case 6: {
            *step = -1;
            *space = (*space)>>2;
        } break;
        }
    } break;
    case 5: {
        switch (*step)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6: {
            *step += 1;
            *space = (*space)>>1;
        } break;
        case 7: {
            *step += 1;
            *space = (*space)>>9;
        } break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13: {
            *step += 1;
            *space = (*space)>>1;
        } break;
        case 14: {
            *step = -1;
            *space = (*space)>>1;
        } break;
        }
    } break;
    case 6: {
        switch (*step)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13: {
            *step += 1;
            *space = (*space)>>2;
        } break;
        case 14: {
            *step = -1;
            *space = (*space)>>2;
        } break;
        }
    } break;
    case 7: {
        switch (*step)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29: {
            *step += 1;
            *space = (*space)>>1;
        } break;
        case 30: {
            *step = -1;
            *space = (*space)>>1;
        } break;
        }
    } break;
    default: *step = -1;
    }

}

uint8_t trailing_0(uint32_t v) {
    uint32_t r; // result of log2(v) will go here
    uint32_t shift;

    r =     (v > 0xFFFF) << 4; v >>= r;
    shift = (v > 0xFF  ) << 3; v >>= shift; r |= shift;
    shift = (v > 0xF   ) << 2; v >>= shift; r |= shift;
    shift = (v > 0x3   ) << 1; v >>= shift; r |= shift;
                                            r |= (v >> 1);
    return 31-r;
}

void initImages() {
    texture_manager.blocks[0] = 0;
    texture_manager.blocks[1] = 0;
    texture_manager.blocks[2] = 0;
    texture_manager.blocks[3] = 0;
    texture_manager.cluts = 0;
    texture_manager.texture_amount = 0;

    aifInitData(&transp_tex, transp_aif_data);
    aifInitData(&floor_tex, floor_aif_data);
    aifInitData(&ceilBase_tex, ceilBase_aif_data);
    aifInitData(&ceilLight_tex, ceilLight_aif_data);
    aifInitData(&wall_tex, wall_aif_data);
    aifInitData(&cam_tex, cam_aif_data);

    aifInitData(&buttons_idle_tex, buttons_idle_aif_data);
    aifInitData(&buttons_active_tex, buttons_active_aif_data);

    /*
        not handling 64x4bit and assuming 320x480 for 2 draw buffers
        -> ignore smart(TM) implementation, just upload at position
    */

    uploadRawTexture(&transp_tex, 640, 0);
    uploadRawTexture(&floor_tex, 640+64, 0);
    uploadRawTexture(&ceilBase_tex, 640+64+16, 0);
    uploadRawTexture(&ceilLight_tex, 640+64+16+16, 0);
    uploadRawTexture(&wall_tex, 640+64+32+16, 0);
    uploadRawTexture(&cam_tex, 640+64+64, 0);
    uploadRawTexture(&buttons_idle_tex, 640, 256);
    uploadRawTexture(&buttons_active_tex, 640+128, 256);

    transp_tex.is_uploaded = (0x01);
    floor_tex.is_uploaded = (0x01);
    ceilBase_tex.is_uploaded = (0x01);
    ceilLight_tex.is_uploaded = (0x01);
    wall_tex.is_uploaded = (0x01);
    cam_tex.is_uploaded = (0x01);
    buttons_idle_tex.is_uploaded = (0x01);
    buttons_active_tex.is_uploaded = (0x01);
}

void aifInitData(Texture* tex, unsigned char* data) {
    texture_manager.texture_list[texture_manager.texture_amount] = tex;
    texture_manager.texture_amount += 1;

    uint32_t* d32 = (uint32_t*) data;
    {
    tex->aif.header.name[0] = *(data);
    tex->aif.header.name[1] = *(data+1);
    tex->aif.header.name[2] = *(data+2);
    tex->aif.header.name[3] = *(data+3);
    tex->aif.header.name[4] = *(data+4);
    tex->aif.header.name[5] = *(data+5);
    tex->aif.header.name[6] = *(data+6);
    tex->aif.header.name[7] = *(data+7);
    }
    
    tex->aif.header.mode = *(data+8);
    tex->aif.header.transparency = *(data+9);
    tex->aif.header.size = *(data+10);
    tex->aif.header.clut_len = *(data+11);
    tex->aif.header.img_data_len = *(d32+3);
    tex->aif.header.clut_data_len = *(d32+4);

    tex->aif.clut_data = (uint32_t*) (data+20);
    tex->aif.img_data = (uint32_t*) (data+20+tex->aif.header.clut_data_len);
}

Texture* findTex(char* name) {
    for (int i = 0; i < texture_manager.texture_amount; ++i) {
        Texture* tex = texture_manager.texture_list[i];
        char* tname = tex->aif.header.name;
        // printf("%.8s | %.8s\n", name, tname);
        if (memcmp(name, tname, 8) == 0) {
            return tex;
        }
    }
}

void checkTextures() {
    for (int i = 0; i < texture_manager.texture_amount; ++i) {
        if (texture_manager.texture_list[i]->is_uploaded == 0x10) {
            Texture* tex = texture_manager.texture_list[i];
            uint8_t val;
            for (int block_id = 0; block_id < 4; ++block_id) {
                uint32_t space = get_tex_repr(tex, &val);
                uint32_t* b = &texture_manager.blocks[block_id];
                if ((space ^ *b) == (space | *b)) {
                    uploadBlockTexture(tex, block_id, 0, space);
                    block_id = 4; // avoiding goto to escape
                    break;
                }
                int8_t step = 0;
                while (step >= 0) {
                    space_transform(&space, val, &step);
                    if ((space ^ *b) == (space | *b)) {
                        uploadBlockTexture(tex, block_id, 0, space);
                        block_id = 4; // avoiding goto to escape
                        step = -1;
                    }
                }
            }
        }
    }
}

void uploadBlockTexture(Texture* texture, uint8_t block_id, uint8_t clut_id, uint32_t space) {
    texture->space = space;
    texture->block_and_clut_id = (block_id<<5) | clut_id;
    texture->is_uploaded = 1;
    // uint16_t tx = 320 + (block_id%2)*256 + (trailing_0(space)%8)*32;
    // uint16_t ty = (block_id/2)*256 + (trailing_0(space)/8)*64;
    uint16_t tx = 640 + (block_id/2)*256 + (trailing_0(space)%8)*32;
    uint16_t ty = (block_id%2)*256 + (trailing_0(space)/8)*64;
    uint16_t cx = 0;
    uint16_t cy = 480+texture_manager.cluts;
    texture->ox = (((trailing_0(space)%8)*32) % 64)*4;
    texture->oy = (((trailing_0(space)/8)*64) % 256);

    // printf("tex %d %d %d %d %d\n", tx, ty, texture->ox, texture->oy, trailing_0(space));
    texture->tpage = getTPage(texture->aif.header.mode, texture->aif.header.transparency>>1, tx, ty);
    texture->clut = getClut(cx, cy);

    // printf("a %lu %d\n", texture_manager.blocks[block_id], block_id);

    texture_manager.blocks[block_id] ^= space;
    texture_manager.cluts += 1;

    // printf("b %lu\n", texture_manager.blocks[block_id]);
    RECT texRect = {tx, ty, (64<<texture->aif.header.size)>>(2-texture->aif.header.mode), 64<<texture->aif.header.size};
    RECT clutRect = {cx, cy, texture->aif.header.clut_len+1, 1};

    LoadImage(&texRect, texture->aif.img_data);
    LoadImage(&clutRect, texture->aif.clut_data);
}

void uploadRawTexture(Texture* texture, uint16_t x, uint16_t y) {
    texture->space = 0;
    texture->block_and_clut_id = 0;
    texture->is_uploaded = 2;
    uint16_t cx = 0;
    uint16_t cy = 480+texture_manager.cluts;
    texture->tpage = getTPage(texture->aif.header.mode, texture->aif.header.transparency>>1, x, y);
    texture->clut = getClut(cx, cy);
    texture_manager.cluts += 1;

    texture->ox = (x&0x3f)<<2;
    texture->oy = y&0xff;

    // printf("tex %d %d %d %d\n", x, y, texture->ox, texture->oy);

    RECT texRect = {x, y, (64<<texture->aif.header.size)>>(2-texture->aif.header.mode), 64<<texture->aif.header.size};
    RECT clutRect = {cx, cy, texture->aif.header.clut_len+1, 1};

    LoadImage(&texRect, texture->aif.img_data);
    LoadImage(&clutRect, texture->aif.clut_data);
}

void unloadTexture(Texture* texture) {
    texture->is_uploaded = 0;
    texture_manager.blocks[texture->block_and_clut_id>>5] ^= texture->space;
    texture_manager.cluts ^= (1<<(texture->block_and_clut_id%32)); 
}

#undef block_16b256
#undef block_8b256 
#undef block_4b256 
#undef block_16b128
#undef block_8b128 
#undef block_4b128 
#undef block_16b64 
#undef block_8b64  