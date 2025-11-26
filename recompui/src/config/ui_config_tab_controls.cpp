#include "recompui/config.h"
#include "recompinput/players.h"
#include "recompinput/profiles.h"
#include "elements/ui_element.h"
#include "ui_config_page_controls.h"
#include "librecomp/game.hpp"

namespace recompui {

static std::vector<struct GameInputContext> game_input_contexts = {};

void config::controls::add_game_input(
    const std::string &name,
    const std::string &description,
    recompinput::GameInput input_id,
    bool clearable
) {
    game_input_contexts.push_back(GameInputContext{
        name,
        description,
        input_id,
        clearable
    });
}

void config::create_controls_tab(const std::string &name) {
    for (int i = 0; i < static_cast<int>(recompinput::GameInput::COUNT); i++) {
        recompinput::GameInput input = static_cast<recompinput::GameInput>(i);
        if (recompinput::get_game_input_disabled(input)) {
            continue;
        }

        config::controls::add_game_input(
            recompinput::get_game_input_name(input),
            recompinput::get_game_input_description(input),
            input,
            !(input == recompinput::GameInput::TOGGLE_MENU || input == recompinput::GameInput::ACCEPT_MENU)
        );
    }

    config::create_tab(
        name,
        config::controls::id,
        [](ContextId context, Element* parent) {
            context.create_element<ConfigPageControls>(
                parent,
                recompinput::players::get_max_number_of_players(),
                game_input_contexts
            );
        },
        nullptr,
        [](TabCloseContext close_context) {
            // Save the current control mappings when closing the controls tab.
            recompinput::profiles::save_controls_config(recomp::get_config_path() / (config::controls::id + ".json"));
        }
    );
}

} // namespace recompui
