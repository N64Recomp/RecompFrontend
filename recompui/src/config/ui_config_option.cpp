#include "ui_config_option.h"

namespace recompui {

constexpr float config_option_element_margin_vertical = 12.0f;
RenameMe_ConfigOptionElement::RenameMe_ConfigOptionElement(
    Element *parent,
    std::string option_id,
    size_t option_index,
    const recomp::config::Config *config,
    set_option_value_t set_option_value,
    on_option_hover_t on_hover
) :
    Element(parent, Events(EventType::Hover, EventType::Focus), "div", false),
    option_id(option_id),
    option_index(option_index),
    config(config),
    set_option_value(set_option_value),
    on_hover(on_hover)
{
    set_display(Display::Flex);
    set_position(Position::Relative);
    set_flex_direction(FlexDirection::Column);
    set_align_items(AlignItems::FlexStart);
    set_justify_content(JustifyContent::FlexStart);
    set_padding_top(config_option_element_margin_vertical);
    set_padding_left(12.0f);
    set_padding_right(12.0f);
    set_padding_bottom(config_option_element_margin_vertical);
    set_height_auto();
    set_width(100.0f, Unit::Percent);
    set_as_navigation_container(NavigationType::Auto);

    const recomp::config::ConfigOption &option = config->get_option(option_index);

    ContextId context = recompui::get_current_context();
    auto name_text = option.name;
    if (!name_text.empty()) {
        name_label = context.create_element<Label>(this, name_text, theme::Typography::LabelMD);
        name_label->set_margin_bottom(8.0f);
    } else {
        name_label = context.create_element<Label>(this, "", theme::Typography::LabelMD);
        set_padding_top(0.0f);
    }

    update_hidden();
}

recomp::config::ConfigValueVariant RenameMe_ConfigOptionElement::get_value() {
    if (config->requires_confirmation) {
        return config->get_temp_option_value(option_id);
    }
    return config->get_option_value(option_id);
}

bool RenameMe_ConfigOptionElement::get_disabled() {
    return config->is_config_option_disabled(option_index);
}

bool RenameMe_ConfigOptionElement::get_hidden() {
    return config->is_config_option_hidden(option_index);
}

void RenameMe_ConfigOptionElement::process_event(const Event &e) {
    switch (e.type) {
    case EventType::Hover: {
        bool active = std::get<EventHover>(e.variant).active;
        if (active) {
            on_hover(option_id);
        }
        break;
    }
    case EventType::Focus: {
        bool active = std::get<EventFocus>(e.variant).active;
        if (active) {
            on_hover(option_id);
            scroll_into_view();
        }
        break;
    }
    case EventType::Update:
        break;
    default:
        assert(false && "Unknown event type.");
        break;
    }
}

void RenameMe_ConfigOptionElement::update_hidden() {
    if (config->is_config_option_hidden(option_index)) {
        set_display(Display::None);
        set_enabled(false);
    } else {
        set_display(Display::Flex);
        set_enabled(true);
    }
}

// RenameMe_ConfigOptionEnum
RenameMe_ConfigOptionEnum::RenameMe_ConfigOptionEnum(
    Element *parent,
    std::string option_id,
    size_t option_index,
    const recomp::config::Config *config,
    set_option_value_t set_option_value,
    on_option_hover_t on_hover
) : RenameMe_ConfigOptionElement(parent, option_id, option_index, config, set_option_value, on_hover)
{
    ContextId context = recompui::get_current_context();
    Element *wrapper = context.create_element<Element>(this, 0, "div", false);
    wrapper->set_display(Display::Flex);
    wrapper->set_flex_direction(FlexDirection::Row);
    wrapper->set_align_items(AlignItems::Center);
    wrapper->set_justify_content(JustifyContent::FlexStart);
    wrapper->set_width(100.0f, Unit::Percent);
    wrapper->set_gap(12.0f);

    // Negates the extra padding that radio tabs automatically add.
    name_label->set_margin_bottom(4.0f);

    radio = context.create_element<Radio>(wrapper);
    radio->set_focus_callback([this](bool active) {
        if (active) {
            this->on_hover(this->option_id);
        }
    });
    radio->add_index_changed_callback([this](uint32_t index) {
        auto &enum_opt = this->config->get_option_config<recomp::config::ConfigOptionEnum>(this->option_index);
        if (index < enum_opt.options.size()) {
            this->set_option_value(this->option_id, enum_opt.options[index].value);
        }
    });

    details_label = context.create_element<Label>(wrapper, "", theme::Typography::LabelXS);
    details_label->set_color(theme::color::Primary);

    auto &enum_opt = config->get_option_config<recomp::config::ConfigOptionEnum>(this->option_index);
    for (uint32_t i = 0; i < enum_opt.options.size(); i++) {
        radio->add_option(enum_opt.options[i].name);
    }

    update_value();
    update_enum_details();
    update_enum_disabled();
    // enable_focus();
};

void RenameMe_ConfigOptionEnum::update_value() {
    recomp::config::ConfigValueVariant value_variant = get_value();
    uint32_t option_value = std::get<uint32_t>(value_variant);

    auto &enum_opt = config->get_option_config<recomp::config::ConfigOptionEnum>(this->option_index);
    for (uint32_t i = 0; i < enum_opt.options.size(); i++) {
        if (enum_opt.options[i].value == option_value) {
            radio->set_index(i);
            return;
        }
    }
    // No matching option, set invalid index so no option is selected
    radio->set_index(enum_opt.options.size());
};

void RenameMe_ConfigOptionEnum::update_disabled() {
    update_enum_disabled();
};

void RenameMe_ConfigOptionEnum::update_enum_details() {
    const std::string &enum_details = config->get_enum_option_details(option_index);
    if (enum_details.empty()) {
        details_label->set_text("");
    } else {
        details_label->set_text(enum_details);
    }
};

void RenameMe_ConfigOptionEnum::update_enum_disabled() {
    bool all_disabled = get_disabled();
    auto &enum_opt = config->get_option_config<recomp::config::ConfigOptionEnum>(option_index);
    for (uint32_t i = 0; i < enum_opt.options.size(); i++) {
        bool enum_disabled = all_disabled || config->get_enum_option_disabled(option_index, i);
        auto opt_element = radio->get_option_element(i);
        opt_element->set_enabled(!enum_disabled);
    }
};

// RenameMe_ConfigOptionNumber
RenameMe_ConfigOptionNumber::RenameMe_ConfigOptionNumber(
    Element *parent,
    std::string option_id,
    size_t option_index,
    const recomp::config::Config *config,
    set_option_value_t set_option_value,
    on_option_hover_t on_hover
) : RenameMe_ConfigOptionElement(parent, option_id, option_index, config, set_option_value, on_hover)
{
    ContextId context = recompui::get_current_context();
    auto &num_opt = config->get_option_config<recomp::config::ConfigOptionNumber>(option_index);

    slider = context.create_element<Slider>(this, num_opt.percent ? SliderType::Percent : SliderType::Double);
    slider->set_width(100.0f, Unit::Percent);
    slider->set_max_width(100.0f, Unit::Percent);

    slider->set_min_value(num_opt.min);
    slider->set_max_value(num_opt.max);
    slider->set_step_value(num_opt.step);
    slider->set_precision(num_opt.precision);
    slider->add_value_changed_callback([this](double v){
        this->set_option_value(this->option_id, v);
    });
    slider->set_focus_callback([this](bool active) {
        if (active) {
            this->on_hover(this->option_id);
        }
    });
    update_value();
    // enable_focus();
    update_disabled();
};

void RenameMe_ConfigOptionNumber::update_value() {
    recomp::config::ConfigValueVariant value_variant = get_value();
    double value = std::get<double>(value_variant);
    slider->set_value(value);
};

void RenameMe_ConfigOptionNumber::update_disabled() {
    bool disabled = get_disabled();
    set_enabled(!disabled);
};

// RenameMe_ConfigOptionString
RenameMe_ConfigOptionString::RenameMe_ConfigOptionString(
    Element *parent,
    std::string option_id,
    size_t option_index,
    const recomp::config::Config *config,
    set_option_value_t set_option_value,
    on_option_hover_t on_hover
) : RenameMe_ConfigOptionElement(parent, option_id, option_index, config, set_option_value, on_hover)
{
    // Negates the extra padding that text inputs automatically add.
    name_label->set_margin_bottom(4.0f);

    ContextId context = recompui::get_current_context();
    text_input = context.create_element<TextInput>(this);
    text_input->set_width(100.0f, Unit::Percent);
    text_input->set_max_width(100.0f, Unit::Percent);
    update_value();
    text_input->add_text_changed_callback([this](const std::string &text){
        this->set_option_value(this->option_id, text);
    });
    text_input->set_focus_callback([this](bool active) {
        if (active) {
            this->on_hover(this->option_id);
        }
    });
    // enable_focus();
};

void RenameMe_ConfigOptionString::update_value() {
    std::string value = std::get<std::string>(get_value());
    text_input->set_text(value);
};

void RenameMe_ConfigOptionString::update_disabled() {
    bool disabled = get_disabled();
    text_input->set_enabled(!disabled);
};

// RenameMe_ConfigOptionBool
RenameMe_ConfigOptionBool::RenameMe_ConfigOptionBool(
    Element *parent,
    std::string option_id,
    size_t option_index,
    const recomp::config::Config *config,
    set_option_value_t set_option_value,
    on_option_hover_t on_hover
) : RenameMe_ConfigOptionElement(parent, option_id, option_index, config, set_option_value, on_hover)
{
    // Negates the extra padding that text inputs automatically add.
    name_label->set_margin_bottom(4.0f);

    ContextId context = recompui::get_current_context();
    toggle = context.create_element<Toggle>(this, ToggleSize::Medium);

    update_value();
    toggle->add_checked_callback([this](bool checked){
        this->set_option_value(this->option_id, checked);
    });
    // toggle->set_focus_callback([this](bool active) {
    //     if (active) {
    //         this->on_hover(this->option_id);
    //     }
    // });
    // enable_focus();
};

void RenameMe_ConfigOptionBool::update_value() {
    bool value = std::get<bool>(get_value());
    toggle->set_checked(value);
};

void RenameMe_ConfigOptionBool::update_disabled() {
    bool disabled = get_disabled();
    toggle->set_enabled(!disabled);
};

} // namespace recompui
