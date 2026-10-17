#include "../../../lib/N64ModernRuntime/N64Recomp/lib/tomlplusplus/vendor/json.hpp"
#include "./ui_mod_discovery.h"
#include "librecomp/game.hpp"
#include "recompui/recompui.h"
#include "ui_utils.h"

#include <algorithm>
#include <cctype>
#include <thread>

extern std::vector<recomp::GameEntry> supported_games;

namespace recompui
{
    // Gets the URL for the discovery so we can determine if we have to render the
    // button for the UI
    std::string get_discovery_url()
    {
        return supported_games.empty() ? "" : supported_games[0].discovery_url;
    }

    // Each Mod Entry on the UI
    ModDiscoveryEntry::ModDiscoveryEntry(ResourceId rid, Element *parent,
                                             const DiscoveryMod &mod_data)
                : Element(rid, parent, Events(EventType::Click)),
                    mod_data(mod_data)
    {
        ContextId context = get_current_context();

        entry_container = context.create_element<Container>(
            this, FlexDirection::Row, JustifyContent::FlexStart);
        entry_container->set_width(100.0f, Unit::Percent);
        entry_container->set_padding(16.0f);
        entry_container->set_margin_bottom(12.0f);
        entry_container->set_background_color(Color{26, 24, 32, 255});
        entry_container->set_border_radius(8.0f);
        entry_container->set_border_width(1.0f);
        entry_container->set_border_color(Color{242, 242, 242, 64});

        thumbnail_image = context.create_element<Image>(entry_container, "");
        thumbnail_image->set_width(80.0f, Unit::Px);
        thumbnail_image->set_height(80.0f, Unit::Px);
        thumbnail_image->set_min_width(80.0f);
        thumbnail_image->set_min_height(80.0f);
        thumbnail_image->set_max_width(80.0f);
        thumbnail_image->set_max_height(80.0f);
        thumbnail_image->set_background_color(Color{190, 184, 219, 100});
        thumbnail_image->set_border_radius(4.0f);
        thumbnail_image->set_margin_right(16.0f);

        init_thumbnail_image();

        Container *content_container = context.create_element<Container>(
            entry_container, FlexDirection::Column, JustifyContent::FlexStart);
        content_container->set_flex(1.0f, 1.0f);
        content_container->set_gap(8.0f);

        name_label = context.create_element<Label>(content_container, mod_data.name,
                                                   LabelStyle::Normal);
        name_label->set_color(Color{242, 242, 242, 255});
        name_label->set_font_weight(600);

        description_label = context.create_element<Label>(
            content_container, mod_data.short_description, LabelStyle::Small);
        description_label->set_color(Color{204, 204, 204, 255});

        // If we have dependencies list all of them
        if (!mod_data.dependencies.empty())
        {
            std::string dep_text = "Requires: ";
            for (size_t i = 0; i < mod_data.dependencies.size(); i++)
            {
                auto [dep_id, dep_ver] = parse_dep_string(mod_data.dependencies[i]);
                if (i > 0)
                    dep_text += ", ";
                dep_text += dep_id;
                if (!dep_ver.empty())
                    dep_text += " v" + dep_ver;
            }
            Label *deps_label = context.create_element<Label>(
                content_container, dep_text, LabelStyle::Small);
            deps_label->set_color(Color{180, 150, 100, 255});
        }

        Container *button_container = context.create_element<Container>(
            entry_container, FlexDirection::Column, JustifyContent::Center);
        download_button = context.create_element<Button>(button_container, "Download",
                                                         ButtonStyle::Primary);
        download_button->add_pressed_callback([this]()
                                              {
    if (download_callback)
      download_callback(this->mod_data); });
    }

    ModDiscoveryEntry::~ModDiscoveryEntry() {}

    // Sets the callback for the click event
    void ModDiscoveryEntry::set_download_callback(
        std::function<void(const DiscoveryMod &)> callback)
    {
        download_callback = callback;
    }

