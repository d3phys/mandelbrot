#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <immintrin.h>
#include "mandelbrot.h"

const char MB_WINDOW_TITLE[] = "Mandelbrot"; 

const int MB_WINDOW_WIDTH  = 640;
const int MB_WINDOW_HEIGHT = 480;

static inline int terminate(const char *msg) 
{
        fprintf(stderr, msg, SDL_GetError());
        return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
        int opt = 0;
        int loop = 0;

        SDL_Surface* (*Blit)(SDL_Surface *const, float, int, float, Point) = &BlitMandelbrotAVX;
        
        while ((opt = getopt(argc, argv, "o:l:")) != -1){
                switch (opt) {
                case 'o': 
                        fprintf(stderr, "found argument \"b = %s\".\n", optarg);
                        if (!strncmp(optarg, "avx", sizeof("avx"))) {
                                Blit = &BlitMandelbrotAVX;
                        } else if (!strncmp(optarg, "slow", sizeof("slow"))) {
                                Blit = &BlitMandelbrot;
                        } else {
                                fprintf(stderr, "Undefined optimization \"%s\".\n", optarg); 
                                return EXIT_FAILURE;
                        }
                        break;
                case 'l': 
                        fprintf(stderr, "found argument \"C = %s\".\n", optarg);
                        loop = atoi(optarg);
                        if (loop <= 0) {
                                fprintf(stderr, "Incorrect value \"%s\".\n", optarg); 
                                return EXIT_FAILURE;
                        }
                        break;
                case '?': 
                        return EXIT_FAILURE;
                }
        } 

        int error = SDL_Init(SDL_INIT_VIDEO);
        if (error)
                return terminate("Unable to initialize SDL:  %s\n");

        atexit(SDL_Quit);
        
        SDL_Window *window = SDL_CreateWindow(
                MB_WINDOW_TITLE, 
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                MB_WINDOW_WIDTH,
                MB_WINDOW_HEIGHT,
                SDL_WINDOW_SHOWN
        );
        
        if (!window)
                return terminate("Could not create window: %s\n");

        SDL_Renderer *renderer = SDL_CreateRenderer(
                window, 
                -1, 
                SDL_RENDERER_ACCELERATED
        );

        if (!renderer)
                return terminate("Could not create renderer: %s\n");

        int active = SDL_TRUE;
        int step = 10;
        float scale  = 1.0f;
        float dscale = 1.0f;
        Point origin = {0, 0};
        
        SDL_Surface *surface = SDL_GetWindowSurface(window);

        float elapsed = 0.0f;
        SDL_Event event;
        while (active) {
                Uint64 startf = SDL_GetPerformanceCounter();
        
                while (SDL_PollEvent(&event)) {
                        switch(event.type) {
                        case SDL_QUIT:                           
                                active = SDL_FALSE;
                                break;
                        case SDL_KEYDOWN:
                                if(event.key.keysym.mod & KMOD_LSHIFT)
                                        step = 2000;
                                else
                                        step = 500;
                                                                
                                switch(event.key.keysym.sym) {
                                case SDLK_UP:    origin.y -= step * scale * elapsed; break;
                                case SDLK_DOWN:  origin.y += step * scale * elapsed; break;
                                case SDLK_LEFT:  origin.x += step * scale * elapsed; break;
                                case SDLK_RIGHT: origin.x -= step * scale * elapsed; break;
                                
                                case SDLK_KP_PLUS:  scale += dscale * scale * elapsed; break;
                                case SDLK_KP_MINUS: scale -= dscale * scale * elapsed; break;
                                }

                                break;
                        default:
                                break;
                        }
                }

                for (int i = 0; i < loop; i++)
                        Blit(surface, 100, 128, scale, origin);
                        
                SDL_UpdateWindowSurface(window);

                Uint64 endf   = SDL_GetPerformanceCounter();
                elapsed = (endf - startf) / (float)SDL_GetPerformanceFrequency();
                printf("FPS: %f\n", 1 / elapsed);
        }
        
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return EXIT_SUCCESS;
}
