#pragma once
#include <memory>
#include <winutil/window.hpp>

namespace Winutil {

/// @brief Main screen class. The top of windows hierarchy
class Screen {
  public:
    Screen(unsigned width, unsigned height);
    Screen(const Screen &) = delete;
    Screen(Screen &&) = default;

    ~Screen();

    void clear();
    void update();

    Window &get_window();

    /// @brief create child window of type _Win
    ///
    /// This method must be called only once for Screen
    template <typename _Win>
        requires(std::derived_from<_Win, Window>)
    _Win &make_window();

    /// @brief returns maximum width available for this terminal
    static unsigned max_width() noexcept;

    /// @brief returns maximum height available for this terminal
    static unsigned max_height() noexcept;

    /// @brief signal handler that does all the window shutdown work
    static void destroy_handler(int signal) noexcept;

    void resize(unsigned width, unsigned height);

  private:
    std::unique_ptr<Window> _main;
    engine::MainDrawArea main_area;
    engine::MainDrawArea repl_area;
};

template <typename _Win>
    requires(std::derived_from<_Win, Window>)
_Win &Screen::make_window() {
    if (_main != nullptr)
        throw std::runtime_error("Only one window could be cateated into main");
    _main = std::make_unique<_Win>(std::move(main_area.make_area()));
    return dynamic_cast<_Win &>(*_main);
}

}; // namespace Winutil
