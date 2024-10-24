#include "enemy.h"

#include <psxgpu.h>

#define MIN(x, y) ((y) ^ (((x) ^ (y)) & -((x) < (y))))
#define MAX(x, y) ((x) ^ (((x) ^ (y)) & -((x) < (y))))
#define BETWEEN(min, max, x) ((min) < (x) && (x) <= (max))

extern AAMF enemy_aamf[5];
extern SFX metal_sfx;

static Enemy enemies[5];
static uint8_t curr_enemies = 0;

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
    0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0,
    0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0,
    0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0,
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
extern uint8_t player_should_stun;
extern uint32_t score;
extern uint8_t curr_level;

uint16_t choose_next(uint16_t last, uint16_t id) {
    int x =  id % 23;
    int y =  id / 23;
    int possib = 0;
    uint16_t c[4];
    if (id-1 != last && x - 1 >= 0 && map[id-1]) {
        c[possib] = id-1; ++possib;
    }
    if (id-23 != last && y - 1 >= 0 && map[id-23]) {
        c[possib] = id-23; ++possib;
    }
    if (id+1 != last && x + 1 < 23 && map[id+1]) {
        c[possib] = id+1; ++possib;
    }
    if (id+23 != last && y + 1 < 29 && map[id+23]) {
        c[possib] = id+23; ++possib;
    }
    return c[get_random()%possib];
}

void initEnemies(uint8_t level) {
    curr_enemies = 5;
    if (level < 2) curr_enemies = 3;
    else if (level < 5) curr_enemies = 4;
    else if (level < 10) curr_enemies = 3;
    else if (level < 12) curr_enemies = 4;
    for (int i = 0; i < curr_enemies; ++i) {
        Enemy* enemy = &enemies[i];
        enemy->root = enemy_aamf[i].root;
        enemy->model = &enemy_aamf[i];
        enemy->curr_block = 17*23+11;
        enemy->last_position.vx = (0)<<12;
        enemy->last_position.vy = 0;
        enemy->last_position.vz = (1)<<12;
        enemy->next_block = 18*23+11;
        enemy->target.vx = (0)<<12;
        enemy->target.vy = 0;
        enemy->target.vz = (2)<<12;
        enemy->stunned = 0;
        enemy->chasing = 0;
        enemy->alive = 1;
        enemy->visible = 0;
        enemy->turning = 0;
        enemy->movement_progress = 0;
        enemy->mv_speed = level+1 < 10 ? level+1 : 10;
        activateAnimation(&enemy_aamf[i], "Move----");
    }
}


void moveEnemies() {
    for (int i = 0; i < curr_enemies; ++i) {
        Enemy* enemy = &enemies[i];
        if (!(enemy->stunned)) {
            if (enemy->turning) {
                enemy->movement_progress += enemy->mv_speed;
                if (enemy->movement_progress >= 64) {
                    activateAnimation(enemy->model, "Move----");
                    enemy->movement_progress = 0;
                    switch (enemy->turning) {
                        case 1: {
                            set_rot(enemy->root, 0, 1024, 0);
                            } break;
                        case 23: {
                            set_rot(enemy->root, 0, 0, 0);
                            } break;
                        case -1: {
                            set_rot(enemy->root, 0, 3072, 0);
                            } break;
                        case -23: {
                            set_rot(enemy->root, 0, 2048, 0);
                            } break;
                    }
                    enemy->turning = 0;
                }
            } else {
                enemy->position.vx = enemy->last_position.vx + 
                                    (((enemy->target.vx - enemy->last_position.vx) * enemy->movement_progress)>>8);
                enemy->position.vz = enemy->last_position.vz + 
                                    (((enemy->target.vz - enemy->last_position.vz) * enemy->movement_progress)>>8);
                enemy->movement_progress += enemy->mv_speed;
                // if reached end of current move
                if (enemy->movement_progress < enemy->mv_speed) {
                    uint16_t temp = enemy->curr_block;
                    int8_t curr_dir = enemy->next_block - temp;
                    enemy->curr_block = enemy->next_block;
                    enemy->next_block = choose_next(temp, enemy->curr_block);
                    enemy->last_position = enemy->target;
                    enemy->target.vx = ((enemy->next_block % 23) - 11)<<12;
                    enemy->target.vz = ((enemy->next_block / 23) - 16)<<12;
                    if (curr_dir != enemy->next_block - enemy->curr_block) {
                        enemy->turning = enemy->next_block - enemy->curr_block;
                        enemy->movement_progress = 0;
                        if (((curr_dir == -23 || curr_dir == 1) && (enemy->turning < 0)) 
                        || ((curr_dir == -1 || curr_dir == 23) && (enemy->turning > 0))) {
                            activateAnimation(enemy->model, "TurnLeft");
                        } else {
                            activateAnimation(enemy->model, "TurnRigh");
                        }
                    }
                }
            }
            // enemy->movement_progress += 1;
        } else {
            enemy->stunned -= 1;
        }
    }
}

