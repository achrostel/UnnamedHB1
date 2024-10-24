#ifndef UI_PARTS_H
#define UI_PARTS_H

#include <inline_c.h>

#include "disp.h"
#include "texture_manager.h"

typedef struct {
    uint8_t u;
    uint8_t v;
    uint8_t w;
    uint8_t h;
} UV_RECT;


#define BUTTON_IDLE    (0)
#define BUTTON_HOVERED (1)
#define BUTTON_PRESSED (2)

#define BUTTON_UP    (0)
#define BUTTON_RIGHT (1)
#define BUTTON_DOWN  (2)
#define BUTTON_LEFT  (3)

typedef struct _UI_TextureButton UI_TextureButton;
typedef void (*Button_Callback)(UI_TextureButton*);

// basically the same, meh
typedef POLY_GT4 UI_TextureBox;

typedef struct _UI_TextureButton {
    UI_TextureBox box;
    Texture* idle_tex;
    Texture* hovered_tex;
    Texture* pressed_tex;
    uint8_t state;
    uint8_t _pad0;
    uint8_t _pad1;
    uint8_t _pad2;
    Button_Callback cb;
    /* neighbors
    0 up
    1 right
    2 down
    3 left
    */
    struct _UI_TextureButton* neighbors[4]; 
} UI_TextureButton;

UI_TextureButton uiCreateTextureButton(RECT* rect, Texture* idle_tex, Texture* hovered_tex, Texture* pressed_tex, UV_RECT* uvs);
void uiSetTextureButtonCallback(UI_TextureButton* button, Button_Callback cb);
void uiSetTextureButtonNeighbor(UI_TextureButton* button, uint8_t direction, UI_TextureButton* neighbor);
void uiSetTextureButtonBox(UI_TextureButton* button, RECT* rect);
void uiSetUVTextureButtonBox(UI_TextureButton* button, UV_RECT* rect);

void uiDrawTextureButton(UI_TextureButton* button, RenderContext* ctx);

#endif // UI_PARTS_H