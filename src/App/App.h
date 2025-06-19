#ifndef IG20240519210141
#define IG20240519210141

#include "Simulation.h"
#include <raylib.h>

struct AppConfig;

class App
{
public:
    Simulation simulation{};

    Shader shader{};
    unsigned int vao{};
    unsigned int vbo{};

    int screenWidth{};
    int screenHeight{};
    Vector2 mousePosition{};
    float dt{ 0 };

private:
    void setupAppEvents();
    void setupShaders( AppConfig const& config );
    void setupShadersTest( AppConfig const& config );

public:
    void init( AppConfig const& config );
    /// Main app loop
    void run();
    void render();
    void deinit();
};

#endif
