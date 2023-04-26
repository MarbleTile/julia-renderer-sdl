#include <SDL2/SDL_events.h>
#include <SDL2/SDL_hints.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

//#define SCREEN_WIDTH 1280
//#define SCREEN_HEIGHT 720
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000 

#define WHITE   0xFFFFFFFF
#define BLACK   0x00000000
#define YELLOW  0xFF2AD9FC
#define CYAN    0xFFFFFF00
#define MAGENTA 0xFFFF00FF
#define RED     0xFF00FFFF
#define GREEN   0xFF00FF00
#define BLUE    0xFFFF0000

#define MAX_ITERATION 1000

typedef float f32;
typedef double f64;
typedef uint8_t u8;
typedef int8_t i8;
typedef uint32_t u32;
typedef int32_t i32;

struct {
    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool quit;
} state;

typedef struct cpx {
    f32 r;
    f32 i;
} cpx;

struct cpx cpx_next(cpx curr, cpx c){
    const f32 nr = curr.r * curr.r - curr.i * curr.i;
    const f32 ni = 2.0 * curr.r * curr.i;
    return (cpx) {c.r+nr, c.i+ni};
}

f32 cpx_mag(cpx num){
    return num.r*num.r + num.i*num.i;
}

void cpx_plot(cpx num, u32 color){
    state.pixels[((i32) num.r + SCREEN_WIDTH/2) + ((i32) num.i + SCREEN_HEIGHT/2)*SCREEN_WIDTH] = color;
}

u32 cpx_in_mandelbrot(cpx con){
    cpx num = {0.0, 0.0};
    u32 it = 0;
    while(it < MAX_ITERATION){
        num = cpx_next(num, con);
        it++;
        if(cpx_mag(num) > 4)
            return it;
    }
    return 0;
}

//wikipedia
f32 lerp_f32(f32 v0, f32 v1, f32 t){
    return (1-t) * v0 + t * v1;
}

u32 lerp_u32(u32 v0, u32 v1, f32 t){
    return (1-t) * v0 + t * v1;
}

//k->w fine; w->k fucked up
u32 sqerp_u32(u32 v0, u32 v1, f32 t){
    return (v1-v0)*sqrtf(t) + v0;
}

//0xAA_BB_GG_RR
void gen_color(u32 *colors, u32 begin, u32 end){
    const u32 r_b =  0x000000FF & begin;
    const u32 g_b = (0x0000FF00 & begin) >> 8;
    const u32 b_b = (0x00FF0000 & begin) >> 16;
    const u32 a_b = (0xFF000000 & begin) >> 24;
    const u32 r_e =  0x000000FF & end;
    const u32 g_e = (0x0000FF00 & end) >> 8;
    const u32 b_e = (0x00FF0000 & end) >> 16;
    const u32 a_e = (0xFF000000 & end) >> 24;
    const u32 tol = 10;
    for(u32 i = 0; i < MAX_ITERATION - tol; i++){
        f32 i_scale = (f32) i / MAX_ITERATION;
        colors[i] = sqerp_u32(a_b, a_e, i_scale) << 24 | sqerp_u32(b_b, b_e, i_scale) << 16 
            | sqerp_u32(g_b, g_e, i_scale) << 8 | sqerp_u32(r_b, r_e, i_scale);
    }
    for(u32 i = MAX_ITERATION - tol; i < MAX_ITERATION; i++)
        colors[i] = end;
}

void draw_julia(const cpx con, u32 *colors, const f32 zoom, const cpx pos){
    cpx z = {0.0, 0.0};
    u32 it = 0;
    u32 smooth_it = 0;
    const f32 esc = 2.0;
    const f32 R = esc * esc;
    const u32 samples = 16;
    const f32 jigg = 0.001;
    srand(time(NULL));
    for(u32 x = 0; x < SCREEN_WIDTH; x++){
        for(u32 y = 0; y < SCREEN_HEIGHT; y++){
            for(u32 s = 0; s < samples; s++){
                z.r = lerp_f32(-1*esc/zoom - pos.r, 
                        esc/zoom - pos.r - lerp_f32(-1*jigg, jigg, (f32) rand() / (f32) RAND_MAX), 
                        (f32) x/SCREEN_WIDTH);
                z.i = lerp_f32(-1*esc/zoom - pos.i, 
                        esc/zoom - pos.i - lerp_f32(-1*jigg, jigg, (f32) rand() / (f32) RAND_MAX), 
                        (f32) y/SCREEN_HEIGHT);
                //z.r = lerp_f32(-1*esc/zoom - pos.r, esc/zoom - pos.r, (f32) x/SCREEN_WIDTH);
                //z.i = lerp_f32(-1*esc/zoom - pos.i, esc/zoom - pos.i, (f32) y/SCREEN_HEIGHT);
                it = 0; 
                while(cpx_mag(z) < R && it < MAX_ITERATION){
                    z = cpx_next(z, con);
                    it++;
                }
               smooth_it += it;
            }
            state.pixels[y*SCREEN_WIDTH + x] = colors[smooth_it/samples]; 
            //state.pixels[y*SCREEN_WIDTH + x] = colors[it];
            smooth_it = 0;
        }
    }
}


