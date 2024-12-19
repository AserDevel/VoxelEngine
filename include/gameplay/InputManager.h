#pragma once

#include "utilities/standard.h"

enum class Gamestate {
    INGAME = 1,
};

class InputManager {
private:
    Gamestate state;
    
public:
    InputManager(/* args */);
    ~InputManager();
};
