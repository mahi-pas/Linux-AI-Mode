#include "LLMProvider.h"
#include "Utils.h"
#include <curl/curl.h>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <thread>

LLMProvider::LLMProvider() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    load_api_keys();
}

LLMProvider::~LLMProvider() {
    curl_global_cleanup();
}

void LLMProvider::set_api_key(Provider provider, const std::string& api_key) {
    switch (provider) {
        case Provider::GEMINI:
            api_keys.gemini_key = api_key;
            break;
        case Provider::CLAUDE:
            api_keys.claude_key = api_key;
            break;
        case Provider::CHATGPT:
            api_keys.chatgpt_key = api_key;
            break;
    }
    save_api_keys();
}

void LLMProvider::send_message_async(Provider provider, const std::string& message,
                                    std::function<void(const std::string&, bool)> callback) {
    std::thread([this, provider, message, callback]() {
        try {
            std::string response;
            switch (provider) {
                case Provider::GEMINI:
                    response = send_gemini_request(message);
                    break;
                case Provider::CLAUDE:
                    response = send_claude_request(message);
                    break;
                case Provider::CHATGPT:
                    response = send_chatgpt_request(message);
                    break;
            }
            callback(response, !response.empty());
        } catch (const std::exception& e) {
            callback("Error: " + std::string(e.what()), false);
        }
    }).detach();
}

std::string LLMProvider::get_provider_name(Provider provider) const {
    switch (provider) {
        case Provider::GEMINI: return "Gemini";
        case Provider::CLAUDE: return "Claude";
        case Provider::CHATGPT: return "ChatGPT";
        default: return "Unknown";
    }
}

size_t LLMProvider::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string LLMProvider::send_gemini_request(const std::string& message) {
    if (api_keys.gemini_key.empty()) {
        return "Error: Gemini API key not configured";
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error: Failed to initialize CURL";
    }
    
    std::string response;
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash-exp:generateContent?key=" + api_keys.gemini_key;
    std::string json_data = build_gemini_request(message);
    
    // Debug logging
    std::cout << "=== GEMINI DEBUG INFO ===" << std::endl;
    std::cout << "URL: " << url << std::endl;
    std::cout << "Request JSON: " << json_data << std::endl;
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // Enable verbose output
    
    CURLcode res = curl_easy_perform(curl);
    
    // Get HTTP response code
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    std::cout << "HTTP Response Code: " << response_code << std::endl;
    std::cout << "CURL Result: " << curl_easy_strerror(res) << std::endl;
    std::cout << "Raw Response: " << response << std::endl;
    std::cout << "=========================" << std::endl;
    
    if (res != CURLE_OK) {
        return "Error: Failed to connect to Gemini API: " + std::string(curl_easy_strerror(res));
    }
    
    return parse_gemini_response(response);
}

std::string LLMProvider::send_claude_request(const std::string& message) {
    if (api_keys.claude_key.empty()) {
        return "Error: Claude API key not configured";
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error: Failed to initialize CURL";
    }
    
    std::string response;
    std::string url = "https://api.anthropic.com/v1/messages";
    std::string json_data = build_claude_request(message);
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("x-api-key: " + api_keys.claude_key).c_str());
    headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        return "Error: Failed to connect to Claude API";
    }
    
    return parse_claude_response(response);
}

std::string LLMProvider::send_chatgpt_request(const std::string& message) {
    if (api_keys.chatgpt_key.empty()) {
        return "Error: ChatGPT API key not configured";
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error: Failed to initialize CURL";
    }
    
    std::string response;
    std::string url = "https://api.openai.com/v1/chat/completions";
    std::string json_data = build_chatgpt_request(message);
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_keys.chatgpt_key).c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        return "Error: Failed to connect to ChatGPT API";
    }
    
    return parse_chatgpt_response(response);
}

