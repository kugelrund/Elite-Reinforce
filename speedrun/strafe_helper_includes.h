#ifndef STRAFE_HELPER_INCLUDES_H
#define STRAFE_HELPER_INCLUDES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdio.h>

static const float shi_color_accelerating[4] = {0.0f, 0.5f, 0.125f, 0.375f};
static const float shi_color_optimal[4] = {0.0f, 1.0f, 0.25f, 0.75f};
static const float shi_color_center_marker[4] = {1.0f, 1.0f, 1.0f, 0.75f};
static const float shi_color_speed[4] = {1.0f, 1.0f,  1.0f, 0.9f};

void shi_drawFilledRectangle(float x, float y, float w, float h, const float color[4]);
void shi_drawString(float x, float y, const char* string, float scale, const float color[4]);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // STRAFE_HELPER_INCLUDES_H
