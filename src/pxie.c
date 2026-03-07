#include "pxie.h"

int main(int argc, char *argv[]) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(800, 600, "Simple software for pixel art");
    SetWindowMinSize(800, 600);
    HideCursor();

    init_grid();

    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        set_screen_offset();

        if (IsKeyPressed(KEY_H)) currentMode = !currentMode;

        // Move camera on mouse right click
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) move_camera(&camera);

        update_camera_state(&camera);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
                draw_grid(camera);
            EndMode2D();

            draw_mode_text();

            if (currentMode == MODE_NORMAL) draw_ui();

            // Draw mouse reference
            if (IsCursorOnScreen()) DrawCircleV(GetMousePosition(), 4, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
