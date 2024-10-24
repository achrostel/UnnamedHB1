#ifndef PELLET_H
#define PELLET_H

#include <psxgte.h>

#include "scene.h"
#include "player.h"
#include "random.h"
#include "soundfx.h"

typedef struct {
    Node* model;
    VECTOR position;
    SVECTOR rotation;
    uint16_t alive;
    uint16_t visible;
} Pellet;

void initPellets(uint8_t level);

int checkPlayerCollect(Player* player);

void drawPellets(MATRIX* INVCAM);

extern uint8_t max_pelletes;

#endif // PELLET_H
