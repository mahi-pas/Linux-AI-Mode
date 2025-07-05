#include "AIChat.h"
#include "LLMProvider.h"
#include "ChatHistory.h"
#include "Utils.h"
#include "MarkdownUtils.h"
#include <iostream>
#include <thread>
#include <json/json.h>

AIChat::AIChat() : app(nullptr), window(nullptr), current_chat_id(""), selected_llm("gemini") {
    llm_provider = std::make_unique<LLMProvider>();
    chat_history = std::make_unique<ChatHistory>();
    
    app = gtk_application_new("com.github.linux-ai-mode", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), this);
    g_signal_connect(app, "startup", G_CALLBACK(on_startup), this);
}

AIChat::~AIChat() {
    if (app) {
        g_object_unref(app);
    }
}

void AIChat::run() {
    int status = g_application_run(G_APPLICATION(app), 0, nullptr);
    if (status != 0) {
        std::cerr << "Application failed to run with status: " << status << std::endl;
    }
}

void AIChat::show_window() {
    if (window) {
        gtk_window_present(GTK_WINDOW(window));
        gtk_widget_grab_focus(input_entry);
    }
}

void AIChat::hide_window() {
    if (window) {
        gtk_window_minimize(GTK_WINDOW(window));
    }
}

void AIChat::on_activate(GtkApplication* app, gpointer user_data) {
    AIChat* chat = static_cast<AIChat*>(user_data);
    chat->setup_window();
    chat->setup_css();
    chat->setup_transparency();
    chat->setup_global_hotkey();
    chat->start_new_chat();
}

void AIChat::on_startup(GtkApplication* app, gpointer user_data) {
    // Application startup tasks
}

void AIChat::setup_window() {
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Linux AI Mode");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_window_maximize(GTK_WINDOW(window));
    
    // Set window properties for quick access
    // Note: gtk_window_set_keep_above is not available in GTK4
    gtk_window_set_decorated(GTK_WINDOW(window), TRUE);
    
    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), main_box);
    
    setup_header_bar();
    setup_sidebar();
    setup_chat_area();
    
    gtk_window_present(GTK_WINDOW(window));
}

void AIChat::setup_header_bar() {
    header_bar = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
    
    // New chat button
    GtkWidget* new_chat_btn = gtk_button_new_from_icon_name("document-new-symbolic");
    gtk_widget_set_tooltip_text(new_chat_btn, "New Chat");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), new_chat_btn);
    g_signal_connect(new_chat_btn, "clicked", G_CALLBACK(on_new_chat_clicked), this);
}

void AIChat::setup_sidebar() {
    sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(sidebar, 280, -1);
    gtk_widget_add_css_class(sidebar, "sidebar");
    
    GtkWidget* history_label = gtk_label_new("Chat History");
    gtk_widget_add_css_class(history_label, "sidebar-title");
    gtk_box_append(GTK_BOX(sidebar), history_label);
    
    GtkWidget* history_scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(history_scrolled),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(history_scrolled, TRUE);
    
    history_list = gtk_list_box_new();
    gtk_widget_add_css_class(history_list, "history-list");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(history_scrolled), history_list);
    gtk_box_append(GTK_BOX(sidebar), history_scrolled);
    
    g_signal_connect(history_list, "row-selected", G_CALLBACK(on_chat_selected), this);
    
    gtk_box_append(GTK_BOX(main_box), sidebar);
}

void AIChat::setup_chat_area() {
    chat_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(chat_area, "chat-area");
    gtk_widget_set_hexpand(chat_area, TRUE);
    gtk_widget_set_vexpand(chat_area, TRUE);
    
    // Chat messages area (takes most of the space)
    chat_scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(chat_scrolled),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(chat_scrolled, TRUE);
    
    // Create a container for chat messages
    chat_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(chat_container, "chat-container");
    gtk_widget_set_margin_start(chat_container, 20);
    gtk_widget_set_margin_end(chat_container, 20);
    gtk_widget_set_margin_top(chat_container, 20);
    gtk_widget_set_margin_bottom(chat_container, 20);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(chat_scrolled), chat_container);
    gtk_box_append(GTK_BOX(chat_area), chat_scrolled);
    
    // Keep the old text view for backward compatibility (hidden)
    chat_view = gtk_text_view_new();
    chat_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_view));
    gtk_widget_set_visible(chat_view, FALSE);
    
    // Input area at the bottom
    setup_input_area();
    
    gtk_box_append(GTK_BOX(main_box), chat_area);
}

