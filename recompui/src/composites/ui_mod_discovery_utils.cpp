#include "ui_mod_discovery.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>

namespace recompui
{

    // Simple rewrite of base64 decoding so we don't load a whole library for just
    // this
    std::vector<char> decode_base64(const std::string &encoded)
    {
        static const std::string chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<char> result;

        // Strip data URL prefix if present (e.g. "data:image/png;base64,")
        std::string data = encoded;
        size_t comma_pos = data.find(',');
        if (comma_pos != std::string::npos)
            data = data.substr(comma_pos + 1);

        data.erase(std::remove_if(data.begin(), data.end(), ::isspace), data.end());

        if (data.length() % 4 != 0)
            return result;

        for (size_t i = 0; i < data.length(); i += 4)
        {
            uint32_t tmp = 0;
            for (int j = 0; j < 4; j++)
            {
                tmp <<= 6;
                if (data[i + j] == '=')
                    break;
                auto pos = chars.find(data[i + j]);
                if (pos == std::string::npos)
                    return {};
                tmp |= pos;
            }

            result.push_back((tmp >> 16) & 0xFF);
            if (data[i + 2] != '=')
                result.push_back((tmp >> 8) & 0xFF);
            if (data[i + 3] != '=')
                result.push_back(tmp & 0xFF);
        }

        return result;
    }

    // Splits dependency text into id and optional required version.
    std::pair<std::string, std::string>
    parse_dep_string(const std::string &dep_str)
    {
        size_t colon_pos = dep_str.find(':');
        if (colon_pos != std::string::npos)
        {
            return {dep_str.substr(0, colon_pos), dep_str.substr(colon_pos + 1)};
        }
        return {dep_str, {}};
    }

    // Compares version strings, so we can determine upgrades/downgrades Returns -1 if v1 < v2, 0 if v1 == v2, 1 if v1 > v2
    int compare_versions(const std::string &v1, const std::string &v2)
    {
        if (v1 == v2)
            return 0;

        auto split = [](const std::string &ver)
        {
            std::vector<int> parts;
            std::istringstream ss(ver);
            std::string token;
            while (std::getline(ss, token, '.'))
            {
                try
                {
                    parts.push_back(std::stoi(token));
                }
                catch (...)
                {
                    parts.push_back(0);
                }
            }
            return parts;
        };

        std::vector<int> p1 = split(v1);
        std::vector<int> p2 = split(v2);

        size_t len = (p1.size() > p2.size()) ? p1.size() : p2.size();
        p1.resize(len, 0);
        p2.resize(len, 0);

        for (size_t i = 0; i < len; i++)
        {
            if (p1[i] > p2[i])
                return 1;
            if (p1[i] < p2[i])
                return -1;
        }

        return 0;
    }

}
