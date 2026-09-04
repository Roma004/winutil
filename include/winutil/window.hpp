#pragma once
#include <winutil/engine/draw-area.hpp>

namespace Winutil {

/// @brief Window interface
class Window {
  public:
    virtual ~Window() = default;

    /// @brief Clear content of the window at next screen redraw
    virtual void clear() = 0;

    /// @brief Change the area of where window draws content
    virtual void move(engine::DrawArea &&new_area) = 0;

    /// @brief Screen update operation
    ///
    /// Every time this method called, content of the screen is being updated.
    /// Every windows is allowed to change content of it's DrawArea
    /// either in this method or everywhere else outside it.
    virtual void update() = 0;

    /// @brief get window size imformation
    virtual engine::WindowDesc get_size() const noexcept = 0;

    /// @brief get draw area of this window
    virtual const engine::DrawArea &get_area() const noexcept = 0;
};

/// @brief concept of any Window, constructible from DrawArea
template <class _Win>
concept WindowType = requires {
    requires std::derived_from<_Win, Window>
                 && std::constructible_from<_Win, engine::DrawArea &&>;
};

} // namespace Winutil
