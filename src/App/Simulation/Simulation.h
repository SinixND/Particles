#ifndef IG20250606152339
#define IG20250606152339

#include "Particle.h"
#include "ThreadPool.h"

int constexpr PARTICLE_COUNT{ 100000 };
float constexpr MULTIPLIER{ 1000 };
float constexpr FRICTION{ 0.995 };

enum class State
{
    SINGLE_CORE,
    GPU,
    MULTITHREAD,
};

class Simulation
{
#if !defined( EMSCRIPTEN )
    ThreadPool threadPool_{};
#endif

public:
    Particle particles[PARTICLE_COUNT];

    // clang-format off
    float vertices[15] = {
        // x, y, r, g, b
        -.5f, -.5f,  1.0f, 0.0f, 0.0f, 
        0.5f, -.5f,  0.0f, 1.0f, 0.0f, 
        0.0f, 0.5f,  0.0f, 0.0f, 1.0f
    };
    // clang-format on

    //* Shader stuff
    Shader shader{};
    unsigned int vao, vbo;

    //* Set to desired computation method
#if defined( EMSCRIPTEN )
    State state{ State::SINGLE_CORE };
#else
    State state{ State::MULTITHREAD };
#endif

public:
    void init();
    void update(
        int screenWidth,
        int screenHeight,
        Vector2 mousePosition,
        float dt
    );
    void deinit();

    void updateGPU(
        int screenWidth,
        int screenHeight,
        Vector2 mousePosition,
        float dt
    );

private:
    void updateSingleCore(
        int screenWidth,
        int screenHeight,
        Vector2 mousePosition,
        float dt
    );

    void updateMultithreaded(
        int screenWidth,
        int screenHeight,
        Vector2 mousePosition,
        float dt
    );
};

#endif
