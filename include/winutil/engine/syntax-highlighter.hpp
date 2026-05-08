#pragma once

#include "winutil/engine/color-string.hpp"
#include <ranges>
#include <regex>
#include <string>
#include <vector>

namespace Winutil::engine {

class Pattern {
  public:
    static std::wstring word(const std::wstring &pattern) {
        return L"\\b(" + pattern + L")\\b";
    }

    static std::wstring prefix(const std::wstring &pattern) {
        return L"\\b(" + pattern + L")";
    }

    static std::wstring suffix(const std::wstring &pattern) {
        return L"(" + pattern + L")\\b";
    }

    static std::wstring wild(const std::wstring &pattern) {
        return L"(" + pattern + L")";
    }
};

class SyntaxHighlighter {
  public:
    struct pattern_config {
        std::wstring pattern;
        std::wstring color;
    };

    template <class R>
        requires(
            std::ranges::input_range<R>
            && std::is_same_v<std::ranges::range_value_t<R>, pattern_config>
        )
    explicit SyntaxHighlighter(R &&configs, std::wstring default_color) {
        std::wstring regex_string = L"";

        _default_color =
            std::wstring(default_color.begin(), default_color.end());

        bool first = true;
        for (auto &conf : configs) {
            if (!first) regex_string += L"|";
            first = false;

            regex_string += conf.pattern;

            _colors.push_back(conf.color);
        }

        _regex = std::wregex(
            regex_string, std::wregex::ECMAScript | std::wregex::optimize
        );
    }

    SyntaxHighlighter(
        std::initializer_list<pattern_config> patterns,
        std::wstring defult_color
    ) : SyntaxHighlighter(std::views::all(patterns), defult_color) {}

    color_string highlight_line(const std::wstring &line) const;

  private:
    std::vector<std::wstring> _colors;
    std::wstring _default_color;
    std::wregex _regex;
};

}; // namespace Winutil::engine
