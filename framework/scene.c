#include "scene.h"

#include "timer.h"

Node* new_node(AMF* model, Node* parent) {
    Node* node = malloc(sizeof(Node));
    node->parent = parent;
    node->next = NULL;
    node->child = NULL;
    node->model = model;
    setVector(&(node->scale), ONE, ONE, ONE);
    set_pos(node , 0, 0, 0);
    set_rot(node , 0, 0, 0);
    return node;
}

Node* add_sibling(Node* n, AMF* model) {
    while (n->next)
        n = n->next;

    n->next = new_node(model, n->parent);
    return n->next;
}

Node* add_child(Node* n, AMF* model) {
    if ( n->child )
        return add_sibling(n->child, model);
    
    n->child = new_node(model, n);
    return (n->child);
}

void set_pos(Node* n, int x, int y, int z) {
    setVector( &(n->pos), x, y, z );
    TransMatrix(&(n->mtx), &(n->pos));
}

void set_rot(Node* n, int x, int y, int z) {
    setVector( &(n->rot), x, y, z );
    RotMat(&(n->rot), &(n->mtx));
}

void set_scale(Node* n, int x, int y, int z) {
    setVector( &(n->scale), x, y, z );
}

void set_pos_vec(Node* n, VECTOR* pos) {
    setVector( &(n->pos), pos->vx>>9, pos->vy>>9, pos->vz>>9 );
    TransMatrix(&(n->mtx), &(n->pos));
}

void set_rot_vec(Node* n, SVECTOR* rot) {
    setVector( &(n->rot), rot->vx, rot->vy, rot->vz );
    RotMat(&(n->rot), &(n->mtx));
}

void set_scale_vec(Node* n, VECTOR* scale) {
    setVector( &(n->scale), scale->vx, scale->vy, scale->vz );
}

void draw_scene(Node* head, MATRIX* worldTransform) {
    MATRIX mtx;

    CompMatrixLV( worldTransform, &(head->mtx), &mtx );

    Node* n = head->child;
    if (n) {
        draw_node(n, &mtx);
        while (n->next) {
            n = n->next;
            draw_node(n, &mtx);
        }
    }
}

void draw_node(Node* n, MATRIX* parentTransform) {
    MATRIX mtx;

    CompMatrixLV( parentTransform, &(n->mtx), &mtx );

    setTransformMatrix(&mtx);
    drawModel(n->model, &ctx);

    Node* c = n->child;
    if (c) {
        draw_node(c, &mtx);
        while (c->next) {
            c = c->next;
            draw_node(c, &mtx);
        }
    }
}

void draw_scene_light(Node* head, MATRIX* worldTransform, MATRIX* lmtx) {
    MATRIX mtx;

    MATRIX tlmtx = *lmtx;

    CompMatrixLV( worldTransform, &(head->mtx), &mtx );

    MulMatrix0( &tlmtx, &mtx, &tlmtx );

    setTransformMatrix(&mtx);
    gte_SetLightMatrix( &tlmtx );
    drawModel(head->model, &ctx);

    Node* n = head->child;
    if (n) {
        draw_node_light(n, &mtx, lmtx);
        while (n->next) {
            n = n->next;
            draw_node_light(n, &mtx, lmtx);
        }
    }
}

void draw_node_light(Node* n, MATRIX* parentTransform, MATRIX* lmtx) {
    MATRIX mtx;

    MATRIX tlmtx = *lmtx;

    CompMatrixLV( parentTransform, &(n->mtx), &mtx );

    MulMatrix0( lmtx, &mtx, &tlmtx );
    gte_SetLightMatrix( &tlmtx );

    ScaleMatrix(&mtx, &(n->scale));

    setTransformMatrix(&mtx);
    drawModel(n->model, &ctx);
    // TIME_FUNCTIONB(drawModel(n->model, &ctx);, "DRAW_MODEL", 0, 100);

    Node* c = n->child;
    if (c) {
        draw_node_light(c, &mtx, lmtx);
        while (c->next) {
            c = c->next;
            draw_node_light(c, &mtx, lmtx);
        }
    }
}

void draw_scene_static(Node* head, MATRIX* worldTransform, Player* p) {
    MATRIX mtx;

    CompMatrixLV( worldTransform, &(head->mtx), &mtx );
    
    // ScaleMatrix(&mtx, &(head->scale));

    setTransformMatrix(&mtx);

    drawWorld(head->model, &ctx, p);
    // TIME_FUNCTIONB(drawModel(head->model, &ctx), "DRAW_MODEL", 0, 100);

    Node* n = head;
    while (n->next) {
        n = n->next;
        TransMatrix(&mtx, &(n->pos));
        RotMat(&(n->rot), &mtx);
        // ScaleMatrix(&mtx, &(n->scale));

        CompMatrixLV( worldTransform, &mtx, &mtx );

        setTransformMatrix(&mtx);
        drawWorld(n->model, &ctx, p);
        // TIME_FUNCTIONB(drawModel(n->model, &ctx);, "DRAW_MODEL", 0, 100);
    }
}