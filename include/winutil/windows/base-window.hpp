#pragma once
#include <winutil/engine/draw-area.hpp>
#include <winutil/window.hpp>

namespace Winutil {

#define WINDOW_CONSTRUCTOR(_cls)                                            \
    _cls(Winutil::engine::DrawArea &&area) : BaseWindow(std::move(area)) {} \
    _cls(const _cls &) = delete;                                            \
    _cls(_cls &&) = default;

/// @brief Base Window class
class BaseWindow : public Window {
  public:
    BaseWindow(engine::DrawArea &&area);
    BaseWindow(const BaseWindow &) = delete;
    BaseWindow(BaseWindow &&) = default;

    /// @brief Clear content of the window at next screen redraw
    void clear() override;

    /// @brief Change the area of where window draws content
    void move(engine::DrawArea &&new_area) override;

    /// @brief Screen update operation
    ///
    /// Every time this method called, content of the screen is being updated.
    /// Every windows is allowed to change content of it's DrawArea
    /// either in this method or everywhere else outside it.
    void update() override;

    /// @brief get window size imformation
    engine::WindowDesc get_size() const noexcept { return area.get_info(); };

    /// @brief get draw area of this window
    const engine::DrawArea &get_area() const noexcept { return area; };

  protected:
    engine::DrawArea area;
};

} // namespace Winutil

