#include "MarkdownUtils.h"
#include <regex>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <ctime>

std::vector<CodeBlock> MarkdownUtils::extract_code_blocks(const std::string& text) {
    std::vector<CodeBlock> blocks;
    
    try {
        // Regex to match code blocks with optional language specification
        std::regex code_block_regex(R"(```(\w*)\n?([\s\S]*?)```)");
        std::sregex_iterator iter(text.begin(), text.end(), code_block_regex);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            const std::smatch& match = *iter;
            CodeBlock block;
            block.language = match[1].str();
            block.code = match[2].str();
            block.start_pos = static_cast<size_t>(match.position());
            block.end_pos = static_cast<size_t>(match.position() + match.length());
            
            // Validate positions
            if (block.start_pos < text.length() && block.end_pos <= text.length() && 
                block.start_pos < block.end_pos) {
                
                // Trim leading/trailing whitespace from code
                block.code = std::regex_replace(block.code, std::regex("^\\s+|\\s+$"), "");
                
                blocks.push_back(block);
                
                std::cout << "Found code block: language='" << block.language 
                          << "', start=" << block.start_pos 
                          << ", end=" << block.end_pos 
                          << ", text_length=" << text.length() << std::endl;
            } else {
                std::cout << "Invalid code block positions: start=" << block.start_pos 
                          << ", end=" << block.end_pos 
                          << ", text_length=" << text.length() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error extracting code blocks: " << e.what() << std::endl;
    }
    
    return blocks;
}

GtkWidget* MarkdownUtils::create_formatted_text_widget(const std::string& markdown_text) {
    GtkWidget* scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    
    GtkWidget* text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    gtk_widget_add_css_class(text_view, "markdown-view");
    
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    apply_markdown_formatting(buffer, markdown_text);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), text_view);
    
    return scrolled;
}

void MarkdownUtils::apply_markdown_formatting(GtkTextBuffer* buffer, const std::string& text) {
    // Clear buffer
    gtk_text_buffer_set_text(buffer, "", -1);
    
    // Create text tags for formatting
    GtkTextTagTable* tag_table = gtk_text_buffer_get_tag_table(buffer);
    
    // Bold tag
    GtkTextTag* bold_tag = gtk_text_tag_table_lookup(tag_table, "bold");
    if (!bold_tag) {
        bold_tag = gtk_text_tag_new("bold");
        g_object_set(bold_tag, "weight", PANGO_WEIGHT_BOLD, nullptr);
        gtk_text_tag_table_add(tag_table, bold_tag);
    }
    
    // Italic tag
    GtkTextTag* italic_tag = gtk_text_tag_table_lookup(tag_table, "italic");
    if (!italic_tag) {
        italic_tag = gtk_text_tag_new("italic");
        g_object_set(italic_tag, "style", PANGO_STYLE_ITALIC, nullptr);
        gtk_text_tag_table_add(tag_table, italic_tag);
    }
    
    // Code tag
    GtkTextTag* code_tag = gtk_text_tag_table_lookup(tag_table, "code");
    if (!code_tag) {
        code_tag = gtk_text_tag_new("code");
        g_object_set(code_tag, "family", "monospace", 
                    "background", "rgba(40,40,50,0.8)", nullptr);
        gtk_text_tag_table_add(tag_table, code_tag);
    }
    
    // Heading tags
    GtkTextTag* h1_tag = gtk_text_tag_table_lookup(tag_table, "h1");
    if (!h1_tag) {
        h1_tag = gtk_text_tag_new("h1");
        g_object_set(h1_tag, "scale", 1.4, "weight", PANGO_WEIGHT_BOLD, nullptr);
        gtk_text_tag_table_add(tag_table, h1_tag);
    }
    
    GtkTextTag* h2_tag = gtk_text_tag_table_lookup(tag_table, "h2");
    if (!h2_tag) {
        h2_tag = gtk_text_tag_new("h2");
        g_object_set(h2_tag, "scale", 1.2, "weight", PANGO_WEIGHT_BOLD, nullptr);
        gtk_text_tag_table_add(tag_table, h2_tag);
    }
    
    // Process text line by line for better formatting
    std::istringstream stream(text);
    std::string line;
    GtkTextIter iter;
    
    while (std::getline(stream, line)) {
        gtk_text_buffer_get_end_iter(buffer, &iter);
        
        // Handle different markdown elements
        if (line.empty()) {
            // Empty line - add paragraph spacing
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        } else if (line.find("# ") == 0) {
            // H1 heading
            std::string heading_text = line.substr(2) + "\n";
            gtk_text_buffer_get_end_iter(buffer, &iter);
            GtkTextIter start = iter;
            gtk_text_buffer_insert(buffer, &iter, heading_text.c_str(), -1);
            gtk_text_buffer_apply_tag(buffer, h1_tag, &start, &iter);
        } else if (line.find("## ") == 0) {
            // H2 heading
            std::string heading_text = line.substr(3) + "\n";
            gtk_text_buffer_get_end_iter(buffer, &iter);
            GtkTextIter start = iter;
            gtk_text_buffer_insert(buffer, &iter, heading_text.c_str(), -1);
            gtk_text_buffer_apply_tag(buffer, h2_tag, &start, &iter);
        } else if (line.find("- ") == 0 || line.find("* ") == 0) {
            // Bullet point
            std::string bullet_text = "• " + line.substr(2) + "\n";
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, bullet_text.c_str(), -1);
        } else if (std::regex_match(line, std::regex("^\\d+\\. .*"))) {
            // Numbered list
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, (line + "\n").c_str(), -1);
        } else {
            // Regular text - apply inline formatting
            apply_inline_formatting(buffer, line + "\n", bold_tag, italic_tag, code_tag);
        }
    }
}

