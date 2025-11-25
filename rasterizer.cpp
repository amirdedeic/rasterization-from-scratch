#include <cmath>
#include <vector>
#include <chrono>
#include <iostream>
#include <SDL2/SDL.h>

using namespace std;

// CONSTANTS (FIX LATER)

// Window size constants
const int Cw = 400;
const int Ch = 400;

// Viewport size constants
const float Vw = 1.5f;
const float Vh = 1.5f;


// TYPE DEF

struct Vec3 {
    float x, y, z;
};

struct Point {
    int x;
    float y;
};

struct Color {
    int r, g, b;
};

// Color constant
Color red = {255, 255, 255};




// FUNCTION DEF

void PutPixel(SDL_Renderer* renderer, int x, int y, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderDrawPoint(renderer, Cw/2 + x, Ch/2 - y);
}


void DrawLine(SDL_Renderer* renderer, const Point& P0, const Point& P1, const Color& color){
    int dx = P1.x - P0.x;
    int dy = P1.y - P0.y;
    
    // Handle vertical lines
    if (dx == 0) {
        int y_start = min(P0.y, P1.y);
        int y_end = max(P0.y, P1.y);
        for (int y = y_start; y <= y_end; y++) {
            PutPixel(renderer, P0.x, y, color);
        }
        return;
    }
    
    // Swap points if P0 is to the right of P1
    Point start = P0;
    Point end = P1;
    if (P0.x > P1.x) {
        start = P1;
        end = P0;
    }
    
    // Check if line is steep (|slope| > 1)
    if (abs(dy) > abs(dx)) {
        // Iterate over y instead of x
        int y_start = min(start.y, end.y);
        int y_end = max(start.y, end.y);
        float x = (start.y < end.y) ? start.x : end.x;
        float a = (float)dx / dy;
        
        for (int y = y_start; y <= y_end; y++) {
            PutPixel(renderer, (int)x, y, color);
            x += a;
        }
    } else {
        // Original approach for shallow lines
        float a = (float)dy / dx;
        float y = start.y;
        
        for (int x = start.x; x <= end.x; x++) {
            PutPixel(renderer, x, (int)y, color);
            y += a;
        }
    }
}


int main() {


    SDL_Init(SDL_INIT_VIDEO);
    // omp_set_num_threads(8);
    
    SDL_Window* window = SDL_CreateWindow("Rasterizer", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Cw, Ch, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // SDL_RenderClear(renderer);
    

    bool running = true;
    SDL_Event event;
	
    // SDL_RenderPresent(renderer);



    Point P1 = {-60, -200};
    Point P0 = {0, 100};

    DrawLine(renderer, P0, P1, red);
    SDL_RenderPresent(renderer);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }
        SDL_Delay(16); // Prevents CPU from spinning at 100%
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}