void AIChat::setup_input_area() {
    input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_add_css_class(input_box, "input-box");
    gtk_widget_set_margin_start(input_box, 12);
    gtk_widget_set_margin_end(input_box, 12);
    gtk_widget_set_margin_top(input_box, 8);
    gtk_widget_set_margin_bottom(input_box, 12);
    
    input_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_entry), "Type your message...");
    gtk_widget_add_css_class(input_entry, "message-input");
    gtk_widget_set_hexpand(input_entry, TRUE);
    
    // LLM selector next to send button
    GtkStringList* string_list = gtk_string_list_new(NULL);
    gtk_string_list_append(string_list, "Gemini");
    gtk_string_list_append(string_list, "Claude");
    gtk_string_list_append(string_list, "ChatGPT");
    llm_selector = gtk_drop_down_new(G_LIST_MODEL(string_list), NULL);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(llm_selector), 0);
    gtk_widget_add_css_class(llm_selector, "llm-selector");
    gtk_widget_set_tooltip_text(llm_selector, "Select AI Model");
    g_signal_connect(llm_selector, "notify::selected", G_CALLBACK(on_llm_changed), this);
    
    send_button = gtk_button_new_from_icon_name("mail-send-symbolic");
    gtk_widget_add_css_class(send_button, "send-button");
    gtk_widget_set_tooltip_text(send_button, "Send Message (Ctrl+Enter)");
    
    g_signal_connect(input_entry, "activate", G_CALLBACK(on_input_activate), this);
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_clicked), this);
    
    gtk_box_append(GTK_BOX(input_box), input_entry);
    gtk_box_append(GTK_BOX(input_box), llm_selector);
    gtk_box_append(GTK_BOX(input_box), send_button);
    
    gtk_box_append(GTK_BOX(chat_area), input_box);
}

void AIChat::setup_transparency() {
    // Apply CSS for transparency
    GtkCssProvider* provider = gtk_css_provider_new();
    
    const char* css_data = R"(
        window {
            background: rgba(0, 0, 0, 0.85);
            backdrop-filter: blur(10px);
        }
    )";
    
    gtk_css_provider_load_from_string(provider, css_data);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    
    g_object_unref(provider);
}

void AIChat::setup_global_hotkey() {
    // Set up keyboard shortcuts
    GtkEventController* key_controller = gtk_event_controller_key_new();
    gtk_widget_add_controller(window, key_controller);
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_press), this);
}

void AIChat::setup_css() {
    GtkCssProvider* provider = gtk_css_provider_new();
    
    const char* css_file_path = "css/style.css";
    if (Utils::file_exists(css_file_path)) {
        gtk_css_provider_load_from_path(provider, css_file_path);
    } else {
        // Fallback inline CSS
        const char* css_data = R"(
            .sidebar {
                background: rgba(30, 30, 30, 0.9);
                border-right: 1px solid rgba(255, 255, 255, 0.1);
                padding: 10px;
            }
            
            .sidebar-title {
                font-weight: bold;
                color: #ffffff;
                margin-bottom: 10px;
            }
            
            .chat-area {
                background: rgba(20, 20, 20, 0.8);
                padding: 10px;
            }
            
            .chat-view {
                background: transparent;
                color: #ffffff;
                font-family: 'Ubuntu Mono', monospace;
                padding: 10px;
            }
            
            .input-box {
                padding: 10px;
                background: rgba(40, 40, 40, 0.9);
                border-top: 1px solid rgba(255, 255, 255, 0.1);
            }
            
            .message-input {
                background: rgba(60, 60, 60, 0.8);
                color: #ffffff;
                border: 1px solid rgba(255, 255, 255, 0.2);
                border-radius: 20px;
                padding: 10px 15px;
            }
            
            .send-button {
                background: rgba(0, 150, 255, 0.8);
                color: white;
                border: none;
                border-radius: 50%;
                padding: 10px;
                margin-left: 5px;
            }
            
            .send-button:hover {
                background: rgba(0, 150, 255, 1.0);
            }
        )";
        gtk_css_provider_load_from_string(provider, css_data);
    }
    
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    
    g_object_unref(provider);
}