void translate(f32 x, f32 y, u32 *x_o, u32 *y_o){
    *x_o = x * SCREEN_WIDTH;
    *y_o = y * SCREEN_HEIGHT;
}

void init_state(void){ 
    state.window = SDL_CreateWindow("FRACTAL VIEWER 9000", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE); //| SDL_WINDOW_FULLSCREEN);
    state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_PRESENTVSYNC);
    state.texture = SDL_CreateTexture(state.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 
        SCREEN_WIDTH, SCREEN_HEIGHT);
    state.quit = false;
    memset(&state.pixels, BLACK, sizeof(state.pixels));
}

u32 handle_cli(char a){
    switch(a){
        case 'k':
            return BLACK;
        case 'c':
            return CYAN;
        case 'm':
            return MAGENTA;
        case 'y':
            return YELLOW;
        case 'w':
            return WHITE;
        case 'r':
            return RED;
        case 'g':
            return GREEN;
        case 'b':
            return BLUE;
    }
    return BLACK;
}

int main (int argc, char **argv){
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        exit(1);

    u32 color_begin = BLACK;
    u32 color_end   = WHITE;
    
    if(argc > 3)
        exit(1);

    if(argc == 3){
        color_begin = handle_cli(argv[1][0]);
        color_end   = handle_cli(argv[2][0]);
    }

    memset(&state, 0, sizeof(state));

    init_state();
    
    u32 colors[MAX_ITERATION];
    memset(colors, BLACK, sizeof(colors));
    gen_color(colors, color_begin, color_end);

    srand(time(NULL));

    const cpx neat_julia = {-0.8, 0.156};
    //draw_julia(neat_julia, colors);

//    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 1);
    const f32 d_zoom = 0.5;
    const f32 d_pos = 0.1;
    cpx pos = {0.0, 0.0};
    f32 zoom = 1.0;
    cpx con = {-1.0, -0.5};
    while (!state.quit){
//        draw_julia(con, colors, zoom, pos);
        draw_julia(neat_julia, colors, zoom, pos);

        SDL_UpdateTexture(state.texture, NULL, state.pixels, SCREEN_WIDTH*4);
        SDL_RenderCopyEx(state.renderer, state.texture, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
        SDL_RenderPresent(state.renderer);

        SDL_Event ev;
        while(SDL_PollEvent(&ev)){
            if(ev.type == SDL_QUIT){
                state.quit = true;
                break;
            }
            switch(ev.key.keysym.scancode){
                case SDL_SCANCODE_EQUALS:
                    zoom += d_zoom;
                    break;
                case SDL_SCANCODE_MINUS:
                    zoom -= d_zoom;
                    break;
                case SDL_SCANCODE_UP:
                    pos.i -= d_pos/zoom;
                    break;
                case SDL_SCANCODE_DOWN:
                    pos.i += d_pos/zoom;
                    break;
                case SDL_SCANCODE_LEFT:
                    pos.r += d_pos/zoom;
                    break;
                case SDL_SCANCODE_RIGHT:
                    pos.r -= d_pos/zoom;
                    break;                    
                case SDL_SCANCODE_SPACE:
                    zoom = 1.0;
                    pos = (cpx) {0.0, 0.0};
                    break;
                default:
                    continue;
            }
        }
        con.r   += 0.001;
        con.i   += 0.001;
        if(con.r == 2.0)
            con = (cpx) {0.0, 0.0};
        SDL_Delay(1);
    }
    //free()
    SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    return EXIT_SUCCESS;
}
