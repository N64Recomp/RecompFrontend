#include "elements/ui_document.h"
#include "recompui/recompui.h"

namespace recompui {
    Document::Document(Rml::Element *base) : Element(base) {
        this->shim = false;
        set_width(100.0f, Unit::Percent);
        set_height(100.0f, Unit::Percent);

        is_root_document = true;
        is_nav_container = true;
        this->nav_type = NavigationType::Vertical;

        register_event_listeners(
            recompui::Events(
                recompui::EventType::Navigate,
                recompui::EventType::Focus));
    }
} // namespace recompui
