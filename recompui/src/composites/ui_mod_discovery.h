#ifndef RECOMPUI_MOD_DISCOVERY_H
#define RECOMPUI_MOD_DISCOVERY_H

#include "elements/ui_button.h"
#include "elements/ui_container.h"
#include "elements/ui_element.h"
#include "elements/ui_image.h"
#include "elements/ui_label.h"
#include "elements/ui_scroll_container.h"
#include "elements/ui_text_input.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace recompui
{

    std::string get_discovery_url();
    void curl_global_initialize();
    std::string http_fetch_string(const std::string &url);
    std::vector<char> http_fetch_bytes(const std::string &url);
    void http_download_to_file(const std::string &url,
                               const std::string &output_path);
    std::vector<char> decode_base64(const std::string &encoded);
    std::pair<std::string, std::string>
    parse_dep_string(const std::string &dep_str);
    int compare_versions(const std::string &version1,
                         const std::string &version2);

    enum class ModInstallStatus
    {
        NotInstalled,
        Installed,
        UpdateAvailable,
        DowngradeAvailable,
        MissingDependencies
    };

    struct DiscoveryMod
    {
        std::string name;
        std::string description;
        std::string short_description;
        std::vector<std::string> tags;
        std::string authors;
        std::string file_url;
        std::string thumbnail_image;
        std::string thumbnail_url;
        std::string version;
        std::string id;
        std::string game_id;
        std::vector<std::string> dependencies;
    };

    class ModDiscoveryEntry : public Element
    {
    public:
        DiscoveryMod mod_data;
        ModDiscoveryEntry(ResourceId rid, Element *parent, const DiscoveryMod &mod_data);
        virtual ~ModDiscoveryEntry();
        void set_download_callback(std::function<void(const DiscoveryMod &)> callback);
        void update_install_status(ModInstallStatus status);
        Button *get_download_button() { return download_button; }

    protected:
        std::string_view get_type_name() override { return "ModDiscoveryEntry"; }

    private:
        void init_thumbnail_image();

        Container *entry_container = nullptr;
        Image *thumbnail_image = nullptr;
        Label *name_label = nullptr;
        Label *description_label = nullptr;
        Button *download_button = nullptr;
        std::string thumbnail_src;
        std::function<void(const DiscoveryMod &)> download_callback;
    };

    class ModDownloadsPanel : public Element
    {
    public:
        ModDownloadsPanel(ResourceId rid, Element *parent);
        virtual ~ModDownloadsPanel();
        void fetch_discovery_data();

    protected:
        std::string_view get_type_name() override { return "ModDownloadsPanel"; }

    private:
        void load_discovery_mods(const std::vector<DiscoveryMod> &mods);
        void refresh_discovery_mods();
        void download_mod(const DiscoveryMod &mod);
        std::string fetch_json_from_url(const std::string &url);
        std::vector<DiscoveryMod>
        parse_discovery_json(const std::string &json_data);
        void download_file_from_url(const std::string &url,
                                    const std::string &output_path);
        ModInstallStatus get_mod_install_status(const DiscoveryMod &mod);
        bool install_single_mod_file(const DiscoveryMod &mod,
                                     std::vector<std::string> &out_errors);
        void resolve_and_install_dependencies(
            const DiscoveryMod &mod, std::unordered_set<std::string> &visited_ids,
            std::vector<std::string> &out_warnings,
            std::vector<std::string> &out_errors,
            std::vector<std::string> &out_installed_deps);

        Container *content_panel = nullptr;
        Label *title_label = nullptr;
        Label *status_label = nullptr;
        TextInput *search_input = nullptr;
        Button *sort_name_button = nullptr;
        ScrollContainer *mod_list_container = nullptr;
        std::vector<ModDiscoveryEntry *> mod_entries;
        std::vector<DiscoveryMod> fetched_mods;
        std::string name_search_query;
        bool sort_name_ascending = true;
        std::string fetch_error;
        bool thumbnail_viewport_dirty = false;
        int thumbnail_viewport_retry_frames = 0;
        bool is_loading = false;
        bool fetch_completed = false;
    };
    class ElementModDownloads : public Rml::Element
    {
    public:
        ElementModDownloads(const Rml::String &tag);
        virtual ~ElementModDownloads();
    };

}

#endif