#include "ui_handler.h"

#include <psxpad.h>
#include <psxgpu.h>
#include "pmath.h"
#include "random.h"

static UI_TextureButton start_button;
static UI_TextureButton settings_button;
static UI_TextureButton rm0_button;
static UI_TextureButton rm1_button;
static UI_TextureButton return_button;
static UI_TextureButton* startmenu_buttons[2];
static uint8_t smbuttons_amount = 0;
static UI_TextureButton* settings_buttons[3];
static uint8_t stbuttons_amount = 0;

// I'll add actual texture box support later
static UI_TextureButton score_tb;
static UI_TextureButton samples_tb;
static UI_TextureButton ammo_tb;
static UI_TextureButton score_num_tbs[8];
static UI_TextureButton sample_num_tbs[5];
static UI_TextureButton ammo_num_tbs[3];
static UI_TextureButton* gameUI_tbs[19];
static uint8_t gameUI_tbs_amount = 0;

static uint16_t last_buttons = 0;
static UI_TextureButton* button_selection;

extern RenderContext ctx;
extern uint16_t game_started;
extern uint16_t current_view;

extern Texture buttons_idle_tex;
extern Texture buttons_active_tex;
extern Texture floor_tex;
extern Texture wall_tex;
Texture bg_texes[2];
uint8_t active_bg_tex = 1;

extern uint32_t highscore;
extern uint32_t lastscore;

static void start_game(UI_TextureButton* self) {
    game_started = 1;
    current_view = 2;
    enable_gameUI();
}

void enable_mainMenu() {
    current_view = 0;
    game_started = 0;
    button_selection = startmenu_buttons[0];
    active_bg_tex = get_random()&1;
}
void enable_settings() {
    current_view = 1;
    button_selection = settings_buttons[0];
    active_bg_tex = get_random()&1;
}
void enable_gameUI() {
    button_selection = NULL;
}

static void goto_settings(UI_TextureButton* self) {
    enable_settings();
}

static void return_from_settings(UI_TextureButton* self) {
    if (game_started) {
        current_view = 2;
    } else {
        enable_mainMenu();
    }
}

