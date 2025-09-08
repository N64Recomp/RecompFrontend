#ifndef __RECOMP_INPUT_MAPPING_H__
#define __RECOMP_INPUT_MAPPING_H__

#include "input_types.h"
#include "SDL.h"

namespace recompinput {
    constexpr SDL_GameControllerButton SDL_CONTROLLER_BUTTON_SOUTH = SDL_CONTROLLER_BUTTON_A;
    constexpr SDL_GameControllerButton SDL_CONTROLLER_BUTTON_EAST = SDL_CONTROLLER_BUTTON_B;
    constexpr SDL_GameControllerButton SDL_CONTROLLER_BUTTON_WEST = SDL_CONTROLLER_BUTTON_X;
    constexpr SDL_GameControllerButton SDL_CONTROLLER_BUTTON_NORTH = SDL_CONTROLLER_BUTTON_Y;
    using default_game_input_device_mapping = std::unordered_map<GameInput, std::vector<InputField>>;

    const std::vector<InputField>& get_default_mapping_for_input(recompinput::InputDevice device, const GameInput input);
}

#endif
