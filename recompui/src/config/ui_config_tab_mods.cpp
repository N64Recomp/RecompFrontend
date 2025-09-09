#include "recompui/config.h"
#include "recompinput/players.h"
#include "recompinput/profiles.h"
#include "elements/ui_element.h"
#include "librecomp/game.hpp"

namespace recompui {

void config::create_mods_tab(const std::string &name) {
    config::create_tab(
        name,
        config::mods::id,
        [](ContextId context, Element* parent) {
            auto el = context.create_element<Element>(
                parent,
                0,
                "div",
                true
            );
            el->set_text("INSERT MODS TAB STUFF HERE LOL");
        }
    );
}

} // namespace recompui
