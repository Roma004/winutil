#pragma once

#include <generator>
#include <string_view>
#include <utility>

/// Get a string-diff sequence of pairs (skip, diff_str)
///
/// Returns:
///    std::get<0> -- the number of charachers that are equal in both strings
///                   starting from the previous stop position
///    std::get<1> -- the substring of new_str which differs from current_str
///                   starting from the previous stop position plus std::get<0>
///
/// Example:
///    current_str: ababa__baba_
///    new_str:     cbaca_aabab_
///    result sequence: (0,"c"), (2,"c"), (2,"aabab"), (1,"")
///
/// this way you can make a change pattern as this:  c--c--aabab-
template <class CharT = char>
std::generator<std::pair<size_t, std::basic_string_view<CharT>>> strdiff_view(
    std::basic_string_view<CharT> current_str,
    std::basic_string_view<CharT> new_str
) noexcept {
    if (current_str.size() != new_str.size()) {
        co_yield std::make_pair(current_str.size(), new_str);
        co_return;
    }

    size_t skip_len = 0, diff_len = 0, pair_start = 0, idx = 0;
    bool skip_char = true;
    for (idx = 0; idx < current_str.size(); ++idx) {
        if (current_str[idx] == new_str[idx]) {
            if (skip_char) skip_len += 1;
            else {
                co_yield std::make_pair(
                    skip_len, new_str.substr(pair_start + skip_len, diff_len)
                );
                skip_len = 1;
                diff_len = 0;
                pair_start = idx;
                skip_char = true;
            }
        } else {
            if (skip_char) skip_char = false;
            diff_len += 1;
        }
    }
    co_yield std::make_pair(
        skip_len, new_str.substr(pair_start + skip_len, diff_len)
    );
}

template <class CharT = char>
std::generator<std::pair<size_t, std::basic_string_view<CharT>>> strdiff_view(
    std::basic_string_view<CharT> current_str,
    std::basic_string_view<CharT> new_str,
    size_t skip_at_least
) noexcept {
    auto gen = strdiff_view<CharT>(current_str, new_str);
    auto it = gen.begin();
    auto [curr_skip, curr_diff] = *it;

    size_t diff_from = 0;

    // if there are less than skip_at_least non-equal characters at the
    // beginning of the line, consider them as a part of the diff
    if (curr_skip < skip_at_least) {
        curr_diff = new_str.substr(diff_from, curr_skip + curr_diff.size());
        curr_skip = 0;
    }
    // otherwise move the diff index to the beginning of current diff
    else
        diff_from = curr_skip;

    while (++it != gen.end()) {
        // get the consequent pair -- it is the next pair.
        auto [skip, diff] = *it;

        // if the distance between current and next diffs is less then
        // skip_at_least, consider this distance as a diff already
        if (skip < skip_at_least) {
            curr_diff = new_str.substr(
                diff_from, curr_diff.size() + skip + diff.size()
            );
        }
        // otherwise return current diff and propogate current position
        else {
            co_yield std::make_pair(curr_skip, curr_diff);
            diff_from += curr_diff.size() + skip;
            curr_skip = skip;
            curr_diff = diff;
        }
    }

    // anyway the last pair is valid
    co_yield std::make_pair(curr_skip, curr_diff);
}
