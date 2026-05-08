#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <winutil/engine/strdiff.hpp>

using namespace std::literals;

// Helper function to collect generator results into a vector
template <typename CharT> auto collect_generator(auto &&gen) {
    std::vector<std::pair<size_t, std::basic_string_view<CharT>>> result;
    for (auto &&item : gen) { result.emplace_back(item.first, item.second); }
    return result;
}

TEST_CASE("strdiff_view basic functionality", "[strdiff]") {
    SECTION("Identical strings produce only empty diff at end") {
        auto gen = strdiff_view("hello"sv, "hello"sv);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 5);
        CHECK(result[0].second.empty());
    }

    SECTION("Completely different strings") {
        auto gen = strdiff_view("abc"sv, "xyz"sv);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 0);
        CHECK(result[0].second == "xyz"sv);
    }

    SECTION("Example from documentation") {
        auto gen = strdiff_view("ababa__baba_"sv, "cbaca_aabab_"sv);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 4);
        CHECK(result[0].first == 0);
        CHECK(result[0].second == "c"sv);
        CHECK(result[1].first == 2);
        CHECK(result[1].second == "c"sv);
        CHECK(result[2].first == 2);
        CHECK(result[2].second == "aabab"sv);
        CHECK(result[3].first == 1);
        CHECK(result[3].second == ""sv);
    }

    SECTION("Prefix match") {
        auto gen = strdiff_view("abcdef"sv, "abcxyz"sv);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 3);
        CHECK(result[0].second == "xyz"sv);
    }

    SECTION("Suffix match") {
        auto gen = strdiff_view("abcdef"sv, "xyzdef"sv);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 2);
        CHECK(result[0].first == 0);
        CHECK(result[0].second == "xyz"sv);
        CHECK(result[1].first == 3);
        CHECK(result[1].second == ""sv);
    }

    SECTION("Multiple alternating diffs") {
        auto gen = strdiff_view("axbyczdw"sv, "a1b2c3d4"sv);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 4);
        for (size_t i = 0; i < 4; ++i) {
            CHECK(result[i].first == 1);
            CHECK(result[i].second == std::to_string(i + 1));
        }
    }

    SECTION("Empty strings") {
        SECTION("Both empty") {
            auto gen = strdiff_view(""sv, ""sv);
            auto result = collect_generator<char>(gen);

            REQUIRE(result.size() == 1);
            CHECK(result[0].first == 0);
            CHECK(result[0].second == ""sv);
        }

        SECTION("Current empty, new non-empty") {
            auto gen = strdiff_view(""sv, "hello"sv);
            auto result = collect_generator<char>(gen);

            REQUIRE(result.size() == 1);
            CHECK(result[0].first == 0);
            CHECK(result[0].second == "hello"sv);
        }

        SECTION("Current non-empty, new empty") {
            auto gen = strdiff_view("hello"sv, ""sv);
            auto result = collect_generator<char>(gen);

            REQUIRE(result.size() == 1);
            CHECK(result[0].first == 5);
            CHECK(result[0].second == ""sv);
        }
    }

    SECTION("Unicode and wide characters") {
        auto gen = strdiff_view(L"héllö"sv, L"héllå"sv);
        auto result = collect_generator<wchar_t>(gen);

        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 4);
        CHECK(result[0].second == L"å"sv);
    }

    SECTION("Edge case: match at very end of string") {
        auto gen = strdiff_view("abcdef"sv, "wxyzef"sv);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 2);
        CHECK(result[0].first == 0);
        CHECK(result[0].second == "wxyz"sv);
        CHECK(result[1].first == 2);
        CHECK(result[1].second == ""sv);
    }

    SECTION("Edge case: match at very beginning of string") {
        auto gen = strdiff_view("efabcd"sv, "efwxyz"sv);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 2);
        CHECK(result[0].second == "wxyz"sv);
    }
}

