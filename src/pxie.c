#include <stdio.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define CELL_SIZE 20
#define GRID_COLS 16
#define GRID_ROWS GRID_COLS
#define GRID_CELLS GRID_COLS * GRID_ROWS
#define GRID_SIZE GRID_COLS * CELL_SIZE

typedef struct {
    float x;
    float y;
} ScreenOffset;

Color Grid[GRID_CELLS] = { 0 };
ScreenOffset offset = { 0 };

float r = 0.0f;
float g = 0.0f;
float b = 0.0f;

void init_grid() {
    for (int cell = 0; cell < GRID_CELLS; cell++) {
        Grid[cell] = WHITE;
    }
}

void move_camera(Camera2D *camera) {
    Vector2 delta = GetMouseDelta();
    delta = Vector2Scale(delta, -1.0f / camera->zoom);
    camera->target = Vector2Add(camera->target, delta);
}

void update_camera_state(Camera2D *camera) {
    // Get the world point that is under the mouse
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);

    // Set the offset to where the mouse is
    camera->offset = GetMousePosition();

    // Set the target to match, so that the camera maps the world space point
    // under the cursor to the screen space point under the cursor at any zoom
    camera->target = mouseWorldPos;

    // Zoom increment (uses log scaling for consistent zoom speed)
    float wheel = GetMouseWheelMove();
    float scale = 0.2f * wheel;
    camera->zoom = Clamp(expf(logf(camera->zoom) + scale), 0.125f, 64.0f);
}

void set_screen_offset() {
    offset.x = (GetScreenWidth() - GRID_SIZE) / 2;
    offset.y = (GetScreenHeight() - GRID_SIZE) / 2;
}

void draw_rgb_rect(Rectangle rect, char *name, float *ref, Color color) {
    GuiSliderBar(rect, name, TextFormat("%d", (int)*ref), ref, 0.0f, 255.0f);

    Rectangle fill = rect;
    fill.width = rect.width * (*ref / 255.0f);

    DrawRectangleRec(fill, color);
}

void draw_rgb() {
    draw_rgb_rect((Rectangle){ 16, 10, 140, 16 }, "R", &r, RED);
    draw_rgb_rect((Rectangle){ 16, 30, 140, 16 }, "G", &g, GREEN);
    draw_rgb_rect((Rectangle){ 16, 50, 140, 16 }, "B", &b, BLUE);
}

void draw_pixel(Camera2D camera) {
    Vector2 mouse = GetMousePosition();

    int coord_x = floorf(camera.target.x - offset.x) / CELL_SIZE;
    int coord_y = floorf(camera.target.y - offset.y) / CELL_SIZE;

    if (coord_x < 0 || coord_x >= GRID_COLS) return;
    if (coord_y < 0 || coord_y >= GRID_ROWS) return;

    int cell_x = coord_x * CELL_SIZE + offset.x;
    int cell_y = coord_y * CELL_SIZE + offset.y;

    Color color = { r, g, b, 255 };

    DrawRectangle(cell_x, cell_y, CELL_SIZE, CELL_SIZE, color);

    Grid[coord_x * GRID_ROWS + coord_y] = color;
}

void draw_grid(Camera2D camera) {
    // Paint each cell
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            DrawRectangleV(
                (Vector2){ CELL_SIZE * row + offset.x, CELL_SIZE * col + offset.y },
                (Vector2){ CELL_SIZE, CELL_SIZE },
                Grid[row * GRID_ROWS + col]
            );
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) draw_pixel(camera);

    for (int cell = 0; cell < GRID_COLS + 1; cell++) {
        float cell_length = cell * CELL_SIZE;

        // Draw vertical lines
        DrawLineV(
            (Vector2){ cell_length + offset.x, offset.y },                     // Vector2 startPos
            (Vector2){ cell_length + offset.x, (float)GRID_SIZE + offset.y },  // Vector2 endPos
            LIGHTGRAY                                                          // Color color
        );

        // Draw horizontal lines
        DrawLineV(
            (Vector2){ offset.x, cell_length + offset.y },                     // Vector2 startPos
            (Vector2){ (float)GRID_SIZE + offset.x, cell_length + offset.y },  // Vector2 endPos
            LIGHTGRAY                                                          // Color color
        );
    }

    // Draw a reference circle
    DrawCircle(GRID_SIZE / 2 + offset.x, GRID_SIZE / 2 + offset.y, 4, MAROON);
}

int main(int argc, char *argv[]) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(800, 600, "Simple software for pixel art");
    HideCursor();

    init_grid();

    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        set_screen_offset();

        // Move camera on mouse right click
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) move_camera(&camera);

        update_camera_state(&camera);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
                draw_grid(camera);
            EndMode2D();

            draw_rgb();

            // Draw mouse reference
            DrawCircleV(GetMousePosition(), 4, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
