#include <stdio.h>
#include <math.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <raymath.h>
#include <rlgl.h>

#include <stb_image.h>
#include <stb_image_write.h>

typedef struct {
    float x;
    float y;
} ScreenOffset;

typedef enum {
    MODE_NORMAL = 0,
    MODE_HIDDEN = 1,
} Mode;

int cell_size = 20;

int grid_cols = 0;
int grid_rows = 0;

int grid_cells = 0;
int grid_size = 0;

Color *grid = NULL;

Color color = { 0, 0, 0, 255 };

ScreenOffset offset = { 0 };
Mode currentMode = MODE_NORMAL;

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
    offset.x = (GetScreenWidth() - grid_size) / 2;
    offset.y = (GetScreenHeight() - grid_size) / 2;
}

void draw_mode_text() {
    if (currentMode == MODE_NORMAL) DrawText("[ H ] Toggle UI (ON)", 10, 10, 14, BLACK);
    if (currentMode == MODE_HIDDEN) DrawText("[ H ] Toggle UI (OFF)", 10, 10, 14, BLACK);
}

void draw_rgb_rect(Rectangle rect, char *name, unsigned char *ref, Color color) {
    float value = *ref;
    GuiSliderBar(rect, name, TextFormat("%d", *ref), &value, 0.0f, 255.0f);
    *ref = (unsigned char)value;

    Rectangle fill = rect;
    fill.width = rect.width * (*ref / 255.0f);

    DrawRectangleRec(fill, color);
}

void draw_rgb_preview() {
    Rectangle area = { 150, 40, 40, 40 };

    DrawRectangleRec(area, color);
    DrawRectangleLinesEx(area, 2, BLACK);
}

void draw_rgb_as_text() {
    // Right shift to remove alpha channel and display color as RGB
    const char *formatted = TextFormat("#%06X", (unsigned int)(ColorToInt(color)) >> 8);

    DrawText(TextFormat("HEX %s", formatted), 6, 95, 14, BLACK);

    int copy_hex = GuiButton((Rectangle){ 115, 90, 20, 20 }, "#16#");
    if (copy_hex) SetClipboardText(formatted);
}

void draw_rgb() {
    draw_rgb_rect((Rectangle){ 16, 40, 110, 12 }, "R", &color.r, RED);
    draw_rgb_rect((Rectangle){ 16, 55, 110, 12 }, "G", &color.g, GREEN);
    draw_rgb_rect((Rectangle){ 16, 70, 110, 12 }, "B", &color.b, BLUE);

    draw_rgb_preview();
    draw_rgb_as_text();
}

void draw_pixel(Camera2D camera) {
    int coord_x = floorf(camera.target.x - offset.x) / cell_size;
    int coord_y = floorf(camera.target.y - offset.y) / cell_size;

    if (coord_x < 0 || coord_x >= grid_cols) return;
    if (coord_y < 0 || coord_y >= grid_rows) return;

    int cell_x = coord_x * cell_size + offset.x;
    int cell_y = coord_y * cell_size + offset.y;

    DrawRectangle(cell_x, cell_y, cell_size, cell_size, color);

    grid[coord_x * grid_rows + coord_y] = color;
}

void draw_grid(Camera2D camera) {
    // Paint each cell
    for (int row = 0; row < grid_rows; row++) {
        for (int col = 0; col < grid_cols; col++) {
            DrawRectangleV(
                (Vector2){ cell_size * row + offset.x, cell_size * col + offset.y },
                (Vector2){ cell_size, cell_size },
                grid[row * grid_rows + col]
            );
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) draw_pixel(camera);

    for (int cell = 0; cell < grid_cols + 1; cell++) {
        float cell_length = cell * cell_size;

        // Draw vertical lines
        DrawLineV(
            (Vector2){ cell_length + offset.x, offset.y },                     // Vector2 startPos
            (Vector2){ cell_length + offset.x, (float)grid_size + offset.y },  // Vector2 endPos
            LIGHTGRAY                                                          // Color color
        );

        // Draw horizontal lines
        DrawLineV(
            (Vector2){ offset.x, cell_length + offset.y },                     // Vector2 startPos
            (Vector2){ (float)grid_size + offset.x, cell_length + offset.y },  // Vector2 endPos
            LIGHTGRAY                                                          // Color color
        );
    }

    // Draw a reference circle
    DrawCircle(grid_size / 2 + offset.x, grid_size / 2 + offset.y, 4, MAROON);
}

void save_png() {
    unsigned char image[grid_size * grid_size * STBI_rgb_alpha];

    for (int row = 0; row < grid_rows; row++) {
        for (int col = 0; col < grid_cols; col++) {
            Color color = grid[row * grid_cols + col];

            for (int cy = 0; cy < cell_size; cy++) {
                for (int cx = 0; cx < cell_size; cx++) {
                    int px = row * cell_size + cx;
                    int py = col * cell_size + cy;

                    int i = (py * grid_size + px) * STBI_rgb_alpha;

                    image[i]     = color.r;
                    image[i + 1] = color.g;
                    image[i + 2] = color.b;
                    image[i + 3] = color.a;
                }
            }
        }
    }

    stbi_write_png("pixel_art.png", grid_size, grid_size, STBI_rgb_alpha, image, grid_size * STBI_rgb_alpha);
}

void draw_save_png_button() {
    int w = GetScreenWidth();
    Rectangle area = { w - 50, 40, 40, 40 };

    int copy_png = GuiButton(area, "#7#");

    if (copy_png) save_png();
}

void draw_ui() {
    draw_rgb();
    draw_save_png_button();
}

void draw_loop(int pixels) {
    // Define global variables
    grid_cols = pixels;
    grid_rows = pixels;

    grid_cells = grid_cols * grid_rows;
    grid_size = grid_cols * cell_size;

    grid = malloc(sizeof(Color) * grid_cells);
    for (int cell = 0; cell < grid_cells; cell++) grid[cell] = WHITE;

    Camera2D camera = { .zoom = 1.0f };

    // Initialize drawing
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    InitWindow(800, 600, "Simple software for pixel art");
    SetWindowMinSize(800, 600);
    HideCursor();
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
}
