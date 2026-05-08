#include "winutil/windows/window-file-view.hpp"
#include "winutil/engine/color-string.hpp"
#include "winutil/engine/common.hpp"
#include "winutil/engine/draw-area.hpp"
#include <algorithm>
#include <fstream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

namespace Winutil {

void WindowFileView::open(std::string_view filename) {
    std::ifstream fin(std::string(filename), std::ios::binary);
    std::string raw_line_buf;

    if (!fin.is_open() || fin.fail())
        throw std::runtime_error(
            std::format("No such file or directory: {}", filename)
        );

    while (std::getline(fin, raw_line_buf)) {
        std::wstring line_buf = engine::from_utf8(raw_line_buf);
        if (!_config.syntax_highlight) {
            _lines.emplace_back(line_buf.begin(), line_buf.end());
        } else {
            _lines.push_back(_highlighter->get().highlight_line(line_buf));
        }
    }

    auto max_line = std::to_string(_lines.size());
    if (_config.line_numbers) { _first_screen_col = max_line.size() + 1; }

    write_content();
}

void WindowFileView::scroll(signed nrows) noexcept {
    int new_first_line = _first_line + nrows;
    new_first_line = std::max(0, new_first_line);
    new_first_line = std::min((int)_lines.size() - 1, new_first_line);
    apply_selection();
    _first_line = new_first_line;
    write_content();
}

void WindowFileView::scroll_to(unsigned line_no, bool at_center) noexcept {
    if (line_no >= _lines.size()) line_no = _lines.size() - 1;
    if (at_center) {
        unsigned from_top = get_area().get_info().height / 2;
        if (from_top > line_no) line_no = 0;
        else line_no -= from_top;
    }
    apply_selection();
    _first_line = line_no;
    write_content();
}

void WindowFileView::scroll_horizontal(signed ncols) noexcept {
    int new_first_col = _first_col + ncols;
    new_first_col = std::max(0, new_first_col);
    apply_selection();
    _first_col = new_first_col;
    write_content();
}

void WindowFileView::write_content() noexcept {
    auto desc = get_area().get_info();

    if (_first_screen_col > desc.width) return;

    for (int line_no = 0; line_no < desc.height; ++line_no) {
        auto screen_line = get_area().get_line(line_no);
        if (line_no + _first_line >= _lines.size()) {
            for (auto &chr : screen_line) chr.set_char(WINUTIL_EMPTY_CHAR);
            continue;
        }

        auto file_line = _lines[line_no + _first_line];
        for (int i = 0; i < desc.width - _first_screen_col; ++i) {
            if (i + _first_col < file_line.size())
                screen_line[i + _first_screen_col].set(
                    file_line[i + _first_col]
                );
            else
                screen_line[i + _first_screen_col].set_char(WINUTIL_EMPTY_CHAR);
        }

        if (_config.line_numbers) {
            std::string line_num = std::to_string(line_no + _first_line + 1);
            for (auto &c : screen_line.substr(0, _first_screen_col)) {
                c.set_char(WINUTIL_EMPTY_CHAR);
            }
            for (auto &&[num_c, c] : std::views::zip(
                     line_num | std::views::reverse,
                     screen_line.substr(0, _first_screen_col - 1)
                         | std::views::reverse
                 )) {
                c.set_char(num_c);
            }
        }
    }
    apply_selection();
}

void WindowFileView::apply_selection() noexcept {
    auto desc = get_area().get_info();
    auto select_from = linepos2winpos(_select_from);
    auto select_to = linepos2winpos(_select_to);

    if (_first_screen_col > desc.width) return;

    auto apply_row_selection =
        [this](unsigned row, unsigned from, unsigned to) noexcept {
            if (from > to) {
                unsigned tmp = from;
                from = to;
                to = tmp;
            } else if (from == to) return;

            auto line = get_area().get_line(row);
            engine::invert_color(
                line.substr(_first_screen_col + from, to - from + 1)
            );
        };

    unsigned first_row = select_from.row;
    unsigned last_row = select_to.row;
    unsigned first_col = select_from.col;
    unsigned last_col = select_to.col;
    if (first_row == last_row) {
        if (first_col == last_col) return;
        apply_row_selection(first_row, first_col, last_col);
        return;
    } else if (first_row > last_row) {
        first_row = select_to.row;
        last_row = select_from.row;
        first_col = select_to.col;
        last_col = select_from.col;
    }
    apply_row_selection(first_row, first_col, desc.width - 1);
    for (int i = first_row + 1; i < last_row; ++i) {
        apply_row_selection(i, 0, desc.width - 1);
    }
    apply_row_selection(last_row, 0, last_col);
}

engine::WindowPos WindowFileView::linepos2winpos(LinePos pos) noexcept {
    if (pos.line_no > last_line_no()) pos.line_no = last_line_no();
    if (pos.line_no != 0) pos.line_no -= 1;

    signed screen_line = pos.line_no - _first_line;
    auto desc = get_area().get_info();
    if (screen_line < 0) return {0, 0};
    if (screen_line >= desc.height)
        return {.row = desc.height - 1, .col = desc.width - 1};

    signed screen_col;
    if (pos.char_no == -1) {
        if (_lines[pos.line_no].size() != 0)
            screen_col = _lines[pos.line_no].size() - _first_col - 1;
        else screen_col = 1;
    } else screen_col = pos.char_no - _first_col - 1;

    if (screen_col < 0) return {.row = (unsigned)screen_line, .col = 0};
    if (screen_col >= desc.width)
        return {.row = (unsigned)screen_line, .col = desc.width - 1};
    return {.row = (unsigned)screen_line, .col = (unsigned)screen_col};
}

void WindowFileView::select(LinePos from, LinePos to) noexcept {
    apply_selection();
    _select_from = from;
    _select_to = to;
    apply_selection();
}

void WindowFileView::clear_selection() noexcept {
    _select_from = _select_to = {0, 0};
    apply_selection();
}

void WindowFileView::set_highlighter(
    const engine::SyntaxHighlighter &highlighter
) noexcept {
    _highlighter = highlighter;
    _config.syntax_highlight = true;
}

void WindowFileView::move(engine::DrawArea &&new_area) {
    BaseWindow::move(std::move(new_area));
    write_content();
}

}; // namespace Winutil
