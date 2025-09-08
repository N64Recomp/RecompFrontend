#include <unordered_map>
#include "recompinput.h"

namespace recompinput {

    using default_game_input_device_mapping = std::unordered_map<GameInput, std::vector<InputField>>;

    #define N64_MAPPING_KB(gameinput, scancode) \
        { \
            gameinput, \
            {{.input_type = InputType::Keyboard, .input_id = scancode}} \
        }

    default_game_input_device_mapping default_n64_mappings_keyboard = {
        N64_MAPPING_KB(GameInput::A,                SDL_SCANCODE_SPACE),
        N64_MAPPING_KB(GameInput::B,                SDL_SCANCODE_LSHIFT),
        N64_MAPPING_KB(GameInput::L,                SDL_SCANCODE_E),
        N64_MAPPING_KB(GameInput::B,                SDL_SCANCODE_LSHIFT),
        N64_MAPPING_KB(GameInput::R,                SDL_SCANCODE_R),
        N64_MAPPING_KB(GameInput::Z,                SDL_SCANCODE_Q),
        N64_MAPPING_KB(GameInput::START,            SDL_SCANCODE_RETURN),
        N64_MAPPING_KB(GameInput::C_LEFT,           SDL_SCANCODE_LEFT),
        N64_MAPPING_KB(GameInput::C_RIGHT,          SDL_SCANCODE_RIGHT),
        N64_MAPPING_KB(GameInput::C_UP,             SDL_SCANCODE_UP),
        N64_MAPPING_KB(GameInput::C_DOWN,           SDL_SCANCODE_DOWN),
        N64_MAPPING_KB(GameInput::DPAD_LEFT,        SDL_SCANCODE_J),
        N64_MAPPING_KB(GameInput::DPAD_RIGHT,       SDL_SCANCODE_L),
        N64_MAPPING_KB(GameInput::DPAD_UP,          SDL_SCANCODE_I),
        N64_MAPPING_KB(GameInput::DPAD_DOWN,        SDL_SCANCODE_K),
        N64_MAPPING_KB(GameInput::X_AXIS_NEG,       SDL_SCANCODE_A),
        N64_MAPPING_KB(GameInput::X_AXIS_POS,       SDL_SCANCODE_D),
        N64_MAPPING_KB(GameInput::Y_AXIS_POS,       SDL_SCANCODE_W),
        N64_MAPPING_KB(GameInput::Y_AXIS_NEG,       SDL_SCANCODE_S),

        N64_MAPPING_KB(GameInput::TOGGLE_MENU,      SDL_SCANCODE_ESCAPE),
        N64_MAPPING_KB(GameInput::ACCEPT_MENU,      SDL_SCANCODE_RETURN),
        N64_MAPPING_KB(GameInput::BACK_MENU,        SDL_SCANCODE_F15),
        N64_MAPPING_KB(GameInput::APPLY_MENU,       SDL_SCANCODE_F),
        N64_MAPPING_KB(GameInput::TAB_LEFT_MENU,    SDL_SCANCODE_F16),
        N64_MAPPING_KB(GameInput::TAB_RIGHT_MENU,   SDL_SCANCODE_F17)
    };

    #define N64_MAPPING_CONT(gameinput, ...) \
        { \
            gameinput, \
            {__VA_ARGS__} \
        }

    #define TYPE_DIGITAL(scancode) \
        {.input_type = InputType::ControllerDigital, .input_id = (scancode)}

    #define TYPE_ANALOG_POS(scancode) \
        {.input_type = InputType::ControllerAnalog, .input_id =  ((scancode) + 1)}
    #define TYPE_ANALOG_NEG(scancode) \
        {.input_type = InputType::ControllerAnalog, .input_id = -((scancode) + 1)}

