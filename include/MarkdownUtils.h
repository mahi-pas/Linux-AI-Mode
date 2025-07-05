#ifndef MARKDOWNUTILS_H
#define MARKDOWNUTILS_H

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <memory>

struct CodeBlock {
    std::string language;
    std::string code;
    size_t start_pos;
    size_t end_pos;
};

class MarkdownUtils {
public:
    static std::vector<CodeBlock> extract_code_blocks(const std::string& text);
    static GtkWidget* create_formatted_text_widget(const std::string& markdown_text);
    static void apply_markdown_formatting(GtkTextBuffer* buffer, const std::string& text);
    static GtkWidget* create_code_block_widget(const CodeBlock& block, GCallback run_callback, gpointer user_data);
    
private:
    static void apply_bold_formatting(GtkTextBuffer* buffer, const std::string& text);
    static void apply_italic_formatting(GtkTextBuffer* buffer, const std::string& text);
    static void apply_code_formatting(GtkTextBuffer* buffer, const std::string& text);
    static void apply_heading_formatting(GtkTextBuffer* buffer, const std::string& text);
    static void apply_inline_formatting(GtkTextBuffer* buffer, const std::string& text,
                                       GtkTextTag* bold_tag, GtkTextTag* italic_tag, GtkTextTag* code_tag);
};

class TerminalManager {
public:
    static TerminalManager& get_instance();
    
    void run_command(const std::string& command);
    void ensure_terminal_open();
    bool is_terminal_open();
    
private:
    TerminalManager() = default;
    std::string terminal_id;
    bool terminal_active = false;
    
    void open_new_terminal();
};

#endif // MARKDOWNUTILS_H
