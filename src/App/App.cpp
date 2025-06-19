#include "App.h"

#define TEST

#include "AppConfigs.h"
#include "AppData.h"
#include "ColorData.h"
#include "DeveloperMode.h"
#include "EventDispatcher.h"
#include "EventId.h"
#include "ParticleSystem.h"
#include <raylib.h>
#include <rlgl.h>

#if defined( EMSCRIPTEN )
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

void setupRaylib( [[maybe_unused]] AppConfig const& config )
{
#if !defined( NOGUI )
    //* Raylib flags
    SetConfigFlags( FLAG_WINDOW_RESIZABLE );

    if ( config.vSync )
    {
        SetConfigFlags( FLAG_VSYNC_HINT );
    }

    //* Initialize window
    InitWindow(
        config.windowWidth,
        config.windowHeight,
        "Particle Simulation"
    );

    //* Raylib Settings
    SetWindowIcon( AppData::FAVICON );

#if defined( EMSCRIPTEN )
    MaximizeWindow();
#endif
#endif

    SetTargetFPS( AppData::FPS_TARGET );

    SetExitKey( AppData::EXIT_KEY );
}

void setupFrameworks( AppConfig const& config )
{
    setupRaylib( config );
}

void App::setupShaders( AppConfig const& config )
{
    //* Init shaders
    //* Shader
    shader = LoadShader(
        config.vertexShaderPath,
        config.fragmentShaderPath
    );

    //* VAO
    vao = rlLoadVertexArray();
    rlEnableVertexArray( vao );

    //* VBO
    vbo = rlLoadVertexBuffer(
        simulation.particles,
        sizeof( simulation.particles ),
        true
    );

    //* Connect VBO with vertex shader
    // (location)index: position in shader
    // compSize: n consecutive values of:
    // type: type of comp
    // normalized: bool - should data be normalized
    // stride: size of one vertex;
    // offset: byte offset into VBO
    //* Position
    rlSetVertexAttribute( 0, 2, RL_FLOAT, false, 20, 0 );
    //* Velocity
    rlSetVertexAttribute( 1, 2, RL_FLOAT, false, 20, 0 );
    //* Color
    rlSetVertexAttribute( 2, 4, RL_FLOAT, false, 20, 0 );

    SetShaderValue(
        shader,
        // 4,
        GetShaderLocationAttrib( shader, "multiplier" ),
        &MULTIPLIER,
        SHADER_UNIFORM_FLOAT
    );

    SetShaderValue(
        shader,
        // 5,
        GetShaderLocationAttrib( shader, "friction" ),
        &FRICTION,
        SHADER_UNIFORM_FLOAT
    );

    rlEnableVertexAttribute( 0 );
    rlEnableVertexAttribute( 1 );
    rlEnableVertexAttribute( 2 );

    rlDisableVertexArray();
}

void App::setupShadersTest( AppConfig const& config )
{
    shader = LoadShader(
        config.vertexShaderPath,
        config.fragmentShaderPath
    );
    vao = rlLoadVertexArray();
    rlEnableVertexArray( vao );
    vbo = rlLoadVertexBuffer(
        simulation.vertices,
        sizeof( simulation.vertices ),
        false
    );
    //* Position
    rlSetVertexAttribute(
        0,
        2,
        RL_FLOAT,
        false,
        5 * sizeof( float ),
        0
    );
    //* Color
    rlSetVertexAttribute(
        1,
        3,
        RL_FLOAT,
        false,
        5 * sizeof( float ),
        0
    );
    rlEnableVertexAttribute( 0 );
    rlEnableVertexAttribute( 1 );
}

void App::init( AppConfig const& config )
{
    setupFrameworks( config );

    setupAppEvents();

    simulation.init();

#if !defined( TEST )
    setupShaders( config );
#else
    setupShadersTest( config );
#endif
}

void updateFullscreenState()
{
#if !defined( NOGUI )
    if ( IsKeyPressed( KEY_F11 ) )
    {
        if ( IsWindowMaximized() )
        {
            RestoreWindow();
        }

        else
        {
            MaximizeWindow();
        }
    }

    if ( IsWindowResized() )
    {
        snx::EventDispatcher::notify( EventId::WINDOW_RESIZED );
    }
#endif
}

void updateDeveloperMode()
{
    if ( IsKeyPressed( KEY_F1 ) )
    {
        DeveloperMode::toggle();
    }
}

/// @brief Void argument in function signature needed for emscripten
void updateApp( void* arg )
{
    App& app = *(App*)arg;

    updateFullscreenState();
    updateDeveloperMode();

    app.screenWidth = { GetScreenWidth() };
    app.screenHeight = { GetScreenHeight() };

    app.mousePosition = { 0, 0 };
    if ( IsMouseButtonDown( MOUSE_LEFT_BUTTON ) )
    {
        app.mousePosition = GetMousePosition();
    }

    app.dt = GetFrameTime();

    app.simulation.update(
        app.screenWidth,
        app.screenHeight,
        app.mousePosition,
        app.dt
    );

    app.render();
}

void App::run()
{
#if defined( EMSCRIPTEN )
    emscripten_set_main_loop_arg(
        updateApp,
        this,
        AppData::FPS_TARGET /*FPS*/,
        1 /*Simulate infinite loop*/
    );
#elif defined( NOGUI )
    while ( !( IsKeyDown( AppData::EXIT_KEY ) ) )
    {
        updateApp( this );
    }
#else
    while ( !( WindowShouldClose() ) )
    {
        updateApp( this );
    }
#endif
}

void App::render()
{
#if !defined( NOGUI )
#if !defined( TEST )
    BeginDrawing();
    ClearBackground( ColorData::BG );

    DrawFPS( 0, 0 );

    switch ( simulation.state )
    {
        default:
            break;

        case State::SINGLE_CORE:
        case State::MULTITHREAD:
        {
            for ( Particle const& particle : simulation.particles )
            {
                ParticleSystem::drawParticle( particle );
            }

            break;
        }
        case State::GPU:
        {
            BeginShaderMode( shader );

            //* Draw vbo triangle as points
            rlEnablePointMode();

            rlEnableShader( shader.id );
            rlEnableVertexArray( vao );

            // glDrawArrays(
            //     GL_POINTS,
            //     0,
            //     PARTICLE_COUNT
            // );
            rlDrawVertexArray(
                0,
                PARTICLE_COUNT
            );

            rlDisableVertexArray();

            EndShaderMode();

            break;
        }
    }

    EndDrawing();
#else
    BeginDrawing(); // Seems to only update time?
    ClearBackground( BLACK );
    rlEnableShader( shader.id );
    rlEnableVertexArray( vao );
    rlDrawVertexArray(
        0,
        3
    );
    rlDisableVertexArray();
    EndDrawing();
#endif
#endif
}

void App::deinit()
{
    simulation.deinit();

#if !defined( NOGUI )
    //* Close window and opengl context
    CloseWindow();
#endif
}

void App::setupAppEvents()
{
    snx::EventDispatcher::addListener(
        EventId::WINDOW_RESIZED,
        [&]()
        {
            simulation.init();
        },
        true
    );
}

