#pragma once

#include "winutil/engine/colors.hpp"
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

struct ColoredChar {
    wchar_t ch;
    uint32_t color;
    static std::vector<std::wstring> colors;

    ColoredChar() = default;
    ColoredChar(wchar_t c, std::wstring_view color_string = COLOR_NONE);

    void set_char(
        char c, std::wstring_view color_string = COLOR_NONE
    ) const noexcept;
    void set_char(
        wchar_t c, std::wstring_view color_string = COLOR_NONE
    ) const noexcept;
    void set_color(std::wstring_view color_string) const noexcept;
    void set(const ColoredChar c) const noexcept;
    std::wstring_view get_color() const noexcept;

    bool operator==(const ColoredChar &other) const;
    bool operator<(const ColoredChar &other) const;
};
static_assert(sizeof(ColoredChar) == 8);
static_assert(std::is_trivially_default_constructible_v<ColoredChar>);

namespace std {
template <> struct char_traits<ColoredChar> {
    using char_type = ColoredChar;
    using int_type = uint32_t;
    using off_type = streamoff;
    using pos_type = streampos;
    using state_type = mbstate_t;

    static void assign(char_type &r, const char_type &c) noexcept;
    static char_type *assign(char_type *p, size_t n, char_type c);

    static bool eq(const char_type &a, const char_type &b) noexcept;
    static bool lt(const char_type &a, const char_type &b) noexcept;

    static int compare(const char_type *s1, const char_type *s2, size_t n);

    static size_t length(const char_type *s);
    static const char_type *
    find(const char_type *s, size_t n, const char_type &a);

    static char_type *move(char_type *s1, const char_type *s2, size_t n);
    static char_type *copy(char_type *s1, const char_type *s2, size_t n);

    static int_type eof() noexcept;
    static int_type not_eof(const int_type &i) noexcept;

    static char_type to_char_type(const int_type &i) noexcept;
    static int_type to_int_type(const char_type &c) noexcept;
    static bool eq_int_type(const int_type &a, const int_type &b) noexcept;
};

} // namespace std

namespace Winutil::engine {

using color_string = std::basic_string<ColoredChar>;
using color_string_view = std::basic_string_view<ColoredChar>;

color_string to_color_string(std::string_view);

}; // namespace Winutil::engine
