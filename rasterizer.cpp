#include <cmath>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <SDL2/SDL.h>
#include <list>

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
    int y;
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


std::vector<float> Interpolate(int i0, float d0, int i1, float d1){
    if (i0 == i1) {
        return {d0};
    };
    std::vector<float> values;
    float a = (d1 - d0) / (i1 - i0);
    float d = d0;
    for (int i = i0; i <= i1; i++){
        values.push_back(d);
        d = d + a;
    }
    return values;
}


void DrawLine(SDL_Renderer* renderer, Point P0, Point P1, const Color& color){
    
    if (abs(P1.x - P0.x) > abs(P1.y - P0.y)){
        if (P0.x > P1.x){
            std::swap(P0, P1);   
        }
        std::vector<float> ys = Interpolate(P0.x, P0.y, P1.x, P1.y);
        for (int x = P0.x; x <= P1.x; x++) {
            PutPixel(renderer, x, ys[x - P0.x], color);
        }
    } else {
        if (P0.y > P1.y){
            std::swap(P0, P1);   
        }
        std::vector<float> xs = Interpolate(P0.y, P0.x, P1.y, P1.x);
        for (int y = P0.y; y <= P1.y; y++) {
            PutPixel(renderer, xs[y - P0.y], y, color);
        }
    }
}

void DrawWireframeTriangle(SDL_Renderer* renderer, Point P0, Point  P1, Point  P2, const Color& color){
    DrawLine(renderer, P0, P1, color);
    DrawLine(renderer, P1, P2, color);
    DrawLine(renderer, P0, P2, color);
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


    Point P0 = {0, 100}; 
    Point P1 = {0, 0};
    Point P2 = {100, 0};

    // DrawLine(renderer, P0, P1, red);
    // DrawLine(renderer, P2, P1, red);

    DrawWireframeTriangle(renderer, P0, P1, P2, red);

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