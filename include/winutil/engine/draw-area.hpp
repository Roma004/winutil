#pragma once
#include "winutil/engine/color-string.hpp"
#include "winutil/engine/common.hpp"
#include <vector>

namespace Winutil::engine {

class DrawArea {
  public:
    using lines_t = std::vector<color_string_view>;
    using iterator = lines_t::iterator;
    using const_iterator = lines_t::const_iterator;

    DrawArea() = default;

    DrawArea subarea(WindowPos at, unsigned width, unsigned height);
    DrawArea copy();

    color_string_view get_line(unsigned line_no);
    const color_string_view get_line(unsigned line_no) const;

    const_iterator begin() const noexcept { return _lines.begin(); }
    const_iterator end() const noexcept { return _lines.end(); }

    iterator begin() noexcept { return _lines.begin(); }
    iterator end() noexcept { return _lines.end(); }

    void clear() noexcept;

    WindowDesc get_info() const noexcept { return _desc; }

  private:
    DrawArea(std::vector<color_string_view> &&);
    friend class MainDrawArea;

    std::vector<color_string_view> _lines;
    WindowDesc _desc;
};

class MainDrawArea {
  public:
    using lines_t = std::vector<color_string>;
    using iterator = lines_t::iterator;
    using const_iterator = lines_t::const_iterator;

    MainDrawArea(unsigned width, unsigned height);

    DrawArea make_area();
    MainDrawArea copy();

    WindowDesc get_info() const noexcept { return _desc; }

    const_iterator begin() const noexcept { return _lines.begin(); }
    const_iterator end() const noexcept { return _lines.end(); }

    iterator begin() noexcept { return _lines.begin(); }
    iterator end() noexcept { return _lines.end(); }

    void resize(unsigned width, unsigned height);

    color_string_view get_line(unsigned line_no);

  private:
    std::vector<color_string> _lines;
    WindowDesc _desc;
};

}; // namespace Winutil::engine