static void set320x240(UI_TextureButton* self) {
    if (ctx.render_mode == 0) return;
    update_render_mode(0);
    for (uint8_t i = 0; i < smbuttons_amount; ++i) {
        startmenu_buttons[i]->box.x0 >>= 1;
        startmenu_buttons[i]->box.x1 >>= 1;
        startmenu_buttons[i]->box.x2 >>= 1;
        startmenu_buttons[i]->box.x3 >>= 1;
        startmenu_buttons[i]->box.y0 >>= 1;
        startmenu_buttons[i]->box.y1 >>= 1;
        startmenu_buttons[i]->box.y2 >>= 1;
        startmenu_buttons[i]->box.y3 >>= 1;
    }
    for (uint8_t i = 0; i < stbuttons_amount; ++i) {
        settings_buttons[i]->box.x0 >>= 1;
        settings_buttons[i]->box.x1 >>= 1;
        settings_buttons[i]->box.x2 >>= 1;
        settings_buttons[i]->box.x3 >>= 1;
        settings_buttons[i]->box.y0 >>= 1;
        settings_buttons[i]->box.y1 >>= 1;
        settings_buttons[i]->box.y2 >>= 1;
        settings_buttons[i]->box.y3 >>= 1;
    }
    for (uint8_t i = 0; i < gameUI_tbs_amount; ++i) {
        gameUI_tbs[i]->box.x0 >>= 1;
        gameUI_tbs[i]->box.x1 >>= 1;
        gameUI_tbs[i]->box.x2 >>= 1;
        gameUI_tbs[i]->box.x3 >>= 1;
        gameUI_tbs[i]->box.y0 >>= 1;
        gameUI_tbs[i]->box.y1 >>= 1;
        gameUI_tbs[i]->box.y2 >>= 1;
        gameUI_tbs[i]->box.y3 >>= 1;
    }
}
static void set640x480(UI_TextureButton* self) {
    if (ctx.render_mode == 1) return;
    update_render_mode(1);
    for (uint8_t i = 0; i < smbuttons_amount; ++i) {
        startmenu_buttons[i]->box.x0 <<= 1;
        startmenu_buttons[i]->box.x1 <<= 1;
        startmenu_buttons[i]->box.x2 <<= 1;
        startmenu_buttons[i]->box.x3 <<= 1;
        startmenu_buttons[i]->box.y0 <<= 1;
        startmenu_buttons[i]->box.y1 <<= 1;
        startmenu_buttons[i]->box.y2 <<= 1;
        startmenu_buttons[i]->box.y3 <<= 1;
    }
    for (uint8_t i = 0; i < stbuttons_amount; ++i) {
        settings_buttons[i]->box.x0 <<= 1;
        settings_buttons[i]->box.x1 <<= 1;
        settings_buttons[i]->box.x2 <<= 1;
        settings_buttons[i]->box.x3 <<= 1;
        settings_buttons[i]->box.y0 <<= 1;
        settings_buttons[i]->box.y1 <<= 1;
        settings_buttons[i]->box.y2 <<= 1;
        settings_buttons[i]->box.y3 <<= 1;
    }
    for (uint8_t i = 0; i < gameUI_tbs_amount; ++i) {
        gameUI_tbs[i]->box.x0 <<= 1;
        gameUI_tbs[i]->box.x1 <<= 1;
        gameUI_tbs[i]->box.x2 <<= 1;
        gameUI_tbs[i]->box.x3 <<= 1;
        gameUI_tbs[i]->box.y0 <<= 1;
        gameUI_tbs[i]->box.y1 <<= 1;
        gameUI_tbs[i]->box.y2 <<= 1;
        gameUI_tbs[i]->box.y3 <<= 1;
    }
}

void init_mainMenu() {
    bg_texes[0] = floor_tex;
    bg_texes[1] = wall_tex;
    RECT rect = {96, 104, 128, 32};
    UV_RECT uvs = {0, 0, 127, 31};
    start_button = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
    uiSetTextureButtonCallback(&start_button, start_game);
    startmenu_buttons[smbuttons_amount] = &start_button;
    smbuttons_amount += 1;

    rect.y = 104+32+20;
    uvs.u = 128;
    settings_button = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
    uiSetTextureButtonCallback(&settings_button, goto_settings);
    startmenu_buttons[smbuttons_amount] = &settings_button;
    smbuttons_amount += 1;

    start_button.neighbors[BUTTON_DOWN] = &settings_button;
    settings_button.neighbors[BUTTON_UP] = &start_button;

    button_selection = &start_button;
}

void init_pauseMenu() {
    RECT rect = {21, 104, 128, 32};
    UV_RECT uvs = {0, 32, 127, 31};
    rm0_button = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
    uiSetTextureButtonCallback(&rm0_button, set320x240);
    settings_buttons[stbuttons_amount] = &rm0_button;
    stbuttons_amount += 1;
    
    rect.x = 21+128+22;
    uvs.u = 128;
    rm1_button = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
    uiSetTextureButtonCallback(&rm1_button, set640x480);
    settings_buttons[stbuttons_amount] = &rm1_button;
    stbuttons_amount += 1;
    
    rect.x = 96;
    rect.y = 104+32+20;
    uvs.v = 64;
    return_button = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
    uiSetTextureButtonCallback(&return_button, return_from_settings);
    settings_buttons[stbuttons_amount] = &return_button;
    stbuttons_amount += 1;

    rm0_button.neighbors[BUTTON_RIGHT] = &rm1_button;
    rm1_button.neighbors[BUTTON_LEFT] = &rm0_button;
    rm0_button.neighbors[BUTTON_DOWN] = &return_button;
    rm1_button.neighbors[BUTTON_DOWN] = &return_button;
    return_button.neighbors[BUTTON_UP] = &rm0_button;
}

