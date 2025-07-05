#ifndef AICHAT_H
#define AICHAT_H

#include <gtk/gtk.h>
#include <memory>
#include <string>
#include <vector>

class LLMProvider;
class ChatHistory;
class MarkdownUtils;

class AIChat {
public:
    AIChat();
    ~AIChat();
    
    void run();
    void show_window();
    void hide_window();
    
private:
    GtkApplication* app;
    GtkWidget* window;
    GtkWidget* main_box;
    GtkWidget* header_bar;
    GtkWidget* sidebar;
    GtkWidget* chat_area;
    GtkWidget* chat_container;  // Container for all chat messages
    GtkWidget* input_box;
    GtkWidget* input_entry;
    GtkWidget* send_button;
    GtkWidget* llm_selector;
    GtkWidget* chat_scrolled;
    GtkWidget* chat_view;
    GtkTextBuffer* chat_buffer;
    GtkWidget* history_list;
    
    std::unique_ptr<LLMProvider> llm_provider;
    std::unique_ptr<ChatHistory> chat_history;
    
    std::string current_chat_id;
    std::string selected_llm;
    
    // Callbacks
    static void on_activate(GtkApplication* app, gpointer user_data);
    static void on_startup(GtkApplication* app, gpointer user_data);
    static void on_send_clicked(GtkButton* button, gpointer user_data);
    static void on_input_activate(GtkEntry* entry, gpointer user_data);
    static void on_llm_changed(GtkDropDown* dropdown, GParamSpec* pspec, gpointer user_data);
    static void on_new_chat_clicked(GtkButton* button, gpointer user_data);
    static void on_chat_selected(GtkListBox* box, GtkListBoxRow* row, gpointer user_data);
    static gboolean on_key_press(GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data);
    static void on_run_code_clicked(GtkButton* button, gpointer user_data);
    
    // UI setup methods
    void setup_window();
    void setup_header_bar();
    void setup_sidebar();
    void setup_chat_area();
    void setup_input_area();
    void setup_transparency();
    void setup_global_hotkey();
    void setup_css();
    
    // Chat methods
    void send_message();
    void add_message_to_chat(const std::string& message, bool is_user);
    void add_formatted_message_to_chat(const std::string& message, bool is_user);
    void start_new_chat();
    void load_chat(const std::string& chat_id);
    void update_history_sidebar();
    void show_typing_indicator();
    void hide_typing_indicator();
    
    // Async response handling
    static gboolean on_llm_response(gpointer user_data);
    struct ResponseData {
        AIChat* chat;
        std::string response;
        bool success;
    };
    
    void process_llm_response_async(const std::string& message);
};

#endif // AICHAT_H
