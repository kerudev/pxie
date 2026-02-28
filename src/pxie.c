#include <stdio.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>


#define CELL_SIZE 40
#define GRID_CELLS 16
#define GRID_SIZE GRID_CELLS * CELL_SIZE

void drawGrid() {
    int screen_w = GetScreenWidth();
    int screen_h = GetScreenHeight();

    float x_offset = (screen_w - GRID_SIZE) / 2;
    float y_offset = (screen_h - GRID_SIZE) / 2;

    for (int i = 0; i < GRID_CELLS + 1; i++) {
        float cell_length = CELL_SIZE * i;

        // Draw vertical lines
        DrawLineV(
            (Vector2){ cell_length + x_offset, y_offset },                     // Vector2 startPos
            (Vector2){ cell_length + x_offset, (float)GRID_SIZE + y_offset },  // Vector2 endPos
            LIGHTGRAY                                                          // Color color
        );
        
        // Draw horizontal lines
        DrawLineV(
            (Vector2){ x_offset, cell_length + y_offset },                     // Vector2 startPos
            (Vector2){ (float)GRID_SIZE + x_offset, cell_length + y_offset },  // Vector2 endPos
            LIGHTGRAY                                                          // Color color
        );
    }

    // Draw coordinates on each cell
    for (int i = 0; i < GRID_CELLS; i++) {
        for (int j = 0; j < GRID_CELLS; j++) {
            DrawText(
                TextFormat("[%i,%i]", i, j),    // const char *text
                CELL_SIZE * i + 10 + x_offset,  // int posX
                CELL_SIZE * j + 15 + y_offset,  // int posY
                10,                             // int fontSize
                LIGHTGRAY                       // Color color
            );
        }
    }

    // Draw a reference circle
    DrawCircle(GRID_SIZE / 2 + x_offset, GRID_SIZE / 2 + y_offset, 4, MAROON);
}

int main(int argc, char *argv[]) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "pxie - Simple software for pixel art");

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
                drawGrid();
            EndMode2D();

            // Draw mouse reference
            //Vector2 mousePos = GetWorldToScreen2D(GetMousePosition(), camera)
            DrawCircleV(GetMousePosition(), 4, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
