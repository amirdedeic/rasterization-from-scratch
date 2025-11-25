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
    int x, y;
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
    float a = (P1.y - P0.y) / (P1.x - P0.x); 
    float b = (P0.y - a * P0.x);

    for (int x = P0.x; x < P1.x; x++){
        float y = a*x + b;
        PutPixel(renderer, x, y, color);
    }

}

int main() {


    SDL_Init(SDL_INIT_VIDEO);
    // omp_set_num_threads(8);
    
    SDL_Window* window = SDL_CreateWindow("Rasterizer", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Cw, Ch, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    

    bool needs_redraw = true;
    bool running = true;
    SDL_Event event;

    // SDL_RenderPresent(renderer);



    Point P0 = {10, 40};
    Point P1 = {200, 300};

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