std::string LLMProvider::build_gemini_request(const std::string& message) {
    Json::Value root;
    Json::Value contents(Json::arrayValue);
    Json::Value content;
    Json::Value parts(Json::arrayValue);
    Json::Value part;
    
    part["text"] = message;
    parts.append(part);
    content["parts"] = parts;
    contents.append(content);
    root["contents"] = contents;
    
    // Add generation config for better responses
    Json::Value generationConfig;
    generationConfig["temperature"] = 0.7;
    generationConfig["topK"] = 40;
    generationConfig["topP"] = 0.95;
    generationConfig["maxOutputTokens"] = 1024;
    root["generationConfig"] = generationConfig;
    
    // Add safety settings to be less restrictive
    Json::Value safetySettings(Json::arrayValue);
    Json::Value safetySetting;
    safetySetting["category"] = "HARM_CATEGORY_HARASSMENT";
    safetySetting["threshold"] = "BLOCK_MEDIUM_AND_ABOVE";
    safetySettings.append(safetySetting);
    root["safetySettings"] = safetySettings;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, root);
}

std::string LLMProvider::build_claude_request(const std::string& message) {
    Json::Value root;
    Json::Value messages(Json::arrayValue);
    Json::Value msg;
    
    msg["role"] = "user";
    msg["content"] = message;
    messages.append(msg);
    
    root["model"] = "claude-3-sonnet-20240229";
    root["max_tokens"] = 1000;
    root["messages"] = messages;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, root);
}

std::string LLMProvider::build_chatgpt_request(const std::string& message) {
    Json::Value root;
    Json::Value messages(Json::arrayValue);
    Json::Value msg;
    
    msg["role"] = "user";
    msg["content"] = message;
    messages.append(msg);
    
    root["model"] = "gpt-3.5-turbo";
    root["messages"] = messages;
    root["max_tokens"] = 1000;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, root);
}

std::string LLMProvider::parse_gemini_response(const std::string& response) {
    std::cout << "=== GEMINI RESPONSE PARSING ===" << std::endl;
    std::cout << "Response length: " << response.length() << std::endl;
    std::cout << "Raw response: " << response << std::endl;
    
    try {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream stream(response);
        
        if (!Json::parseFromStream(builder, stream, &root, &errors)) {
            std::cout << "JSON Parse Error: " << errors << std::endl;
            return "Error: Failed to parse Gemini response - Invalid JSON: " + errors;
        }
        
        std::cout << "JSON parsed successfully" << std::endl;
        std::cout << "Root members: ";
        for (const auto& member : root.getMemberNames()) {
            std::cout << member << " ";
        }
        std::cout << std::endl;
        
        // Check for error in response
        if (root.isMember("error")) {
            std::cout << "API Error detected" << std::endl;
            std::string error_msg = "Gemini API Error: ";
            if (root["error"].isMember("message")) {
                error_msg += root["error"]["message"].asString();
            } else {
                error_msg += "Unknown error";
            }
            std::cout << "Error message: " << error_msg << std::endl;
            return error_msg;
        }
        
        // Parse candidates array
        if (root.isMember("candidates")) {
            std::cout << "Candidates found: " << root["candidates"].isArray() << std::endl;
            if (root["candidates"].isArray()) {
                std::cout << "Candidates count: " << root["candidates"].size() << std::endl;
                
                if (root["candidates"].size() > 0) {
                    auto candidate = root["candidates"][0];
                    std::cout << "First candidate members: ";
                    for (const auto& member : candidate.getMemberNames()) {
                        std::cout << member << " ";
                    }
                    std::cout << std::endl;
                    
                    // Check if candidate has content
                    if (candidate.isMember("content")) {
                        std::cout << "Content found" << std::endl;
                        auto content = candidate["content"];
                        std::cout << "Content members: ";
                        for (const auto& member : content.getMemberNames()) {
                            std::cout << member << " ";
                        }
                        std::cout << std::endl;
                        
                        if (content.isMember("parts") && content["parts"].isArray()) {
                            std::cout << "Parts found, count: " << content["parts"].size() << std::endl;
                            
                            if (content["parts"].size() > 0) {
                                auto part = content["parts"][0];
                                std::cout << "First part members: ";
                                for (const auto& member : part.getMemberNames()) {
                                    std::cout << member << " ";
                                }
                                std::cout << std::endl;
                                
                                if (part.isMember("text")) {
                                    std::string text = part["text"].asString();
                                    std::cout << "Text found: " << text.substr(0, 100) << "..." << std::endl;
                                    return text;
                                } else {
                                    std::cout << "No 'text' field in part" << std::endl;
                                }
                            } else {
                                std::cout << "Parts array is empty" << std::endl;
                            }
                        } else {
                            std::cout << "No 'parts' array in content" << std::endl;
                        }
                    } else {
                        std::cout << "No 'content' in candidate" << std::endl;
                    }
                    
                    // Check for finish reason that might indicate filtering
                    if (candidate.isMember("finishReason")) {
                        std::string reason = candidate["finishReason"].asString();
                        std::cout << "Finish reason: " << reason << std::endl;
                        if (reason == "SAFETY") {
                            return "Response was filtered for safety reasons. Please try rephrasing your message.";
                        } else if (reason == "MAX_TOKENS") {
                            return "Response was truncated due to length limits.";
                        }
                    } else {
                        std::cout << "No finish reason found" << std::endl;
                    }
                } else {
                    std::cout << "Candidates array is empty" << std::endl;
                }
            } else {
                std::cout << "Candidates is not an array" << std::endl;
            }
        } else {
            std::cout << "No 'candidates' field in response" << std::endl;
        }
        
        std::cout << "Unexpected response format detected" << std::endl;
        return "Error: Unexpected Gemini response format. Check console for debug info.";
        
    } catch (const std::exception& e) {
        std::cout << "Exception in parsing: " << e.what() << std::endl;
        return "Error: Failed to parse Gemini response: " + std::string(e.what());
    }
}

