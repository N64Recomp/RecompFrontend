#pragma once

#include "recompui/recompui.h"
#include "elements/ui_element.h"
#include "elements/ui_config_page.h"
#include "elements/ui_tab_set.h"

namespace recompui {

    class Modal;

    enum class ModalType {
        Fullscreen,
        Prompt
    };

    using create_contents_t = std::function<void(recompui::ContextId context, Element* parent)>;
    struct TabCallbacks {
        create_contents_t create_contents;
        std::function<bool()> can_close = nullptr;
        std::function<void()> on_close = nullptr;
    };

    class TabContext {
    public:
        std::string name;
        const std::string id;
        TabContext(const std::string &name, const std::string &id, create_contents_t &&create_contents) : name(name), id(id), callbacks(std::move(create_contents)) {};
        TabContext(const std::string &name, const std::string &id, TabCallbacks &&callbacks) : name(name), id(id), callbacks(std::move(callbacks)) {};
        virtual ~TabContext() = default;

        void create_contents(recompui::ContextId context, Element* parent) {
            callbacks.create_contents(context, parent);
        }

        bool can_close() { return callbacks.can_close ? callbacks.can_close() : true; }
        void on_close() { if (callbacks.on_close) callbacks.on_close(); }
    protected:
        TabCallbacks callbacks;
    };

    class Modal : public Element {
    protected:
        bool is_open = false;
        Element *modal_element = nullptr;
        ConfigHeaderFooter *header = nullptr;
        Element *body = nullptr;
        ModalType modal_type;
        std::vector<TabContext> tab_contexts;
        std::function<void()> on_close_callback;
        std::unordered_map<MenuAction, std::function<void()>> menu_action_callbacks;
        TabSet *tabs = nullptr;
        int previous_tab_index = -1;
        int current_tab_index = -1;

        virtual void process_event(const Event &e) override;
        std::string_view get_type_name() override { return "Modal"; }
        void on_tab_change(int tab_index);
        void navigate_tab_direction(int direction);
        void initialize_tab(TabContext &tab_context);
    public:
        recompui::ContextId modal_root_context;
        Modal(Element *parent, recompui::ContextId modal_root_context, ModalType modal_type);
        static Modal *create_modal(ModalType modal_type = ModalType::Fullscreen);
        virtual ~Modal();
        void open();
        bool close();
        bool is_open_now() const { return is_open; }

        void add_tab(TabContext &&tab_context);
        void add_tab(
            const std::string &name,
            const std::string &id,
            create_contents_t create_contents,
            std::function<bool()> can_close = nullptr,
            std::function<void()> on_close = nullptr
        );
        void set_selected_tab(int tab_index);
        void set_selected_tab(const std::string &id);

        ConfigHeaderFooter *get_header() { return header; }
        Element *get_body() { return body; }
        Element *get_active_tab() { return tabs != nullptr ? static_cast<Tab*>(tabs->get_active_tab_element()) : nullptr; }
        void set_on_close_callback(std::function<void()> callback);
        void set_menu_action_callback(MenuAction action, std::function<void()> callback);
        void set_tab_visible(const std::string &id, bool is_visible);
    };
} // namespace recompui