void init_gameUI() {
    RECT rect = {0, 0, 64, 16};
    UV_RECT uvs = {0, 64, 63, 15};
    score_tb = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
    uiSetTextureButtonCallback(&score_tb, NULL);
    gameUI_tbs[gameUI_tbs_amount] = &score_tb;
    gameUI_tbs_amount += 1;
    score_tb.state = BUTTON_IDLE;

    rect.y += 32;
    uvs.u = 64;
    samples_tb = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
    uiSetTextureButtonCallback(&samples_tb, NULL);
    gameUI_tbs[gameUI_tbs_amount] = &samples_tb;
    gameUI_tbs_amount += 1;
    samples_tb.state = BUTTON_IDLE;

    rect.y += 32;
    uvs.u = 0;
    uvs.v = 80;
    ammo_tb = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
    uiSetTextureButtonCallback(&ammo_tb, NULL);
    gameUI_tbs[gameUI_tbs_amount] = &ammo_tb;
    gameUI_tbs_amount += 1;
    ammo_tb.state = BUTTON_IDLE;

    uvs.u = 0;
    uvs.v = 96;
    uvs.w = 7;
    uvs.h = 15;
    rect.w = 8;

    rect.x = 0;
    rect.y = 16;
    for (uint8_t i = 0; i < 8; ++i) {
        score_num_tbs[i] = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
        gameUI_tbs[gameUI_tbs_amount] = &score_num_tbs[i];
        gameUI_tbs_amount += 1;
        score_num_tbs[i].state = BUTTON_IDLE;
        rect.x += 8;
    }
    rect.x = 12;
    rect.y += 32;
    for (uint8_t i = 0; i < 5; ++i) {
        sample_num_tbs[i] = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
        gameUI_tbs[gameUI_tbs_amount] = &sample_num_tbs[i];
        gameUI_tbs_amount += 1;
        sample_num_tbs[i].state = BUTTON_IDLE;
        rect.x += 8;
    }
    rect.x = 20;
    rect.y += 32;
    for (uint8_t i = 0; i < 3; ++i) {
        ammo_num_tbs[i] = uiCreateTextureButton(&rect, &buttons_idle_tex, &buttons_active_tex, &buttons_active_tex, &uvs);
        gameUI_tbs[gameUI_tbs_amount] = &ammo_num_tbs[i];
        gameUI_tbs_amount += 1;
        ammo_num_tbs[i].state = BUTTON_IDLE;
        rect.x += 8;
    }

    uvs.u = 8*10;
    uiSetUVTextureButtonBox(&sample_num_tbs[2], &uvs);
    uiSetUVTextureButtonBox(&ammo_num_tbs[1], &uvs);
    uvs.u = 8*8;
    uiSetUVTextureButtonBox(&ammo_num_tbs[2], &uvs);
}

