cmake_minimum_required(VERSION 3.16)
project(LinuxAIMode VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(LIBCURL REQUIRED libcurl)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Create executable
add_executable(linux-ai-mode
    src/main.cpp
    src/AIChat.cpp
    src/LLMProvider.cpp
    src/ChatHistory.cpp
    src/Utils.cpp
    src/MarkdownUtils.cpp
)

# Include directories
target_include_directories(linux-ai-mode PRIVATE
    ${GTK4_INCLUDE_DIRS}
    ${LIBCURL_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    include
)

# Link libraries
target_link_libraries(linux-ai-mode
    ${GTK4_LIBRARIES}
    ${LIBCURL_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

# Compiler flags
target_compile_options(linux-ai-mode PRIVATE
    ${GTK4_CFLAGS_OTHER}
    ${LIBCURL_CFLAGS_OTHER}
    ${JSONCPP_CFLAGS_OTHER}
)

# Install configuration
install(TARGETS linux-ai-mode DESTINATION bin)
install(FILES linux-ai-mode.desktop DESTINATION share/applications)
install(FILES css/style.css DESTINATION share/linux-ai-mode)
