#include "winutil/engine/common.hpp"
#include "winutil/engine/colors.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cuchar>
#include <cwchar>
#include <iostream>
#include <numeric>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace Winutil::engine {

void normalize_window_size_sequence(
    std::vector<unsigned> &sizes, unsigned max_size
) {
    // if some sizes is less than minimal required, enlarge it
    for (unsigned &size : sizes) size = std::max(WINDOW_MIN_SIZE, size);

    // calculate the whole size of sequence. Adjust it with windows_num - 1
    // chars for windows delimiters
    unsigned expected_size =
        std::accumulate(sizes.begin(), sizes.end(), sizes.size() - 1);

    // if expected size is less than required, just enlarging last window
    if (expected_size < max_size) sizes.back() += max_size - expected_size;

    // in case the size is more than required, recalculation is needed
    else if (expected_size > max_size) {
        // find out the number of excess chracters
        unsigned excess = expected_size - max_size;

        // start decreasing sizes from the last one
        for (auto &size : std::ranges::reverse_view(sizes)) {
            if (excess >= size - WINDOW_MIN_SIZE) {
                excess -= size - WINDOW_MIN_SIZE;
                size = WINDOW_MIN_SIZE;
            } else {
                size -= excess;
                break;
            }
        }
    }
}

std::wstring from_utf8(std::string str) {
    mbstate_t state{};
    char32_t c32;
    const char *ptr = str.c_str(), *end = str.c_str() + str.size() + 1;
    std::wstring res = L"";

    while (std::size_t rc = std::mbrtoc32(&c32, ptr, end - ptr, &state)) {
        if (rc == (size_t)-1) break;
        if (rc == (size_t)-2) break;
        if (rc == (size_t)-3) break;
        ptr += rc;
        res += c32;
    }
    return res;
}

bool parse_color(std::wstring_view color, std::vector<int> &attrs) {
    if (color.size() < 3 || color[0] != '\033' || color[1] != '['
        || color.back() != 'm') {
        return false;
    }
    attrs.clear();
    // clang-format off
    attrs.append_range(
        color.substr(2, color.size() - 3)
        | std::views::transform([](auto c) { return (char)c; })
        | std::views::split(';')
        | std::views::transform([](auto &&rng) {
              return std::stoi(std::string(rng.begin(), rng.end()));
          })
    );
    // clang-format on
    return true;
}

std::wstring build_color(const std::vector<int> &attrs) {
    std::wstringstream ss;
    ss << L"\033[";

    for (int attr : attrs | std::views::take(attrs.size() - 1)) {
        ss << std::to_wstring(attr) << ';';
    }

    ss << attrs.back() << L'm';

    return std::move(ss.str());
}

ColoredChar
invert_color(ColoredChar c, std::wstring_view ncl_inverse, bool force) {
    std::wstring_view color = c.get_color();
    ColoredChar res = c;

    if (res.get_color().empty()) {
        res.set_color(ncl_inverse);
        return res;
    } else if (res.get_color() == ncl_inverse) {
        res.set_color(COLOR_NONE);
        return res;
    } else if (res.get_color() == COLOR_NONE) {
        res.set_color(ncl_inverse);
        return res;
    }

    std::vector<int> attrs;
    if (!parse_color(color, attrs)) return res;

    for (int i = 0; i < attrs.size(); ++i) {
        if (30 <= attrs[i] && attrs[i] < 38) attrs[i] += 10;
        else if (90 <= attrs[i] && attrs[i] < 98) attrs[i] += 10;
        else if (40 <= attrs[i] && attrs[i] < 48) attrs[i] -= 10;
        else if (100 <= attrs[i] && attrs[i] < 108) attrs[i] -= 10;
        else if (attrs[i] == 38) {
            attrs[i] += 10;
            if (i == attrs.size() - 1) continue;
            if (attrs[i + 1] == 5) i += 2;
            else if (attrs[i + 1] == 2) i += 4;
        } else if (attrs[i] == 48) {
            attrs[i] -= 10;
            if (i == attrs.size() - 1) continue;
            if (attrs[i + 1] == 5) i += 2;
            else if (attrs[i + 1] == 2) i += 4;
        }
    }

    res.set_color(build_color(attrs));

    return res;
}

void invert_color(color_string_view str, std::wstring_view ncl_inverse) {
    for (auto &ch : str) ch.set(invert_color(ch, ncl_inverse));
}

}; // namespace Winutil::engine
