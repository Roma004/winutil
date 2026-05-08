#pragma once
#include "winutil/engine/common.hpp"
#include "winutil/windows/base-window.hpp"

namespace Winutil {

/// @brief Window wich implements base serial output protocol.
class WindowOutput : public BaseWindow {
  public:
    WINDOW_CONSTRUCTOR(WindowOutput)

    /// @brief put string at the write position
    void write(std::wstring_view);

    /// @brief change write position
    void set_cursor(engine::WindowPos);

    /// @brief get current write position
    engine::WindowPos get_cursor();

    /// @brief same as write
    WindowOutput &operator<<(std::wstring_view str) {
        write(str);
        return *this;
    }

    /// @brief same set_cursor
    WindowOutput &operator<<(engine::WindowPos pos) {
        set_cursor(pos);
        return *this;
    }

    void clear() override;

  private:
    void write_line(std::wstring_view);
    void write_char(wchar_t);
    void insert_color(std::wstring_view);
    void newline();

    engine::WindowPos _cursor = {};
};

} // namespace Winutil
