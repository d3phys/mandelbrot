#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

const char MB_WINDOW_TITLE[] = "Mandelbrot"; 

const int MB_WINDOW_WIDTH  = 640;
const int MB_WINDOW_HEIGHT = 480;

struct Point {
        double x = 0;
        double y = 0;
};

static SDL_Surface *BlitMandelbrot(
        SDL_Surface *const srfc,
        double r2,
        int iterations,
        double scale = 1.0f,
        Point origin = {0, 0}
) {
        SDL_LockSurface(srfc);

        SDL_memset(srfc->pixels, 0, srfc->h * srfc->pitch);

        double x = 0.0f;
        double y = 0.0f;

        double xi = 0.0f;
        double yi = 0.0f;

        for (int h = 0; h < srfc->h; h++) {
        
                yi = (srfc->h - 2.0 * h) / srfc->h * scale - origin.y * 1.0 / srfc->h;
                for (int w = 0; w < srfc->w; w++) {
                
                        xi = (2.0 * w - srfc->w) / srfc->w * scale - origin.x * 1.0 / srfc->w;
                        int iter = 0;
                        x = xi;
                        y = yi;
                        for (iter = 0; iter < iterations; iter++) {
                                double temp = x*x - y*y + xi;   
                                y = 2 * x * y + yi;
                                x = temp;

                                if (x*x + y*y > r2)
                                        break;
                        }                        

                        ((Uint32 *)srfc->pixels)[srfc->w * h + w] = iter < iterations ? 0xffffffff : 0xff000000;
                }
        }
        
        SDL_UnlockSurface(srfc);
        return srfc;
}

static inline int terminate(const char *msg) 
{
        fprintf(stderr, msg, SDL_GetError());
        return EXIT_FAILURE;
}

int main()
{
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

        SDL_Surface *image = SDL_LoadBMP("Table.bmp");
        SDL_Surface *surface = SDL_GetWindowSurface(window);
        SDL_BlitSurface(image, nullptr, surface, nullptr);
        SDL_FreeSurface(image);

        int step = 10;
        float scale  = 1.0f;
        float dscale = 0.1f;
        Point origin = {0, 0};
        
        SDL_Event event;
        while (active) {
                Uint64 startf = SDL_GetPerformanceCounter();
        
                while (SDL_PollEvent(&event)) {
                        switch(event.type) {
                        case SDL_QUIT:                           
                                active = SDL_FALSE;
                                break;
                        case SDL_KEYDOWN:
                                if(event.key.keysym.mod & KMOD_LSHIFT) {
                                        step = 100;
                                } else {
                                        step = 10;
                                }
                                                                
                                switch(event.key.keysym.sym) {
                                case SDLK_UP:    origin.y -= step * scale; break;
                                case SDLK_DOWN:  origin.y += step * scale; break;
                                case SDLK_LEFT:  origin.x += step * scale; break;
                                case SDLK_RIGHT: origin.x -= step * scale; break;
                                
                                case SDLK_KP_PLUS:  scale += dscale * scale; break;
                                case SDLK_KP_MINUS: scale -= dscale * scale; break;
                                }

                                break;
                        default:
                                break;
                        }
                }

                BlitMandelbrot(surface, 3, 100, scale, origin);
                SDL_UpdateWindowSurface(window);

                Uint64 endf   = SDL_GetPerformanceCounter();
                float elapsed = (endf - startf) / (float)SDL_GetPerformanceFrequency();
                printf("FPS: %f\n", 1 / elapsed);
        }
        
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return EXIT_SUCCESS;
}
