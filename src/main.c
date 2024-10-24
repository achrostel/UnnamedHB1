#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <psxapi.h>
#include <psxpad.h>

#include "disp.h"
#include "soundfx.h"
#include "texture_manager.h"
#include "model.h"
#include "scene.h"
#include "anim_model.h"
#include "collision.h"
#include "player.h"
#include "enemy.h"
#include "pellet.h"

#include "ui_handler.h"

#include "random.h"

#include "timer.h"

#define M_ONE (ONE>>4)

/* Main */
extern AMF room_amf;
extern AMF cyl_amf;

char pad_buff[2][34];

#define MAIN_MENU (0)
#define SETTINGS (1)
#define GAME_VIEW (2)
uint16_t game_started = 0;
uint16_t current_view = MAIN_MENU;

uint32_t score = 0;
uint32_t highscore = 0;
uint32_t lastscore = 0;
uint8_t curr_level = 0;

void reset_game(Player* player, uint8_t level);

int main(int argc, const char **argv) {

	printf("START\n");

	initDisplay();
	printf("DISP\n");
	initSFX();
	printf("SFX\n");
	initImages();
	printf("IMG\n");
	checkTextures();
	printf("TEX\n");

	printf("POST_INIT\n");

	// Init BIOS pad driver and set pad buffers (buffers are updated
	// automatically on every V-Blank)
	InitPAD(&pad_buff[0][0], 34, &pad_buff[1][0], 34);
	
	// Start pad
	StartPAD();
	
	// Don't make pad driver acknowledge V-Blank IRQ (recommended)
	ChangeClearPAD(0);

	printf("POST_PAD\n");

	printf("MENUS\n");
	init_mainMenu();
	init_pauseMenu();
	init_gameUI();

	printf("X_X\n");

	initModels();
	initAnimatedModels();
	initCollisions();

	initEnemies(curr_level);
	initPellets(curr_level);

	// gte_SetBackColor(64, 64, 64);
	gte_SetBackColor(0, 0, 0);

	MATRIX INV_CAM;
	player_mode = 1;
	Player player;
	player.mv_speed = 136; // about 2 m/s from 2m/s / 60 frames * 4096 fixed
	player.cam.rot_speed = ONE*24;
	player.radius = (184);
	player.cam_height = ONE>>1;
	player.stun_ammo = 2;

	setVector( &player.cam.rot, 2048*ONE, 2048*ONE, 0 );
	setVector( &player.pos, 0, 0, -14<<10 );
	setVector( &player.velocity, 0, 0, 0 );

	PADTYPE* pad;
	PADTYPE* mouse;
	uint16_t buttons;
	uint16_t last_buttons = 0;

	char str[50];

	Node* room = new_node(&room_amf, NULL);
	// Node* cyl = new_node(&cyl_amf, NULL);
	// set_pos(cyl, 0, -4<<8, 40<<8);

	play_background();

	printf("LOOP_START\n");
	for (;;) {

		TIME_FUNCTION_S(15);

		pad = (PADTYPE*)&pad_buff[0][0];
		buttons = pad->btn;
		mouse = (PADTYPE*)pad_buff[1];

		switch (current_view)
		{
		case MAIN_MENU: {
			update_channel_volume(23, 0, 0);
			handle_mainMenu(buttons);
		} break;
		case SETTINGS: {
			update_channel_volume(23, 0, 0);
			handle_pauseMenu(buttons);
		} break;
		case GAME_VIEW: {
			update_channel_volume(23, 0x2ff, 0x2ff);
			// TIME_FUNCTION_S(14);
			player_handle_input(&player, buttons, mouse);
			moveEnemies();
			if (checkPlayerHit(&player)) {
				curr_level = 0;
				reset_game(&player, curr_level);
			}
			add_entropy(buttons);
			// TIME_FUNCTION_E(14);
			// TIME_FUNCTION_P("COLLISION");

			// if (!(buttons & PAD_R2) && (last_buttons & PAD_R2)) {
			// 	player_mode = 1 - player_mode;
			// 	setVector( &player.velocity, 0, 0, 0 );
			// }

			if (!(buttons & PAD_START) && (last_buttons & PAD_START)) {
				enable_settings();
			}

			camera_get_inverse(&player.cam, &INV_CAM);
			// rot.vy += 2;
			// rot.vz += 5;
			// set_rot_vec(room, &rot);
			
			TIME_FUNCTION_S(14);
			draw_scene_static(room, &INV_CAM, &player);
			TIME_FUNCTION_E(14);
			TIME_FUNCTION_P("DRAW WORLD");
			// TIME_FUNCTION_ACC_P("SET ", 0);
			// draw_scene_static(light, &INV_CAM, &light_mtx);
			// draw_scene_static(cyl, &INV_CAM, &light_mtx);
			// drawModel(&windTest, &ctx);

			TIME_FUNCTION_S(13);
			drawPellets(&INV_CAM);
			TIME_FUNCTION_E(13);
			TIME_FUNCTION_P("DRAW PELLETS");
			TIME_FUNCTION_S(12);
			drawEnemies(&INV_CAM);
			TIME_FUNCTION_E(12);
			TIME_FUNCTION_P("DRAW ENEMY");

			// sprintf(str, "POS: %d %d %d", player.pos.vx<<3, player.pos.vy<<3, player.pos.vz<<3);
			// draw_text(&ctx, 8, 16, 0, str);
			// sprintf(str, "ROT: %d %d", player.cam.rot.vx>>12, player.cam.rot.vy>>12);
			// draw_text(&ctx, 8, 24, 0, str);
			// // sprintf(str, "TRI: %d", ctx.poly_amount);
			// // draw_text(&ctx, 8, 32, 0, str);
			// sprintf(str, "MODE: %d", player_mode);
			// draw_text(&ctx, 8, 40, 0, str);
			// if (player_mode == 1) {
			// 	sprintf(str, "VEL: %d %d %d", player.velocity.vx, player.velocity.vy, player.velocity.vz);
			// 	draw_text(&ctx, 8, 48, 0, str);
			// }
			int collected = checkPlayerCollect(&player);
			if (collected >= 0) {
				// sprintf(str, "COLLECTED: %d/%d", collected, max_pelletes);
				// draw_text(&ctx, 8, 72, 0, str);
				// sprintf(str, "AMMO: %d/8", player.stun_ammo);
				// draw_text(&ctx, 8, 80, 0, str);
			} else {
				curr_level += 1;
				reset_game(&player, curr_level);
			}

			handle_gameUI(collected, max_pelletes, player.stun_ammo, score);
			// draw_debug_info();

			int32_t time = 0;
			TIME_FUNCTION_E(15);
			TIME_FUNCTION_V(time);
			// sprintf(str, "PHYSSTEPS %d", speed_factor);
			// draw_text(&ctx, 8, 64, 0, str);
			// if (frame_rate < 40) speed_factor = 2;
			// else speed_factor = 1;

		} break;
		}
		// sprintf(str, "HBLANK %d", VSync(1));
		// draw_text(&ctx, 8, 200, 0, str);
		add_entropy(VSync(1));
		flip_buffers(&ctx);
		last_buttons = buttons;
	}

	return 0;
}

void reset_game(Player* player, uint8_t level) {
	initEnemies(level);
	initPellets(level);
	setVector( &player->cam.rot, 2048*ONE, 2048*ONE, 0 );
	setVector( &player->pos, 0, 0, -14<<10 );
	setVector( &player->velocity, 0, 0, 0 );
	if (level == 0) {
		player->stun_ammo = 2;
		highscore = score > highscore ? score : highscore;
		lastscore = score;
		score = 0;
		enable_mainMenu();
	}
}