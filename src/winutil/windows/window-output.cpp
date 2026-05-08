#include "winutil/windows/window-output.hpp"
#include "winutil/engine/common.hpp"
#include "winutil/engine/draw-area.hpp"
#include <ranges>
#include <stdexcept>
#include <string_view>

namespace Winutil {

void WindowOutput::write_line(std::wstring_view str) {
    for (wchar_t chr : str) write_char(chr);
}

void WindowOutput::write_char(wchar_t chr) {
    auto desc = area.get_info();
    if (_cursor.row >= desc.height || _cursor.col >= desc.width) return;
    area.get_line(_cursor.row)[_cursor.col].set(chr);
    _cursor.col += 1;
}

void WindowOutput::clear() {
    BaseWindow::clear();
    _cursor = {};
}

void WindowOutput::insert_color(std::wstring_view color) {
    auto desc = area.get_info();
    if (_cursor.row >= desc.height || _cursor.col >= desc.width) return;
    area.get_line(_cursor.row)[_cursor.col].set_color(color);
}

void WindowOutput::newline() {
    auto desc = area.get_info();
    _cursor.col = 0;
    if (_cursor.row == desc.height - 1) {
        for (int i = 1; i < desc.height; ++i) {
            for (auto &&[c1, c2] :
                 std::views::zip(area.get_line(i - 1), area.get_line(i))) {
                c1.set(c2);
            }
        }
        for (auto &c : area.get_line(desc.height - 1)) {
            c.set(WINUTIL_EMPTY_CHAR);
        }
    } else {
        _cursor.row += 1;
    }
}

void WindowOutput::write(std::wstring_view str) {
    auto desc = area.get_info();
    size_t escape_pos = 0;
    size_t npos = std::wstring_view::npos;
    size_t m_pos;
    if (_cursor.row >= desc.height) return;
    while ((escape_pos = str.find_first_of(L"\n\t\e\r")) != npos) {
        write_line(str.substr(0, escape_pos));
        str = str.substr(escape_pos);
        switch (str[0]) {
        case L'\n': newline(); break;
        case L'\033':
            m_pos = str.find(L'm');
            if (m_pos == std::string_view::npos)
                throw std::runtime_error("Invalid color escape!");
            insert_color(str.substr(0, m_pos + 1));
            str = str.substr(m_pos);
            break;
        case L'\r': _cursor.col = 0; break;
        case L'\t': do { write_char(WINUTIL_EMPTY_CHAR);
            } while (_cursor.col % WINUTIL_TAB_SIZE != 0);
            break;
        }
        str = str.substr(1);
    }
    write_line(str);
}

void WindowOutput::set_cursor(engine::WindowPos pos) { _cursor = pos; }

engine::WindowPos WindowOutput::get_cursor() { return _cursor; };

}; // namespace Winutil