uint8_t checkPlayerHit(Player* player) {
    int32_t pposx = player->pos.vx<<2;
    int32_t pposz = player->pos.vz<<2;
    uint8_t stun_amount = 0;
    for (int i = 0; i < curr_enemies; ++i) {
        Enemy* enemy = &enemies[i];
        int32_t x = pposx - enemy->position.vx;
        int32_t z = pposz - enemy->position.vz;
        int32_t xp = (x + (x >> 31)) ^ (x >> 31);
        int32_t zp = (z + (z >> 31)) ^ (z >> 31);
        // printf("enem %d %d %d %d %d\n", enemy->movement_progress, BETWEEN(enemy->movement_progress - enemy->mv_speed, enemy->movement_progress, 0)
        //     , BETWEEN(enemy->movement_progress - enemy->mv_speed, enemy->movement_progress, 64)
        //     , BETWEEN(enemy->movement_progress - enemy->mv_speed, enemy->movement_progress, 128)
        //     , BETWEEN(enemy->movement_progress - enemy->mv_speed, enemy->movement_progress, 192));
        if (!(enemy->stunned) && xp+zp < 1<<15 && (BETWEEN(enemy->movement_progress - enemy->mv_speed, enemy->movement_progress, 0)
            || BETWEEN(enemy->movement_progress - enemy->mv_speed, enemy->movement_progress, 64)
            || BETWEEN(enemy->movement_progress - enemy->mv_speed, enemy->movement_progress, 128)
            || BETWEEN(enemy->movement_progress - enemy->mv_speed, enemy->movement_progress, 192))) {
            // printf("XX %d %d %d\n", i, ((player->cam.rot.vy>>12) - (patan(z, x)>>4)) & 4095, sin_t(((player->cam.rot.vy>>12) - (patan(z, x)>>4)) & 4095));
            int l = 8192;
            int r = 8192;
            int dist = (xp+zp)>>2;
            int val = sin_t(((player->cam.rot.vy>>12) - (patan(z, x)>>4)) & 4095) << 1;
            l -= val; l = MIN(MAX(0, l), 8192 - dist)<<1;
            r += val; r = MIN(MAX(0, r), 8192 - dist)<<1;
            l = (l*l)>>14;
            r = (r*r)>>14;
            // printf("dist %d %04x %04x\n", dist, l, r);
            printf("step\n");
            enemy->audio.volumeL = MIN(l, 0x3fff);
            enemy->audio.volumeR = MIN(r, 0x3fff);
            enemy->audio.channel = play_sample_stereo(metal_sfx.addr, metal_sfx.sample_rate, enemy->audio.volumeL, enemy->audio.volumeR);
        }
        if (xp+zp < 2048) return 1;
        if (xp+zp < ((1<<14) + (1<<13))) {
            enemy->visible = 1;
        } else {
            enemy->visible = 0;
        }
        if (xp+zp < ((1<<13) + (1<<12)) && player_should_stun) {
            enemy->stunned = 120;
            ++stun_amount;
        }
    }
    score += (stun_amount * stun_amount) * 1000 * (curr_level+1);
    return 0;
}


void drawEnemies(MATRIX* INVCAM) {
    for (int i = 0; i < curr_enemies; ++i) {
        Enemy* enemy = &enemies[i];
        if (!enemy->visible) continue;
        setAnimationKeyframe(enemy->model, enemy->movement_progress);
        set_pos(enemy->root, enemy->position.vx>>2, enemy->position.vy>>2, enemy->position.vz>>2);
        draw_scene(enemy->root, INVCAM);
    }
}
