#include "librecomp/mods.hpp"
#include "recompui/recompui.h"
#include "ui_mod_installer.h"
#include "ui_mod_discovery.h"
#include <algorithm>
#include <filesystem>

namespace recompui
{

    // Download and install a mod from the discovery
    bool ModDownloadsPanel::install_single_mod_file(
        const DiscoveryMod &mod, std::vector<std::string> &out_errors)
    {
        // If theres no download URL just throw out a message, because something
        // upstream went wrong
        if (mod.file_url.empty())
        {
            out_errors.push_back("No download URL available for " + mod.name);
            return false;
        }

        try
        {
            std::filesystem::path temp_dir = std::filesystem::temp_directory_path();

            const std::string &url = mod.file_url;
            size_t last_slash = url.find_last_of('/');
            std::string filename = (last_slash != std::string::npos)
                                       ? url.substr(last_slash + 1)
                                       : "download.zip";
            std::filesystem::path temp_file = temp_dir / filename;

            http_download_to_file(url, temp_file.string());

            ModInstaller::Result install_result;
            std::list<std::filesystem::path> file_paths = {temp_file};

            // We're just using the normal built in mod installer, this dosen't support
            // multiple zips split out currently, I'd love to add this, but dealing with
            // each zip type eg winrar didn't really scale.
            ModInstaller installer;
            installer.start_mod_installation(
                file_paths, [](std::filesystem::path, size_t, size_t) {},
                install_result);

            bool needs_close = std::any_of(install_result.pending_installations.begin(),
                                           install_result.pending_installations.end(),
                                           [](const ModInstaller::Installation &i)
                                           {
                                               return i.needs_overwrite_confirmation;
                                           });
            if (needs_close)
                recomp::mods::close_mods();

            std::vector<std::string> finish_errors;
            installer.finish_mod_installation(install_result, finish_errors);
            install_result.error_messages.insert(install_result.error_messages.end(),
                                                 finish_errors.begin(),
                                                 finish_errors.end());

            std::filesystem::remove(temp_file);

            if (!install_result.error_messages.empty())
            {
                out_errors.insert(out_errors.end(), install_result.error_messages.begin(),
                                  install_result.error_messages.end());
                return false;
            }

            return true;
        }
        catch (const std::exception &e)
        {
            out_errors.push_back("Download/install error for " + mod.name + ": " +
                                 e.what());
            return false;
        }
    }

    // Resolves and installs dependencies recursively before target install.
    void ModDownloadsPanel::resolve_and_install_dependencies(
        const DiscoveryMod &mod, std::unordered_set<std::string> &visited_ids,
        std::vector<std::string> &out_warnings,
        std::vector<std::string> &out_errors,
        std::vector<std::string> &out_installed_deps)
    {
        // For each dep that the mod has we need to actually look up if the sub mod is avaiable.
        for (const std::string &dep_str : mod.dependencies)
        {
            auto dep = parse_dep_string(dep_str);
            const std::string &dep_id = dep.first;
            const std::string &required_version = dep.second;

            if (dep_id.empty() || visited_ids.count(dep_id))
                continue;

            // Check if we already have it installed, and that it satisfies the required version, if we do then we can skip trying to install it.
            auto installed_dep_details = recomp::mods::get_details_for_mod(dep_id);
            if (installed_dep_details.has_value() && !required_version.empty())
            {
                std::string installed_ver = installed_dep_details->version.to_string();
                if (!installed_ver.empty() && compare_versions(installed_ver, required_version) >= 0)
                {
                    visited_ids.insert(dep_id);
                    continue;
                }
            }
            // If we don't have the mod or its too old on the discovery just warn the user we can't install it.
            auto dep_it = std::find_if(fetched_mods.begin(), fetched_mods.end(),
                [&](const DiscoveryMod &m) { return m.id == dep_id; });
            const DiscoveryMod *dep_mod = (dep_it != fetched_mods.end()) ? &*dep_it : nullptr;
            if (!dep_mod)
            {
                out_warnings.push_back(
                    "Dependency '" + dep_id + "' required by '" + mod.name +
                    "' was not found in the discovery and must be installed manually.");
                continue;
            }

            // If its on the discovery but its the wrong version, just throw out some warnings based on if its new or not.
            if (!required_version.empty() && !dep_mod->version.empty())
            {
                int cmp = compare_versions(dep_mod->version, required_version);
                if (cmp < 0)
                {
                    out_warnings.push_back(
                        "Dependency '" + dep_mod->name + "' requires v" + required_version +
                        " but the discovery only has v" + dep_mod->version +
                        ". Skipping auto-install; please install it manually.");
                    continue;
                }
                else if (cmp > 0)
                {
                    out_warnings.push_back(
                        "Dependency '" + dep_mod->name + "' requires v" + required_version +
                        " but the discovery has a newer version v" + dep_mod->version +
                        ". Installing the newer version.");
                }
            }

            visited_ids.insert(dep_id);
            // Make sure we recursively resolve dependencies for the current mods dependency before we try to install it, just in case.
            resolve_and_install_dependencies(*dep_mod, visited_ids, out_warnings,
                                             out_errors, out_installed_deps);

            status_label->set_text("Installing dependency: " + dep_mod->name + "...");

            std::vector<std::string> dep_errors;
            // Actually install the dependency
            if (!install_single_mod_file(*dep_mod, dep_errors))
            {
                for (const auto &err : dep_errors)
                    out_errors.push_back("Dependency '" + dep_mod->name + "': " + err);
            }
            else
            {
                out_installed_deps.push_back(dep_mod->name);

                for (ModDiscoveryEntry *entry : mod_entries)
                {
                    if (entry->mod_data.id == dep_mod->id)
                    {
                        entry->update_install_status(ModInstallStatus::Installed);
                        entry->queue_update();
                        break;
                    }
                }
            }
        }
    }

