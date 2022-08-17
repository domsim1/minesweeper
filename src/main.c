#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_SCALE 32
#define BEGINNER_GRID_WIDTH 9
#define BEGINNER_GRID_HEIGHT 9
#define BEGINNER_MINE_COUNT 10
#define INTERMEDIATE_GRID_WIDTH 16
#define INTERMEDIATE_GRID_HEIGHT 16
#define INTERMEDIATE_MINE_COUNT 40
#define EXPERT_GRID_WIDTH 30
#define EXPERT_GRID_HEIGHT 16
#define EXPERT_MINE_COUNT 99
#define GRID_LINE_SIZE 2
#define VALUE_MASK 0b00001111
#define MINE_MASK 0b00010000
#define FLAG_MASK 0b00100000
#define OPEN_MASK 0b01000000

typedef struct {
  int size;
  unsigned char *data;
  int width;
  int height;
} grid_t;

typedef struct {
  grid_t *grid;
  int scale;
  int mineCount;
  bool gameOver;
  int foundMineCount;
  int placedFlagCount;
} state_t;

state_t *setup(int scale, int width, int height, int mineCount);
grid_t *createGrid(int width, int height, int mineCount);
void gameLoop(state_t *state);
bool isMouseHoveringCell(int x, int y, int scale);
void drawCell(int x, int y, int scale, bool isHover, grid_t *grid, int i);
void openCell(int i, grid_t *grid, bool *gameOver);
void placeFlag(int i, grid_t *grid, int *foundMineCount, int *placedFlagCount);
void reset(state_t *state);
void placeMines(grid_t *grid, int mineCount);

int main() {
  state_t *state = setup(DEFAULT_SCALE, INTERMEDIATE_GRID_WIDTH,
                         INTERMEDIATE_GRID_HEIGHT, INTERMEDIATE_MINE_COUNT);
  int width = (state->grid->width * state->scale) +
              (GRID_LINE_SIZE * state->grid->width);
  int height = (state->grid->height * state->scale) +
               (GRID_LINE_SIZE * state->grid->width);
  InitWindow(width, height, "Minesweeper");
  placeMines(state->grid, state->mineCount);
  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    gameLoop(state);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}

state_t *setup(int scale, int width, int height, int mineCount) {
  state_t *state = malloc(sizeof(state_t));
  if (!state) {
    perror("Error: Failed to allocate state memory.");
    abort();
  }
  state->grid = createGrid(width, height, mineCount);
  state->scale = scale;
  state->mineCount = mineCount;
  state->foundMineCount = 0;
  state->placedFlagCount = 0;
  state->gameOver = false;
  return state;
}

grid_t *createGrid(int width, int height, int mineCount) {
  const int size = width * height;
  grid_t *grid = malloc(sizeof(grid_t));
  if (!grid) {
    perror("Error: Failed to allocate grid memory.");
    abort();
  }
  grid->size = size;
  grid->width = width;
  grid->height = height;
  grid->size = size;
  grid->data = malloc(sizeof(unsigned char) * size);
  if (!grid->data) {
    perror("Error: Failed to allocate grid data memory.");
    abort();
  }
  return grid;
}

void gameLoop(state_t *state) {
  for (int i = 0; i < state->grid->size; ++i) {
    int x = ((i % state->grid->width) * state->scale) +
            (i % state->grid->width) * GRID_LINE_SIZE;
    int y = ((i / state->grid->width) * state->scale) +
            (i / state->grid->height) * GRID_LINE_SIZE;
    bool isHover = isMouseHoveringCell(x, y, state->scale);
    if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      openCell(i, state->grid, &state->gameOver);
    if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
      placeFlag(i, state->grid, &state->foundMineCount,
                &state->placedFlagCount);
    drawCell(x, y, state->scale, isHover, state->grid, i);
  }
  if (state->gameOver) {
    DrawText("YOU LOSE!", 1, 1, state->scale, WHITE);
  }
  if (state->foundMineCount == state->mineCount &&
      state->foundMineCount == state->placedFlagCount) {
    DrawText("YOU WIN!", 1, 1, state->scale, WHITE);
  }
  if (IsKeyPressed(KEY_R))
    reset(state);
}

void drawCell(int x, int y, int scale, bool isHover, grid_t *grid, int i) {
  Color color = BLUE;
  if (isHover)
    color = DARKBLUE;
  if (grid->data[i] & OPEN_MASK) {
    DrawRectangle(x, y, scale, scale, DARKGRAY);
    if (grid->data[i] & MINE_MASK) {
      DrawText("*", x + (scale / 4.5), y + (scale / 12), scale, WHITE);
    } else {
      if (!(grid->data[i] & VALUE_MASK)) {
        DrawRectangle(x, y, scale, scale, DARKGRAY);
        return;
      }
      char valueString[sizeof(char) * 4];
      sprintf(valueString, "%d", grid->data[i] & VALUE_MASK);
      switch (grid->data[i] & VALUE_MASK) {
      case (1):
        DrawText(valueString, x + (scale / 4.5), y + (scale / 12), scale, BLUE);
        break;
      case (2):
        DrawText(valueString, x + (scale / 4.5), y + (scale / 12), scale,
                 GREEN);
        break;
      case (3):
        DrawText(valueString, x + (scale / 4.5), y + (scale / 12), scale, RED);
        break;
      case (4):
        DrawText(valueString, x + (scale / 4.5), y + (scale / 12), scale,
                 DARKBLUE);
        break;
      case (5):
        DrawText(valueString, x + (scale / 4.5), y + (scale / 12), scale,
                 MAROON);
        break;
      case (6):
        DrawText(valueString, x + (scale / 4.5), y + (scale / 12), scale, LIME);
        break;
      case (7):
        DrawText(valueString, x + (scale / 4.5), y + (scale / 12), scale,
                 BLACK);
        break;
      case (8):
        DrawText(valueString, x + (scale / 4.5), y + (scale / 12), scale,
                 LIGHTGRAY);
        break;
      }
    }
  } else if (grid->data[i] & FLAG_MASK) {
    DrawRectangle(x, y, scale, scale, DARKBLUE);
    DrawText("?", x + (scale / 4.5), y + (scale / 12), scale, WHITE);
  } else {
    DrawRectangle(x, y, scale, scale, color);
  }
}

