#pragma once

#include "recompinput/recompinput.h"
#include "elements/ui_element.h"
#include "elements/ui_svg.h"
#include "elements/ui_button.h"
#include "elements/ui_pill_button.h"
#include "elements/ui_select.h"

namespace recompui {

// player index, profile index
using on_select_player_profile_callback = std::function<void(int, int)>;
// player index
using on_edit_player_profile_callback = std::function<void(int)>;

enum class PlayerCardIcon {
    None,
    Keyboard,
    Controller,
    Waiting,
    Recording
};

class PlayerCard : public Element {
protected:
    bool is_open = false;
    Element *card = nullptr;
    Svg* icon = nullptr;
    Select *profile_select = nullptr;
    PillButton *multiplayer_pill = nullptr;
    int player_index = -1;
    bool is_assignment_card = false;
    PlayerCardIcon cur_icon = PlayerCardIcon::None;
    bool was_player_assigning = false;

    on_select_player_profile_callback on_select_profile_callback;
    on_edit_player_profile_callback on_edit_profile_callback;

    virtual void process_event(const Event &e) override;
    std::string_view get_type_name() override { return "PlayerCard"; }
private:
    void on_select_player_profile(int profile_index);
    void on_edit_profile();
    void create_add_multiplayer_pill();
public:
    PlayerCard(Element *parent, int player_index, bool is_assignment_card = false);
    virtual ~PlayerCard();
    void update_assignment_player_card();
    void update_player_card_icon();
    void set_on_select_profile_callback(on_select_player_profile_callback callback) {
        on_select_profile_callback = std::move(callback);
    }
    void set_on_edit_profile_callback(on_edit_player_profile_callback callback) {
        on_edit_profile_callback = std::move(callback);
    }

    static constexpr float assign_player_card_size = 128.0f;
    static constexpr float assign_player_card_icon_size = 64.0f;

    static constexpr float static_player_card_size = 256.0f;
    static constexpr float static_player_card_icon_size = 128.0f;
};

} // namespace recompui
