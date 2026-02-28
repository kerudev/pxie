#include <stdio.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>


#define CELL_SIZE 40
#define GRID_CELLS 16
#define GRID_SIZE GRID_CELLS * CELL_SIZE

int main(int argc, char *argv[]) {
    InitWindow(800, 600, "test");

    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Move camera on mouse right click
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        // Zoom based on mouse wheel
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            // Get the world point that is under the mouse
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            // Set the offset to where the mouse is
            camera.offset = GetMousePosition();

            // Set the target to match, so that the camera maps the world space point
            // under the cursor to the screen space point under the cursor at any zoom
            camera.target = mouseWorldPos;

            // Zoom increment
            // Uses log scaling to provide consistent zoom speed
            float scale = 0.2f*wheel;
            camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 64.0f);
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
                // Draw vertical lines
                for (int i = 0; i < GRID_CELLS + 1; i++) {
                    DrawLineV(
                        (Vector2){ (float)CELL_SIZE * i, 0 },
                        (Vector2){ (float)CELL_SIZE * i, (float)GRID_SIZE },
                        LIGHTGRAY
                    );
                }

                // Draw horizontal lines
                for (int i = 0; i < GRID_CELLS + 1; i++) {
                    DrawLineV(
                        (Vector2){ 0, (float)CELL_SIZE * i },
                        (Vector2){ (float)GRID_SIZE, (float)CELL_SIZE * i },
                        LIGHTGRAY
                    );
                }

                // Draw coordinates on each cell
                for (int i = 0; i < GRID_CELLS; i++) {
                    for (int j = 0; j < GRID_CELLS; j++) {
                        DrawText(TextFormat("[%i,%i]", i, j), 10 + CELL_SIZE * i, 15 + CELL_SIZE * j, 10, LIGHTGRAY);
                    }
                }

                // Draw a reference circle
                DrawCircle(GRID_SIZE/2, GRID_SIZE/2, 4, MAROON);
            EndMode2D();

            // Draw mouse reference
            //Vector2 mousePos = GetWorldToScreen2D(GetMousePosition(), camera)
            DrawCircleV(GetMousePosition(), 4, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