void MarkdownUtils::apply_inline_formatting(GtkTextBuffer* buffer, const std::string& text,
                                           GtkTextTag* bold_tag, GtkTextTag* italic_tag, GtkTextTag* code_tag) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    GtkTextIter start = iter;
    
    // Insert the text first
    gtk_text_buffer_insert(buffer, &iter, text.c_str(), -1);
    
    // Apply formatting using regex
    std::string::size_type offset = 0;
    
    // Bold formatting **text**
    std::regex bold_regex(R"(\*\*(.*?)\*\*)");
    std::sregex_iterator bold_iter(text.begin(), text.end(), bold_regex);
    std::sregex_iterator end;
    
    for (; bold_iter != end; ++bold_iter) {
        const std::smatch& match = *bold_iter;
        GtkTextIter format_start, format_end;
        
        gtk_text_buffer_get_iter_at_offset(buffer, &format_start, 
                                          gtk_text_iter_get_offset(&start) + match.position());
        gtk_text_buffer_get_iter_at_offset(buffer, &format_end, 
                                          gtk_text_iter_get_offset(&start) + match.position() + match.length());
        
        // Replace **text** with text and apply bold formatting
        gtk_text_buffer_delete(buffer, &format_start, &format_end);
        gtk_text_buffer_insert(buffer, &format_start, match[1].str().c_str(), -1);
        
        GtkTextIter bold_end;
        gtk_text_buffer_get_iter_at_offset(buffer, &bold_end, 
                                          gtk_text_iter_get_offset(&format_start) + match[1].length());
        gtk_text_buffer_apply_tag(buffer, bold_tag, &format_start, &bold_end);
    }
    
    // Inline code `text`
    std::regex code_regex(R"(`([^`]+)`)");
    std::sregex_iterator code_iter(text.begin(), text.end(), code_regex);
    
    for (; code_iter != end; ++code_iter) {
        const std::smatch& match = *code_iter;
        GtkTextIter format_start, format_end;
        
        gtk_text_buffer_get_iter_at_offset(buffer, &format_start, 
                                          gtk_text_iter_get_offset(&start) + match.position());
        gtk_text_buffer_get_iter_at_offset(buffer, &format_end, 
                                          gtk_text_iter_get_offset(&start) + match.position() + match.length());
        
        // Replace `text` with text and apply code formatting
        gtk_text_buffer_delete(buffer, &format_start, &format_end);
        gtk_text_buffer_insert(buffer, &format_start, match[1].str().c_str(), -1);
        
        GtkTextIter code_end;
        gtk_text_buffer_get_iter_at_offset(buffer, &code_end, 
                                          gtk_text_iter_get_offset(&format_start) + match[1].length());
        gtk_text_buffer_apply_tag(buffer, code_tag, &format_start, &code_end);
    }
}

