/* points.c ... */

/*
 * This example creates an SDL window and renderer, and then draws some points
 * to it every frame.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static void rotate_point(float cx, float cy, float* x, float* y, float angle)
{
    float s = SDL_sinf(angle);
    float c = SDL_cosf(angle);

    // translate point back to origin
    float tx = *x - cx;
    float ty = *y - cy;

    // rotate
    float rx = tx * c - ty * s;
    float ry = tx * s + ty * c;

    // translate back
    *x = rx + cx;
    *y = ry + cy;
}

 /* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static Uint64 last_time = 0;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define NUM_POINTS 500
#define MIN_PIXELS_PER_SECOND 30  /* move at least this many pixels per second. */
#define MAX_PIXELS_PER_SECOND 960  /* move this many pixels per second at most. */

/* (track everything as parallel arrays instead of a array of structs,
   so we can pass the coordinates to the renderer in a single function call.) */

   /* Points are plotted as a set of X and Y coordinates.
      (0, 0) is the top left of the window, and larger numbers go down
      and to the right. This isn't how geometry works, but this is pretty
      standard in 2D graphics. */
static SDL_FPoint points[NUM_POINTS];
static float point_speeds[NUM_POINTS];

static SDL_Texture* glowTex = NULL;

static SDL_Texture* create_glow_texture(SDL_Renderer* r, int radius)
{
    SDL_Texture* tex = SDL_CreateTexture(
        r,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        radius * 2,
        radius * 2
    );

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_ADD);
    SDL_SetRenderTarget(r, tex);

    for (int y = 0; y < radius * 2; y++) {
        for (int x = 0; x < radius * 2; x++) {
            float dx = x - radius;
            float dy = y - radius;
            float dist = SDL_sqrtf(dx * dx + dy * dy) / radius;
            if (dist > 1.0f) dist = 1.0f;
            Uint8 alpha = (Uint8)((1.0f - dist) * 255);

            SDL_SetRenderDrawColor(r, 255, 255, 200, alpha);
            SDL_RenderPoint(r, x, y);
        }
    }

   

    SDL_SetRenderTarget(r, NULL);
    return tex;
}



/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    int i;

    SDL_SetAppMetadata("Example Renderer Points", "1.0", "com.example.renderer-points");

    

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/renderer/points", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    glowTex = create_glow_texture(renderer, 40);

    /* set up the data for a bunch of points. */
    for (i = 0; i < SDL_arraysize(points); i++) {
        points[i].x = SDL_randf() * ((float)WINDOW_WIDTH);
        points[i].y = SDL_randf() * ((float)WINDOW_HEIGHT);
        point_speeds[i] = MIN_PIXELS_PER_SECOND + (SDL_randf() * (MAX_PIXELS_PER_SECOND - MIN_PIXELS_PER_SECOND));
    }

    last_time = SDL_GetTicks();

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    const Uint64 now = SDL_GetTicks();
    const float elapsed = ((float)(now - last_time)) / 1000.0f;
    int i;

    // Move stars
    for (i = 0; i < SDL_arraysize(points); i++) {
        const float distance = elapsed * point_speeds[i];
        points[i].x += distance;
        points[i].y += distance;

        if ((points[i].x >= WINDOW_WIDTH) || (points[i].y >= WINDOW_HEIGHT)) {
            if (SDL_rand(2)) {
                points[i].x = SDL_randf() * WINDOW_WIDTH;
                points[i].y = 0.0f;
            }
            else {
                points[i].x = 0.0f;
                points[i].y = SDL_randf() * WINDOW_HEIGHT;
            }
            point_speeds[i] = MIN_PIXELS_PER_SECOND + (SDL_randf() * (MAX_PIXELS_PER_SECOND - MIN_PIXELS_PER_SECOND));
        }
    }

    last_time = now;

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw stars
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderPoints(renderer, points, SDL_arraysize(points));

    // Ship center
    float cx = WINDOW_WIDTH * 0.5f;
    float cy = WINDOW_HEIGHT * 0.5f;
    float size = 20.0f;

    // Ship vertices (triangle)
    float x1 = cx;        float y1 = cy - size;
    float x2 = cx - size; float y2 = cy + size;
    float x3 = cx + size; float y3 = cy + size;

    // Rotation + wobble
    float base_angle = -SDL_PI_F / 4.0f;
    float wobble = SDL_sinf(now * 0.005f) * 0.1f;
    float angle = base_angle + wobble;

    rotate_point(cx, cy, &x1, &y1, angle);
    rotate_point(cx, cy, &x2, &y2, angle);
    rotate_point(cx, cy, &x3, &y3, angle);

    // Thruster position (middle of base)
    float tx = (x2 + x3) * 0.5f;
    float ty = (y2 + y3) * 0.5f;

    // Thruster flame flicker animation
    float flame_len = 25.0f + SDL_sinf(now * 0.02f) * 6.0f;
    float fx = tx - SDL_cosf(angle) * flame_len;
    float fy = ty - SDL_sinf(angle) * flame_len;

    // Flame triangle
    SDL_Vertex flame[3] = {
        { { tx, ty }, { 255, 100, 30, 255 }, {0,0} },
        { { x2, y2 }, { 255, 200, 30, 255 }, {0,0} },
        { { x3, y3 }, { 255, 200, 30, 255 }, {0,0} }
    };
    SDL_RenderGeometry(renderer, NULL, flame, 3, NULL, 0);

    // Draw ship
    SDL_Vertex ship[3] = {
        { { x1, y1 }, { 0, 255, 0, 255 }, { 0,0 } },
        { { x2, y2 }, { 0, 200, 0, 255 }, { 0,0 } },
        { { x3, y3 }, { 0, 200, 0, 255 }, { 0,0 } }
    };
    SDL_RenderGeometry(renderer, NULL, ship, 3, NULL, 0);

    // Thruster glow light
    SDL_FRect glowDest = { fx - 40, fy - 40, 80, 80 };
    SDL_RenderTexture(renderer, glowTex, NULL, &glowDest);

    // Ship body glow (soft lighting)
    SDL_FRect shipGlow = { cx - 35, cy - 35, 70, 70 };
    SDL_RenderTexture(renderer, glowTex, NULL, &shipGlow);

    // Ambient faint background bloom
    SDL_FRect ambient = { cx - 120, cy - 120, 240, 240 };
    SDL_RenderTexture(renderer, glowTex, NULL, &ambient);

    SDL_SetTextureAlphaMod(glowTex, 40);  // 0 = invisible, 255 = full bright

    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}