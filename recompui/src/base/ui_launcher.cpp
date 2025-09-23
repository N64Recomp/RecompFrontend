#include "recompui/recompui.h"
#include "recompui/config.h"
#include "recompui/program_config.h"
#include "base/ui_launcher.h"
#include "composites/ui_mod_menu.h"
#include "util/file.h"
#include "librecomp/game.hpp"
#include "ultramodern/ultramodern.hpp"
#include "nfd.h"
#include <filesystem>
#include "elements/ui_svg.h"

// TODO: Store somewhere else and have base game register/initialize.
extern std::vector<recomp::GameEntry> supported_games;



namespace recompui {
    GameOptionsMenu::GameOptionsMenu(
        Element* parent, std::u8string game_id, std::string mod_game_id, GameOptionsMenuLayout layout
    ) : Element(parent, 0, "div", false),
        game_id(game_id),
        mod_game_id(mod_game_id),
        layout(layout)
    {
        rom_valid = recomp::is_rom_valid(game_id);

        set_position(Position::Absolute);
        set_display(Display::Flex);
        set_height_auto();
        set_width(50.0f, Unit::Percent);
        set_flex_direction(FlexDirection::Column);
        set_gap(8.0f);
        set_align_items(AlignItems::Center);

        set_as_navigation_container(NavigationType::Vertical);
        set_nav_wrapping(true);

        switch (layout) {
            case GameOptionsMenuLayout::Left:
                set_left(24.0f);
                set_bottom(24.0f);
                break;
            case GameOptionsMenuLayout::Center:
                set_left(50.0f, Unit::Percent);
                set_bottom(25.0f, Unit::Percent);
                set_translate_2D(-50.0f, 0.0f, Unit::Percent);
                break;
            case GameOptionsMenuLayout::Right:
                set_right(24.0f);
                set_bottom(24.0f);
                break;
        }
    }

    void GameOptionsMenu::select_rom(std::function<void(bool)> callback) {
        recompui::file::open_file_dialog([this, callback](bool success, const std::filesystem::path& path) {
            if (success) {
                recomp::RomValidationError rom_error = recomp::select_rom(path, this->game_id);
                switch (rom_error) {
                    case recomp::RomValidationError::Good:
                        callback(true);
                        return;
                    case recomp::RomValidationError::FailedToOpen:
                        recompui::message_box("Failed to open ROM file.");
                        break;
                    case recomp::RomValidationError::NotARom:
                        recompui::message_box("This is not a valid ROM file.");
                        break;
                    case recomp::RomValidationError::IncorrectRom:
                        recompui::message_box("This ROM is not the correct game.");
                        break;
                    case recomp::RomValidationError::NotYet:
                        recompui::message_box("This game isn't supported yet.");
                        break;
                    case recomp::RomValidationError::IncorrectVersion:
                        recompui::message_box(
                                "This ROM is the correct game, but the wrong version.\nThis project requires the NTSC-U N64 version of the game.");
                        break;
                    case recomp::RomValidationError::OtherError:
                        recompui::message_box("An unknown error has occurred.");
                        break;
                }
            }
            callback(false);
        });
    }

    GameOption *GameOptionsMenu::add_option(const std::string& title, std::function<void()> callback) {
        auto context = get_current_context();
        auto option = context.create_element<GameOption>(this, title, callback, layout);
        options.push_back(option);
        return option;
    }

    GameOption *GameOptionsMenu::add_start_game_or_load_rom_option(const std::string& load_rom_title, const std::string& start_game_title) {
        GameOption *option = add_option(rom_valid ? start_game_title : load_rom_title, nullptr);
        option->set_as_primary_focus(true);

        auto context = get_current_context();
        context.set_autofocus_element(option);

        option->set_callback([this, option, start_game_title]() {
            if (this->rom_valid) {
                recomp::start_game(this->game_id);
                recompui::update_game_mod_id(this->mod_game_id);
                recompui::hide_all_contexts();
            } else {
                select_rom([this, option, start_game_title](bool success) {
                    if (success) {
                        this->rom_valid = true;
                        option->set_text(start_game_title);
                    }
                });
            }
        });

        return option;
    }

    GameOption *GameOptionsMenu::add_setup_controls_option(const std::string& title) {
        return add_option(title, [this]() {
            recompui::update_game_mod_id(this->mod_game_id);
            recompui::config::set_tab(recompui::config::controls::id);
            recompui::hide_all_contexts();
            recompui::config::open();
        });
    }

    GameOption *GameOptionsMenu::add_settings_option(const std::string& title) {
        return add_option(title, [this]() {
            recompui::update_game_mod_id(this->mod_game_id);
            recompui::config::set_tab(recompui::config::general::id);
            recompui::hide_all_contexts();
            recompui::config::open();
        });
    }

