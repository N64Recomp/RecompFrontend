#include "recompui/config.h"
#include "recompinput/players.h"
#include "recompinput/profiles.h"
#include "composites/ui_mod_menu.h"
#include "composites/ui_mod_discovery.h"
#include "elements/ui_element.h"
#include "librecomp/game.hpp"

namespace recompui {

void config::create_mods_tab(const std::string &name) {
    config::create_tab(
        name,
        config::mods::id,
        [](ContextId context, Element* parent) {
            context.create_element<ModMenu>(parent);
        }
    );

    recompui::update_mod_list(false);

    // Verify we actually SUPPORT mod discovery
    if (!get_discovery_url().empty()) {
        // We don't really want to deal with the risk of doing a mod reload/install while the game is running
        // So I just disable displaying this entirely because its the stupid simple option.
        if (ultramodern::is_game_started()) {
            return;
        }

        config::create_tab(
            config::mod_discovery::tab_name,
            config::mod_discovery::id,
            [](ContextId context, Element* parent) {
                context.create_element<ModDownloadsPanel>(parent);
            }
        );
    }}

} // namespace recompui