    // Sets the status for the downlaod button based on the mod state.
    void ModDiscoveryEntry::update_install_status(ModInstallStatus status)
    {
        switch (status)
        {
        case ModInstallStatus::NotInstalled:
            download_button->set_text("Download");
            download_button->set_enabled(true);
            break;

        case ModInstallStatus::Installed:
            download_button->set_text("Installed");
            download_button->set_enabled(false);
            download_button->set_background_color(Color{100, 100, 100, 255});
            break;

        case ModInstallStatus::UpdateAvailable:
            download_button->set_text("Update Available");
            download_button->set_enabled(true);
            download_button->set_background_color(Color{50, 150, 50, 255});
            break;

        case ModInstallStatus::DowngradeAvailable:
            download_button->set_text("Downgrade Available");
            download_button->set_enabled(true);
            download_button->set_background_color(Color{150, 150, 50, 255});
            break;

        case ModInstallStatus::MissingDependencies:
            download_button->set_text("Missing Dependencies");
            download_button->set_enabled(false);
            download_button->set_background_color(Color{120, 60, 60, 255});
            break;
        }

        download_button->queue_update();
        queue_update();
    }

    // Initializes the discovery modal panel and child controls.
    ModDownloadsPanel::ModDownloadsPanel(ResourceId rid, Element *parent)
        : Element(rid, parent)
    {
        ContextId context = get_current_context();

        set_display(Display::Flex);
        set_flex_direction(FlexDirection::Column);
        set_width(100.0f, Unit::Percent);
        set_height(100.0f, Unit::Percent);

        content_panel = context.create_element<Container>(
            this, FlexDirection::Column, JustifyContent::FlexStart);
        content_panel->set_width(100.0f, Unit::Percent);
        content_panel->set_height(100.0f, Unit::Percent);
        content_panel->set_padding(30.0f);
        content_panel->set_gap(20.0f);

        Container *header_container = context.create_element<Container>(
            content_panel, FlexDirection::Row, JustifyContent::SpaceBetween);
        header_container->set_align_items(AlignItems::Center);
        header_container->set_width(100.0f, Unit::Percent);
        header_container->set_margin_bottom(12.0f);

        // Filter row: search label + input on the left, sort button on the right
        Container *filter_container = context.create_element<Container>(
            content_panel, FlexDirection::Row, JustifyContent::SpaceBetween);
        filter_container->set_align_items(AlignItems::Center);
        filter_container->set_width(100.0f, Unit::Percent);

        // Left side: "Search:" label + text input — fixed width, does not grow
        Container *search_group = context.create_element<Container>(
            filter_container, FlexDirection::Row, JustifyContent::FlexStart);
        search_group->set_align_items(AlignItems::Center);
        search_group->set_gap(10.0f);
        search_group->set_flex(0.0f, 0.0f);
        search_group->set_width(280.0f, Unit::Px);

        Label *search_label = context.create_element<Label>(
            search_group, "Search:", LabelStyle::Small);
        search_label->set_color(Color{180, 180, 180, 255});

        search_input = context.create_element<TextInput>(search_group);
        search_input->set_flex(1.0f, 1.0f);
        search_input->set_min_width(120.0f);
        search_input->set_background_color(Color{40, 38, 50, 255});
        search_input->add_text_changed_callback(
            [this](const std::string &text)
            {
                name_search_query = text;
                refresh_discovery_mods();
            });

        // Right side: sort toggle button
        sort_name_button =
            context.create_element<Button>(filter_container, "Name (A\u2192Z)",
                                           ButtonStyle::Secondary);
        sort_name_button->add_pressed_callback([this]()
                                               {
                                                   sort_name_ascending =
                                                       !sort_name_ascending;
                                                   sort_name_button->set_text(
                                                       sort_name_ascending
                                                           ? "Name (A\u2192Z)"
                                                           : "Name (Z\u2192A)");
                                                   refresh_discovery_mods();
                                               });

        status_label = context.create_element<Label>(
            content_panel, "Loading discovery data...", LabelStyle::Normal);
        status_label->set_color(Color{204, 204, 204, 255});
        status_label->set_text_align(TextAlign::Center);

        mod_list_container = context.create_element<ScrollContainer>(
            content_panel, ScrollDirection::Vertical);
        mod_list_container->set_flex(1.0f, 1.0f);
        mod_list_container->set_width(100.0f, Unit::Percent);

        sort_name_button->set_nav(NavDirection::Up, search_input);
        search_input->set_nav(NavDirection::Down, sort_name_button);

        fetch_discovery_data();
    }