GtkWidget* MarkdownUtils::create_code_block_widget(const CodeBlock& block, GCallback run_callback, gpointer user_data) {
    GtkWidget* frame = gtk_frame_new(nullptr);
    gtk_widget_add_css_class(frame, "code-block-frame");
    
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_frame_set_child(GTK_FRAME(frame), vbox);
    
    // Header with language and run button
    GtkWidget* header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(header, "code-block-header");
    
    // Language label
    std::string lang_text = block.language.empty() ? "code" : block.language;
    GtkWidget* lang_label = gtk_label_new(lang_text.c_str());
    gtk_widget_add_css_class(lang_label, "code-language");
    gtk_box_append(GTK_BOX(header), lang_label);
    
    // Spacer
    GtkWidget* spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(header), spacer);
    
    // Run button (only for bash, sh, shell, terminal)
    if (block.language == "bash" || block.language == "sh" || 
        block.language == "shell" || block.language == "terminal") {
        
        GtkWidget* run_button = gtk_button_new_from_icon_name("media-playback-start-symbolic");
        gtk_widget_add_css_class(run_button, "run-button");
        gtk_widget_set_tooltip_text(run_button, "Run in Terminal");
        
        // Store the code in the button's data
        g_object_set_data_full(G_OBJECT(run_button), "code", 
                              g_strdup(block.code.c_str()), g_free);
        
        g_signal_connect(run_button, "clicked", run_callback, user_data);
        gtk_box_append(GTK_BOX(header), run_button);
    }
    
    gtk_box_append(GTK_BOX(vbox), header);
    
    // Code text view
    GtkWidget* code_scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(code_scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(code_scrolled, -1, 150);
    
    GtkWidget* code_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(code_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(code_view), TRUE);
    gtk_widget_add_css_class(code_view, "code-view");
    
    GtkTextBuffer* code_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(code_view));
    gtk_text_buffer_set_text(code_buffer, block.code.c_str(), -1);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(code_scrolled), code_view);
    gtk_box_append(GTK_BOX(vbox), code_scrolled);
    
    return frame;
}

void MarkdownUtils::apply_bold_formatting(GtkTextBuffer* buffer, const std::string& text) {
    // Simple bold formatting (**text**)
    std::regex bold_regex(R"(\*\*(.*?)\*\*)");
    std::sregex_iterator iter(text.begin(), text.end(), bold_regex);
    std::sregex_iterator end;
    
    GtkTextTagTable* tag_table = gtk_text_buffer_get_tag_table(buffer);
    GtkTextTag* bold_tag = gtk_text_tag_new("bold");
    g_object_set(bold_tag, "weight", PANGO_WEIGHT_BOLD, nullptr);
    gtk_text_tag_table_add(tag_table, bold_tag);
    
    for (; iter != end; ++iter) {
        const std::smatch& match = *iter;
        // Apply bold tag (this is simplified - in practice you'd need to track positions)
    }
}

void MarkdownUtils::apply_italic_formatting(GtkTextBuffer* buffer, const std::string& text) {
    // Simple italic formatting (*text*)
    GtkTextTagTable* tag_table = gtk_text_buffer_get_tag_table(buffer);
    GtkTextTag* italic_tag = gtk_text_tag_new("italic");
    g_object_set(italic_tag, "style", PANGO_STYLE_ITALIC, nullptr);
    gtk_text_tag_table_add(tag_table, italic_tag);
}

