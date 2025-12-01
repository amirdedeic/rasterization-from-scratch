#include <cmath>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <SDL2/SDL.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace std;
using namespace Eigen;

// CONSTANTS
const int Cw = 400;
const int Ch = 400;
const float Vw = 1.5f;
const float Vh = 1.5f;
const float d = 0.8f;

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

struct triangle {
    int a, b, c;
    Color color;
};

struct Camera {
    Vector3f position;
    float yaw;
    float pitch;

    Matrix4f GetCameraMatrix() const {
        // Create rotation matrix
        float cy = cos(yaw);
        float sy = sin(yaw);
        float cp = cos(pitch);
        float sp = sin(pitch);

        Matrix4f rotation = Matrix4f::Identity();
        rotation(0, 0) = cy;   rotation(0, 1) = sy*sp;  rotation(0, 2) = sy*cp;
        rotation(1, 0) = 0;    rotation(1, 1) = cp;     rotation(1, 2) = -sp;
        rotation(2, 0) = -sy;  rotation(2, 1) = cy*sp;  rotation(2, 2) = cy*cp;

        // Create translation matrix
        Matrix4f translation = Matrix4f::Identity();
        translation(0, 3) = -position.x();
        translation(1, 3) = -position.y();
        translation(2, 3) = -position.z();

        // Combine: Camera = Rotation * Translation
        return rotation * translation;
    }
};

struct Model {
    string name;
    vector<Vector4f> vertices;  // Homogeneous coordinates
    vector<triangle> triangles;
};

struct Instance {
    Model* model;
    Vector3f position;
    float yaw;
    float pitch;
    float scale;

    Matrix4f GetModelMatrix() const {
        // Scale matrix
        Matrix4f scaleMatrix = Matrix4f::Identity();
        scaleMatrix(0, 0) = scale;
        scaleMatrix(1, 1) = scale;
        scaleMatrix(2, 2) = scale;

        // Rotation matrix
        float cy = cos(yaw);
        float sy = sin(yaw);
        float cp = cos(pitch);
        float sp = sin(pitch);

        Matrix4f rotation = Matrix4f::Identity();
        rotation(0, 0) = cy;   rotation(0, 1) = sy*sp;  rotation(0, 2) = sy*cp;
        rotation(1, 0) = 0;    rotation(1, 1) = cp;     rotation(1, 2) = -sp;
        rotation(2, 0) = -sy;  rotation(2, 1) = cy*sp;  rotation(2, 2) = cy*cp;

        // Translation matrix
        Matrix4f translation = Matrix4f::Identity();
        translation(0, 3) = position.x();
        translation(1, 3) = position.y();
        translation(2, 3) = position.z();

        // Combine: Model = Translation * Rotation * Scale
        return translation * rotation * scaleMatrix;
    }
};

// Color constants
Color red = {255, 0, 0};
Color green = {0, 255, 0};
Color blue = {0, 0, 255};
Color yellow = {255, 255, 0};
Color purple = {255, 0, 255};
Color cyan = {0, 255, 255};

// Create projection matrix
Matrix4f GetProjectionMatrix() {
    Matrix4f proj = Matrix4f::Identity();
    proj(0, 0) = d * Cw / Vw;
    proj(1, 1) = d * Ch / Vh;
    proj(2, 2) = 1;
    proj(3, 2) = 1.0f / d;  // w = z/d for perspective
    proj(3, 3) = 0;
    return proj;
}

// Define cube model
Model cubeModel = {
    "cube",
    {
        Vector4f(1, 1, 1, 1),
        Vector4f(-1, 1, 1, 1),
        Vector4f(-1, -1, 1, 1),
        Vector4f(1, -1, 1, 1),
        Vector4f(1, 1, -1, 1),
        Vector4f(-1, 1, -1, 1),
        Vector4f(-1, -1, -1, 1),
        Vector4f(1, -1, -1, 1)
    },
    {
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
    }
};

// Create camera
Camera camera = {Vector3f(0, 0, 0), 0, 0};

