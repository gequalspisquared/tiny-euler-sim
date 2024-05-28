#include "sim.h"

#include <stdio.h>

#include "raymath.h"

static void apply_forces(Sim* sim, float delta_time);
static void projection(Sim* sim);  // Enforce incompressibility;
static void advection(Sim* sim, float delta_time);  // Propogate fluid props

Cell init_cell(int id, Vector2 center) {
    Cell cell;
    cell.fluid = init_fluid();
    cell.id = id;
    cell.free = 1;
    cell.center = center;

    return cell;
}

Sim init_simulation(int width, int height, float spacing) {
    Sim sim;
    sim.width = width;
    sim.height = height;
    sim.spacing = spacing;

    sim.num_cells = width * height;
    sim.cells = malloc(sim.num_cells * sizeof(Cell));
    for (int i = 0; i < sim.num_cells; i++) {
        int row = i % width;
        int col = i / width;
        Vector2 center = {(float)row + 0.5f, (float)col + 0.5f};
        center = Vector2Scale(center, spacing);

        sim.cells[i] = init_cell(i, center);
    }

    return sim;
}

void free_simulation(Sim* sim) { free(sim->cells); }

void update_simulation(Sim* sim, float delta_time) {
    apply_forces(sim, delta_time);
    projection(sim);
    // advection(sim, delta_time);
}

Cell* get_cell(Sim* sim, int row, int col) {
    // Boundary conditions
    if (row < 0 || row >= sim->height || col < 0 || col >= sim->width) {
        return NULL;
    }

    return &sim->cells[row * sim->width + col];
}

int get_state(Sim* sim, int row, int col) {
    // Boundary conditions
    // for now, the boundaries are solid walls
    // if (row < 0 || row >= sim->height || col < 0 || col >= sim->width) {
    //     return 0;
    // }

    Cell* cell = get_cell(sim, row, col);
    if (cell != NULL) {
        return cell->free;
    }

    return 0;
}

void print_average_velocity(Sim* sim) {
    Vector2 total_velocity = {0.0f};
    for (int i = 0; i < sim->num_cells; i++) {
        total_velocity =
            Vector2Add(total_velocity, sim->cells[i].fluid.velocity);
    }
    total_velocity = Vector2Scale(total_velocity, 1.0f / sim->num_cells);

    printf("Average fluid velocity: {%f, %f}\n", total_velocity.x,
           total_velocity.y);
}

static void apply_forces(Sim* sim, float delta_time) {
    // Only gravity at the moment
    // static const float gravity_acceleration = -9.81;
    Vector2 gravity_force = {0.0f, -9.81f / sim->spacing};

    Vector2 gravity_dv = Vector2Scale(gravity_force, delta_time);
    for (int i = 0; i < sim->num_cells; i++) {
        sim->cells[i].fluid.velocity =
            Vector2Add(sim->cells[i].fluid.velocity, gravity_dv);
    }
}

// static float compute_divergence()

static void projection(Sim* sim) {
    float max_divergence = 1.0f;
    int max_iterations = 1000;
    int i = 0;
    while (max_divergence > 0.01f && i < max_iterations) {
        max_divergence = 0.0f;
        for (int i = 0; i < sim->num_cells; i++) {
            int row = i % sim->width;
            int col = i / sim->width;
            Cell* cell = get_cell(sim, row, col);

            Cell* up = get_cell(sim, row, col + 1);
            Cell* down = get_cell(sim, row, col - 1);
            Cell* left = get_cell(sim, row - 1, col);
            Cell* right = get_cell(sim, row + 1, col);

            float divergence = 0;
            int states = 0;
            if (up != NULL) {
                divergence -= up->fluid.velocity.y;
                states += up->free;
            }
            if (down != NULL) {
                divergence += down->fluid.velocity.y;
                states += down->free;
            }
            if (left != NULL) {
                divergence += left->fluid.velocity.x;
                states += left->free;
            }
            if (right != NULL) {
                divergence -= right->fluid.velocity.x;
                states += right->free;
            }

            if (divergence > max_divergence) {
                max_divergence = divergence;
            }
            divergence *= 1.9f;

            if (up != NULL) {
                up->fluid.velocity.y += divergence * up->free / states;
            }
            if (down != NULL) {
                down->fluid.velocity.y -= divergence * down->free / states;
            }
            if (left != NULL) {
                left->fluid.velocity.x -= divergence * left->free / states;
            }
            if (right != NULL) {
                right->fluid.velocity.x += divergence * right->free / states;
            }
        }

        i++;
        // if (i == max_iterations) {
        //     printf("Hit max iterations in projection!\n");
        // }
    }

    // printf("Projection took %d iterations\n", i);
}

