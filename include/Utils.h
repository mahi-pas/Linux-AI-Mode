#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace Utils {
    std::string get_current_timestamp();
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string join(const std::vector<std::string>& parts, const std::string& delimiter);
    std::string encode_url(const std::string& str);
    std::string decode_url(const std::string& str);
    bool file_exists(const std::string& path);
    bool create_directory(const std::string& path);
    std::string get_home_directory();
    std::string escape_html(const std::string& text);
    std::string markdown_to_html(const std::string& markdown);
}

#endif // UTILS_H
