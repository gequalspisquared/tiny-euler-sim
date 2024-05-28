#ifndef FLUID_H
#define FLUID_H

#include <raylib.h>

typedef struct {
    Vector2 velocity;
    Vector2 old_velocity;  // Need old velocity for physics calculations
} Fluid;

Fluid init_fluid();

#endif