void MarkdownUtils::apply_code_formatting(GtkTextBuffer* buffer, const std::string& text) {
    // Inline code formatting (`code`)
    GtkTextTagTable* tag_table = gtk_text_buffer_get_tag_table(buffer);
    GtkTextTag* code_tag = gtk_text_tag_new("inline-code");
    g_object_set(code_tag, "family", "monospace", 
                "background", "#f0f0f0", nullptr);
    gtk_text_tag_table_add(tag_table, code_tag);
}

void MarkdownUtils::apply_heading_formatting(GtkTextBuffer* buffer, const std::string& text) {
    // Heading formatting (# ## ###)
    GtkTextTagTable* tag_table = gtk_text_buffer_get_tag_table(buffer);
    GtkTextTag* h1_tag = gtk_text_tag_new("h1");
    g_object_set(h1_tag, "scale", 1.5, "weight", PANGO_WEIGHT_BOLD, nullptr);
    gtk_text_tag_table_add(tag_table, h1_tag);
}

// Terminal Manager Implementation

TerminalManager& TerminalManager::get_instance() {
    static TerminalManager instance;
    return instance;
}

void TerminalManager::run_command(const std::string& command) {
    std::cout << "Executing command: " << command << std::endl;
    
    if (!terminal_active) {
        // First time - create a named terminal session
        terminal_id = "linux_ai_mode_" + std::to_string(std::time(nullptr));
        
        // Create the initial terminal with tmux or screen for session management
        std::string create_session = "gnome-terminal --title=\"Linux AI Mode Terminal\" -- bash -c '"
                                   "echo \"Linux AI Mode Terminal Session Started\"; "
                                   "echo \"Commands will be executed here.\"; "
                                   "echo \"---\"; "
                                   + command + "; "
                                   "echo \"---\"; "
                                   "echo \"Command completed. Terminal will remain open for further commands.\"; "
                                   "exec bash'";
        
        system(create_session.c_str());
        terminal_active = true;
        
        // Store the terminal process info for future use
        std::ofstream term_file("/tmp/linux_ai_mode_terminal.pid");
        term_file << terminal_id << std::endl;
        term_file.close();
        
    } else {
        // Try to reuse existing terminal by sending commands to it
        // This is a simplified approach - in a full implementation you'd use
        // proper IPC or terminal session management
        
        // For now, we'll create a script and open it in the existing terminal context
        std::string script_path = "/tmp/linux_ai_mode_command_" + std::to_string(std::time(nullptr)) + ".sh";
        std::ofstream script_file(script_path);
        script_file << "#!/bin/bash\n";
        script_file << "echo \"[$(date)] Executing: " << command << "\"\n";
        script_file << command << "\n";
        script_file << "echo \"[$(date)] Command completed\"\n";
        script_file << "echo \"Press Enter to continue...\"\n";
        script_file << "read\n";
        script_file.close();
        
        // Make script executable
        system(("chmod +x " + script_path).c_str());
        
        // Try to send to existing terminal or create new tab
        std::string send_command = "gnome-terminal --tab --title=\"AI Command\" -- bash " + script_path;
        system(send_command.c_str());
        
        // Clean up old script files
        system("find /tmp -name 'linux_ai_mode_command_*.sh' -mmin +60 -delete 2>/dev/null");
    }
}

void TerminalManager::ensure_terminal_open() {
    if (!terminal_active) {
        // Check if there's an existing session
        if (std::ifstream("/tmp/linux_ai_mode_terminal.pid")) {
            terminal_active = true;
        }
    }
}

bool TerminalManager::is_terminal_open() {
    ensure_terminal_open();
    return terminal_active;
}

void TerminalManager::open_new_terminal() {
    terminal_active = false;  // Force creation of new session
    run_command("echo 'New terminal session started'");
}
