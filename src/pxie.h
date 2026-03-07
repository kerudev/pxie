#include <stdio.h>
#include <math.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <raymath.h>
#include <rlgl.h>

#include <stb_image.h>
#include <stb_image_write.h>

#define CELL_SIZE 20
#define GRID_COLS 16
#define GRID_ROWS GRID_COLS

#define GRID_CELLS GRID_COLS * GRID_ROWS
#define GRID_SIZE GRID_COLS * CELL_SIZE

typedef struct {
    float x;
    float y;
} ScreenOffset;

typedef enum {
    MODE_NORMAL = 0,
    MODE_HIDDEN = 1,
} Mode;

Color Grid[GRID_CELLS] = { 0 };

ScreenOffset offset = { 0 };
Mode currentMode = MODE_NORMAL;

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

Color get_current_color() {
    return (Color){ r, g, b, 255 };
}

void draw_mode_text() {
    if (currentMode == MODE_NORMAL) DrawText("[ H ] Toggle UI (ON)", 10, 10, 14, BLACK);
    if (currentMode == MODE_HIDDEN) DrawText("[ H ] Toggle UI (OFF)", 10, 10, 14, BLACK);
}

void draw_rgb_rect(Rectangle rect, char *name, float *ref, Color color) {
    GuiSliderBar(rect, name, TextFormat("%d", (int)*ref), ref, 0.0f, 255.0f);

    Rectangle fill = rect;
    fill.width = rect.width * (*ref / 255.0f);

    DrawRectangleRec(fill, color);
}

void draw_rgb_preview() {
    Rectangle area = { 150, 40, 40, 40 };

    DrawRectangleRec(area, get_current_color());
    DrawRectangleLinesEx(area, 2, BLACK);
}

void draw_rgb_as_text() {
    unsigned int color = (unsigned int)(ColorToInt(get_current_color()));
    const char *formatted = TextFormat("#%06X", color >> 8);

    // Right shift to remove alpha channel and display color as RGB
    DrawText(TextFormat("HEX %s", formatted), 6, 95, 14, BLACK);

    int copy_hex = GuiButton((Rectangle){ 115, 90, 20, 20 }, "#16#");

    if (copy_hex) SetClipboardText(formatted);
}

void draw_rgb() {
    draw_rgb_rect((Rectangle){ 16, 40, 110, 12 }, "R", &r, RED);
    draw_rgb_rect((Rectangle){ 16, 55, 110, 12 }, "G", &g, GREEN);
    draw_rgb_rect((Rectangle){ 16, 70, 110, 12 }, "B", &b, BLUE);

    draw_rgb_preview();
    draw_rgb_as_text();
}

void draw_pixel(Camera2D camera) {
    int coord_x = floorf(camera.target.x - offset.x) / CELL_SIZE;
    int coord_y = floorf(camera.target.y - offset.y) / CELL_SIZE;

    if (coord_x < 0 || coord_x >= GRID_COLS) return;
    if (coord_y < 0 || coord_y >= GRID_ROWS) return;

    int cell_x = coord_x * CELL_SIZE + offset.x;
    int cell_y = coord_y * CELL_SIZE + offset.y;

    Color color = get_current_color();

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

void save_png() {
    unsigned char image[GRID_SIZE * GRID_SIZE * STBI_rgb_alpha] = { 0 };

    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            Color color = Grid[row * GRID_COLS + col];

            for (int cy = 0; cy < CELL_SIZE; cy++) {
                for (int cx = 0; cx < CELL_SIZE; cx++) {
                    int px = row * CELL_SIZE + cx;
                    int py = col * CELL_SIZE + cy;

                    int i = (py * GRID_SIZE + px) * STBI_rgb_alpha;

                    image[i]     = color.r;
                    image[i + 1] = color.g;
                    image[i + 2] = color.b;
                    image[i + 3] = color.a;
                }
            }
        }
    }

    stbi_write_png("pixel_art.png", GRID_SIZE, GRID_SIZE, STBI_rgb_alpha, image, GRID_SIZE * STBI_rgb_alpha);
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
