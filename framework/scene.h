#ifndef SCENE_H
#define SCENE_H

#include <stdlib.h>
#include <psxgte.h>
#include <inline_c.h>

#include "pmath.h"
#include "disp.h"
#include "player.h"
#include "model.h"

typedef struct node {
	struct node* parent;
    // next sibling node
    struct node* next;
    struct node* child;

    VECTOR pos;
    SVECTOR rot;
    VECTOR scale;
    MATRIX mtx;
    AMF* model;
} Node;

Node* new_node(AMF* model, Node* parent);
Node* add_sibling(Node* n, AMF* model);
Node* add_child(Node* n, AMF* model);

void set_pos(Node* n, int x, int y, int z);
void set_rot(Node* n, int x, int y, int z);
void set_scale(Node* n, int x, int y, int z);

void set_pos_vec(Node* n, VECTOR* pos);
void set_rot_vec(Node* n, SVECTOR* rot);
void set_scale_vec(Node* n, VECTOR* scale);

// normal scene is a tree
void draw_scene(Node* head, MATRIX* worldTransform);
void draw_node(Node* n, MATRIX* parentTransform);

// same as normal, but with light calculation
void draw_scene_light(Node* head, MATRIX* worldTransform, MATRIX* lmtx);
void draw_node_light(Node* n, MATRIX* parentTransform, MATRIX* lmtx);

// static scene is made of siblings
void draw_scene_static(Node* head, MATRIX* worldTransform, Player* p);

#endif // _SCENE_H