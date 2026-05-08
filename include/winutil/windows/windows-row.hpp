#pragma once
#include <memory>
#include <ranges>
#include <winutil/windows/base-window.hpp>

namespace Winutil {

class WindowsRow : public BaseWindow {
  public:
    WINDOW_CONSTRUCTOR(WindowsRow)

    void clear() override;
    void move(engine::DrawArea &&new_area) override;
    void update() override;

    BaseWindow &get_child(unsigned);

    void resize_windows(std::ranges::input_range auto &&sizes);

    template <class Iter>
        requires std::input_iterator<Iter>
    void resize_windows(Iter begin, Iter end);

    /// @brief create child window of type _Win
    template <typename _Win>
        requires(std::derived_from<_Win, BaseWindow>)
    _Win &make_window();

  private:
    void clear_rulers();
    unsigned place_windows(unsigned size);
    unsigned place_windows(const std::vector<unsigned> &sizes);
    unsigned place_window(unsigned win_id, unsigned pos, unsigned size);

    std::vector<std::unique_ptr<BaseWindow>> _cols;
};

void WindowsRow::resize_windows(std::ranges::input_range auto &&sizes) {
    resize_windows(sizes.begin(), sizes.end());
}

template <class Iter>
    requires std::input_iterator<Iter>
void WindowsRow::resize_windows(Iter begin, Iter end) {
    if (_cols.size() == 0) return;

    std::vector<unsigned> sizes(begin, end);
    if (sizes.size() > _cols.size()) sizes.resize(_cols.size());
    else if (sizes.size() < _cols.size()) {
        int start_idx = sizes.size(), end_idx = _cols.size();
        sizes.resize(_cols.size());
        for (int i = start_idx; i < end_idx; ++i) {
            sizes[i] = _cols[i]->get_size().width;
        }
    }

    engine::normalize_window_size_sequence(sizes, area.get_info().width);

    place_windows(sizes);
}

template <typename _Win>
    requires(std::derived_from<_Win, BaseWindow>)
_Win &WindowsRow::make_window() {
    std::unique_ptr<_Win> res_ptr;
    if (_cols.size() == 0) {
        res_ptr = std::make_unique<_Win>(area.copy());
    } else {
        unsigned full_height = area.get_info().height;
        unsigned full_width = area.get_info().width;
        unsigned win_width = full_width / (_cols.size() + 1);
        if (win_width < WINDOW_MIN_SIZE)
            throw std::runtime_error("Space for windows ran out!");
        unsigned pos = place_windows(win_width - 1);
        auto win_area = area.subarea({0, pos}, full_width - pos, full_height);
        win_area.clear();
        res_ptr = std::make_unique<_Win>(std::move(win_area));
    }
    auto &res = *res_ptr;
    _cols.push_back(std::move(res_ptr));
    return res;
}

} // namespace Winutil