static Vector2 sample_velocity(Sim* sim, Vector2 position);

static void advection(Sim* sim, float delta_time) {
    // Update old velocities
    for (int i = 0; i < sim->num_cells; i++) {
        sim->cells[i].fluid.old_velocity = sim->cells[i].fluid.velocity;
    }

    // Advect
    for (int i = 0; i < sim->num_cells; i++) {
        int row = i % sim->width;
        int col = i / sim->width;
        Cell* cell = get_cell(sim, row, col);

        // Vector2 xc = {(float)row + 0.5f, (float)col + 0.5f};
        Vector2 xc = cell->center;
        Vector2 dx =
            Vector2Scale(cell->fluid.old_velocity, delta_time * sim->spacing);
        Vector2 xd = Vector2Subtract(xc, dx);

        Vector2 sampled_velocity = sample_velocity(sim, xd);
        cell->fluid.velocity = sampled_velocity;
    }
}

static Vector2 get_center(Sim* sim, int row, int col) {
    Vector2 center = {(float)row + 0.5f, (float)col + 0.5f};
    center = Vector2Scale(center, sim->spacing);

    return center;
}

static Vector2 sample_velocity(Sim* sim, Vector2 position) {
    Vector2 interpolated_velocity = {0.0f};
    int row = (int)position.x;
    int col = (int)position.y;
    Vector2 xc = {(float)row + 0.5f, (float)col + 0.5f};
    xc = Vector2Scale(xc, sim->spacing);
    Vector2 xd = Vector2Subtract(xc, position);

    Cell* ne;
    Cell* se;
    Cell* sw;
    Cell* nw;
    float x, y;
    if (xd.x > 0.0f && xd.y > 0.0f) {  // NE
        ne = get_cell(sim, row, col);
        se = get_cell(sim, row - 1, col);
        sw = get_cell(sim, row - 1, col - 1);
        nw = get_cell(sim, row, col - 1);

        Vector2 origin = get_center(sim, row, col - 1);
        x = position.x - origin.x;
        y = position.y - origin.y;
    } else if (xd.x > 0.0f && xd.y < 0.0f) {  // SE
        ne = get_cell(sim, row + 1, col);
        se = get_cell(sim, row, col);
        sw = get_cell(sim, row, col - 1);
        nw = get_cell(sim, row + 1, col - 1);

        Vector2 origin = get_center(sim, row + 1, col - 1);
        x = position.x - origin.x;
        y = position.y - origin.y;
    } else if (xd.x < 0.0f && xd.y < 0.0f) {  // SW
        ne = get_cell(sim, row + 1, col + 1);
        se = get_cell(sim, row, col + 1);
        sw = get_cell(sim, row, col);
        nw = get_cell(sim, row + 1, col);

        Vector2 origin = get_center(sim, row + 1, col);
        x = position.x - origin.x;
        y = position.y - origin.y;
    } else {  // NW
        ne = get_cell(sim, row, col + 1);
        se = get_cell(sim, row - 1, col + 1);
        sw = get_cell(sim, row - 1, col);
        nw = get_cell(sim, row, col);

        Vector2 origin = get_center(sim, row, col);
        x = position.x - origin.x;
        y = position.y - origin.y;
    }

    float h = sim->spacing;
    float w_ne = (x / h) * (1.0f - y / h);
    float w_se = (x / h) * (y / h);
    float w_sw = (1.0f - x / h) * (1.0f - y / h);
    float w_nw = (1.0f - x / h) * (y / h);

    if (ne != NULL) {
        Vector2 weighted = Vector2Scale(ne->fluid.velocity, w_ne);
        interpolated_velocity = Vector2Add(interpolated_velocity, weighted);
    }
    if (se != NULL) {
        Vector2 weighted = Vector2Scale(se->fluid.velocity, w_se);
        interpolated_velocity = Vector2Add(interpolated_velocity, weighted);
    }
    if (sw != NULL) {
        Vector2 weighted = Vector2Scale(sw->fluid.velocity, w_sw);
        interpolated_velocity = Vector2Add(interpolated_velocity, weighted);
    }
    if (nw != NULL) {
        Vector2 weighted = Vector2Scale(nw->fluid.velocity, w_nw);
        interpolated_velocity = Vector2Add(interpolated_velocity, weighted);
    }

    return interpolated_velocity;
}