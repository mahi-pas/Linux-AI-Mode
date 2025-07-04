#include "Utils.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <regex>

namespace Utils {
    
    std::string get_current_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r\f\v");
        if (start == std::string::npos) {
            return "";
        }
        
        size_t end = str.find_last_not_of(" \t\n\r\f\v");
        return str.substr(start, end - start + 1);
    }
    
    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string item;
        
        while (std::getline(ss, item, delimiter)) {
            result.push_back(item);
        }
        
        return result;
    }
    
    std::string join(const std::vector<std::string>& parts, const std::string& delimiter) {
        if (parts.empty()) {
            return "";
        }
        
        std::stringstream ss;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) {
                ss << delimiter;
            }
            ss << parts[i];
        }
        
        return ss.str();
    }
    
    std::string encode_url(const std::string& str) {
        std::ostringstream encoded;
        encoded.fill('0');
        encoded << std::hex;
        
        for (char c : str) {
            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                encoded << c;
            } else {
                encoded << std::uppercase;
                encoded << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
                encoded << std::nouppercase;
            }
        }
        
        return encoded.str();
    }
    
    std::string decode_url(const std::string& str) {
        std::string decoded;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '%' && i + 2 < str.length()) {
                std::string hex = str.substr(i + 1, 2);
                char c = static_cast<char>(std::stoi(hex, nullptr, 16));
                decoded += c;
                i += 2;
            } else if (str[i] == '+') {
                decoded += ' ';
            } else {
                decoded += str[i];
            }
        }
        return decoded;
    }
    
    bool file_exists(const std::string& path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }
    
    bool create_directory(const std::string& path) {
        // Create directory recursively
        std::string current_path;
        std::vector<std::string> parts = split(path, '/');
        
        for (const auto& part : parts) {
            if (part.empty()) {
                current_path += "/";
                continue;
            }
            
            if (!current_path.empty() && current_path.back() != '/') {
                current_path += "/";
            }
            current_path += part;
            
            if (!file_exists(current_path)) {
                if (mkdir(current_path.c_str(), 0755) != 0) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    std::string get_home_directory() {
        const char* home = getenv("HOME");
        if (home) {
            return std::string(home);
        }
        
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            return std::string(pw->pw_dir);
        }
        
        return "/tmp"; // Fallback
    }
    
    std::string escape_html(const std::string& text) {
        std::string escaped;
        for (char c : text) {
            switch (c) {
                case '<': escaped += "&lt;"; break;
                case '>': escaped += "&gt;"; break;
                case '&': escaped += "&amp;"; break;
                case '"': escaped += "&quot;"; break;
                case '\'': escaped += "&#39;"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }
    
    std::string markdown_to_html(const std::string& markdown) {
        std::string html = escape_html(markdown);
        
        // Convert **bold** to <b>bold</b>
        std::regex bold_regex(R"(\*\*(.*?)\*\*)");
        html = std::regex_replace(html, bold_regex, "<b>$1</b>");
        
        // Convert *italic* to <i>italic</i>
        std::regex italic_regex(R"(\*(.*?)\*)");
        html = std::regex_replace(html, italic_regex, "<i>$1</i>");
        
        // Convert `code` to <code>code</code>
        std::regex code_regex(R"(`(.*?)`)");
        html = std::regex_replace(html, code_regex, "<code>$1</code>");
        
        // Convert newlines to <br>
        std::regex newline_regex(R"(\n)");
        html = std::regex_replace(html, newline_regex, "<br>");
        
        return html;
    }
    
}
