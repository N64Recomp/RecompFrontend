#include "elements/ui_document.h"
#include "recompui/recompui.h"

namespace recompui {
    Document::Document(Rml::Element *base) : Element(base) {
        this->shim = false;
        set_width(100.0f, Unit::Percent);
        set_height(100.0f, Unit::Percent);

        is_nav_container = true;
        this->nav_type = NavigationType::Vertical;

        register_event_listeners(
            recompui::Events(
                recompui::EventType::Navigate,
                recompui::EventType::Focus));
    }

    bool Document::handle_navigation_event(Rml::Event &event) {
        if (
            event.GetId() == Rml::EventId::Keydown &&
            event.GetPhase() == Rml::EventPhase::Bubble &&
            event.IsPropagating()
        ) {
            int key_identifier = event.GetParameter<int>("key_identifier", Rml::Input::KI_UNKNOWN);
            if (
                key_identifier != Rml::Input::KI_LEFT &&
                key_identifier != Rml::Input::KI_RIGHT &&
                key_identifier != Rml::Input::KI_UP &&
                key_identifier != Rml::Input::KI_DOWN
            ) {
                return false;
            }

            std::string nav_key_name;
            int vertical_nav = 0;
            int horizontal_nav = 0;
            switch (key_identifier) {
                case Rml::Input::KI_UP:    vertical_nav   = -1; nav_key_name = "up";    break;
                case Rml::Input::KI_DOWN:  vertical_nav   =  1; nav_key_name = "down";  break;
                case Rml::Input::KI_LEFT:  horizontal_nav = -1; nav_key_name = "left";  break;
                case Rml::Input::KI_RIGHT: horizontal_nav =  1; nav_key_name = "right"; break;
            }

            auto ctx = get_current_context();
            auto navigation_from_element = ctx.get_focused_element();
            auto original_focused_element = navigation_from_element;
            if (navigation_from_element == nullptr) {
                // No focused element, cannot navigate.
                return false;
            }

            nav_children.clear();
            build_navigation(this, navigation_from_element);

            Element *nav_parent = navigation_from_element->get_nav_parent();
            Element *fallback_wrap_element = nullptr;
            while (nav_parent != nullptr) {
                Element *next_focus = nav_parent->try_get_nav_direction(
                    vertical_nav,
                    horizontal_nav,
                    navigation_from_element,
                    &fallback_wrap_element
                );

                // If found, dive into the next focus element until finding the bottom most nav container
                if (next_focus != nullptr) {
                    next_focus = Element::dive_to_best_nav_child(next_focus, navigation_from_element);
                }

                if (next_focus != nullptr) {
                    event.StopPropagation();
                    next_focus->focus();
                    return true;
                }

                navigation_from_element = nav_parent;
                nav_parent = nav_parent->get_nav_parent();
            }

            if (fallback_wrap_element != nullptr) {
                // Wrap to the fallback element if set
                fallback_wrap_element = Element::dive_to_best_nav_child(fallback_wrap_element, original_focused_element);

                event.StopPropagation();
                fallback_wrap_element->focus();
                return true;
            }

            return false;
        } else {
            return false;
        }
    }
} // namespace recompui