bool isMouseHoveringCell(int x, int y, int scale) {
  Vector2 pos = GetMousePosition();
  if (x >= pos.x - scale && x <= pos.x)
    if (y >= pos.y - scale && y <= pos.y)
      return true;
  return false;
}

void openCell(int i, grid_t *grid, bool *gameOver) {
  if (grid->data[i] & OPEN_MASK)
    return;
  if (grid->data[i] & FLAG_MASK)
    return;
  grid->data[i] |= OPEN_MASK;
  if ((grid->data[i] & VALUE_MASK) != 0)
    return;
  if (grid->data[i] & MINE_MASK) {
    for (int idx = 0; idx < grid->size; ++idx) {
      if (grid->data[idx] & MINE_MASK) {
        grid->data[idx] |= OPEN_MASK;
      }
    }
    *gameOver = true;
    return;
  }
  int isTopRow = i / grid->width == 0;
  int isBottomRow = i / grid->width == grid->width - 1;
  int isLeftCol = i % grid->height == 0;
  int isRightCol = i % grid->height == grid->height - 1;
  if (!isTopRow && !isLeftCol)
    openCell((i - grid->width) - 1, grid, gameOver);
  if (!isTopRow)
    openCell(i - grid->width, grid, gameOver);
  if (!isTopRow && !isRightCol)
    openCell((i - grid->width) + 1, grid, gameOver);
  if (!isLeftCol)
    openCell(i - 1, grid, gameOver);
  if (!isRightCol)
    openCell(i + 1, grid, gameOver);
  if (!isBottomRow && !isLeftCol)
    openCell((i + grid->width) - 1, grid, gameOver);
  if (!isBottomRow)
    openCell(i + grid->width, grid, gameOver);
  if (!isBottomRow && !isRightCol)
    openCell((i + grid->width) + 1, grid, gameOver);
}

void placeFlag(int i, grid_t *grid, int *foundMineCount, int *placedFlagCount) {
  if (grid->data[i] & OPEN_MASK)
    return;
  if (grid->data[i] & FLAG_MASK) {
    if (grid->data[i] & MINE_MASK)
      *foundMineCount -= 1;
    *placedFlagCount -= 1;
  }
  if (!(grid->data[i] & FLAG_MASK)) {
    if (grid->data[i] & MINE_MASK)
      *foundMineCount += 1;
    *placedFlagCount += 1;
  }
  grid->data[i] ^= FLAG_MASK;
}

void placeMines(grid_t *grid, int mineCount) {
  for (int i = 0; i < grid->size; ++i) {
    grid->data[i] = 0;
  }
  int placedMines = 0;
  while (placedMines < mineCount) {
    int i = GetRandomValue(0, grid->size - 1);
    if (grid->data[i] & MINE_MASK)
      continue;
    grid->data[i] = MINE_MASK;
    ++placedMines;
  }
  for (int i = 0; i < grid->size; ++i) {
    if (grid->data[i] & MINE_MASK)
      continue;
    int isTopRow = i / grid->width == 0;
    int isBottomRow = i / grid->width == grid->width - 1;
    int isLeftCol = i % grid->height == 0;
    int isRightCol = i % grid->height == grid->height - 1;
    if (!isTopRow && !isLeftCol)
      if (grid->data[(i - grid->width) - 1] & MINE_MASK)
        grid->data[i] += 1;
    if (!isTopRow)
      if (grid->data[i - grid->width] & MINE_MASK)
        grid->data[i] += 1;
    if (!isTopRow && !isRightCol)
      if (grid->data[(i - grid->width) + 1] & MINE_MASK)
        grid->data[i] += 1;
    if (!isLeftCol)
      if (grid->data[i - 1] & MINE_MASK)
        grid->data[i] += 1;
    if (!isRightCol)
      if (grid->data[i + 1] & MINE_MASK)
        grid->data[i] += 1;
    if (!isBottomRow && !isLeftCol)
      if (grid->data[(i + grid->width) - 1] & MINE_MASK)
        grid->data[i] += 1;
    if (!isBottomRow)
      if (grid->data[i + grid->width] & MINE_MASK)
        grid->data[i] += 1;
    if (!isBottomRow && !isRightCol)
      if (grid->data[(i + grid->width) + 1] & MINE_MASK)
        grid->data[i] += 1;
  }
}

void reset(state_t *state) {
  state->foundMineCount = 0;
  state->placedFlagCount = 0;
  state->gameOver = false;
  placeMines(state->grid, state->mineCount);
}