std::string LLMProvider::parse_claude_response(const std::string& response) {
    try {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream stream(response);
        
        if (!Json::parseFromStream(builder, stream, &root, &errors)) {
            return "Error: Failed to parse Claude response";
        }
        
        if (root.isMember("content") && root["content"].isArray() && 
            root["content"].size() > 0) {
            return root["content"][0]["text"].asString();
        }
        
        return "Error: Unexpected Claude response format";
    } catch (const std::exception& e) {
        return "Error: Failed to parse Claude response: " + std::string(e.what());
    }
}

std::string LLMProvider::parse_chatgpt_response(const std::string& response) {
    try {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream stream(response);
        
        if (!Json::parseFromStream(builder, stream, &root, &errors)) {
            return "Error: Failed to parse ChatGPT response";
        }
        
        if (root.isMember("choices") && root["choices"].isArray() && 
            root["choices"].size() > 0) {
            auto choice = root["choices"][0];
            if (choice.isMember("message") && 
                choice["message"].isMember("content")) {
                return choice["message"]["content"].asString();
            }
        }
        
        return "Error: Unexpected ChatGPT response format";
    } catch (const std::exception& e) {
        return "Error: Failed to parse ChatGPT response: " + std::string(e.what());
    }
}

void LLMProvider::load_api_keys() {
    std::string config_path = Utils::get_home_directory() + "/.config/linux-ai-mode/api_keys.json";
    
    if (!Utils::file_exists(config_path)) {
        return;
    }
    
    std::ifstream file(config_path);
    if (!file.is_open()) {
        return;
    }
    
    try {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        
        if (Json::parseFromStream(builder, file, &root, &errors)) {
            if (root.isMember("gemini")) {
                api_keys.gemini_key = root["gemini"].asString();
            }
            if (root.isMember("claude")) {
                api_keys.claude_key = root["claude"].asString();
            }
            if (root.isMember("chatgpt")) {
                api_keys.chatgpt_key = root["chatgpt"].asString();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading API keys: " << e.what() << std::endl;
    }
}

void LLMProvider::save_api_keys() {
    std::string config_dir = Utils::get_home_directory() + "/.config/linux-ai-mode";
    Utils::create_directory(config_dir);
    
    std::string config_path = config_dir + "/api_keys.json";
    
    Json::Value root;
    root["gemini"] = api_keys.gemini_key;
    root["claude"] = api_keys.claude_key;
    root["chatgpt"] = api_keys.chatgpt_key;
    
    std::ofstream file(config_path);
    if (file.is_open()) {
        Json::StreamWriterBuilder builder;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
    }
}
