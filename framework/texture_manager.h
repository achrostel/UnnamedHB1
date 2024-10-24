#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <psxgpu.h>

/*
Texture Block is 4x2 Texture Pages large
indexed as
0 1 2 3 4 5 6 7
8 9 a b c d e f
...
from binary digit (reversed, because i already wrote it down)
0: empty space
1: filled with texture

4b64x64 handled outside (not implemented yet)
ehhhhh, I want the above
*/

typedef struct _AIF_HEADER {
    char name[8];
    uint8_t mode; // 0: 4-bit | 1: 8-bit | 2: 16-bit
    uint8_t transparency; // transparency | (transparency_mode << 1)
    uint8_t size; // 0: 64 | 1: 128 | 2: 256
    uint8_t clut_len;
    uint32_t img_data_len; // in bytes
    uint32_t clut_data_len; // in bytes
} AIF_HEADER;

typedef struct _AIF {
    AIF_HEADER header;
    uint32_t* clut_data;
    uint32_t* img_data;
} AIF;

typedef struct _Texture {
    uint32_t space;
    uint16_t tpage;
    uint16_t clut;
    uint16_t ox, oy;
    uint8_t is_uploaded; // xxxaxxbc (a) be uploaded | (b) custom location | (c) in block
    uint8_t block_and_clut_id; // aaabbbbb a block b clut
    uint8_t b1, b2;
    AIF aif;
} Texture;

typedef struct _Texture_Manager {
    uint32_t blocks[4];
    uint32_t cluts;
    Texture* texture_list[256]; // meh, rather fixed size than malloc
    uint8_t texture_amount;
} Texture_Manager;

void initImages();
void aifInitData(Texture* tex, unsigned char* data);

Texture* findTex(char* name);

void checkTextures();

void uploadBlockTexture(Texture* texture, uint8_t block_id, uint8_t clut_id, uint32_t space);
void uploadRawTexture(Texture* texture, uint16_t x, uint16_t y);

// untested, old, pre-changes, don't use
void unloadTexture(Texture* texture);

#endif // TEXTURE_MANAGER_H