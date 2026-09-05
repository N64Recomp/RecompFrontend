#include "ui_mod_discovery.h"

#include <algorithm>
#include <chrono>
#include <curl/curl.h>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace recompui
{
    // Simple Curl Wrapper for getting mod info
    struct CurlSession
    {
        CURL *handle = nullptr;

        CurlSession()
        {
            handle = curl_easy_init();
            if (!handle)
                throw std::runtime_error("Failed to initialize libcurl easy handle");

            curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(handle, CURLOPT_USERAGENT, "ModDownloader/1.0");
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 2L);
        }

        ~CurlSession()
        {
            if (handle)
                curl_easy_cleanup(handle);
        }

        CurlSession(const CurlSession &) = delete;
        CurlSession &operator=(const CurlSession &) = delete;

        CURLcode perform() { return curl_easy_perform(handle); }

        long response_code() const
        {
            long code = 0;
            curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
            return code;
        }
    };

    static size_t write_string_callback(void *contents, size_t size, size_t nmemb,
                                        std::string *out)
    {
        out->append(static_cast<char *>(contents), size * nmemb);
        return size * nmemb;
    }

    static size_t write_file_callback(void *contents, size_t size, size_t nmemb,
                                      FILE *file)
    {
        return fwrite(contents, size, nmemb, file);
    }

    static size_t write_bytes_callback(void *contents, size_t size, size_t nmemb,
                                       std::vector<char> *out)
    {
        const size_t byte_count = size * nmemb;
        const char *bytes = static_cast<char *>(contents);
        out->insert(out->end(), bytes, bytes + byte_count);
        return byte_count;
    }

    static std::string append_cache_busting_query(const std::string &url)
    {
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();
        return url + (url.find('?') == std::string::npos ? "?" : "&") +
               "t=" + std::to_string(now);
    }

    static curl_slist *make_no_cache_headers()
    {
        curl_slist *headers = nullptr;
        headers = curl_slist_append(
            headers, "Cache-Control: no-cache, no-store, must-revalidate");
        headers = curl_slist_append(headers, "Pragma: no-cache");
        headers = curl_slist_append(headers, "Expires: 0");
        return headers;
    }

    void curl_global_initialize()
    {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
            throw std::runtime_error("Failed to initialize libcurl globally");
    }

    std::string http_fetch_string(const std::string &url)
    {
        CurlSession session;
        std::string response;

        std::string busted_url = append_cache_busting_query(url);

        curl_easy_setopt(session.handle, CURLOPT_URL, busted_url.c_str());
        curl_easy_setopt(session.handle, CURLOPT_WRITEFUNCTION,
                         write_string_callback);
        curl_easy_setopt(session.handle, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(session.handle, CURLOPT_TIMEOUT, 30L);

        struct curl_slist *headers = make_no_cache_headers();
        curl_easy_setopt(session.handle, CURLOPT_HTTPHEADER, headers);

        CURLcode res = session.perform();
        curl_slist_free_all(headers);

        if (res != CURLE_OK)
            throw std::runtime_error(std::string("libcurl error: ") +
                                     curl_easy_strerror(res));

        long http_code = session.response_code();
        if (http_code != 200)
            throw std::runtime_error("HTTP error: " + std::to_string(http_code));

        if (response.empty())
            throw std::runtime_error("No data received from server");

        return response;
    }

    std::vector<char> http_fetch_bytes(const std::string &url)
    {
        CurlSession session;
        std::vector<char> response;

        std::string busted_url = append_cache_busting_query(url);

        curl_easy_setopt(session.handle, CURLOPT_URL, busted_url.c_str());
        curl_easy_setopt(session.handle, CURLOPT_WRITEFUNCTION,
                         write_bytes_callback);
        curl_easy_setopt(session.handle, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(session.handle, CURLOPT_TIMEOUT, 30L);

        struct curl_slist *headers = make_no_cache_headers();
        curl_easy_setopt(session.handle, CURLOPT_HTTPHEADER, headers);

        CURLcode res = session.perform();
        curl_slist_free_all(headers);

        if (res != CURLE_OK)
            throw std::runtime_error(std::string("libcurl error: ") +
                                     curl_easy_strerror(res));

        long http_code = session.response_code();
        if (http_code != 200)
            throw std::runtime_error("HTTP error: " + std::to_string(http_code));

        if (response.empty())
            throw std::runtime_error("No data received from server");

        return response;
    }

    void http_download_to_file(const std::string &url,
                               const std::string &output_path)
    {
        FILE *file = fopen(output_path.c_str(), "wb");
        if (!file)
        {
            throw std::runtime_error("Failed to open output file: " + output_path);
        }

        try
        {
            CurlSession session;

            curl_easy_setopt(session.handle, CURLOPT_URL, url.c_str());
            curl_easy_setopt(session.handle, CURLOPT_WRITEFUNCTION,
                             write_file_callback);
            curl_easy_setopt(session.handle, CURLOPT_WRITEDATA, file);
            curl_easy_setopt(session.handle, CURLOPT_TIMEOUT, 120L);

            CURLcode res = session.perform();

            if (res != CURLE_OK)
            {
                fclose(file);
                std::filesystem::remove(output_path);
                throw std::runtime_error(std::string("libcurl download error: ") +
                                         curl_easy_strerror(res));
            }

            long http_code = session.response_code();
            if (http_code != 200)
            {
                fclose(file);
                std::filesystem::remove(output_path);
                throw std::runtime_error("HTTP download error: " +
                                         std::to_string(http_code));
            }
        }
        catch (...)
        {
            fclose(file);
            std::filesystem::remove(output_path);
            throw;
        }

        fclose(file);
    }

}
