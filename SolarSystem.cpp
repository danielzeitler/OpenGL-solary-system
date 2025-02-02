#include "SolarSystem.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "Skybox.h"
#include "Moon.h"
#include "SpecialPlanet.h"
#include "Venus.h"
#include "ShaderPlanet.h"
#include "ParticleEmitter.h"
#include "Model.h"

std::string progName = "SolarSystem";

// Our SDL_Window ( just like with SDL2 wihout OpenGL)
SDL_Window *mainWindow;

// Our opengl context handle
SDL_GLContext mainContext;

// SDL-Timer callback pointer
SDL_TimerID timerID;

// Frame Counter
int frameCount = 0;

// Avg FPS
float avgFps = 0;

int screenWidth = 1024;
int screenHeight = 768;

float renderTime = 0;

// Init Skybox
Skybox skybox;

// Init Camera
Camera g_Camera;

glm::vec3 g_InitialCameraPosition = glm::vec3(0,0,15);

//glm::quat g_InitialCameraRotation = QUAT_IDENTITY;

//-----------------------------------------------------------------

bool stopRenderLoop = false;

//-----------------------------------------------------------------

int g_W, g_A, g_S, g_D;

int g_M1, g_M2, g_M3;

//-----------------------------------------------------------------

// Init Moon
Moon *moon;

// Init Special Planet
SpecialPlanet *specialPlanet;

// Init Special Planet
Venus *venus;

// Init Special Planet
ShaderPlanet *shaderPlanet;

// Init Particle Emiter
ParticleEmitter *particleEmitter;

// Init Model Loader
Model* model;

glm::quat g_SunRotation = QUAT_IDENTITY;
glm::vec3 g_SunPosition(10,10,10);
glm::vec4 g_lightColor = glm::vec4(1.0, 1.0, 1.0, 1.0);

void UpdateModelMatrices();

void Cleanup();

void PrintOpenGLVersion() {
    std::cout << "-------------------------------------------------------\n";
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "INFO: OpenGL Shader: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "INFO: OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "-------------------------------------------------------\n";
}

void InitSDL() {
    // init SDL video system
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Faild to init SDL\n";
        exit(1);
    }

    // create window
    mainWindow = SDL_CreateWindow(progName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!mainWindow) {
        std::cerr << "Unable to create window\n";
        exit(1);
    }

    // Set OpenGL context profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // set OpenGL version (version 3.3)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // set double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    //-------------------------------------------

    // create context
    mainContext = SDL_GL_CreateContext(mainWindow);

    if (!mainContext) {
        std::cerr << "Unable to create context\n";
        Cleanup();
        exit(1);
    }

    // turn (on/off) V-Sync
    SDL_GL_SetSwapInterval(0);

    std::cout << "Init SDL done.\n";

}

void InitGLEW() {

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        std::cerr << "Threr was a Problem initalizing GLEW\n";
        Cleanup();
        exit(1);
    }

    if (!GLEW_VERSION_4_1) {
        std::cerr << "OpenGL 4.1 required version support not present\n";
        Cleanup();
        exit(1);
    }

    std::cout << "Init GLEW done.\n";
}

void InitGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    std::cout << "Init GL done.\n";
}

void InitViewport() {
    g_Camera.SetPosition(g_InitialCameraPosition);
    g_Camera.SetRotation(QUAT_IDENTITY);

    g_Camera.SetViewport(0,0,screenWidth, screenHeight);
    g_Camera.SetProjectionRH(60.0f, screenWidth / (float) screenHeight, 0.1f, 1000.0f);

    UpdateModelMatrices();

    model->setGLightColor(g_lightColor);

    std::cout << "Init Viewport done.\n";
}


void InitModel() {
    Model::ModelOptions opt;
    opt.light_scale = 3.0f;
    opt.model_scale = glm::vec3(3.0f);
    opt.model_position = glm::vec3(0.0, 3.0, 0.0);


    model = new Model("spaceship.obj", "../../res/models/spaceship/", opt);

    std::cout << "Model initialized.\n";
}

void UpdateModelMatrices() {
    model->setGProjectionMatrix(g_Camera.GetProjectionMatrix());
    model->setGViewMatrix(g_Camera.GetViewMatrix());

    model->setGCameraPosition(g_Camera.GetPosition());
    model->setGSunPosition(g_SunPosition);

}

