#include "winutil/engine/color-string.hpp"
#include <algorithm>
#include <string>

std::vector<std::wstring> ColoredChar::colors(1, L"");

ColoredChar::ColoredChar(wchar_t c, std::wstring_view color_str) : ch(c) {
    set_color(color_str);
}

std::wstring_view ColoredChar::get_color() const noexcept {
    return colors[color];
}

void ColoredChar::set_char(
    char c, std::wstring_view color_string
) const noexcept {
    const_cast<ColoredChar *>(this)->ch = c;
    set_color(color_string);
}

void ColoredChar::set_char(
    wchar_t c, std::wstring_view color_string
) const noexcept {
    const_cast<ColoredChar *>(this)->ch = c;
    set_color(color_string);
}

void ColoredChar::set_color(std::wstring_view color_string) const noexcept {
    auto color_pos = std::ranges::find(colors, color_string);
    if (color_pos == colors.end()) {
        colors.emplace_back(color_string);
        color_pos = colors.begin() + (colors.size() - 1);
    }
    const_cast<ColoredChar *>(this)->color = color_pos - colors.begin();
}

void ColoredChar::set(const ColoredChar c) const noexcept {
    ColoredChar *self = const_cast<ColoredChar *>(this);
    self->color = c.color;
    self->ch = c.ch;
}

bool ColoredChar::operator==(const ColoredChar &other) const {
    return ch == other.ch && color == other.color;
}
bool ColoredChar::operator<(const ColoredChar &other) const {
    return ch < other.ch && color < other.color;
}

namespace std {
using ColCharTraits = char_traits<ColoredChar>;

void ColCharTraits::assign(char_type &r, const char_type &c) noexcept { r = c; }

ColCharTraits::char_type *
ColCharTraits::assign(char_type *p, size_t n, char_type c) {
    for (size_t i = 0; i < n; ++i) p[i] = c;
    return p;
}

bool ColCharTraits::eq(const char_type &a, const char_type &b) noexcept {
    return a == b;
}
bool ColCharTraits::lt(const char_type &a, const char_type &b) noexcept {
    return a < b;
}

int ColCharTraits::compare(const char_type *s1, const char_type *s2, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (lt(s1[i], s2[i])) return -1;
        if (lt(s2[i], s1[i])) return 1;
    }
    return 0;
}

size_t ColCharTraits::length(const char_type *s) {
    size_t len = 0;
    while (!eq(s[len], char_type())) ++len;
    return len;
}

const ColCharTraits::char_type *
ColCharTraits::find(const char_type *s, size_t n, const char_type &a) {
    for (size_t i = 0; i < n; ++i) {
        if (eq(s[i], a)) return s + i;
    }
    return nullptr;
}

ColCharTraits::char_type *
ColCharTraits::move(char_type *s1, const char_type *s2, size_t n) {
    if (n == 0) return s1;
    return static_cast<char_type *>(memmove(s1, s2, n * sizeof(char_type)));
}

ColCharTraits::char_type *
ColCharTraits::copy(char_type *s1, const char_type *s2, size_t n) {
    if (n == 0) return s1;
    return static_cast<char_type *>(memcpy(s1, s2, n * sizeof(char_type)));
}

ColCharTraits::int_type ColCharTraits::eof() noexcept {
    return static_cast<int_type>(-1);
}
ColCharTraits::int_type ColCharTraits::not_eof(const int_type &i) noexcept {
    return i != eof() ? i : 0;
}

ColCharTraits::char_type
ColCharTraits::to_char_type(const int_type &i) noexcept {
    return char_type(static_cast<wchar_t>(i));
}

ColCharTraits::int_type
ColCharTraits::to_int_type(const char_type &c) noexcept {
    return (static_cast<int_type>(c.color));
}

bool ColCharTraits::eq_int_type(const int_type &a, const int_type &b) noexcept {
    return a == b;
}

}; // namespace std

namespace Winutil::engine {

color_string to_color_string(std::string_view str) {
    return color_string(str.begin(), str.end());
}

}; // namespace Winutil::engine

