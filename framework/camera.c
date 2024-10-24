#include "camera.h"

void camera_get_inverse(Camera* cam, MATRIX* out) {
    VECTOR tpos;

    tpos.vx = -(cam->pos.vx);
    tpos.vy = -(cam->pos.vy);
    tpos.vz = -(cam->pos.vz);

    RotMat( &cam->_trot, out );

    ApplyMatrixLV( out, &tpos, &tpos );

    TransMatrix( out, &tpos );
}

void camera_set_pos(Camera* cam, VECTOR* pos) {
    cam->pos.vx = pos->vx;
    cam->pos.vy = pos->vy;
    cam->pos.vz = pos->vz;
}

void camera_move(Camera* cam, VECTOR* offset) {
    cam->pos.vx += offset->vx;
    cam->pos.vy += offset->vy;
    cam->pos.vz += offset->vz;
}

void camera_rotate(Camera* cam, VECTOR* offset) {
    cam->rot.vx += offset->vx;
    cam->rot.vy += offset->vy;
    cam->rot.vz += offset->vz;
    if (cam->rot.vx < 1280<<12) cam->rot.vx = 1280<<12;
    if (cam->rot.vx > 2880<<12) cam->rot.vx = 2880<<12;
    cam->rot.vy = cam->rot.vy & ((4096<<12)-1);
    // (2880-1280)
    cam->_trot.vx = (cam->rot.vx) >> 12;
    cam->_trot.vy = -(cam->rot.vy) >> 12;
    cam->_trot.vz = (cam->rot.vz) >> 12;
}