
#include "winutil/engine/syntax-highlighter.hpp"

#include <regex>
#include <vector>

#include "winutil/engine/color-string.hpp"

namespace Winutil::engine {

color_string SyntaxHighlighter::highlight_line(const std::wstring &line) const {
    color_string res = color_string(line.begin(), line.end());

    if (res.empty()) return res;

    std::wsregex_iterator it(line.begin(), line.end(), _regex);
    std::wsregex_iterator end;

    if (it == end) {
        for (auto &c : res) c.set_color(_default_color);
        return res;
    }

    std::wsmatch match = *it;

    for (size_t cur_pos = 0; cur_pos < line.size(); ++cur_pos) {
        if (it == end || cur_pos < match.position()) {
            res[cur_pos].set_color(_default_color);
            continue;
        }

        int group_no = 1;
        for (; group_no < match.size(); ++group_no) {
            if (match[group_no].matched && (group_no - 1) < _colors.size()) {
                break;
            }
        }

        std::wstring_view color = _colors[group_no - 1];
        for (int i = 0; i < match.length(); ++i)
            res[cur_pos + i].set_color(color);
        cur_pos += match.length() - 1;
        ++it;
        if (it != end) match = *it;
    }

    return res;
}

};  // namespace Winutil::engine
