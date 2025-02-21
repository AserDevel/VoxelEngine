#include "gameplay/Player.h"

Player::Player(const Vec3& startPosition, WorldManager& worldManager) : worldManager(worldManager) {
    shape.position = startPosition;
    shape.dimensions = Vec3(0.7, 1.8, 0.7);

    camera = Camera(
        startPosition,             // Position
        Vec3(0.0f, 1.0f, 0.0f),    // Up vector
        0,                         // Yaw
        0,                         // Pitch
        90.0f,                     // FOV
        16.0f / 9.0f,              // Aspect ratio
        1.0f,                      // Near plane
        5000.0f                    // Far plane
    );

    inventory[0] = ID_DIRT;
    inventory[1] = ID_GRASS;
    inventory[2] = ID_LEAVES;
    inventory[3] = ID_WOOD;
    inventory[4] = ID_SAND;
    inventory[5] = ID_WATER;
    inventory[6] = ID_STONE;
    inventory[7] = ID_SNOW;
    inventory[8] = ID_METAL;
}

void Player::processInput() {
    float speed = baseSpeed;
    const Uint8* state = SDL_GetKeyboardState(NULL);
    shape.velocity = Vec3(0.0);
    if (state[SDL_SCANCODE_LCTRL])
        speed *= 2;
    if (state[SDL_SCANCODE_W]) {
        shape.velocity.x -= (cosf(camera.yaw) * speed);
        shape.velocity.z -= (sinf(camera.yaw) * speed);
    }
    if (state[SDL_SCANCODE_S]) {
        shape.velocity.x += (cosf(camera.yaw) * speed);
        shape.velocity.z += (sinf(camera.yaw) * speed);
    }
    if (state[SDL_SCANCODE_A]) 
        shape.velocity -= camera.right * speed;
    if (state[SDL_SCANCODE_D]) 
        shape.velocity += camera.right * speed;
    if (state[SDL_SCANCODE_SPACE]) 
        shape.velocity.y += speed;
    if (state[SDL_SCANCODE_LSHIFT])
        shape.velocity.y -= speed;
}

void Player::update() {
    if (camera.position != shape.position) {
        camera.position = shape.position;
        camera.isDirty = true;
    }
}

void Player::handleEvent(const SDL_Event& event) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT) {                    
            Vec3 startPoint = camera.position;
            Vec3 endPoint = camera.position - camera.front * 4;
            Vec3 voxelPos;
            Vec3 normal;
            if (worldManager.worldRayDetection(startPoint, endPoint, voxelPos, normal)) {
                worldManager.removeVoxel(voxelPos);
            }
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            Vec3 startPoint = camera.position;
            Vec3 endPoint = camera.position - camera.front * 4;
            Vec3 voxelPos;
            Vec3 normal;
            if (worldManager.worldRayDetection(startPoint, endPoint, voxelPos, normal)) {
                worldManager.addVoxel(voxelPos + normal, inventory[selectedItem]);
            }
        }
        break;
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_1:
            selectedItem = 0;
            break;
        case SDLK_2:
            selectedItem = 1;
            break;
        case SDLK_3:
            selectedItem = 2;
            break;
        case SDLK_4:
            selectedItem = 3;
            break;
        case SDLK_5:
            selectedItem = 4;
            break;
        case SDLK_6:
            selectedItem = 5;
            break;
        case SDLK_7:
            selectedItem = 6;
            break;
        case SDLK_8:
            selectedItem = 7;
            break;
        case SDLK_9:
            selectedItem = 8;
            break;
        default:
            break;
        } 
        break;
    default:
        break;
    }
}