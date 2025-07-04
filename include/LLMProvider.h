#ifndef LLMPROVIDER_H
#define LLMPROVIDER_H

#include <string>
#include <memory>
#include <functional>
#include <map>

class LLMProvider {
public:
    enum class Provider {
        GEMINI,
        CLAUDE,
        CHATGPT
    };
    
    LLMProvider();
    ~LLMProvider();
    
    void set_api_key(Provider provider, const std::string& api_key);
    void send_message_async(Provider provider, const std::string& message, 
                           std::function<void(const std::string&, bool)> callback);
    
    std::string get_provider_name(Provider provider) const;
    
private:
    struct APIKeys {
        std::string gemini_key;
        std::string claude_key;
        std::string chatgpt_key;
    } api_keys;
    
    // HTTP request handling
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);
    
    // Provider-specific methods
    std::string send_gemini_request(const std::string& message);
    std::string send_claude_request(const std::string& message);
    std::string send_chatgpt_request(const std::string& message);
    
    // JSON request builders
    std::string build_gemini_request(const std::string& message);
    std::string build_claude_request(const std::string& message);
    std::string build_chatgpt_request(const std::string& message);
    
    // Response parsers
    std::string parse_gemini_response(const std::string& response);
    std::string parse_claude_response(const std::string& response);
    std::string parse_chatgpt_response(const std::string& response);
    
    void load_api_keys();
    void save_api_keys();
};

#endif // LLMPROVIDER_H