void updateObjectMatrices() {
    skybox.setGProjectionMatrix(g_Camera.GetProjectionMatrix());
    skybox.setGViewMatrix(g_Camera.GetViewMatrix());

    // forEach planet
    moon->setGProjectionMatrix(g_Camera.GetProjectionMatrix());
    moon->setGViewMatrix(g_Camera.GetViewMatrix());
    moon->setGCameraPosition(g_Camera.GetPosition());
    moon->setGSunPosition(g_SunPosition);

    // TODO: move to initPlanet
    moon->setGLightColor(g_lightColor);

    // specialPlanet planet
    specialPlanet->setGProjectionMatrix(g_Camera.GetProjectionMatrix());
    specialPlanet->setGViewMatrix(g_Camera.GetViewMatrix());
    specialPlanet->setGCameraPosition(g_Camera.GetPosition());
    specialPlanet->setGSunPosition(g_SunPosition);

    // TODO: move to initPlanet
    specialPlanet->setGLightColor(g_lightColor);

    // Venus planet
    venus->setGProjectionMatrix(g_Camera.GetProjectionMatrix());
    venus->setGViewMatrix(g_Camera.GetViewMatrix());
    venus->setGCameraPosition(g_Camera.GetPosition());
    venus->setGSunPosition(g_SunPosition);

    // TODO: move to initPlanet
    shaderPlanet->setGLightColor(g_lightColor);

    // ShaderPlanet planet
    shaderPlanet->setGProjectionMatrix(g_Camera.GetProjectionMatrix());
    shaderPlanet->setGViewMatrix(g_Camera.GetViewMatrix());
    shaderPlanet->setGCameraPosition(g_Camera.GetPosition());
    shaderPlanet->setGSunPosition(g_SunPosition);

    // TODO: move to initPlanet
    shaderPlanet->setGLightColor(g_lightColor);


    // Particle Emitter
    particleEmitter->updateMatrices(g_Camera.GetProjectionMatrix(), g_Camera.GetViewMatrix());
}


void DrawScene(float tpf, float time) {
    skybox.draw();

    // forEach planet
    moon->draw(tpf);
    specialPlanet->draw(tpf, time);
    venus->draw(tpf, time);
    shaderPlanet->draw(tpf, time);

    // Particle Emitter draw
    particleEmitter->draw(tpf);

    // Draw modle
    model->draw();

    // swap back and front buffer (show frame)
    SDL_GL_SwapWindow(mainWindow);

    // inc FrameCount
    frameCount++;
}

void MoveObjects(float tpf) {

    float speed = 0.005f * tpf;

    g_Camera.Translate(glm::vec3((g_A - g_D), (g_S - g_W),0) * speed);

    updateObjectMatrices();

    // TODO: forEach planets
    moon->update(tpf);
    specialPlanet->update(tpf);
    venus->update(tpf);
    shaderPlanet->update(tpf);

    // Particle Emitter
    particleEmitter->update(tpf);
}

Uint32 CalcFPS(Uint32 interval, void *param) {

    // Calc avg fps
    avgFps = frameCount / (interval / 1000.0f);

    // reset frame count
    frameCount = 0;

    // Set window title with fps
    std::stringstream winTitle;

    winTitle << progName << " @ " << std::fixed << std::setprecision(1) << avgFps << " fps";

#ifndef __APPLE__
    SDL_SetWindowTitle(mainWindow, winTitle.str().c_str());
#else
    std::cout << winTitle.str() << std::endl;
#endif

    timerID = SDL_AddTimer(interval, CalcFPS, param);

    return 0;
}

void SDLKeyboardHandler(SDL_Keysym key, int down) {
    switch(key.sym) {
        case SDLK_w:
            g_W = down;
            break;
        case SDLK_a:
            g_A = down;
            break;
        case SDLK_v:
            break;
        case SDLK_c:
            break;
        case SDLK_s:
            g_S = down;
            break;
        case SDLK_d:
            g_D = down;
            break;
        case SDLK_r:
            g_Camera.SetPosition(g_InitialCameraPosition);
            g_Camera.SetRotation(QUAT_IDENTITY);

            particleEmitter->setRotation(QUAT_IDENTITY);
            model->setGRotation(QUAT_IDENTITY);
            UpdateModelMatrices();
            updateObjectMatrices();

            break;
        case SDLK_ESCAPE:
            stopRenderLoop = true;
            break;
        default:
            break;
    }
}

