//-------------------------------------------------
// Source for this project:
// https://youtu.be/kT-Mz87-HcQ?si=Mp8TjgPSoIvKXzHR
// ------------------------------------------------

#include <assert.h> // for assert
#include <stdio.h>
#include <stdint.h> // for type uint32_t
#include <stdlib.h> // for exit()
#include <string.h> // for strerror
#include <time.h>   // initialize random seed
#include <errno.h>  // for errno

#define WIDTH  800
#define HEIGTH 600
#define SEEDS_COUNT 10
#define OUTPUT_FILE_PATH "output.ppm"

// define color hex: 0xAABBGGRR (Alpha>Transparenz; Blue; Green; Red)
// Stored in Memory: RR GG BB AA

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0xFF000000
#define COLOR_RED   0xFF0000FF
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE  0xFFFF0000

#define FOREST_MOSS   0xFF009966
#define YELLOW_GREEN  0xFF33CC99
#define LIME_CREAM    0xFF66EECC
#define BALTIC_BLUE   0xFF996600
#define BLUE_BELL     0xFFCC9933
#define DARK_RASPERRY 0xFF660099
#define RASPERRY_PLUM 0xFF9933CC
#define BLAZE_ORANGE  0xFF0066FF
#define AMBER_GLOW    0xFF0099FF
#define BRIGHT_AMBER  0xFF00CCFF

#define BACKGROUND_COLOR 0xFF181818

#define SEED_MARKER_RADIUS 7
#define SEED_MARKER_COLOR COLOR_BLACK

typedef uint32_t Color32; // Fakultativ, reine Umbenennung

// struct Point für einen einzelnen Punkt im Bild
typedef struct {
  int x, y;
}Point;

// Allocate Memory for 2D-Array mit Namen image
static Color32 image[HEIGTH][WIDTH];

// Allocate Memory for SEEDS_COUNT seeds
static Point seeds[SEEDS_COUNT];

static Color32 palette[] = {
  FOREST_MOSS,
  YELLOW_GREEN,
  LIME_CREAM,
  BALTIC_BLUE,
  BLUE_BELL,
  DARK_RASPERRY,
  RASPERRY_PLUM,
  BLAZE_ORANGE,
  AMBER_GLOW,
  BRIGHT_AMBER
};
#define palette_count (sizeof(palette)/sizeof(palette[0]))

//---------------------------------------------------------
// Funktion füllt Pixel des Bilds mit einer bestimten Farbe
// --------------------------------------------------------
void fill_image(Color32 color) {
  for (size_t y = 0; y < HEIGTH; y++) {
    for (size_t x = 0; x < WIDTH; x++) {
      image[y][x] = color;
    }
  }
}


//--------------------------------------------------------
// Funktion speichert Bild als PPM-Datei
// -------------------------------------------------------
void save_image_as_ppm(const char *file_path) {
  FILE *f = fopen(file_path, "wb"); // wb = write binary
  if (f == NULL) {
    fprintf(stderr, "ERROR: could not write into file %s: %s\n", file_path, strerror(errno));
    exit(1);
  }
  fprintf(f, "P6\n%d %d 255\n", WIDTH, HEIGTH);
  for (size_t y = 0; y < HEIGTH; ++y) {
    for (size_t x = 0; x < WIDTH; ++x) {
      uint32_t pixel = image[y][x]; // get color of pixel
      uint8_t bytes[3] = { // PPM stores colors in 3 bytes=3*8 Bits (RGB)
        (pixel&0x0000FF)>>8*0, // extract R, store in 1st byte
                               /// extraction begins on the right
                               /// works also with pixel&0xFF
                               /// >> means: shift to the right
        (pixel&0x00FF00)>>8*1, // extract G, store in 2nd byte
        (pixel&0xFF0000)>>8*2, // extract B, store in 3rd byte
      };
      fwrite(bytes, sizeof(bytes), 1, f);
      assert(!ferror(f));
    }
  }

  int ret = fclose(f);
  assert(ret == 0);
}


void generate_random_seeds() {
  srand(time(NULL));
  for (size_t i = 0; i < SEEDS_COUNT; i++) {
    seeds[i].x = rand()%WIDTH;
    seeds[i].y = rand()%HEIGTH;
  }
}


void fill_circle(int cx, int cy, int radius, uint32_t color) {

  // .......   >> define rectangle (*) arount Point (@)
  // ..***..   >> are the points inside radius?
  // ..*@*..   >> Formula: is dx^2 + dy^2 <= radius^2
  // ..***..   >> if yes: color
  // .......   >> if not: not color

  int x0 = cx - radius;
  int y0 = cy - radius;
  int x1 = cx + radius;
  int y1 = cy + radius;
  for (int x = x0; x <= x1; x++) {
    if (0 <= x && x < WIDTH) { // Check, ob Punkt ausserhalb Bild
      for (int y = y0; y <= y1; y++) {
        if (0 <= y && y < HEIGTH) {// Check, ob Punkt ausserhalb Bild
          int dx = cx - x; // dx = "distance" between x and center x
          int dy = cy - y; // dy = "distance" between y and center y
          if (dx*dx + dy*dy <= radius*radius) {
            image[y][x] = color;
          }
        }
      }
    }
  }
}


void render_seed_markers() {
  for (size_t i = 0; i < SEEDS_COUNT; i++) {
    fill_circle(seeds[i].x, seeds[i].y, SEED_MARKER_RADIUS, SEED_MARKER_COLOR);
  }
}


int sqr_dist(int x1, int y1, int x2, int y2) {
  int dx = x1 - x2;
  int dy = y1 - y2;
  return dx*dx + dy*dy;
}


void render_voronoi() {
  for (int y = 0; y < HEIGTH; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int j = 0;
      for (size_t i = 1; i < SEEDS_COUNT; i++) {
        if (sqr_dist(seeds[i].x, seeds[i].y, x, y) < sqr_dist(seeds[j].x, seeds[j].y, x, y))
            j = i;
      }
      image[y][x] = palette[j%palette_count]; // wrap-around: starts from
                                              // 0 after palette_count
    }
  }
}


int main() {
  fill_image(BACKGROUND_COLOR);
  generate_random_seeds();
  render_voronoi();
  render_seed_markers();
  save_image_as_ppm(OUTPUT_FILE_PATH);
  return 0;
}
