#ifndef PLAYER_H
#define PLAYER_H

#include <psxgte.h>
#include <psxpad.h>

#include "camera.h"
#include "collision.h"
#include "random.h"
#include "soundfx.h"

extern unsigned char player_mode;

typedef struct player {
    Camera cam;
    VECTOR pos; // hitbox center
    VECTOR velocity;
    int32_t mv_speed;
    int16_t radius;
    int16_t cam_height;
    uint32_t stun_ammo;
} Player;

void player_handle_input(Player* player, uint16_t buttons, PADTYPE* mouse);

void player_move(Player* player, VECTOR* offset);

#endif // _PLAYER_H