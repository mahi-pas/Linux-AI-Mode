#ifndef CHATHISTORY_H
#define CHATHISTORY_H

#include <string>
#include <vector>
#include <map>

struct ChatMessage {
    std::string content;
    bool is_user;
    long timestamp;
};

struct Chat {
    std::string id;
    std::string title;
    std::vector<ChatMessage> messages;
    long created_at;
    long last_updated;
    std::string llm_provider;
};

class ChatHistory {
public:
    ChatHistory();
    ~ChatHistory();
    
    std::string create_new_chat(const std::string& llm_provider = "gemini");
    void add_message(const std::string& chat_id, const std::string& content, bool is_user);
    std::vector<Chat> get_all_chats();
    Chat get_chat(const std::string& chat_id);
    void delete_chat(const std::string& chat_id);
    void clear_all_chats();
    
    void save_to_file();
    void load_from_file();
    
private:
    std::map<std::string, Chat> chats;
    std::string data_file_path;
    
    std::string generate_chat_id();
    std::string generate_chat_title(const std::string& first_message);
    std::string get_data_directory();
    
    void ensure_data_directory_exists();
};

#endif // CHATHISTORY_H
