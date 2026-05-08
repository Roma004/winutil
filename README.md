# Winutil - Terminal Window Management Library

A C++23 library for creating pseudo-graphic terminal applications with window
management, colors, and real-time rendering.

## Features

- Full-screen terminal window management
- Nested window hierarchies (rows and columns)
- Custom window implementations
- Color support (predefined and RGB)
- Syntax highlighting for file views
- Efficient incremental rendering (only changed characters are updated)
- Signal handling for clean shutdown

## Table of Contents

- [Quick Start](#quick-start)
- [Installation](#installation)
  - [Build Requirements](#build-requirements)
  - [As Subdirectory](#as-subdirectory)
  - [System Installation](#system-installation)
  - [FetchContent](#fetchcontent-cmake-311)
- [Core Components](#core-components)
  - [Screen](#screen)
  - [Window Containers](#window-containers)
    - [WindowsRow](#windowsrow)
    - [WindowsColumn](#windowscolumn)
  - [Standard Windows](#standard-windows)
    - [WindowOutput](#windowoutput)
    - [WindowFileView](#windowfileview)
- [Creating Custom Windows](#creating-custom-windows)
  - [Understanding DrawArea](#understanding-drawarea)
  - [ColoredChar and Performance](#performance-considerations-for-coloredchar)
- [Rendering Pipeline](#rendering-pipeline)
- [Color System](#color-macros-colorshpp)
- [Syntax Highlighting](#syntaxhighlighter---advanced-pattern-matching)
- [Signal Handling](#signal-handling)

## Quick Start

Here's a minimal example demonstrating how to create a screen with a row
containing two output windows:
```cpp
#include <winutil/screen.hpp>
#include <winutil/windows/windows-row.hpp>
#include <winutil/windows/window-output.hpp>

int main() {
    // Create a screen (reserving 30 lines at bottom for other content)
    Winutil::Screen screen(
        Winutil::Screen::max_width(),
        Winutil::Screen::max_height() - 30
    );

    // Create a row that will hold multiple windows
    auto &main_row = screen.make_window<Winutil::WindowsRow>();
    
    // Create two output windows side by side
    auto &output1 = main_row.make_window<Winutil::WindowOutput>();
    auto &output2 = main_row.make_window<Winutil::WindowOutput>();

    // Write to the first window
    output1.write(L"Hello from window 1!");
    output1.write(L"\nSecond line");
    
    // Write to the second window
    output2.write(L"Window 2 content");
    
    // Render everything to screen
    screen.update();
    
    // Wait for user input...
}
```

## Installation 

### Build Requirements

* C++23 compatible compiler
* POSIX system (uses termios, ioctl)
* Dependencies (all headers-only):
  * `<regex>` for syntax highlighting
  * <ranges> for C++20 ranges

### As Subdirectory

Clone or add this repository as a subdirectory in your project:
```bash
# Clone directly into your project
git clone https://github.com/Roma004/winutil.git third_party/winutil

# Or add as git submodule
git submodule add https://github.com/Roma004/winutil.git third_party/winutil
``` 

Then configure your CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyProject)

# Add winutil as a subdirectory
add_subdirectory(third_party/winutil)

# Include winutil headers
include_directories(third_party/winutil/include)

# Create your executable
add_executable(my_app main.cpp)

# Link against winutil
target_link_libraries(my_app PRIVATE winutil)
```

### System Installation

If you wath to install this lib globaly in your system:
```bash
# Clone the repository
git clone https://github.com/Roma004/winutil.git
cd winutil

# Build and install
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make install
```

Then in your project:
```cmake
find_package(winutil REQUIRED)
target_link_libraries(my_app PRIVATE winutil::winutil)
```

### FetchContent (CMake 3.11+)

```cmake
include(FetchContent)

FetchContent_Declare(
    winutil
    GIT_REPOSITORY https://github.com/Roma004/winutil.git
    GIT_TAG main
)

FetchContent_MakeAvailable(winutil)

target_include_directories(myapp PRIVATE ${winutil_SOURCE_DIR}/include)

target_link_libraries(my_app PRIVATE winutil)
```

## Core Components

### Screen

The Screen class represents the entire terminal display. Only one screen can
exist at a time.
```cpp
Winutil::Screen screen(width, height);
```

Key methods:
* `update()` - Renders all changes to the terminal
* `resize(width, height)` - Resizes the screen and all child windows
* `make_window<T>()` - Creates the root window (must be called once)
* `max_width()/max_height()` - Get terminal dimensions (static)

### Window Containers

#### WindowsRow

Arranges child windows horizontally. New windows are added to the right.

```cpp
auto &row = screen.make_window<Winutil::WindowsRow>();
auto &window1 = row.make_window<MyWindow>();
auto &window2 = row.make_window<MyWindow>();
row.resize_windows({30, 70}); // Set custom widths (in percent or absolute)
```

#### WindowsColumn

Arranges child windows vertically. New windows are added below.

```cpp
auto &col = screen.make_window<Winutil::WindowsColumn>();
auto &window1 = col.make_window<MyWindow>();
auto &window2 = col.make_window<MyWindow>();
col.resize_windows({40, 60}); // Set custom heights
```

### Standard Windows

#### WindowOutput

A text output window supporting colored text, cursor positioning, and special
characters.
```cpp
Winutil::WindowOutput out(std::move(area));

// Write operations
out.write(L"Hello World");
out << L"Stream syntax works too" << L"\n";

// Cursor control
out.set_cursor({0, 0});        // Move to line 0, column 0
auto pos = out.get_cursor();   // Get current position

// Colors (using macros from colors.hpp)
out.write(L"\033[31mRed text\033[0m");
out.write(COLOR(L"1", L"4"));  // Red on blue background

// Clear the window
out.clear();
```

Supported escape sequences:
* `\n` - Newline (scrolls if at bottom)
* `\r` - Carriage return
* `\t` - Tab (advances to next tab stop)
* `\033[Xm` - ANSI color codes

#### WindowFileView

A file viewer with line numbers and optional syntax highlighting.

```cpp
Winutil::WindowFileView viewer(std::move(area));

// Configuration
viewer.config.line_numbers = true;
viewer.config.syntax_highlight = true;

// Open a file
viewer.open("path/to/file.txt");

// Navigation
viewer.scroll(10);                    // Scroll down 10 lines
viewer.scroll_to(100, true);          // Scroll to line 100, center it
viewer.scroll_horizontal(5);          // Scroll right 5 columns

// Selection
viewer.select({10, 5}, {15, 20});    // Select lines 10-15, chars 5-20
viewer.clear_selection();

// Syntax highlighting
Winutil::engine::SyntaxHighlighter highlighter(
    {
        {L"int|float|double", COLOR_FG(BLUE)},
        {L"if|else|while|for", COLOR_FG(PURPLE)},
        {L"//.*$", COLOR_FG(GREEN)}
    },
    COLOR_NONE
);
viewer.set_highlighter(highlighter);
```

## Creating Custom Windows

To create your own window, inherit from `BaseWindow` and use the
`WINDOW_CONSTRUCTOR` macro:
```cpp
class MyWindow : public Winutil::BaseWindow {
public:
    WINDOW_CONSTRUCTOR(MyWindow)  // Generates constructor and move/copy
    
    void update() override {
        // This is called during screen.update()
        // You can modify the draw area here
        modify_draw_area();
    }
    
    void clear() override {
        BaseWindow::clear();  // Clear all characters
        // Additional cleanup...
    }
    
    void move(Winutil::engine::DrawArea &&new_area) override {
        BaseWindow::move(std::move(new_area));
        // Handle resize/reposition
    }
    
private:
    void modify_draw_area() {
        auto info = get_size();  // Get {width, height}
        
        // Access and modify draw area
        auto line = area.get_line(0);  // Get first line
        line[0].set_char(L'X');        // Set character at column 0
        line[0].set_color(COLOR_FG(RED)); // Set color
        
        // Or iterate over entire area
        for (auto &line : area) {
            for (auto &ch : line) {
                ch.set_char(L'*');
                ch.set_color(COLOR(BLUE, WHITE));
            }
        }
    }
};
```

### Understanding DrawArea

The `DrawArea` class represents a window's drawing buffer:

```cpp
Winutil::engine::DrawArea area;

// Get information
WindowDesc info = area.get_info();  // {width, height}
unsigned width = info.width;
unsigned height = info.height;

// Access lines
auto line = area.get_line(row);  // Returns color_string_view

// DrawArea is iterable
for (auto &line : area) {
    for (auto &character : line) {
        // character is ColoredChar
    }
}

// Create subareas (for complex layouts)
auto sub = area.subarea({start_row, start_col}, width, height);
```

### ColoredChar Structure

Each character in the buffer has:
* `ch` - The character (`wchar_t`)
* `color` - index of the color in color-lookup table

```cpp
ColoredChar c;
c.set_char(L'A');
c.set_color(COLOR_FG(RED));
c.set_color(L"\033[38;2;255;128;0m");  // RGB color
```

#### Performance Considerations for ColoredChar

The `ColoredChar` structure stores colors as an index into a global color table.
Each time you set a color that hasn't been used before, it's added to this
table:
```cpp
ColoredChar c;
c.set_color(COLOR_RGB(255, 128, 0));  // Adds new color to table
c.set_color(COLOR_RGB(255, 128, 0));  // Reuses existing index
```

Important performance implications:
* Every unique color string adds an entry to the global color table
* The table grows throughout the program's lifetime
* Color lookups become slower as the table grows (linear search)
* For optimal performance, limit the total number of unique colors in your
  application

The color table is shared across all windows, so excessive colors in one window
affect rendering performance globally.

### Rendering Pipeline

1. Windows modify their DrawArea buffers (can happen anytime)
2. Screen::update() is called
3. The screen compares current buffer with previous buffer
4. Only changed characters are sent to terminal
5. Terminal displays the updated content

Important: Always call screen.update() after making changes you want to display.

### Color Macros (colors.hpp)

```cpp
// Predefined colors
BLACK, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE

// Composite colors
COLOR(fg, bg)           // e.g., COLOR(RED, BLUE)
COLOR_FG(color)         // Foreground only
COLOR_BG(color)         // Background only
COLOR_NONE              // Reset to defaults

// RGB colors (24-bit)
COLOR_RGB(r, g, b)     // e.g., COLOR_RGB(255, 128, 0)
```

### SyntaxHighlighter - Advanced Pattern Matching

The `SyntaxHighlighter` uses regex patterns to colorize text with support for
word boundaries and custom matching strategies.

Pattern Helpers:
```cpp
using namespace Winutil::engine;

// Word matching (exact whole words)
Pattern::word(L"int|float|double")  // Matches "int" but not "print"

// Prefix matching (word starts with pattern)
Pattern::prefix(L"std::")  // Matches "std::cout", "std::vector"

// Suffix matching (word ends with pattern)  
Pattern::suffix(L"_t")  // Matches "size_t", "uint32_t"

// Wildcard matching (any occurrence)
Pattern::wild(L"[0-9]+")  // Matches any numbers
```

Complete Example:
```cpp
// Define syntax for C++ with multiple pattern types
Winutil::engine::SyntaxHighlighter cpp_highlighter(
    {
        // Keywords (whole words)
        {Pattern::word(L"auto|break|case|catch|class|const|continue|default|delete|do|else|enum|explicit|export|extern|false|final|for|friend|goto|if|inline|mutable|namespace|new|noexcept|nullptr|operator|override|private|protected|public|register|reinterpret_cast|return|sizeof|static|static_assert|struct|switch|template|this|throw|true|try|typedef|typeid|typename|union|using|virtual|void|volatile|while"),
         COLOR_FG(BLUE)},

        // Types (highlight differently)
        {Pattern::word(L"int|char|short|long|float|double|bool|void|size_t|uint32_t|int32_t|uint64_t"),
         COLOR_FG(PURPLE)},

        // String literals
        {Pattern::wild(L"\"[^\"]*\""), COLOR_FG(GREEN)},

        // Single-line comments
        {Pattern::wild(L"//[^\n]*"), COLOR_FG(CYAN)},

        // Numbers
        {Pattern::wild(L"[0-9]+"), COLOR_FG(YELLOW)},

        // Preprocessor directives (prefix matching)
        {Pattern::prefix(L"#include|#define|#ifdef|#ifndef|#endif|#pragma"),
         COLOR_FG(RED)}
    },
    COLOR_NONE  // Default color for unmatched text
);

// Apply to file viewer
viewer.set_highlighter(cpp_highlighter);
viewer.open("main.cpp");
```

How It Works:
1. Pattern Compilation: All patterns are combined into a single regex for O(n)
   matching
2. Group Capture: Each pattern corresponds to a capture group
3. Color Assignment: When text matches, the corresponding color is applied
4. Default Fallback: Non-matching text gets the default color

Note: The highlighter processes each line independently. For multi-line comments
or strings, you'll need to implement state tracking in a custom window.

### Signal Handling

The library provides a static signal handler for clean shutdown:

```cpp
std::signal(SIGINT, Winutil::Screen::destroy_handler);
std::signal(SIGTERM, Winutil::Screen::destroy_handler);
```

This ensures the alternate screen buffer is properly restored on exit.

