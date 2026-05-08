#include "winutil/windows/windows-column.hpp"
#include <ranges>
#include <stdexcept>
#include <vector>

namespace Winutil {

void WindowsColumn::update() {
    for (auto &win : _rows) win->update();
}

void WindowsColumn::clear() {
    for (auto &row : _rows) row->clear();
}

BaseWindow &WindowsColumn::get_child(unsigned idx) {
    if (idx >= _rows.size())
        throw std::runtime_error("Invalid window id passed!");
    return *_rows[idx];
}

void WindowsColumn::move(engine::DrawArea &&new_area) {
    engine::WindowDesc old_desc = area.get_info();
    engine::WindowDesc new_desc = new_area.get_info();

    clear_rulers();

    area = std::move(new_area);

    std::vector<unsigned> sizes(_rows.size());
    for (auto &&[win, size] : std::ranges::zip_view(_rows, sizes)) {
        size = win->get_area().get_info().height;
    }

    if (old_desc.height != new_desc.height) {
        engine::normalize_window_size_sequence(sizes, new_desc.height);
    }

    place_windows(sizes);
}

void WindowsColumn::clear_rulers() {
    unsigned pos = 0;
    unsigned width = area.get_info().width;
    unsigned height = area.get_info().height;
    for (auto &win : _rows) {
        pos += win->get_area().get_info().height;

        if (pos >= height) break;

        for (auto &chr : area.get_line(pos)) {
            chr.set_char(WINUTIL_EMPTY_CHAR);
        }
        pos += 1;
    }
}

unsigned WindowsColumn::place_windows(unsigned size) {
    clear_rulers();
    unsigned pos = 0;
    for (int i = 0; i < _rows.size(); ++i) { pos = place_window(i, pos, size); }
    return pos;
}

unsigned WindowsColumn::place_windows(const std::vector<unsigned> &sizes) {
    clear_rulers();
    unsigned pos = 0;
    for (auto &&[id, size] : std::ranges::enumerate_view(sizes)) {
        pos = place_window(id, pos, size);
    }
    return pos;
}

unsigned
WindowsColumn::place_window(unsigned win_id, unsigned pos, unsigned size) {
    auto &win = _rows[win_id];
    unsigned ruller_pos = pos + size;
    unsigned width = area.get_info().width;
    unsigned height = area.get_info().height;
    win->move(area.subarea({pos, 0}, width, size));

    if (ruller_pos >= height) return height;

    area.get_line(ruller_pos)[0].set_color(COLOR_NONE);
    for (auto &chr : area.get_line(ruller_pos)) {
        chr.set_char(WINUTIL_COLUMN_DELIMITER);
    }

    return ruller_pos + 1;
}

}; // namespace Winutil