// Create instances
vector<Instance> instances = {
    {&cubeModel, Vector3f(-1.5f, 0.0f, 7.0f), 0.5f, 0.5f, 1.0f},
    {&cubeModel, Vector3f(1.25f, 2.5f, 7.5f), 0.7f, 0.2f, 1.0f},
    {&cubeModel, Vector3f(1.25f, -4.5f, 7.5f), 3.14f, 1.4f, 1.0f}
};

// FUNCTION DEF
void PutPixel(SDL_Renderer* renderer, int x, int y, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderDrawPoint(renderer, Cw/2 + x, Ch/2 - y);
}

Point ProjectVertex(const Vector4f& v) {
    // Canonicalize: divide by w
    float x = v.x() / v.w();
    float y = v.y() / v.w();
    
    // Convert to canvas coordinates
    return {(int)(x), (int)(y)};
}

vector<float> Interpolate(int i0, float d0, int i1, float d1) {
    if (i0 == i1) {
        return {d0};
    }
    vector<float> values;
    float a = (d1 - d0) / (i1 - i0);
    float d = d0;
    for (int i = i0; i <= i1; i++) {
        values.push_back(d);
        d = d + a;
    }
    return values;
}

void DrawLine(SDL_Renderer* renderer, Point P0, Point P1, const Color& color) {
    if (abs(P1.x - P0.x) > abs(P1.y - P0.y)) {
        if (P0.x > P1.x) {
            swap(P0, P1);   
        }
        vector<float> ys = Interpolate(P0.x, P0.y, P1.x, P1.y);
        for (int x = P0.x; x <= P1.x; x++) {
            PutPixel(renderer, x, ys[x - P0.x], color);
        }
    } else {
        if (P0.y > P1.y) {
            swap(P0, P1);   
        }
        vector<float> xs = Interpolate(P0.y, P0.x, P1.y, P1.x);
        for (int y = P0.y; y <= P1.y; y++) {
            PutPixel(renderer, xs[y - P0.y], y, color);
        }
    }
}

void DrawWireframeTriangle(SDL_Renderer* renderer, Point P0, Point P1, Point P2, const Color& color) {
    DrawLine(renderer, P0, P1, color);
    DrawLine(renderer, P1, P2, color);
    DrawLine(renderer, P0, P2, color);
}

void RenderTriangle(SDL_Renderer* renderer, const triangle& T, const vector<Vector4f>& projected) {
    DrawWireframeTriangle(
        renderer,
        ProjectVertex(projected[T.a]),
        ProjectVertex(projected[T.b]),
        ProjectVertex(projected[T.c]),
        T.color
    );
}

void RenderModel(SDL_Renderer* renderer, const Model* model, const Matrix4f& transform) {
    vector<Vector4f> projected;
    
    // Transform all vertices
    for (size_t v = 0; v < model->vertices.size(); v++) {
        Vector4f transformed = transform * model->vertices[v];
        projected.push_back(transformed);
    }
    
    // Render all triangles
    for (size_t t = 0; t < model->triangles.size(); t++) {
        RenderTriangle(renderer, model->triangles[t], projected);
    }
}

void RenderScene(SDL_Renderer* renderer, vector<Instance>& instances, const Camera& camera) {
    // Create projection and camera matrices (computed once per frame)
    Matrix4f MProjection = GetProjectionMatrix();
    Matrix4f MCamera = camera.GetCameraMatrix();
    
    // Render each instance
    for (size_t i = 0; i < instances.size(); i++) {
        Matrix4f MModel = instances[i].GetModelMatrix();
        
        // Combined transform: F = Projection * Camera * Model
        Matrix4f transform = MProjection * MCamera * MModel;
        
        RenderModel(renderer, instances[i].model, transform);
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Window* window = SDL_CreateWindow("Rasterizer", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Cw, Ch, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        RenderScene(renderer, instances, camera);
        
        SDL_RenderPresent(renderer);
        SDL_Delay(50);

        // Animate instances
        instances[0].yaw += 0.03f;
        instances[0].pitch += 0.01f;

        instances[1].yaw += 0.01f;
        instances[1].pitch -= 0.02f;

        instances[2].yaw -= 0.02f;
        instances[2].pitch -= 0.02f;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}