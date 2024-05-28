#include "raylib.h"
#include "raymath.h"
#include "sim.h"

int main(void) {
    const int window_width = 1600;
    const int window_height = 1600;

    InitWindow(window_width, window_height, "fluid sim test");

    const int simulation_width = 64;
    const int simulation_height = 64;

    Sim sim = init_simulation(simulation_width, simulation_height, 1.0);

    RenderTexture2D target =
        LoadRenderTexture(simulation_width, simulation_height);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    float scale_width = (float)GetScreenWidth() / simulation_width;
    float scale_height = (float)GetScreenHeight() / simulation_height;
    // get minimum
    float scale = (scale_width < scale_height) ? scale_width : scale_height;

    // row-major
    Color grid_colors[64 * 64] = {0};

    int frame = 0;
    int fps = 0;
    // float average_fps = 0.0f;
    while (!WindowShouldClose()) {
        frame++;
        fps += GetFPS();
        float delta_time = GetFrameTime();
        // printf("Frame time: %f", delta_time);
        // average_fps += delta_time;
        update_simulation(&sim, GetFrameTime());

        // if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        //     update_simulation(&sim, 1.0f / 60);
        // }

        // SetWindowTitle("Fluid Sim Test")
        if (frame % 10 == 0) {
            printf("Average FPS: %.1f\n", fps / 10.0f);
            print_average_velocity(&sim);
            printf("Frame time: %f\n", delta_time);
            fps = 0;
        }
        Vector2 mouse = GetMousePosition();
        Vector2 virtualMouse = {0};
        virtualMouse.x =
            (mouse.x - (GetScreenWidth() - (simulation_width * scale)) * 0.5f) /
            scale;
        virtualMouse.y =
            (mouse.y -
             (GetScreenHeight() - (simulation_height * scale)) * 0.5f) /
            scale;
        virtualMouse = Vector2Clamp(
            virtualMouse, (Vector2){0, 0},
            (Vector2){(float)simulation_width, (float)simulation_height});

        // printf("VirtualMouse coords: {%f, %f}\n", virtualMouse.x,
        //        virtualMouse.y);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            static int old_col = 0;
            static int old_row = 0;
            int col = (int)(virtualMouse.x);
            int row = (int)(virtualMouse.y);
            if (old_col != col || old_row != row) {
                grid_colors[row * 64 + col] =
                    (Color){GetRandomValue(100, 250), GetRandomValue(50, 150),
                            GetRandomValue(10, 100), 255};

                old_col = col;
                old_row = row;
            }
        }

        BeginTextureMode(target);
        ClearBackground(RAYWHITE);  // Clear render texture background color

        // for (int i = 0; i < 64 * 64; i++) {
        //     int row = i % 64;
        //     int col = i / 64;
        //     DrawPixel(row, col, grid_colors[i]);
        // }

        for (int i = 0; i < sim.num_cells; i++) {
            Vector2 velocity = sim.cells[i].fluid.velocity;
            float max = (fabs(velocity.x) > fabs(velocity.y))
                            ? fabs(velocity.x)
                            : fabs(velocity.y);
            Color color = (Color){(int)(255 * fabs(velocity.x) / max),
                                  (int)(255 * fabs(velocity.y) / max), 0, 255};

            int row = i % simulation_width;
            int col = i / simulation_width;
            DrawPixel(row, col, color);
        }

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        // Draw render texture to screen, properly scaled
        DrawTexturePro(
            target.texture,
            (Rectangle){0.0f, 0.0f, (float)target.texture.width,
                        (float)-target.texture.height},
            (Rectangle){
                (GetScreenWidth() - ((float)simulation_width * scale)) * 0.5f,
                (GetScreenHeight() - ((float)simulation_height * scale)) * 0.5f,
                (float)simulation_width * scale,
                (float)simulation_height * scale},
            (Vector2){0, 0}, 0.0f, WHITE);
        EndDrawing();
    }

    free_simulation(&sim);

    return 0;
}