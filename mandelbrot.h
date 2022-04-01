#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <SDL2/SDL.h>

struct Point {
        float x;
        float y;
};

SDL_Surface *BlitMandelbrotAVX(
        SDL_Surface *const srfc,
        float radius,
        int iterations,
        float scalef,
        Point origin
);

SDL_Surface *BlitMandelbrot(
        SDL_Surface *const srfc,
        float radius,
        int iterations,
        float scalef,
        Point origin
);

#endif /* MANDELBROT_H */