    ModDownloadsPanel::~ModDownloadsPanel() {}

    // Fetches discovery JSON and refreshes rendered entries.
    void ModDownloadsPanel::fetch_discovery_data()
    {
        if (is_loading)
            return;

        is_loading = true;
        status_label->set_text("Loading discovery data...");
        mod_list_container->clear_children();
        mod_entries.clear();

        try
        {
            std::string discovery_url = get_discovery_url();
            if (discovery_url.empty())
                throw std::runtime_error("No discovery URL configured for this game");
            std::string json_data = http_fetch_string(discovery_url);
            std::vector<DiscoveryMod> mods = parse_discovery_json(json_data);
            fetched_mods = mods;
            load_discovery_mods(fetched_mods);
        }
        catch (const std::exception &e)
        {
            status_label->set_text("Failed to load discovery data: " +
                                   std::string(e.what()));
        }

        is_loading = false;
    }

    void ModDownloadsPanel::refresh_discovery_mods()
    {
        if (is_loading)
            return;

        load_discovery_mods(fetched_mods);
    }

    // Parses discovery JSON payload into mod metadata entries.
    std::vector<DiscoveryMod>
    ModDownloadsPanel::parse_discovery_json(const std::string &json_data)
    {
        if (json_data.empty())
            throw std::runtime_error("Empty JSON data received");

        std::vector<DiscoveryMod> mods;

        try
        {
            nlohmann::json j = nlohmann::json::parse(json_data);

            if (j.empty())
                throw std::runtime_error("JSON data is empty object");

            for (auto it = j.begin(); it != j.end(); ++it)
            {
                DiscoveryMod mod;
                const std::string mod_name = it.key();
                const auto &mod_info = it.value();
                mod.name = mod_name;

                auto try_get = [&](const char *key, std::string &field)
                {
                    if (mod_info.contains(key))
                        field = mod_info[key];
                };

                try_get("description", mod.description);
                try_get("short_description", mod.short_description);
                try_get("authors", mod.authors);
                try_get("file_url", mod.file_url);
                try_get("thumbnail_image", mod.thumbnail_image);
                try_get("thumbnail_url", mod.thumbnail_url);
                try_get("version", mod.version);
                try_get("id", mod.id);
                try_get("game_id", mod.game_id);

                if (mod_info.contains("dependencies") &&
                    mod_info["dependencies"].is_array())
                {
                    for (const auto &dep : mod_info["dependencies"])
                        mod.dependencies.push_back(dep);
                }

                if (mod_info.contains("tags") && mod_info["tags"].is_array())
                {
                    for (const auto &tag : mod_info["tags"])
                        mod.tags.push_back(tag);
                }

                mods.push_back(mod);
            }
        }
        catch (const nlohmann::json::exception &e)
        {
            throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
        }

        if (mods.empty())
            throw std::runtime_error("No valid mods found in JSON");

        return mods;
    }

    // Loads filtered discovery mods into the ui
    void ModDownloadsPanel::load_discovery_mods(
        const std::vector<DiscoveryMod> &mods)
    {
        ContextId context = get_current_context();

        mod_list_container->clear_children();
        mod_entries.clear();

        if (mods.empty())
        {
            status_label->set_text("No mods available in discovery");
            return;
        }

        std::string current_game_id =
            supported_games.empty() ? "" : supported_games[0].mod_game_id;

        auto to_lower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return s;
        };

        std::vector<DiscoveryMod> filtered_mods;
        for (const auto &mod : mods)
        {
            if ((mod.game_id.empty() || mod.game_id == current_game_id) &&
                (name_search_query.empty() ||
                 to_lower(mod.name).find(to_lower(name_search_query)) != std::string::npos))
                filtered_mods.push_back(mod);
        }

