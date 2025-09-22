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

            /**
             * This logic is used to attempt to continue navigation in the same direction, even across different nav parents.
             * For example, the config modal header has two horizontal nav containers in a horizontal nav container,
             * one of the containers is for the main tabs and the other holds exit/close, e.g.
             *      [[tab1, tab2, tab3], [exit, close]]
             * If you are focused on `exit` and navigate left, it should go to `tab3` because it is the same type of navigation.
             * This is prioritized over what is set as the "is_primary_focus" element, which would be focused if you navigated vertically to this row.
             */
            bool should_continue_direction = nav_parent != nullptr;
            NavigationType preserved_nav_type = nav_parent != nullptr ? nav_parent->nav_type : NavigationType::None;
            NavigationType effective_type = get_effective_nav_type(preserved_nav_type);
            int preserved_dir = 0;
            if (effective_type == NavigationType::Horizontal) {
                preserved_dir = horizontal_nav;
            } else if (effective_type == NavigationType::Vertical) {
                preserved_dir = vertical_nav;
            }

            /**
             * - Start at the current focused element.
             * - Try to navigate within the current nav parent (using `nav_type` in `try_get_nav_direction`).
             * - If possible, dive into the next element in that direction until reaching the bottom most nav container.
             * - If not possible, go up one navigation parent level and try again.
             */
            while (nav_parent != nullptr) {
                Element *next_focus = nav_parent->try_get_nav_direction(
                    vertical_nav,
                    horizontal_nav,
                    navigation_from_element,
                    &fallback_wrap_element
                );

                // If found, dive into the next focus element until finding the bottom most nav container
                if (next_focus != nullptr) {
                    next_focus = Element::dive_to_best_nav_child(
                        next_focus,
                        preserved_nav_type,
                        preserved_dir,
                        navigation_from_element
                    );
                }

                if (next_focus != nullptr) {
                    event.StopPropagation();
                    next_focus->focus();
                    return true;
                }

                navigation_from_element = nav_parent;
                nav_parent = nav_parent->get_nav_parent();
                if (nav_parent != nullptr) {
                    should_continue_direction = should_continue_direction && (
                        nav_parent->nav_children.size() == 1 ||
                        are_nav_types_equal(nav_parent->nav_type, preserved_nav_type)
                    );
                } else {
                    should_continue_direction = false;
                    preserved_nav_type = NavigationType::None;
                }
            }

            if (fallback_wrap_element != nullptr) {
                // Wrap to the fallback element if set
                // TODO: The fallback_wrap_element should already be the deepest dive, check if this logic is redundant and remove if so.
                fallback_wrap_element = Element::dive_to_best_nav_child(fallback_wrap_element);

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
