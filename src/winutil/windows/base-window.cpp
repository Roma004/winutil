#include "winutil/windows/base-window.hpp"
#include "winutil/engine/color-string.hpp"
#include "winutil/engine/draw-area.hpp"
#include <algorithm>

namespace Winutil {

BaseWindow::BaseWindow(engine::DrawArea &&area) : area(std::move(area)) {}

void BaseWindow::clear() {
    for (engine::color_string_view line : area) {
        std::ranges::for_each(line, [](auto &c) {
            c.set_char(WINUTIL_EMPTY_CHAR);
        });
    }
}

void BaseWindow::move(engine::DrawArea &&new_area) {
    area = std::move(new_area);
}

void BaseWindow::update() {}

}; // namespace Winutil

