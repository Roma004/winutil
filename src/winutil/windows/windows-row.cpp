#include "winutil/windows/windows-row.hpp"
#include <ranges>
#include <vector>

namespace Winutil {

void WindowsRow::clear() {
    for (auto &col : _cols) col->clear();
}

void WindowsRow::update() {
    for (auto &win : _cols) win->update();
}

BaseWindow &WindowsRow::get_child(unsigned idx) {
    if (idx >= _cols.size())
        throw std::runtime_error("Invalid window id passed!");
    return *_cols[idx];
}

void WindowsRow::move(engine::DrawArea &&new_area) {
    engine::WindowDesc old_desc = area.get_info();
    engine::WindowDesc new_desc = new_area.get_info();

    clear_rulers();

    area = std::move(new_area);

    std::vector<unsigned> sizes(_cols.size());
    for (auto &&[win, size] : std::ranges::zip_view(_cols, sizes)) {
        size = win->get_area().get_info().width;
    }

    if (old_desc.width != new_desc.width) {
        engine::normalize_window_size_sequence(sizes, new_desc.width);
    }

    place_windows(sizes);
}

void WindowsRow::clear_rulers() {
    unsigned pos = 0;
    unsigned width = area.get_info().width;
    unsigned height = area.get_info().height;
    for (auto &win : _cols) {
        pos += win->get_area().get_info().width;

        if (pos >= width) break;

        for (int i = 0; i < height; ++i) {
            area.get_line(i)[pos].set_char(WINUTIL_EMPTY_CHAR);
        }
        pos += 1;
    }
}

unsigned WindowsRow::place_windows(unsigned size) {
    clear_rulers();
    unsigned pos = 0;
    for (int i = 0; i < _cols.size(); ++i) { pos = place_window(i, pos, size); }
    return pos;
}

unsigned WindowsRow::place_windows(const std::vector<unsigned> &sizes) {
    clear_rulers();
    unsigned pos = 0;
    for (auto &&[id, size] : std::ranges::enumerate_view(sizes)) {
        pos = place_window(id, pos, size);
    }
    return pos;
}

unsigned
WindowsRow::place_window(unsigned win_id, unsigned pos, unsigned size) {
    auto &win = _cols[win_id];
    unsigned ruller_pos = pos + size;
    unsigned height = area.get_info().height;
    unsigned width = area.get_info().width;
    win->move(area.subarea({0, pos}, size, height));

    if (ruller_pos >= width) return width;

    for (int i = 0; i < height; ++i) {
        area.get_line(i)[ruller_pos].set_char(WINUTIL_ROW_DELIMITER);
        area.get_line(i)[ruller_pos].set_color(COLOR_NONE);
    }

    return ruller_pos + 1;
}

}; // namespace Winutil
