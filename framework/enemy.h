#ifndef ENEMY_H
#define ENEMY_H

#include <psxgte.h>

#include "scene.h"
#include "anim_model.h"
#include "player.h"
#include "random.h"
#include "soundfx.h"

typedef struct {
    uint16_t volumeL;
    uint16_t volumeR;
    uint32_t channel;
} EnemyAudioInfo;

typedef struct {
    Node* root;
    AAMF* model;
    VECTOR last_position;
    VECTOR position;
    VECTOR target;
    uint16_t next_block;
    uint16_t curr_block;
    uint16_t visible; // ---
    uint8_t stunned; // time until stun runs out (4 seconds -> 240 frames)
    uint8_t chasing;
    uint8_t alive;
    int8_t turning;
    uint8_t movement_progress; // in 256 steps for simplicity
    uint8_t mv_speed; // in progress per frame
    EnemyAudioInfo audio;
} Enemy;

void initEnemies(uint8_t level);

void moveEnemies();
uint8_t checkPlayerHit(Player* player);

void drawEnemies(MATRIX* INVCAM);

#endif // ENEMY_H
