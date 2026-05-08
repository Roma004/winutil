#pragma once

#include "winutil/engine/color-string.hpp"
#include "winutil/engine/colors.hpp"
#include <string_view>
#include <vector>

#ifndef WINUTIL_COLUMN_DELIMITER
#define WINUTIL_COLUMN_DELIMITER L'-'
#endif

#ifndef WINUTIL_ROW_DELIMITER
#define WINUTIL_ROW_DELIMITER L'|'
#endif

#ifndef WINUTIL_EMPTY_CHAR
#define WINUTIL_EMPTY_CHAR L' '
#endif

#ifndef WINUTIL_TAB_SIZE
#define WINUTIL_TAB_SIZE 8
#endif

namespace Winutil::engine {

struct WindowPos {
    unsigned row;
    unsigned col;
};

struct WindowDesc {
    unsigned width;
    unsigned height;
};

#define WINDOW_MIN_SIZE 3u
#define WINDOW_MAX_SIZE ((unsigned)-1)

void normalize_window_size_sequence(
    std::vector<unsigned> &sizes, unsigned max_size
);

std::wstring from_utf8(std::string);

ColoredChar invert_color(
    ColoredChar c,
    std::wstring_view ncl_inverse = COLOR(BLACK, WHITE),
    bool force = false
);

void invert_color(
    color_string_view str, std::wstring_view ncl_inverse = COLOR(BLACK, WHITE)
);

}; // namespace Winutil::engine