        std::sort(filtered_mods.begin(), filtered_mods.end(),
                  [this, &to_lower](const DiscoveryMod &a, const DiscoveryMod &b)
                  {
                      const std::string a_lower = to_lower(a.name);
                      const std::string b_lower = to_lower(b.name);
                      if (a_lower == b_lower)
                          return sort_name_ascending ? (a.name < b.name)
                                                     : (a.name > b.name);

                      return sort_name_ascending ? (a_lower < b_lower)
                                                 : (a_lower > b_lower);
                  });

        if (filtered_mods.empty())
        {
            if (name_search_query.empty())
            {
                status_label->set_text(
                    "No mods available for this game in discovery");
            }
            else
            {
                status_label->set_text("No mods match \"" + name_search_query +
                                       "\"");
            }
            return;
        }

        status_label->set_text("Found " + std::to_string(filtered_mods.size()) +
                               " mod(s) in discovery");

        for (const auto &mod : filtered_mods)
        {
            try
            {
                ModDiscoveryEntry *entry =
                    context.create_element<ModDiscoveryEntry>(mod_list_container, mod);
                entry->set_download_callback(
                    [this](const DiscoveryMod &m)
                    { download_mod(m); });
                entry->update_install_status(get_mod_install_status(mod));
                entry->get_download_button()->set_nav_none(NavDirection::Left);
                entry->get_download_button()->set_nav_none(NavDirection::Right);
                mod_entries.push_back(entry);
            }
            catch (const std::exception &e)
            {
                status_label->set_text("Error creating mod entry: " +
                                       std::string(e.what()));
                break;
            }
        }
    }

    // Creates an Rml host element for the discovery modal container
    ElementModDownloads::ElementModDownloads(const Rml::String &tag)
        : Rml::Element(tag)
    {
        SetProperty("width", "100%");
        SetProperty("height", "100%");
        SetProperty("position", "absolute");
        SetProperty("top", "0px");
        SetProperty("left", "0px");
        SetProperty("z-index", "1000");
    }

    ElementModDownloads::~ElementModDownloads() {}

    void ModDiscoveryEntry::init_thumbnail_image()
    {
        // if we actually have a thumbnail, check if its base64 and if we need to
        // decode it or not.
        if (!mod_data.thumbnail_image.empty())
        {
            static constexpr const char *data_image_prefix = "data:image/";
            if (mod_data.thumbnail_image.compare(
                    0, std::char_traits<char>::length(data_image_prefix),
                    data_image_prefix) == 0)
            {
                thumbnail_src = "discovery_thumb_" + std::to_string(std::hash<std::string>{}(
                    !mod_data.id.empty() ? mod_data.id : (!mod_data.thumbnail_url.empty() ? mod_data.thumbnail_url : mod_data.name)));
                std::vector<char> image_data = decode_base64(mod_data.thumbnail_image);
                if (!image_data.empty())
                    recompui::queue_image_from_bytes_file(thumbnail_src, image_data);
                thumbnail_image->set_src(thumbnail_src);
            }
            else
            {
                thumbnail_image->set_src(mod_data.thumbnail_image);
            }
        }
        else if (!mod_data.thumbnail_url.empty())
        {
            // Set the src immediately — the renderer will show a transparent
            // placeholder until the real texture data arrives.
            thumbnail_src = "discovery_thumb_" + std::to_string(std::hash<std::string>{}(
                !mod_data.id.empty() ? mod_data.id : (!mod_data.thumbnail_url.empty() ? mod_data.thumbnail_url : mod_data.name)));
            thumbnail_image->set_src(thumbnail_src);

            std::string url = mod_data.thumbnail_url;
            std::string src = thumbnail_src;
            std::thread([url, src]() {
                try
                {
                    auto data = recompui::http_fetch_bytes(url);
                    if (!data.empty())
                        recompui::queue_image_from_bytes_file(src, data);
                }
                catch (...) {}
            }).detach();
        }
    }

}
