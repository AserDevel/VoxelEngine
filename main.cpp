#include "utilities/standard.h"
#include "rendering/Mesh.h"
#include "rendering/Shader.h"
#include "rendering/Camera.h"
#include "world/WorldManager.h"
#include "rendering/Renderer.h"
#include "world/ChunkGenerator.h"
#include "physics/AABB.h"
#include "physics/PhysicsEngine.h"
#include "gameplay/Player.h"

// Window dimensions
int WINDOW_SIZE = 600;
float ASPECT_RATIO = 16.0f / 9.0f;
bool isRunning;

bool initializeWindow(SDL_Window** window, SDL_GLContext* context) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    *window = SDL_CreateWindow(
        "Voxels",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_SIZE * ASPECT_RATIO,
        WINDOW_SIZE,
        SDL_WINDOW_OPENGL
    );

    if (!window) {
        std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    *context = SDL_GL_CreateContext(*window);
    if (!*context) {
        std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return false;
    }

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        SDL_GL_DeleteContext(*context);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return false;
    }

    glViewport(0, 0, WINDOW_SIZE*ASPECT_RATIO, WINDOW_SIZE);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    return true;
}

void processKeyStates(float deltaTime, Camera& camera) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    float speed = 10.0f;
    if (state[SDL_SCANCODE_LCTRL])
        speed *= 2;
    if (state[SDL_SCANCODE_W]) 
        camera.processKeyboardInput("FORWARD", deltaTime, speed);
    if (state[SDL_SCANCODE_S]) 
        camera.processKeyboardInput("BACKWARD", deltaTime, speed);
    if (state[SDL_SCANCODE_A]) 
        camera.processKeyboardInput("LEFT", deltaTime, speed);
    if (state[SDL_SCANCODE_D]) 
        camera.processKeyboardInput("RIGHT", deltaTime, speed);
    if (state[SDL_SCANCODE_SPACE]) 
        camera.processKeyboardInput("UP", deltaTime, speed);
    if (state[SDL_SCANCODE_LSHIFT])
        camera.processKeyboardInput("DOWN", deltaTime, speed);
    if (state[SDL_SCANCODE_ESCAPE])
        isRunning = false;
}

int main(int argc, char* argv[]) {
    // Global instances
    SDL_Window* window = nullptr;
    SDL_GLContext glContext;

    // Initialize SDL and OpenGL
    if (!initializeWindow(&window, &glContext)) {
        std::cerr << "Error initializing window " << SDL_GetError() << std::endl;
        return -1;
    }
    
    /*
    Camera camera = Camera(
        Vec3(5.0f, 140.0, 5.0f),  // Position
        Vec3(0.0f, 1.0f, 0.0f),    // Up vector
        0,                         // Yaw
        0,                         // Pitch
        90.0f,                     // FOV
        ASPECT_RATIO,              // Aspect ratio
        1.0f,                      // Near plane
        5000.0f                    // Far plane
    );
    */

    ThreadManager threadManager(4);
    WorldManager worldManager(threadManager, 8);
    Player player(Vec3(5.0, 140.0, 5.0), worldManager);
    Renderer renderer(worldManager, player.camera);

    PhysicsEngine physicsEngine(worldManager);
    physicsEngine.addShape(&player.shape);
    
    float lastFrameTime = SDL_GetTicks() / 1000.0f;
    isRunning = true;
    while (isRunning) {
        float currentTime = SDL_GetTicks() / 1000.0f;
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                } else {
                    player.handleEvent(event);
                } 
                break;
            case SDL_MOUSEMOTION:
                player.camera.processMouseInput(event.motion.xrel, event.motion.yrel, 0.02f);
                break;
            case SDL_MOUSEBUTTONDOWN:
                player.handleEvent(event);
                break;
            default:
                break;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glDepthRange(0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        player.processInput();
        physicsEngine.update(deltaTime);
        player.update();
        worldManager.updateChunks(player.camera.position);
        renderer.render();

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;  
}
