#include "ui_parts.h"

#include "string.h"

UI_TextureButton uiCreateTextureButton(RECT* rect, Texture* idle_tex, Texture* hovered_tex, Texture* pressed_tex, UV_RECT* uvs) {
    UI_TextureButton button;
    setPolyGT4(&button.box);
    setXYWH(&button.box, rect->x, rect->y, rect->w, rect->h);
    setUVWH(&button.box, uvs->u, uvs->v, uvs->w, uvs->h);
    setRGB0(&button.box, 128, 128, 128);
    setRGB1(&button.box, 128, 128, 128);
    setRGB2(&button.box, 128, 128, 128);
    setRGB3(&button.box, 128, 128, 128);
    button.state = BUTTON_IDLE;
    button.cb = NULL;
    button.neighbors[0] = NULL;
    button.neighbors[1] = NULL;
    button.neighbors[2] = NULL;
    button.neighbors[3] = NULL;
    button.idle_tex = idle_tex;
    button.hovered_tex = hovered_tex;
    button.pressed_tex = pressed_tex;
    return button;
}

void uiSetTextureButtonCallback(UI_TextureButton* button, Button_Callback cb) {
    button->cb = cb;
}

void uiSetTextureButtonNeighbor(UI_TextureButton* button, uint8_t direction, UI_TextureButton* neighbor) {
    button->neighbors[direction] = neighbor;
}

void uiSetTextureButtonBox(UI_TextureButton* button, RECT* rect) {
    setXYWH(&button->box, rect->x, rect->y, rect->w, rect->h);
}

void uiSetUVTextureButtonBox(UI_TextureButton* button, UV_RECT* rect) {
    setUVWH(&button->box, rect->u, rect->v, rect->w, rect->h);
}

void uiDrawTextureButton(UI_TextureButton* button, RenderContext* ctx) {
    POLY_GT4* pol = new_primitive(ctx);
    memcpy(pol, &button->box, sizeof(POLY_GT4));
    switch (button->state) {
    case BUTTON_IDLE: {
        pol->tpage = button->idle_tex->tpage;
        pol->clut = button->idle_tex->clut;
    } break;
    case BUTTON_HOVERED: {
        pol->tpage = button->hovered_tex->tpage;
        pol->clut = button->hovered_tex->clut;
    } break;
    case BUTTON_PRESSED: {
        pol->tpage = button->pressed_tex->tpage;
        pol->clut = button->pressed_tex->clut;
    } break;
    }
    // setSemiTrans(pol, 1);
    push_ui_elem(ctx, 0, sizeof(POLY_GT4), (uint8_t*)pol);
}