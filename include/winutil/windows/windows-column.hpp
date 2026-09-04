#pragma once
#include <memory>
#include <winutil/windows/base-window.hpp>

namespace Winutil {

class WindowsColumn : public BaseWindow {
  public:
    using BaseWindow::BaseWindow;

    void clear() override;
    void move(engine::DrawArea &&new_area) override;
    void update() override;

    Window &get_child(unsigned);

    void resize_windows(std::ranges::input_range auto &&sizes);

    template <class Iter>
        requires std::input_iterator<Iter>
    void resize_windows(Iter begin, Iter end);

    /// @brief create child window of type _Win
    template <WindowType _Win> _Win &make_window();

  protected:
    engine::DrawArea alloc_area();
    Window &add_window(std::unique_ptr<Window> &&win);

  private:
    void clear_rulers();
    unsigned place_windows(unsigned size);
    unsigned place_windows(const std::vector<unsigned> &sizes);
    unsigned place_window(unsigned win_id, unsigned pos, unsigned size);

    std::vector<std::unique_ptr<Window>> _rows;
};

void WindowsColumn::resize_windows(std::ranges::input_range auto &&sizes) {
    resize_windows(sizes.begin(), sizes.end());
}

template <class Iter>
    requires std::input_iterator<Iter>
void WindowsColumn::resize_windows(Iter begin, Iter end) {
    if (_rows.size() == 0) return;

    std::vector<unsigned> sizes(begin, end);
    if (sizes.size() > _rows.size()) sizes.resize(_rows.size());
    else if (sizes.size() < _rows.size()) {
        int start_idx = sizes.size(), end_idx = _rows.size();
        sizes.resize(_rows.size());
        for (int i = start_idx; i < end_idx; ++i) {
            sizes[i] = _rows[i]->get_size().height;
        }
    }

    engine::normalize_window_size_sequence(sizes, area.get_info().height);

    place_windows(sizes);
}

template <WindowType _Win> _Win &WindowsColumn::make_window() {
    std::unique_ptr<_Win> res_ptr =
        std::make_unique<_Win>(std::move(alloc_area()));
    return dynamic_cast<_Win>(add_window(std::move(res_ptr)));
}

} // namespace Winutil