void AIChat::on_run_code_clicked(GtkButton* button, gpointer user_data) {
    const char* code = static_cast<const char*>(g_object_get_data(G_OBJECT(button), "code"));
    if (code) {
        TerminalManager& terminal_manager = TerminalManager::get_instance();
        terminal_manager.run_command(code);
    }
}

void AIChat::on_send_clicked(GtkButton* button, gpointer user_data) {
    AIChat* chat = static_cast<AIChat*>(user_data);
    chat->send_message();
}

void AIChat::on_input_activate(GtkEntry* entry, gpointer user_data) {
    AIChat* chat = static_cast<AIChat*>(user_data);
    chat->send_message();
}

void AIChat::on_llm_changed(GtkDropDown* dropdown, GParamSpec* pspec, gpointer user_data) {
    AIChat* chat = static_cast<AIChat*>(user_data);
    guint selected = gtk_drop_down_get_selected(dropdown);
    
    switch (selected) {
        case 0: chat->selected_llm = "gemini"; break;
        case 1: chat->selected_llm = "claude"; break;
        case 2: chat->selected_llm = "chatgpt"; break;
    }
}

void AIChat::on_new_chat_clicked(GtkButton* button, gpointer user_data) {
    AIChat* chat = static_cast<AIChat*>(user_data);
    chat->start_new_chat();
}

void AIChat::on_chat_selected(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
    if (!row) return;
    
    AIChat* chat = static_cast<AIChat*>(user_data);
    const char* chat_id = static_cast<const char*>(g_object_get_data(G_OBJECT(row), "chat-id"));
    if (chat_id) {
        chat->load_chat(chat_id);
    }
}

gboolean AIChat::on_key_press(GtkEventControllerKey* controller, guint keyval, 
                             guint keycode, GdkModifierType state, gpointer user_data) {
    AIChat* chat = static_cast<AIChat*>(user_data);
    
    // Ctrl+Enter to send message
    if ((state & GDK_CONTROL_MASK) && keyval == GDK_KEY_Return) {
        chat->send_message();
        return TRUE;
    }
    
    // Escape to hide window
    if (keyval == GDK_KEY_Escape) {
        chat->hide_window();
        return TRUE;
    }
    
    return FALSE;
}

void AIChat::send_message() {
    const char* text = gtk_editable_get_text(GTK_EDITABLE(input_entry));
    if (!text || strlen(text) == 0) return;
    
    std::string message(text);
    gtk_editable_set_text(GTK_EDITABLE(input_entry), "");
    
    // Add user message to chat
    add_message_to_chat(message, true);
    chat_history->add_message(current_chat_id, message, true);
    
    // Show typing indicator
    show_typing_indicator();
    
    // Process LLM response asynchronously
    process_llm_response_async(message);
}

void AIChat::add_message_to_chat(const std::string& message, bool is_user) {
    // Use formatted message display for AI responses, simple for user messages
    if (is_user) {
        // Create a simple user message widget
        GtkWidget* message_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_widget_add_css_class(message_box, "message-row");
        gtk_widget_add_css_class(message_box, "user-message-row");
        
        // Spacer to align user messages to the right
        GtkWidget* spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_hexpand(spacer, TRUE);
        gtk_box_append(GTK_BOX(message_box), spacer);
        
        // User message bubble
        GtkWidget* user_bubble = gtk_frame_new(nullptr);
        gtk_widget_add_css_class(user_bubble, "user-message");
        
        GtkWidget* user_label = gtk_label_new(message.c_str());
        gtk_label_set_wrap(GTK_LABEL(user_label), TRUE);
        gtk_label_set_wrap_mode(GTK_LABEL(user_label), PANGO_WRAP_WORD_CHAR);
        gtk_widget_set_halign(user_label, GTK_ALIGN_START);
        gtk_frame_set_child(GTK_FRAME(user_bubble), user_label);
        
        gtk_box_append(GTK_BOX(message_box), user_bubble);
        gtk_box_append(GTK_BOX(chat_container), message_box);
    } else {
        add_formatted_message_to_chat(message, is_user);
    }
    
    // Scroll to bottom
    GtkAdjustment* vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(chat_scrolled));
    gtk_adjustment_set_value(vadjustment, gtk_adjustment_get_upper(vadjustment));
}