    GameOption *GameOptionsMenu::add_mods_option(const std::string& title) {
        return add_option(title, [this]() {
            recompui::update_game_mod_id(this->mod_game_id);
            recompui::config::set_tab(recompui::config::mods::id);
            recompui::hide_all_contexts();
            recompui::config::open();
        });
    }

    GameOption *GameOptionsMenu::add_exit_option(const std::string& title) {
        return add_option(title, []() {
            ultramodern::quit();
        });
    }

    LauncherMenu::LauncherMenu(Document* parent, ContextId context) : Element(parent, 0, "div", false) {
        set_position(Position::Relative);
        set_width(100.0f, Unit::Percent);
        set_height(100.0f, Unit::Percent);
        set_background_color(theme::color::Background1);

        background_wrapper = context.create_element<Element>(this, 0, "div", false);
        background_wrapper->set_position(Position::Absolute);
        background_wrapper->set_top(0);
        background_wrapper->set_left(0);
        background_wrapper->set_height(100.0f, Unit::Percent);
        background_wrapper->set_width(100.0f, Unit::Percent);

        {
            default_title_wrapper = context.create_element<Element>(this, 0, "div", false);
            default_title_wrapper->set_position(Position::Absolute);
            default_title_wrapper->set_top(25.0f, Unit::Percent);
            default_title_wrapper->set_left(50.0f, Unit::Percent);
            default_title_wrapper->set_translate_2D(-50.0f, -50.0f, Unit::Percent);
            default_title_wrapper->set_width_auto();
            default_title_wrapper->set_height_auto();
            Label *title = context.create_element<Label>(default_title_wrapper, programconfig::get_program_name(), theme::Typography::Header1);
            title->set_color(theme::color::Primary);
        }

        menu_container = context.create_element<Element>(this, 0, "div", false);
        menu_container->set_position(Position::Absolute);
        menu_container->set_top(24.0f);
        menu_container->set_right(24.0f);
        menu_container->set_bottom(24.0f);
        menu_container->set_left(24.0f);

        // TODO: Render version automatically
        // version_string = recomp::get_project_version().to_string();
    }

    void LauncherMenu::remove_default_title() {
        if (default_title_wrapper != nullptr) {
            remove_child(default_title_wrapper, false);
            default_title_wrapper = nullptr;
        }
    }

    GameOptionsMenu *LauncherMenu::init_game_options_menu(std::u8string game_id, std::string mod_game_id, GameOptionsMenuLayout layout) {
        if (game_options_menu != nullptr) {
            menu_container->remove_child(game_options_menu, false);
        }
        auto context = get_current_context();
        game_options_menu = context.create_element<GameOptionsMenu>(menu_container, game_id, mod_game_id, layout);
        return game_options_menu;
    }

    Svg *LauncherMenu::set_launcher_background_svg(const std::string& svg_path) {
        auto context = get_current_context();
        if (background_svg != nullptr) {
            background_wrapper->clear_children();
            background_svg = nullptr;
        }
        background_svg = context.create_element<Svg>(background_wrapper, svg_path);
        background_svg->set_position(Position::Absolute);
        background_svg->set_top(50.0f, Unit::Percent);
        background_svg->set_left(0);
        background_svg->set_height_auto();
        background_svg->set_width(100.0f, Unit::Percent);
        background_svg->set_translate_2D(0.0f, -50.0f, Unit::Percent);
        return background_svg;
    }

    static ContextId launcher_context;
    static LauncherMenu *launcher_menu = nullptr;
    static std::function<void(LauncherMenu *menu)> launcher_init_callback = nullptr;
    
    ContextId get_launcher_context_id() {
        return launcher_context;
    }
    
    LauncherMenu *get_launcher_menu() {
        return launcher_menu;
    }

    void register_launcher_init_callback(std::function<void(LauncherMenu *menu)> callback) {
        launcher_init_callback = callback;
    }

    void default_launcher_init_callback(LauncherMenu *menu) {
        auto game_options_menu = menu->init_game_options_menu(supported_games[0].game_id, supported_games[0].mod_game_id, GameOptionsMenuLayout::Center);
        recompui::update_game_mod_id(supported_games[0].mod_game_id);
        game_options_menu->add_default_options();
    }
    
    void init_launcher_menu() {
        launcher_context = create_context();
        launcher_context.open();
        launcher_menu = launcher_context.create_element<LauncherMenu>(launcher_context.get_root_element(), launcher_context);
        if (launcher_init_callback == nullptr) {
            default_launcher_init_callback(launcher_menu);
        } else {
            launcher_init_callback(launcher_menu);
        }
        launcher_context.close();
    }
}