void MouseMoveHandler(SDL_MouseMotionEvent event) {

    if (g_M2) {
        float speed = 0.01f;

        g_Camera.Translate(glm::vec3(-event.xrel, event.yrel, 0) * speed);

        updateObjectMatrices();
    }

    if (g_M3) {
        glm::quat cam_rot_x = glm::angleAxis<float>(0.01 * -event.yrel, glm::vec3(1,0,0));
        glm::quat cam_rot_y = glm::angleAxis<float>(0.01 * -event.xrel, glm::vec3(0,1,0));

        g_Camera.SetRotation(g_Camera.GetRotation() * cam_rot_x * cam_rot_y);

        updateObjectMatrices();
    }

    if (g_M1) {
        // Get rotations
        glm::quat particle_rotation = particleEmitter->getRotation();
        glm::quat model_rotation = model->getGRotation();

        glm::vec3 axis_x = glm::normalize(glm::vec3(1,0,0) * glm::toMat3(model_rotation));
        glm::vec3 axis_y = glm::normalize(glm::vec3(0,1,0) * glm::toMat3(model_rotation));

        glm::quat rot_x = glm::angleAxis<float>(0.01 * event.yrel, axis_x);
        glm::quat rot_y = glm::angleAxis<float>(0.01 * event.xrel, axis_y);

        particle_rotation = particle_rotation * rot_x * rot_y;

        // Modell Rotation
        model_rotation = model_rotation * rot_x * rot_y;
        model->setGRotation(model_rotation);


        // Particle Emitter
        particleEmitter->setRotation(particle_rotation);
        particleEmitter->updateMatrices(g_Camera.GetProjectionMatrix(), g_Camera.GetViewMatrix());
    }


}

void MouseWheelHandler(SDL_MouseWheelEvent event) {

    float speed = 0.5f;

    g_Camera.Translate(glm::vec3(0,0,-event.y * speed));

    updateObjectMatrices();

}



void Render() {

    timerID = SDL_AddTimer(3 * 1000 , CalcFPS, nullptr);

    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 last = 0;
    float tpf = 0;

     while (!stopRenderLoop) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) stopRenderLoop = true;

            if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                SDLKeyboardHandler(event.key.keysym, 1);
            }

            if (event.type == SDL_KEYUP && event.key.repeat == 0) {
                SDLKeyboardHandler(event.key.keysym, 0);
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) g_M1 = 1;
                if (event.button.button == SDL_BUTTON_RIGHT) g_M2 = 1;
                if (event.button.button == SDL_BUTTON_MIDDLE) g_M3 = 1;

            }

            if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) g_M1 = 0;
                if (event.button.button == SDL_BUTTON_RIGHT) g_M2 = 0;
                if (event.button.button == SDL_BUTTON_MIDDLE) g_M3 = 0;

            }

            if (event.type == SDL_MOUSEMOTION)
                MouseMoveHandler(event.motion);

            if (event.type == SDL_MOUSEWHEEL)
                MouseWheelHandler(event.wheel);

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {

                screenWidth = event.window.data1;
                screenHeight = event.window.data2;

                g_Camera.SetViewport(0,0,screenWidth, screenHeight);
                g_Camera.SetProjectionRH(60.0f, screenWidth / (float) screenHeight, 0.1f, 1000.0f);

                updateObjectMatrices();

            }

        }

        // Move objects
        MoveObjects(tpf);

        // draw the scene
        DrawScene(tpf, renderTime);

        // calc tpf
        last = now;
        now = SDL_GetPerformanceCounter();
        tpf = ((now - last) * 1000.0 /(float) SDL_GetPerformanceFrequency());

        // store time
        renderTime = SDL_GetTicks() / 1000.0f;
    }
}

int main(int argc, char *argv[]) {

    InitSDL();

    InitGLEW();

    InitGL();

    InitModel();

    InitViewport();

    PrintOpenGLVersion();

    //---------------------------------------

    skybox.initAll();

    moon = new Moon();
    specialPlanet = new SpecialPlanet();
    venus = new Venus();
    shaderPlanet = new ShaderPlanet();

    // Particle Emitter
    particleEmitter = new ParticleEmitter(500, 3);

    updateObjectMatrices();

    //---------------------------------------

    std::cout << "Rendering...\n";

    Render();

    std::cout << "Rendering done, bye.\n";

    Cleanup();

    return 0;
}

void Cleanup() {
    SDL_GL_DeleteContext(mainContext);

    SDL_DestroyWindow(mainWindow);

    SDL_Quit();
}