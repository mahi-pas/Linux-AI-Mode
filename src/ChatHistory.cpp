#include "ChatHistory.h"
#include "Utils.h"
#include <json/json.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>

ChatHistory::ChatHistory() {
    data_file_path = get_data_directory() + "/chat_history.json";
    ensure_data_directory_exists();
    load_from_file();
}

ChatHistory::~ChatHistory() {
    save_to_file();
}

std::string ChatHistory::create_new_chat(const std::string& llm_provider) {
    std::string chat_id = generate_chat_id();
    
    Chat new_chat;
    new_chat.id = chat_id;
    new_chat.title = "New Chat";
    new_chat.created_at = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    new_chat.last_updated = new_chat.created_at;
    new_chat.llm_provider = llm_provider;
    
    chats[chat_id] = new_chat;
    save_to_file();
    
    return chat_id;
}

void ChatHistory::add_message(const std::string& chat_id, const std::string& content, bool is_user) {
    auto it = chats.find(chat_id);
    if (it == chats.end()) {
        return;
    }
    
    ChatMessage message;
    message.content = content;
    message.is_user = is_user;
    message.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    it->second.messages.push_back(message);
    it->second.last_updated = message.timestamp;
    
    // Update chat title if this is the first user message
    if (is_user && it->second.messages.size() == 1) {
        it->second.title = generate_chat_title(content);
    }
    
    save_to_file();
}

std::vector<Chat> ChatHistory::get_all_chats() {
    std::vector<Chat> chat_list;
    for (const auto& pair : chats) {
        chat_list.push_back(pair.second);
    }
    
    // Sort by last_updated in descending order
    std::sort(chat_list.begin(), chat_list.end(), 
              [](const Chat& a, const Chat& b) {
                  return a.last_updated > b.last_updated;
              });
    
    return chat_list;
}

Chat ChatHistory::get_chat(const std::string& chat_id) {
    auto it = chats.find(chat_id);
    if (it != chats.end()) {
        return it->second;
    }
    return Chat{}; // Return empty chat if not found
}

void ChatHistory::delete_chat(const std::string& chat_id) {
    chats.erase(chat_id);
    save_to_file();
}

void ChatHistory::clear_all_chats() {
    chats.clear();
    save_to_file();
}

void ChatHistory::save_to_file() {
    Json::Value root;
    Json::Value chats_array(Json::arrayValue);
    
    for (const auto& pair : chats) {
        const Chat& chat = pair.second;
        Json::Value chat_json;
        
        chat_json["id"] = chat.id;
        chat_json["title"] = chat.title;
        chat_json["created_at"] = static_cast<Json::Int64>(chat.created_at);
        chat_json["last_updated"] = static_cast<Json::Int64>(chat.last_updated);
        chat_json["llm_provider"] = chat.llm_provider;
        
        Json::Value messages_array(Json::arrayValue);
        for (const auto& message : chat.messages) {
            Json::Value message_json;
            message_json["content"] = message.content;
            message_json["is_user"] = message.is_user;
            message_json["timestamp"] = static_cast<Json::Int64>(message.timestamp);
            messages_array.append(message_json);
        }
        chat_json["messages"] = messages_array;
        
        chats_array.append(chat_json);
    }
    
    root["chats"] = chats_array;
    
    std::ofstream file(data_file_path);
    if (file.is_open()) {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
    } else {
        std::cerr << "Error: Could not save chat history to " << data_file_path << std::endl;
    }
}

void ChatHistory::load_from_file() {
    if (!Utils::file_exists(data_file_path)) {
        return;
    }
    
    std::ifstream file(data_file_path);
    if (!file.is_open()) {
        return;
    }
    
    try {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        
        if (!Json::parseFromStream(builder, file, &root, &errors)) {
            std::cerr << "Error parsing chat history JSON: " << errors << std::endl;
            return;
        }
        
        if (!root.isMember("chats") || !root["chats"].isArray()) {
            return;
        }
        
        const Json::Value& chats_array = root["chats"];
        for (const auto& chat_json : chats_array) {
            Chat chat;
            
            if (chat_json.isMember("id")) {
                chat.id = chat_json["id"].asString();
            }
            if (chat_json.isMember("title")) {
                chat.title = chat_json["title"].asString();
            }
            if (chat_json.isMember("created_at")) {
                chat.created_at = chat_json["created_at"].asInt64();
            }
            if (chat_json.isMember("last_updated")) {
                chat.last_updated = chat_json["last_updated"].asInt64();
            }
            if (chat_json.isMember("llm_provider")) {
                chat.llm_provider = chat_json["llm_provider"].asString();
            }
            
            if (chat_json.isMember("messages") && chat_json["messages"].isArray()) {
                const Json::Value& messages_array = chat_json["messages"];
                for (const auto& message_json : messages_array) {
                    ChatMessage message;
                    
                    if (message_json.isMember("content")) {
                        message.content = message_json["content"].asString();
                    }
                    if (message_json.isMember("is_user")) {
                        message.is_user = message_json["is_user"].asBool();
                    }
                    if (message_json.isMember("timestamp")) {
                        message.timestamp = message_json["timestamp"].asInt64();
                    }
                    
                    chat.messages.push_back(message);
                }
            }
            
            chats[chat.id] = chat;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading chat history: " << e.what() << std::endl;
    }
}

std::string ChatHistory::generate_chat_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::string chat_id;
    for (int i = 0; i < 16; ++i) {
        int digit = dis(gen);
        chat_id += (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
    }
    
    return chat_id;
}

std::string ChatHistory::generate_chat_title(const std::string& first_message) {
    std::string title = first_message;
    
    // Limit title length
    if (title.length() > 50) {
        title = title.substr(0, 47) + "...";
    }
    
    // Remove newlines
    std::replace(title.begin(), title.end(), '\n', ' ');
    
    return title;
}

std::string ChatHistory::get_data_directory() {
    return Utils::get_home_directory() + "/.local/share/linux-ai-mode";
}

void ChatHistory::ensure_data_directory_exists() {
    Utils::create_directory(get_data_directory());
}
