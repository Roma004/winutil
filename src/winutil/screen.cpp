#include "winutil/window.hpp"
#include "winutil/screen.hpp"
#include "winutil/engine/color-string.hpp"
#include "winutil/engine/draw-area.hpp"
#include "winutil/engine/strdiff.hpp"
#include <asm-generic/ioctls.h>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <ranges>
#include <sys/ioctl.h>
#include <unistd.h>

#define CURSOR_AT_START          L"\e[1;1f"
#define CURSOR_SAVE              L"\e[s"
#define CURSOR_RESTORE           L"\e[u"
#define CURSOR_HIDE              L"\e[?25l"
#define CURSOR_UNHIDE            L"\e[?25h"
#define ALTERNATE_SCREEN_ENABLE  L"\e[?1049h"
#define ALTERNATE_SCREEN_DISABLE L"\e[?1049l"

static std::wostream &out = std::wcout;
bool alternative_screen_enabled = false;

namespace Winutil {

Screen::Screen(unsigned width, unsigned height) :
    main_area(width, height), repl_area(width, height) {}

Screen::~Screen() {
    if (alternative_screen_enabled) {
        out << ALTERNATE_SCREEN_DISABLE;
        out.flush();
        alternative_screen_enabled = false;
    }
}

void Screen::destroy_handler(int signal) noexcept {
    if (alternative_screen_enabled) {
        out << ALTERNATE_SCREEN_DISABLE;
        out.flush();
        alternative_screen_enabled = false;
    }
    std::exit(0);
}

unsigned Screen::max_width() noexcept {
    winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

unsigned Screen::max_height() noexcept {
    winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

void Screen::clear() { _main->clear(); }

void Screen::update() {
    _main->update();
    std::wstring_view color = COLOR_NONE;
    unsigned line = 1;
    auto move_cursor_rigth = [this](unsigned ncols) {
        if (ncols != 0) { out << L"\e[" << ncols << L'C'; }
    };
    auto output = [this, &color](engine::color_string_view str) {
        for (auto c : str) {
            if (c.get_color() != color) {
                color = c.get_color();
                out << color;
            }
            out << c.ch;
        }
    };
    auto newline = [this, &color, &line]() {
        out << L"\e[" << ++line << ";1H" << (color = COLOR_NONE);
    };

    using namespace std::views;

    if (!alternative_screen_enabled) {
        out << ALTERNATE_SCREEN_ENABLE;
        alternative_screen_enabled = true;
    }

    out << CURSOR_HIDE CURSOR_AT_START;
    for (auto &&[repl_line, curr_line] : zip(repl_area, main_area)) {
        for (auto &&[skip, diff] :
             strdiff_view<ColoredChar>(repl_line, curr_line, 6)) {
            move_cursor_rigth(skip);
            output(diff);
        }
        newline();
    }
    out << CURSOR_UNHIDE;
    out.flush();
    repl_area = main_area.copy();
}

Window &Screen::get_window() {
    return *_main;
}

void Screen::resize(unsigned width, unsigned height) {
    main_area.resize(width, height);
    _main->move(main_area.make_area());
}

}; // namespace Winutil
