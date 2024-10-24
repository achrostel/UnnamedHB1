#ifndef CAMERA_H
#define CAMERA_H

#include <psxgte.h>

#include "pmath.h"

typedef struct camera {
    VECTOR pos;
    VECTOR rot;

    SVECTOR _trot;
    int32_t rot_speed;
} Camera;

void camera_get_inverse(Camera* cam, MATRIX* out);

void camera_set_pos(Camera* cam, VECTOR* pos);

void camera_move(Camera* cam, VECTOR* offset);
void camera_rotate(Camera* cam, VECTOR* offset);

#endif // _CAMERA_H