#include "player.h"

#include <psxgpu.h>

extern CollisionMesh test_col;

extern SFX steps[4];
extern SFX use_ammo_sfx;
static int last_step = 0;
static uint8_t stepped = 0;

unsigned char player_mode;

// hacked in, just finishing fast
static uint16_t last_input;
uint8_t player_should_stun;

void player_handle_input(Player* player, uint16_t buttons, PADTYPE* mouse) {
    VECTOR pos_off = {0, 0, 0};
    VECTOR rot_off = {0, 0, 0};
    
    if (!(buttons & PAD_L1) && (last_input & PAD_L1) && player->stun_ammo) {
        player->stun_ammo -= 1;
        player_should_stun = 1;
        play_sample(use_ammo_sfx.addr, use_ammo_sfx.sample_rate, 0x07ff);
    } else {
        player_should_stun = 0;
    }

    if (!(buttons & PAD_TRIANGLE)) {
        rot_off.vx -= player->cam.rot_speed;
    }
    if (!(buttons & PAD_CROSS)) {
        rot_off.vx += player->cam.rot_speed;
    }
    if (!(buttons & PAD_SQUARE)) {
        rot_off.vy += player->cam.rot_speed;
    }
    if (!(buttons & PAD_CIRCLE)) {
        rot_off.vy -= player->cam.rot_speed;
    }
    if (mouse != NULL && mouse->type == PAD_ID_MOUSE) {
        rot_off.vx += (mouse->y_mov * player->cam.rot_speed)>>3;
        rot_off.vy -= (mouse->x_mov * player->cam.rot_speed)>>3;
    }
    camera_rotate(&player->cam, &rot_off);

    if (player_mode == 0) {
        if (!(buttons & PAD_UP)) {
            pos_off.vx -= (( sin_t( player->cam._trot.vy )*cos_t( player->cam._trot.vx ) )>>12)*player->mv_speed;
            pos_off.vy += sin_t( player->cam._trot.vx )*player->mv_speed;
            pos_off.vz += (( cos_t( player->cam._trot.vy )*cos_t( player->cam._trot.vx ) )>>12)*player->mv_speed;
        }
        if (!(buttons & PAD_DOWN)) {
            pos_off.vx += (( sin_t( player->cam._trot.vy )*cos_t( player->cam._trot.vx ) )>>12)*player->mv_speed;
            pos_off.vy -= sin_t( player->cam._trot.vx )*player->mv_speed;
            pos_off.vz -= (( cos_t( player->cam._trot.vy )*cos_t( player->cam._trot.vx ) )>>12)*player->mv_speed;
        }
        if (!(buttons & PAD_LEFT)) {
            pos_off.vx -= cos_t( player->cam._trot.vy )*player->mv_speed;
            pos_off.vz -= sin_t( player->cam._trot.vy )*player->mv_speed;
        }
        if (!(buttons & PAD_RIGHT)) {
            pos_off.vx += cos_t( player->cam._trot.vy )*player->mv_speed;
            pos_off.vz += sin_t( player->cam._trot.vy )*player->mv_speed;
        }
        if( !(buttons & PAD_R1) ) {
            pos_off.vx -= (( sin_t( player->cam._trot.vy )*sin_t( player->cam._trot.vx ) )>>12)*player->mv_speed;
            pos_off.vy -= cos_t( player->cam._trot.vx )*player->mv_speed;
            pos_off.vz += (( cos_t( player->cam._trot.vy )*sin_t( player->cam._trot.vx ) )>>12)*player->mv_speed;
        }
        if( !(buttons & PAD_L1) ) {
            pos_off.vx += (( sin_t( player->cam._trot.vy )*sin_t( player->cam._trot.vx ) )>>12)*player->mv_speed;
            pos_off.vy += cos_t( player->cam._trot.vx )*player->mv_speed;
            pos_off.vz -= (( cos_t( player->cam._trot.vy )*sin_t( player->cam._trot.vx ) )>>12)*player->mv_speed;
        }
        pos_off.vx = (pos_off.vx>>10);
        pos_off.vy = (pos_off.vy>>10);
        pos_off.vz = (pos_off.vz>>10);
        // test_collide(&test_col, &player->pos, &player->velocity, player->radius);
        player_move(player, &pos_off);
    } else if (player_mode == 1) {
        int moved = 0;
        if (!(buttons & PAD_UP)) {
            pos_off.vx += sin_t( player->cam._trot.vy );
            pos_off.vz -= cos_t( player->cam._trot.vy );
            moved = 1;
        }
        if (!(buttons & PAD_DOWN)) {
            pos_off.vx -= sin_t( player->cam._trot.vy );
            pos_off.vz += cos_t( player->cam._trot.vy );
            moved = 1;
        }
        if (!(buttons & PAD_LEFT)) {
            pos_off.vx -= cos_t( player->cam._trot.vy );
            pos_off.vz -= sin_t( player->cam._trot.vy );
            moved = 1;
        }
        if (!(buttons & PAD_RIGHT)) {
            pos_off.vx += cos_t( player->cam._trot.vy );
            pos_off.vz += sin_t( player->cam._trot.vy );
            moved = 1;
        }

        player->velocity.vy += -11; // about 10m/s^2 gravity
        // player->velocity.vy = -(1<<12);

        // collisions here
        unsigned char is_floor = test_collide(&test_col, &player->pos, &player->velocity, player->radius);

        if (is_floor) {
            player->velocity.vy = -2;
            if( !(buttons & PAD_R1) ) {
                player->velocity.vy = (1<<8);
            }
            
            player->velocity.vx += ((pos_off.vx*player->mv_speed)>>11);
            player->velocity.vz += ((pos_off.vz*player->mv_speed)>>11);
            player->velocity.vx = (player->velocity.vx / 2);
            player->velocity.vz = (player->velocity.vz / 2);

            if (moved) {
                if (VSync(-1) - last_step > 35) {
                    last_step = VSync(-1);
                    SFX* step = &steps[get_random()%4];
                    play_sample(step->addr, step->sample_rate, 0x3fff);
                } 
                stepped = 0;
            } else {
                if (VSync(-1) - last_step > 50) last_step = VSync(-1);
                last_step += ((VSync(-1) - last_step)>>2)+1;
                if (!stepped && VSync(-1) - last_step <= 6) {
                    SFX* step = &steps[get_random()%4];
                    play_sample(step->addr, step->sample_rate, 0x3fff);
                    stepped = 1;
                }
            }
        } else {
            last_step = VSync(-1) - 35;
            stepped = 0;
        }

        player->velocity.vx += ((pos_off.vx*player->mv_speed)>>17);
        player->velocity.vz += ((pos_off.vz*player->mv_speed)>>17);


        player_move(player, &player->velocity);

        // player->velocity.vx = player->velocity.vx / 2;
        // player->velocity.vz = player->velocity.vz / 2;
    }
    last_input = buttons;
}

void player_move(Player* player, VECTOR* offset) {
    player->pos.vx += (offset->vx>>4);
    player->pos.vy += (offset->vy>>4);
    player->pos.vz += (offset->vz>>4);
    VECTOR cam_pos = player->pos;
    cam_pos.vy += (player->cam_height>>4) + (player_mode ? (sin_t((VSync(-1) - last_step)*59)>>7) : 0);
    camera_set_pos(&player->cam, &cam_pos);
}