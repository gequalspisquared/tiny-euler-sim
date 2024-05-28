#include "fluid.h"

Fluid init_fluid() {
    Vector2 velocity = {0};
    Vector2 old_velocity = {0};

    Fluid fluid;
    fluid.velocity = velocity;
    fluid.old_velocity = old_velocity;

    return fluid;
}