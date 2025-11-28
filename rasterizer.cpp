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

const float d = 1.0f;


// TYPE DEF

struct Point {
    int x;
    int y;
};

struct Color {
    int r, g, b;

    Color operator*(float intensity) const {
        return {
            min(255, max(0, (int)(r * intensity))),
            min(255, max(0, (int)(g * intensity))),
            min(255, max(0, (int)(b * intensity)))
        };
    }
};

struct Vec3 {
    float x, y, z;
};

struct triangle {
    int a, b, c;
    Color color;
};

Point ViewportToCanvas(float x, float y) {
    return {(int)(x * Cw / Vw), (int)(y * Ch / Vh)};
}

Point ProjectVertex(Vec3 v){
    return ViewportToCanvas(v.x * d / v.z, v.y * d / v.z);
}


// Color constant
Color red = {255, 0, 0};
Color green = {0, 255, 0};
Color blue = {0, 0, 255};
Color yellow = {255, 255, 0};
Color purple = {255, 0, 255};
Color cyan = {0, 255, 255};


// Defining Vertices

Vec3 VAf = {-1, 1, 1};
Vec3 VBf = {1, 1, 1};
Vec3 VCf = {1, -1, 1};
Vec3 VDf = {-1, -1, 1};

Vec3 VAb = {-3, 1, 2};
Vec3 VBb = {-1, 1, 2};
Vec3 VCb = {-1, -1, 2};
Vec3 VDb = {-2, -1, 2};


// Defining Vertices for model space of cube
std::vector<Vec3> vertices = {
    { 1,  1,  1},
    {-1,  1,  1},
    {-1, -1,  1},
    { 1, -1,  1},
    { 1,  1, -1},
    {-1,  1, -1},
    {-1, -1, -1},
    { 1, -1, -1}
};

std::vector<triangle> triangles = {
    {0, 1, 2, red},
    {0, 2, 3, red},

    {4, 0, 3, green},
    {4, 3, 7, green},

    {5, 4, 7, blue},
    {5, 7, 6, blue},

    {1, 5, 6, yellow},
    {1, 6, 2, yellow},

    {4, 5, 1, purple},
    {4, 1, 0, purple},

    {2, 6, 7, cyan},
    {2, 7, 3, cyan}
};



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

void DrawFilledTriangle(SDL_Renderer* renderer, Point P0, Point P1, Point P2, const Color& color){
    if (P0.y > P1.y){std::swap(P0, P1);}
    if (P1.y > P2.y){std::swap(P1, P2);}
    if (P0.y > P2.y){std::swap(P0, P2);}

    std::vector<float> x01 = Interpolate(P0.y, P0.x, P1.y, P1.x);
    std::vector<float> x12 = Interpolate(P1.y, P1.x, P2.y, P2.x);
    std::vector<float> x02 = Interpolate(P0.y, P0.x, P2.y, P2.x);

    x01.pop_back();
    std::vector<float> x012 = x01;
    x012.insert(x012.end(), x12.begin(), x12.end());

    std::vector<float> x_left;
    std::vector<float> x_right;

    int m = floor(x012.size() / 2);
    if (x02[m] < x012[m]){
        x_left = x02;
        x_right = x012;
    } else {
        x_left = x012;
        x_right = x02;
    }
    // Draw the hotizontal segments
    for (int y = P0.y; y < P2.y; y++){
        for (int x = x_left[y - P0.y]; x < x_right[y - P0.y]; x++){
            PutPixel(renderer, x, y, color);
        }
    }
    
}

void DrawFilledTriangleGradient(SDL_Renderer* renderer, Point P0, Point P1, Point P2, float h0, float h1, float h2, const Color& color){

    if (h1 == -1){
        DrawFilledTriangle(renderer, P0, P1, P2, color);
    }
    if (P0.y > P1.y){std::swap(P0, P1);}
    if (P1.y > P2.y){std::swap(P1, P2);}
    if (P0.y > P2.y){std::swap(P0, P2);}

    // triangle verticies interpolation
    std::vector<float> x01 = Interpolate(P0.y, P0.x, P1.y, P1.x);
    std::vector<float> x12 = Interpolate(P1.y, P1.x, P2.y, P2.x);
    std::vector<float> x02 = Interpolate(P0.y, P0.x, P2.y, P2.x);

    // gradient verticies interpolation
    std::vector<float> h01 = Interpolate(P0.y, h0, P1.y, h1);
    std::vector<float> h12 = Interpolate(P1.y, h1, P2.y, h2);
    std::vector<float> h02 = Interpolate(P0.y, h0, P2.y, h2);

    x01.pop_back();
    std::vector<float> x012 = x01;
    x012.insert(x012.end(), x12.begin(), x12.end());

    h01.pop_back();
    std::vector<float> h012 = h01;
    h012.insert(h012.end(), h12.begin(), h12.end());

    std::vector<float> x_left, x_right, h_left, h_right;

    int m = floor(x012.size() / 2);
    if (x02[m] < x012[m]){
        x_left = x02;
        x_right = x012;
        h_left = h02;
        h_right = h012;
    } else {
        x_left = x012;
        x_right = x02;
        h_left = h012;
        h_right = h02;
    }

    // Draw the hotizontal segments
    for (int y = P0.y; y < P2.y; y++){
        float x_left_this_y(x_left[y - P0.y]);
        float x_right_this_y(x_right[y - P0.y]);
        std::vector<float> h_segment = Interpolate(x_left[y - P0.y], h_left[y - P0.y], x_right[y - P0.y], h_right[y - P0.y]);

        for (int x = x_left_this_y; x < x_right_this_y; x++){
            Color shaded_color = color*h_segment[x - x_left_this_y];
            PutPixel(renderer, x, y, shaded_color);
        }
    }
    
}

void RenderTriangle(SDL_Renderer* renderer, triangle T, std::vector<Vec3> projected)
{
    DrawWireframeTriangle(
        renderer,
        ProjectVertex(projected[T.a]),
        ProjectVertex(projected[T.b]),
        ProjectVertex(projected[T.c]),
        T.color
    );
}

void RenderObject(SDL_Renderer* renderer, const vector<Vec3>& vertices, const vector<triangle>& triangles){
    // Translation vector: move cube 7 units forward (Z+) and 1.5 units left (X-)
    Vec3 translation = {-1.5f, 0.0f, 7.0f};
    
    std::vector<Vec3> projected = {};
    for (size_t v = 0; v < vertices.size(); v++){
        // Translate each vertex: V' = V + T
        Vec3 translated = {
            vertices[v].x + translation.x,
            vertices[v].y + translation.y,
            vertices[v].z + translation.z
        };
        projected.push_back(translated);
    }

    for (size_t t = 0; t < triangles.size(); t++){
        RenderTriangle(renderer, triangles[t], projected);
    }
    
}

int main() {

    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Window* window = SDL_CreateWindow("Rasterizer", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Cw, Ch, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    bool running = true;
    SDL_Event event;
    
    // Render the cube using the triangle system
    RenderObject(renderer, vertices, triangles);

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
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}