    // On mod download button click
    void ModDownloadsPanel::download_mod(const DiscoveryMod &mod)
    {
        if (mod.file_url.empty())
        {
            status_label->set_text("No download URL available for " + mod.name);
            return;
        }

        status_label->set_text("Downloading " + mod.name + "...");

        ModDiscoveryEntry *downloading_entry = nullptr;
        for (ModDiscoveryEntry *entry : mod_entries)
        {
            if (entry->mod_data.id == mod.id && entry->mod_data.name == mod.name)
            {
                downloading_entry = entry;
                break;
            }
        }

        try
        {
            std::vector<std::string> warnings;
            std::vector<std::string> dep_errors;
            std::vector<std::string> installed_deps;
            status_label->set_text("Resolving dependencies for " + mod.name + "...");
            std::unordered_set<std::string> visited_ids = {mod.id};
            resolve_and_install_dependencies(mod, visited_ids, warnings, dep_errors,
                                             installed_deps);

            if (!dep_errors.empty())
            {
                std::string error_msg = "Dependency installation failed:\n";
                for (const auto &err : dep_errors)
                    error_msg += "  " + err + "\n";
                status_label->set_text(error_msg);
                return;
            }

            if (!warnings.empty())
            {
                std::string warn_msg = "Warning(s):\n";
                for (const auto &w : warnings)
                    warn_msg += "  " + w + "\n";
                status_label->set_text(warn_msg);
            }

            status_label->set_text("Installing " + mod.name + "...");

            std::vector<std::string> install_errors;
            if (!install_single_mod_file(mod, install_errors))
            {
                std::string error_msg = "Installation failed: ";
                for (const auto &err : install_errors)
                    error_msg += err + " ";
                status_label->set_text(error_msg);
                return;
            }

            recompui::update_mod_list(true);
            recomp::mods::scan_mods();

            if (downloading_entry)
            {
                downloading_entry->update_install_status(ModInstallStatus::Installed);
                downloading_entry->queue_update();
            }

            for (ModDiscoveryEntry *entry : mod_entries)
            {
                if (entry == downloading_entry)
                    continue;
                ModInstallStatus entry_status = get_mod_install_status(entry->mod_data);
                if (entry_status != ModInstallStatus::NotInstalled)
                {
                    entry->update_install_status(entry_status);
                    entry->queue_update();
                }
            }

            std::string final_msg = "Successfully installed " + mod.name + "!";
            if (!installed_deps.empty())
            {
                final_msg += " Also installed dependencies: ";
                for (size_t i = 0; i < installed_deps.size(); i++)
                {
                    if (i > 0)
                        final_msg += ", ";
                    final_msg += installed_deps[i];
                }
                final_msg += ".";
            }
            if (!warnings.empty())
                final_msg += " (with warnings — check dependency versions)";

            status_label->set_text(final_msg);
            queue_update();
        }
        catch (const std::exception &e)
        {
            status_label->set_text("Download failed: " + std::string(e.what()));
        }
    }

    // Wrapper that checks all our installed mod status'
    ModInstallStatus
    ModDownloadsPanel::get_mod_install_status(const DiscoveryMod &mod)
    {
        auto details = recomp::mods::get_details_for_mod(mod.id);
        if (!details.has_value())
        {
            // Check whether all dependencies are satisfiable before declaring NotInstalled
            bool satisfiable = true;
            for (const std::string &dep_str : mod.dependencies)
            {
                auto dep = parse_dep_string(dep_str);
                const std::string &dep_id = dep.first;
                const std::string &required_version = dep.second;
                if (dep_id.empty())
                    continue;

                auto installed = recomp::mods::get_details_for_mod(dep_id);
                if (installed.has_value())
                {
                    if (required_version.empty())
                        continue;
                    std::string inst_ver = installed->version.to_string();
                    if (!inst_ver.empty() && compare_versions(inst_ver, required_version) >= 0)
                        continue;
                }

                auto dep_it = std::find_if(fetched_mods.begin(), fetched_mods.end(),
                    [&](const DiscoveryMod &m) { return m.id == dep_id; });
                const DiscoveryMod *dep_mod = (dep_it != fetched_mods.end()) ? &*dep_it : nullptr;
                if (!dep_mod || (!required_version.empty() && !dep_mod->version.empty() &&
                    compare_versions(dep_mod->version, required_version) < 0))
                {
                    satisfiable = false;
                    break;
                }
            }
            return satisfiable ? ModInstallStatus::NotInstalled : ModInstallStatus::MissingDependencies;
        }

        std::string installed_version = details->version.to_string();
        if (installed_version.empty() || mod.version.empty())
            return ModInstallStatus::Installed;

        int cmp = compare_versions(mod.version, installed_version);
        if (cmp > 0)
            return ModInstallStatus::UpdateAvailable;
        if (cmp < 0)
            return ModInstallStatus::DowngradeAvailable;
        return ModInstallStatus::Installed;
    }

}