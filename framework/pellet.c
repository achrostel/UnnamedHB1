#include "pellet.h"

#define MIN(x, y) ((y) ^ (((x) ^ (y)) & -((x) < (y))))
#define MAX(x, y) ((x) ^ (((x) ^ (y)) & -((x) < (y))))

extern AMF pellet_amf;
extern SFX collect_sfx;
extern SFX ding_sfx;

static Pellet pellets[30];
static uint8_t collected = 0;
uint8_t max_pelletes;

// x * y map size (23*29)
static uint8_t map[23*29] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1,
    1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1,
    0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
    0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0,
    0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0,
    0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0,
    0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0,
    0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0,
    0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0,
    0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0,
    0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0,
    1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1,
    1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1,
    1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1,
    0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0,
    0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0,
    0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
};

// hacked in, just want to finish
extern uint32_t score;
extern uint8_t curr_level;

void initPellets(uint8_t level) {
    collected = 0;
    max_pelletes = 30;
    if (level < 3) max_pelletes = 15;
    else if (level < 5) max_pelletes = 20;
    else if (level < 8) max_pelletes = 25;
    for (int i = 0; i < max_pelletes; ++i) {
        int rand = get_random() % (23*29);
        while (map[rand] != 1) {
            add_entropy(1);
            rand = get_random() % (23*29);
        }
        Pellet* pellet = &pellets[i];
        pellet->model = new_node(&pellet_amf, 0);
        pellet->position.vx = ((rand%23)-11)<<12;
        pellet->position.vy = 0;
        pellet->position.vz = ((rand/23)-16)<<12;
        pellet->rotation.vx = 0;
        pellet->rotation.vy = 0;
        pellet->rotation.vz = 0;
        pellet->alive = 1;
        pellet->visible = 0;
        set_pos(pellet->model, pellet->position.vx>>2, pellet->position.vy>>2, pellet->position.vz>>2);
        pellet->rotation.vz = 256;
    }
}

int checkPlayerCollect(Player* player) {
    int32_t pposx = player->pos.vx<<2;
    int32_t pposz = player->pos.vz<<2;
    int32_t minx;
    int32_t minz;
    int32_t mind = INT32_MAX;
    for (int i = 0; i < max_pelletes; ++i) {
        Pellet* pellet = &pellets[i];
        if (!pellet->alive) continue;
        int32_t x = pposx - pellet->position.vx;
        int32_t z = pposz - pellet->position.vz;
        int32_t xp = (x + (x >> 31)) ^ (x >> 31);
        int32_t zp = (z + (z >> 31)) ^ (z >> 31);
        if (xp+zp < 1536) {
            collected += 1;
            score += 100 * (curr_level+1);
            if (collected % 5 == 0 && player->stun_ammo < 8) {
                player->stun_ammo += 1;
            }
            pellet->alive = 0;
            play_sample(collect_sfx.addr, collect_sfx.sample_rate, 0x1fff);
        }
        if (xp+zp < ((1<<14) + (1<<13))) {
            pellet->visible = 1;
        } else {
            pellet->visible = 0;
        }
        if (xp+zp < mind) {
            mind = xp+zp;
            minx = x;
            minz = z;
        }
    }

    if (mind < 1<<15 && !(VSync(-1) % 240)) {
        int l = 8192;
        int r = 8192;
        int dist = (mind)>>2;
        int val = sin_t(((player->cam.rot.vy>>12) - (patan(minz, minx)>>4)) & 4095) << 1;
        l -= val; l = MIN(MAX(0, l), 8192 - dist)<<1;
        r += val; r = MIN(MAX(0, r), 8192 - dist)<<1;
        l = (l*l)>>14;
        r = (r*r)>>14;
        printf("ding\n");
        l = MIN(l, 0x3fff)>>2;
        r = MIN(r, 0x3fff)>>2;
        play_sample_stereo(ding_sfx.addr, ding_sfx.sample_rate, l, r);
    }

    if (collected < max_pelletes) return collected;
    return -1;
}


void drawPellets(MATRIX* INVCAM) {
    for (int i = 0; i < max_pelletes; ++i) {
        Pellet* pellet = &pellets[i];
        if (!pellet->alive || !pellet->visible) continue;
        set_rot_vec(pellet->model, &pellet->rotation);
        pellet->rotation.vy += 24;
        draw_node(pellet->model, INVCAM);
    }
}
