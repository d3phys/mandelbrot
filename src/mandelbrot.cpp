/*
 * Mandelbrot set SIMD optimized blitting. 
 * Written 2022 by d3phys
 */

#include "mandelbrot.h"
#include <SDL2/SDL.h>

/* Intel avx2 does not provide useful set_single intrinsics 
   Define it here. */
#define _mm256_set_ps1(x) \
        _mm256_set_ps(x, x, x, x, x, x, x, x)
        
#define _mm256_set_epi32_1(x) \
        _mm256_set_epi32(x, x, x, x, x, x, x, x)

SDL_Surface *BlitMandelbrotAVX(
        SDL_Surface *const srfc,
        float radius,
        int iterations,
        float scale = 1.0f,
        Point origin = {0, 0}
) {
        SDL_LockSurface(srfc);

        float r2 = radius * radius; 
        __m256 rps = _mm256_set_ps1(r2); 

        __m256 range = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);

        float dx = 2.0 / srfc->w * scale;
        for (int h = 0; h < srfc->h; h++) {
        
                float yi = (srfc->h - 2.0 * h) / srfc->h * scale - origin.y * 1.0 / srfc->h;
                float xi = (0.0     - srfc->w) / srfc->w * scale - origin.x * 1.0 / srfc->w;
                for (int w = 0; w < srfc->w; w += 8, xi += 8 * dx) {

                        __m256 x0ps = _mm256_add_ps(
                                _mm256_set_ps1(xi), 
                                _mm256_mul_ps(_mm256_set_ps1(dx), range) 
                        );
                                
                        __m256 y0ps = _mm256_set_ps1(yi);

                        __m256 xps = x0ps;
                        __m256 yps = y0ps;
                        
                        __m256i nsi = _mm256_setzero_si256();
                        for (int it = 0; it < iterations; it++) {

                                __m256 x2ps = _mm256_mul_ps(xps, xps);
                                __m256 y2ps = _mm256_mul_ps(yps, yps);  
                                __m256 xyps = _mm256_mul_ps(xps, yps);

                                xps = _mm256_add_ps(_mm256_sub_ps(x2ps, y2ps), x0ps);
                                yps = _mm256_add_ps(_mm256_add_ps(xyps, xyps), y0ps);

                                __m256 cmp = _mm256_cmp_ps(_mm256_add_ps(x2ps, y2ps), rps, _CMP_LT_OQ);                   
                                if (!_mm256_movemask_ps(cmp))
                                        break;

                                nsi = _mm256_sub_epi32(nsi, _mm256_castps_si256(cmp));
                        }

                        __m256i color = _mm256_cmpgt_epi32(_mm256_set_epi32_1(iterations - 1), nsi);
                        memcpy((Uint32 *)srfc->pixels + srfc->w * h + w, &color, sizeof(Uint32) * 8);
                }
        }
        
        SDL_UnlockSurface(srfc);
        return srfc;
}

#undef __mm265_set_ps1
#undef __mm265_set_epi32_1


SDL_Surface *BlitMandelbrot(
        SDL_Surface *const srfc,
        float r2,
        int iterations,
        float scale = 1.0f,
        Point origin = {0, 0}
) {
        SDL_LockSurface(srfc);

        float x = 0.0f;
        float y = 0.0f;

        float xi = 0.0f;
        float yi = 0.0f;

        for (int h = 0; h < srfc->h; h++) {
        
                yi = (srfc->h - 2.0 * h) / srfc->h * scale - origin.y * 1.0 / srfc->h;
                for (int w = 0; w < srfc->w; w++) {
                
                        xi = (2.0 * w - srfc->w) / srfc->w * scale - origin.x * 1.0 / srfc->w;
                        int iter = 0;
                        x = xi;
                        y = yi;
                        for (iter = 0; iter < iterations; iter++) {
                                
                                float temp = x*x - y*y + xi;   
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