void handle_mainMenu(uint16_t buttons) {
    for (uint8_t i = 0; i < smbuttons_amount; ++i) {
        startmenu_buttons[i]->state = BUTTON_IDLE;
    }
    button_selection->state = BUTTON_HOVERED;

    if (!(buttons & PAD_CROSS) && (last_buttons & PAD_CROSS)) {
        button_selection->cb(button_selection);
    } else if (!(buttons & PAD_UP) && (last_buttons & PAD_UP) && button_selection->neighbors[BUTTON_UP] != NULL) {
        button_selection = button_selection->neighbors[BUTTON_UP];
    } else if (!(buttons & PAD_RIGHT) && (last_buttons & PAD_RIGHT) && button_selection->neighbors[BUTTON_RIGHT] != NULL) {
        button_selection = button_selection->neighbors[BUTTON_RIGHT];
    } else if (!(buttons & PAD_DOWN) && (last_buttons & PAD_DOWN) && button_selection->neighbors[BUTTON_DOWN] != NULL) {
        button_selection = button_selection->neighbors[BUTTON_DOWN];
    } else if (!(buttons & PAD_LEFT) && (last_buttons & PAD_LEFT) && button_selection->neighbors[BUTTON_LEFT] != NULL) {
        button_selection = button_selection->neighbors[BUTTON_LEFT];
    }

    draw_bg();

    for (uint8_t i = 0; i < smbuttons_amount; ++i) {
        uiDrawTextureButton(startmenu_buttons[i], &ctx);
    }
    
    if (highscore > 0) {
        char str[20];
        sprintf(str, "HIGHSCORE:  %lu", highscore);
	    draw_text(&ctx, 16<<ctx.render_mode, 208<<ctx.render_mode, 0, str);
        sprintf(str, "LAST SCORE: %lu", lastscore);
	    draw_text(&ctx, 16<<ctx.render_mode, 216<<ctx.render_mode, 0, str);
    }
    draw_text(&ctx, 16<<ctx.render_mode, (16<<ctx.render_mode)+ 0, 0, "Game");
    draw_text(&ctx, 16<<ctx.render_mode, (16<<ctx.render_mode)+ 8, 0, "D-Pad         Move");
    draw_text(&ctx, 16<<ctx.render_mode, (16<<ctx.render_mode)+16, 0, "Face-Buttons  Look");
    draw_text(&ctx, 16<<ctx.render_mode, (16<<ctx.render_mode)+24, 0, "L1            Stun");
    draw_text(&ctx, 16<<ctx.render_mode, (16<<ctx.render_mode)+32, 0, "R1            Jump");
    draw_text(&ctx, 16<<ctx.render_mode, (16<<ctx.render_mode)+40, 0, "START        Pause");

    draw_text(&ctx, 16<<ctx.render_mode, (16<<ctx.render_mode)+56, 0, "Menu");
    draw_text(&ctx, 16<<ctx.render_mode, (16<<ctx.render_mode)+64, 0, "D-Pad Navigate");
    draw_text(&ctx, 16<<ctx.render_mode, (16<<ctx.render_mode)+72, 0, "X       Select");

    last_buttons = buttons;
}

void handle_pauseMenu(uint16_t buttons) {
    for (uint8_t i = 0; i < stbuttons_amount; ++i) {
        settings_buttons[i]->state = BUTTON_IDLE;
    }
    button_selection->state = BUTTON_HOVERED;

    if (!(buttons & PAD_CROSS) && (last_buttons & PAD_CROSS)) {
        button_selection->cb(button_selection);
    } else if (!(buttons & PAD_UP) && (last_buttons & PAD_UP) && button_selection->neighbors[BUTTON_UP] != NULL) {
        button_selection = button_selection->neighbors[BUTTON_UP];
    } else if (!(buttons & PAD_RIGHT) && (last_buttons & PAD_RIGHT) && button_selection->neighbors[BUTTON_RIGHT] != NULL) {
        button_selection = button_selection->neighbors[BUTTON_RIGHT];
    } else if (!(buttons & PAD_DOWN) && (last_buttons & PAD_DOWN) && button_selection->neighbors[BUTTON_DOWN] != NULL) {
        button_selection = button_selection->neighbors[BUTTON_DOWN];
    } else if (!(buttons & PAD_LEFT) && (last_buttons & PAD_LEFT) && button_selection->neighbors[BUTTON_LEFT] != NULL) {
        button_selection = button_selection->neighbors[BUTTON_LEFT];
    }

    draw_bg();

    for (uint8_t i = 0; i < stbuttons_amount; ++i) {
        uiDrawTextureButton(settings_buttons[i], &ctx);
    }

    last_buttons = buttons;
}