TEST_CASE("strdiff_view with skip_at_least parameter", "[strdiff][threshold]") {
    SECTION("skip_at_least = 0 behaves like basic version") {
        auto gen1 = strdiff_view("ababa__baba_"sv, "cbaca_aabab_"sv);
        auto gen2 = strdiff_view("ababa__baba_"sv, "cbaca_aabab_"sv, 0);

        auto result1 = collect_generator<char>(gen1);
        auto result2 = collect_generator<char>(gen2);

        REQUIRE(result1 == result2);
    }

    SECTION("skip_at_least = 1 behaves like basic version") {
        auto gen1 = strdiff_view("hello"sv, "hxllo"sv);
        auto gen2 = strdiff_view("hello"sv, "hxllo"sv, 1);

        auto result1 = collect_generator<char>(gen1);
        auto result2 = collect_generator<char>(gen2);

        REQUIRE(result1 == result2);
    }

    SECTION("Small matches are merged into diffs") {
        // "a_b_c" vs "axbyc" - with threshold 2, the '_' matches are too short
        auto gen = strdiff_view("a_b_c"sv, "axbyc"sv, 2);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 0);
        CHECK(result[0].second == "axbyc"sv); // Whole string as one diff
    }

    SECTION("Large enough matches remain separate") {
        auto gen = strdiff_view("aa_bb_cc"sv, "aaxbbycc"sv, 2);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 3);
        CHECK(result[0].first == 2);
        CHECK(result[0].second == "x"sv);
        CHECK(result[1].first == 2);
        CHECK(result[1].second == "y"sv);
        CHECK(result[2].first == 2);
        CHECK(result[2].second == ""sv);
    }

    SECTION("Small beginning merged into the next match") {
        auto gen = strdiff_view("a_bb_cc"sv, "axbbycc"sv, 2);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 3);
        CHECK(result[0].first == 0);
        CHECK(result[0].second == "ax"sv);
        CHECK(result[1].first == 2);
        CHECK(result[1].second == "y"sv);
        CHECK(result[2].first == 2);
        CHECK(result[2].second == ""sv);
    }

    SECTION("Small endinging merged into the previous match") {
        auto gen = strdiff_view("aa_bb_c"sv, "aaxbbyc"sv, 2);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 2);
        CHECK(result[0].first == 2);
        CHECK(result[0].second == "x"sv);
        CHECK(result[1].first == 2);
        CHECK(result[1].second == "yc"sv);
    }

    SECTION("Threshold exactly equal to match length") {
        auto gen = strdiff_view("ab__cd"sv, "abxxcd"sv, 2);
        auto result = collect_generator<char>(gen);

        // Match "__" is length 2, which meets threshold of 2
        REQUIRE(result.size() == 2);
        CHECK(result[0].first == 2);
        CHECK(result[0].second == "xx"sv);
        CHECK(result[1].first == 2);
        CHECK(result[1].second == ""sv);
    }

    SECTION("Threshold greater than any possible match") {
        auto gen = strdiff_view("abcdef"sv, "axcyez"sv, 100);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 0);
        CHECK(result[0].second == "axcyez"sv); // Entire string as one diff
    }

    SECTION("Edge case: match at the beginning of string") {
        auto gen = strdiff_view("ef___"sv, "ef---"sv, 2);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 2);
        CHECK(result[0].second == "---"sv);
    }

    SECTION("Edge case: small match at the beginning of string") {
        auto gen = strdiff_view("ef___"sv, "ef---"sv, 3);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 0);
        CHECK(result[0].second == "ef---"sv);
    }

    SECTION("Edge case: match at very end of string") {
        auto gen = strdiff_view("___ef"sv, "---ef"sv, 2);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 2);
        CHECK(result[0].first == 0);
        CHECK(result[0].second == "---"sv);
        CHECK(result[1].first == 2);
        CHECK(result[1].second == ""sv);
    }

    SECTION("Edge case: small match at the end of string") {
        auto gen = strdiff_view("___ef"sv, "---ef"sv, 3);
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 0);
        CHECK(result[0].second == "---ef"sv);
    }
}

TEST_CASE("strdiff_view stress tests", "[strdiff][stress]") {
    SECTION("Long strings with repetitive patterns") {
        std::string current(1000, 'a');
        std::string modified = current;
        modified[500] = 'b';

        auto gen =
            strdiff_view(std::string_view(current), std::string_view(modified));
        auto result = collect_generator<char>(gen);

        REQUIRE(result.size() == 2);
        CHECK(result[0].first == 500);
        CHECK(result[0].second == "b"sv);
        CHECK(result[1].first == 499);
        CHECK(result[1].second == ""sv);
    }

    SECTION("Alternating single character changes") {
        std::string current;
        std::string modified;
        for (int i = 0; i < 100; ++i) {
            current += 'a';
            modified += (i % 2 == 0) ? 'a' : 'b';
        }

        auto gen =
            strdiff_view(std::string_view(current), std::string_view(modified));
        auto result = collect_generator<char>(gen);

        // Should have 50 diffs
        REQUIRE(result.size() == 50);
    }
}
