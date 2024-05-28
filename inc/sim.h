#ifndef SIM_H
#define SIM_H

#include <raylib.h>
#include <stdlib.h>

#include "fluid.h"

typedef struct {
    Fluid fluid;
    int id;
    int free;  // 1 if free fluid, 0 if solid wall
    Vector2 center;
} Cell;

Cell init_cell(int id, Vector2 center);

// typedef struct {
//     int width;
//     int height;
//     double spacing;
//     int num_cells;
// } GridProperties;

typedef struct {
    int width;
    int height;
    float spacing;

    int num_cells;
    // GridProperties grid;
    Cell* cells;
} Sim;

Sim init_simulation(int width, int height, float spacing);
void free_simulation(Sim* sim);
void update_simulation(Sim* sim, float delta_time);
Cell* get_cell(Sim* sim, int row, int col);
int get_state(Sim* sim, int row, int col);
void print_average_velocity(Sim* sim);

#endif