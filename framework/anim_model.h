#ifndef ANIM_MODEL_H
#define ANIM_MODEL_H

#include <psxgte.h>
#include <inline_c.h>

#include "scene.h"
#include "model.h"

typedef struct {
    MATRIX mat;
} Keyframe;

typedef struct {
    char name[8];
    uint32_t keyframeamount;
    Keyframe* keyframe;
} Animation;

typedef struct {
    uint16_t boneamount; 
    uint16_t animamount; 
    Node* root;
    Node** nodes; // 0 is body
    Animation** anims;
    uint32_t active_anim;
} AAMF;

void initAnimatedModels();

void aamfInitData(AAMF* aamf, unsigned char* data);

void activateAnimation(AAMF* aamf, char name[8]);
void setAnimationKeyframe(AAMF* aamf, uint16_t keyframe);

#endif // _ANIM_MODEL_H