void handle_gameUI(int curr_pellets, uint8_t max_pellets, uint32_t ammo, uint16_t score) {
    UV_RECT uvs = {0, 96, 7, 15};
    for (int8_t i = 7; i >= 0; --i) {
        uvs.u = 8*(score%10);
        uiSetUVTextureButtonBox(&score_num_tbs[i], &uvs);
        score = score/10;
    }
    for (int8_t i = 1; i >= 0; --i) {
        uvs.u = 8*(curr_pellets%10);
        uiSetUVTextureButtonBox(&sample_num_tbs[i], &uvs);
        curr_pellets = curr_pellets/10;
    }
    for (int8_t i = 4; i >= 3; --i) {
        uvs.u = 8*(max_pellets%10);
        uiSetUVTextureButtonBox(&sample_num_tbs[i], &uvs);
        max_pellets = max_pellets/10;
    }

    uvs.u = 8*(ammo);
    uiSetUVTextureButtonBox(&ammo_num_tbs[0], &uvs);

    for (uint8_t i = 0; i < gameUI_tbs_amount; ++i) {
        uiDrawTextureButton(gameUI_tbs[i], &ctx);
    }
}

void draw_bg() {
    for (int8_t y = -1; y < 7; ++y) {
        for (int8_t x = -1; x < 9; ++x) {
            POLY_GT4* pol = new_primitive(&ctx);
            setPolyGT4(pol);
            pol->x0 = (x      * (40) + ((sin_t((VSync(-1)<<4) +     (y<<8))*10)>>12) + ((sin_t((VSync(-1)<<5) +     (x<<8))*10)>>12))<<ctx.render_mode;
            pol->x1 = ((x+1)  * (40) + ((sin_t((VSync(-1)<<4) +     (y<<8))*10)>>12) + ((sin_t((VSync(-1)<<5) + ((x+1)<<8))*10)>>12))<<ctx.render_mode;
            pol->x2 = (x      * (40) + ((sin_t((VSync(-1)<<4) + ((y+1)<<8))*10)>>12) + ((sin_t((VSync(-1)<<5) +     (x<<8))*10)>>12))<<ctx.render_mode;
            pol->x3 = ((x+1)  * (40) + ((sin_t((VSync(-1)<<4) + ((y+1)<<8))*10)>>12) + ((sin_t((VSync(-1)<<5) + ((x+1)<<8))*10)>>12))<<ctx.render_mode;
            pol->y0 = (y      * (40) + ((sin_t((VSync(-1)<<5) +     (x<<8))*10)>>12))<<ctx.render_mode;
            pol->y1 = (y      * (40) + ((sin_t((VSync(-1)<<5) + ((x+1)<<8))*10)>>12))<<ctx.render_mode;
            pol->y2 = (((y+1) * (40) + ((sin_t((VSync(-1)<<5) +     (x<<8))*10)>>12))<<ctx.render_mode) + 1;
            pol->y3 = (((y+1) * (40) + ((sin_t((VSync(-1)<<5) + ((x+1)<<8))*10)>>12))<<ctx.render_mode) + 1;

            pol->clut = bg_texes[active_bg_tex].clut;
            pol->tpage = bg_texes[active_bg_tex].tpage;
            if (!(active_bg_tex&1)) {
                setUVWH(pol, bg_texes[active_bg_tex].ox, bg_texes[active_bg_tex].oy, 
                        (64<<(bg_texes[active_bg_tex].aif.header.size))-1, (64<<(bg_texes[active_bg_tex&1].aif.header.size))-1);
                setRGB0(pol, 48, 12, 12);
                setRGB1(pol, 48, 12, 12);
                setRGB2(pol, 48, 12, 12);
                setRGB3(pol, 48, 12, 12);

            } else {
                setUVWH(pol, bg_texes[active_bg_tex].ox, bg_texes[active_bg_tex].oy, 
                        (64<<(bg_texes[active_bg_tex].aif.header.size))-1, (64<<(bg_texes[active_bg_tex].aif.header.size))-1);
                setRGB0(pol, 96, 24, 24);
                setRGB1(pol, 96, 24, 24);
                setRGB2(pol, 96, 24, 24);
                setRGB3(pol, 96, 24, 24);

            }
            push_ui_elem(&ctx, 1, sizeof(POLY_GT4), (uint8_t*) pol);
        }
    }
}