void AIChat::add_formatted_message_to_chat(const std::string& message, bool is_user) {
    // Create AI message container
    GtkWidget* message_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(message_box, "message-row");
    gtk_widget_add_css_class(message_box, "ai-message-row");
    
    // AI message bubble
    GtkWidget* ai_bubble = gtk_frame_new(nullptr);
    gtk_widget_add_css_class(ai_bubble, "ai-message");
    
    GtkWidget* content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_frame_set_child(GTK_FRAME(ai_bubble), content_box);
    
    // Extract code blocks from the message for run buttons
    auto code_blocks = MarkdownUtils::extract_code_blocks(message);
    
    // Always show the full message with markdown formatting first
    GtkWidget* text_widget = MarkdownUtils::create_formatted_text_widget(message);
    gtk_box_append(GTK_BOX(content_box), text_widget);
    
    // Add interactive code blocks below the full message if any executable code exists
    for (const auto& block : code_blocks) {
        // Only add interactive widgets for executable code
        if (block.language == "bash" || block.language == "sh" || 
            block.language == "shell" || block.language == "terminal") {
            
            GtkWidget* code_actions_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
            gtk_widget_add_css_class(code_actions_box, "code-actions");
            
            // Add a small label to identify this code block
            std::string code_preview = block.code;
            if (code_preview.length() > 50) {
                code_preview = code_preview.substr(0, 47) + "...";
            }
            // Remove newlines for preview
            std::replace(code_preview.begin(), code_preview.end(), '\n', ' ');
            
            GtkWidget* code_label = gtk_label_new(("💻 " + code_preview).c_str());
            gtk_widget_add_css_class(code_label, "code-preview");
            gtk_label_set_ellipsize(GTK_LABEL(code_label), PANGO_ELLIPSIZE_END);
            gtk_widget_set_hexpand(code_label, TRUE);
            gtk_widget_set_halign(code_label, GTK_ALIGN_START);
            
            // Run button
            GtkWidget* run_button = gtk_button_new_from_icon_name("media-playback-start-symbolic");
            gtk_widget_add_css_class(run_button, "run-button");
            gtk_widget_set_tooltip_text(run_button, "Run this command in terminal");
            
            // Store the code in the button's data
            g_object_set_data_full(G_OBJECT(run_button), "code", 
                                  g_strdup(block.code.c_str()), g_free);
            
            g_signal_connect(run_button, "clicked", G_CALLBACK(on_run_code_clicked), this);
            
            gtk_box_append(GTK_BOX(code_actions_box), code_label);
            gtk_box_append(GTK_BOX(code_actions_box), run_button);
            gtk_box_append(GTK_BOX(content_box), code_actions_box);
        }
    }
    
    gtk_box_append(GTK_BOX(message_box), ai_bubble);
    
    // Spacer to align AI messages to the left
    GtkWidget* spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(message_box), spacer);
    
    gtk_box_append(GTK_BOX(chat_container), message_box);
    
    // Show all new widgets
    gtk_widget_set_visible(message_box, TRUE);
}

void AIChat::start_new_chat() {
    current_chat_id = chat_history->create_new_chat(selected_llm);
    
    // Clear chat container
    GtkWidget* child = gtk_widget_get_first_child(chat_container);
    while (child) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(chat_container), child);
        child = next;
    }
    
    // Also clear the old text buffer for compatibility
    gtk_text_buffer_set_text(chat_buffer, "", -1);
    
    // Update history sidebar
    update_history_sidebar();
    
    // Focus on input
    gtk_widget_grab_focus(input_entry);
}