    default_game_input_device_mapping default_n64_mappings_controller = {
        N64_MAPPING_CONT(GameInput::A,              TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_SOUTH)),
        N64_MAPPING_CONT(GameInput::B,              TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_WEST)),
        N64_MAPPING_CONT(GameInput::L,              TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_LEFTSHOULDER)),
        N64_MAPPING_CONT(GameInput::R,              TYPE_ANALOG_POS(SDL_CONTROLLER_AXIS_TRIGGERRIGHT)),
        N64_MAPPING_CONT(GameInput::Z,              TYPE_ANALOG_POS(SDL_CONTROLLER_AXIS_TRIGGERLEFT)),
        N64_MAPPING_CONT(GameInput::START,          TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_START)),
        N64_MAPPING_CONT(GameInput::C_LEFT,         TYPE_ANALOG_NEG((SDL_CONTROLLER_AXIS_RIGHTX)),  TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_NORTH)),
        N64_MAPPING_CONT(GameInput::C_RIGHT,        TYPE_ANALOG_POS(SDL_CONTROLLER_AXIS_RIGHTX),    TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_EAST)),
        N64_MAPPING_CONT(GameInput::C_UP,           TYPE_ANALOG_NEG((SDL_CONTROLLER_AXIS_RIGHTY)),  TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_RIGHTSTICK)),
        N64_MAPPING_CONT(GameInput::C_DOWN,         TYPE_ANALOG_POS(SDL_CONTROLLER_AXIS_RIGHTY),    TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)),
        N64_MAPPING_CONT(GameInput::DPAD_LEFT,      TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_DPAD_LEFT)),
        N64_MAPPING_CONT(GameInput::DPAD_RIGHT,     TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_DPAD_RIGHT)),
        N64_MAPPING_CONT(GameInput::DPAD_UP,        TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_DPAD_UP)),
        N64_MAPPING_CONT(GameInput::DPAD_DOWN,      TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_DPAD_DOWN)),
        N64_MAPPING_CONT(GameInput::X_AXIS_NEG,     TYPE_ANALOG_NEG((SDL_CONTROLLER_AXIS_LEFTX))),
        N64_MAPPING_CONT(GameInput::X_AXIS_POS,     TYPE_ANALOG_POS(SDL_CONTROLLER_AXIS_LEFTX)),
        N64_MAPPING_CONT(GameInput::Y_AXIS_POS,     TYPE_ANALOG_NEG((SDL_CONTROLLER_AXIS_LEFTY))),
        N64_MAPPING_CONT(GameInput::Y_AXIS_NEG,     TYPE_ANALOG_POS(SDL_CONTROLLER_AXIS_LEFTY)),

        N64_MAPPING_CONT(GameInput::TOGGLE_MENU,    TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_BACK)),
        N64_MAPPING_CONT(GameInput::ACCEPT_MENU,    TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_SOUTH)),
        N64_MAPPING_CONT(GameInput::BACK_MENU,      TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_WEST)),
        N64_MAPPING_CONT(GameInput::APPLY_MENU,     TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_NORTH),      TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_START)),
        N64_MAPPING_CONT(GameInput::TAB_LEFT_MENU,  TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_LEFTSHOULDER)),
        N64_MAPPING_CONT(GameInput::TAB_RIGHT_MENU, TYPE_DIGITAL(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)),
    };

    #undef N64_MAPPING_KB
    #undef N64_MAPPING_CONT
    #undef TYPE_DIGITAL
    #undef TYPE_ANALOG_POS
    #undef TYPE_ANALOG_NEG

    const std::vector<InputField>& get_default_mapping_for_input(recompinput::InputDevice device, const GameInput input) {
        static const std::vector<InputField> empty_input_field{};
        default_game_input_device_mapping *mapping = nullptr;

        switch (device) {
            case InputDevice::Keyboard: mapping = &default_n64_mappings_keyboard; break;
            case InputDevice::Controller: mapping = &default_n64_mappings_controller; break;
            default: return empty_input_field;
        }

        auto it = mapping->find(input);
        if (it != mapping->end()) {
            return it->second;
        }
        return empty_input_field;
    }
}
