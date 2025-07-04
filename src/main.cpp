#include "AIChat.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        AIChat app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