void AIChat::load_chat(const std::string& chat_id) {
    current_chat_id = chat_id;
    Chat chat = chat_history->get_chat(chat_id);
    
    // Clear chat container
    GtkWidget* child = gtk_widget_get_first_child(chat_container);
    while (child) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(chat_container), child);
        child = next;
    }
    
    // Clear old text buffer for compatibility
    gtk_text_buffer_set_text(chat_buffer, "", -1);
    
    // Load messages with formatting
    for (const auto& msg : chat.messages) {
        add_message_to_chat(msg.content, msg.is_user);
    }
}

void AIChat::update_history_sidebar() {
    // Clear existing items
    GtkWidget* child = gtk_widget_get_first_child(history_list);
    while (child) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(GTK_LIST_BOX(history_list), child);
        child = next;
    }
    
    // Add chat history items
    auto chats = chat_history->get_all_chats();
    for (const auto& chat : chats) {
        GtkWidget* row = gtk_list_box_row_new();
        GtkWidget* label = gtk_label_new(chat.title.c_str());
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
        
        g_object_set_data_full(G_OBJECT(row), "chat-id", 
                              g_strdup(chat.id.c_str()), g_free);
        
        gtk_list_box_append(GTK_LIST_BOX(history_list), row);
    }
}

void AIChat::show_typing_indicator() {
    // Create a typing indicator widget
    GtkWidget* typing_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(typing_box, "message-row");
    gtk_widget_add_css_class(typing_box, "typing-indicator-row");
    g_object_set_data(G_OBJECT(typing_box), "is-typing", GINT_TO_POINTER(1));
    
    GtkWidget* typing_bubble = gtk_frame_new(nullptr);
    gtk_widget_add_css_class(typing_bubble, "typing-indicator");
    
    GtkWidget* typing_label = gtk_label_new("AI is typing...");
    gtk_frame_set_child(GTK_FRAME(typing_bubble), typing_label);
    
    gtk_box_append(GTK_BOX(typing_box), typing_bubble);
    
    // Spacer to align to the left
    GtkWidget* spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(typing_box), spacer);
    
    gtk_box_append(GTK_BOX(chat_container), typing_box);
    
    // Scroll to bottom
    GtkAdjustment* vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(chat_scrolled));
    gtk_adjustment_set_value(vadjustment, gtk_adjustment_get_upper(vadjustment));
}

void AIChat::hide_typing_indicator() {
    // Remove typing indicator widgets
    GtkWidget* child = gtk_widget_get_first_child(chat_container);
    while (child) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        if (g_object_get_data(G_OBJECT(child), "is-typing")) {
            gtk_box_remove(GTK_BOX(chat_container), child);
        }
        child = next;
    }
}

void AIChat::process_llm_response_async(const std::string& message) {
    std::thread([this, message]() {
        LLMProvider::Provider provider;
        if (selected_llm == "gemini") provider = LLMProvider::Provider::GEMINI;
        else if (selected_llm == "claude") provider = LLMProvider::Provider::CLAUDE;
        else provider = LLMProvider::Provider::CHATGPT;
        
        llm_provider->send_message_async(provider, message, 
            [this](const std::string& response, bool success) {
                ResponseData* data = new ResponseData{this, response, success};
                g_idle_add(on_llm_response, data);
            });
    }).detach();
}

gboolean AIChat::on_llm_response(gpointer user_data) {
    ResponseData* data = static_cast<ResponseData*>(user_data);
    
    data->chat->hide_typing_indicator();
    
    if (data->success) {
        data->chat->add_message_to_chat(data->response, false);
        data->chat->chat_history->add_message(data->chat->current_chat_id, data->response, false);
        data->chat->update_history_sidebar();
    } else {
        data->chat->add_message_to_chat("Error: Failed to get response from LLM", false);
    }
    
    delete data;
    return FALSE;
}
