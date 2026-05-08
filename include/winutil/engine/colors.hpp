#pragma once

#define CTRL            L"\e["
#define CTRL_FG(_color) L"3" _color
#define CTRL_BG(_color) L"4" _color
#define CTRL_DELIM      L";"
#define CTRL_END        L"m"

#define BLACK  L"0"
#define RED    L"1"
#define GREEN  L"2"
#define YELLOW L"3"
#define BLUE   L"4"
#define PURPLE L"5"
#define CYAN   L"6"
#define WHITE  L"7"

#define COLOR(_fg, _bg) \
    CTRL CTRL_FG(_fg)   \
    CTRL_DELIM CTRL_BG(_bg) CTRL_END

#define COLOR_RGB(_r, _g, _b) L"8;2;" #_r CTRL_DELIM #_g CTRL_DELIM #_b

#define COLOR_FG(_fg) CTRL CTRL_FG(_fg) CTRL_END
#define COLOR_BG(_bg) CTRL CTRL_BG(_bg) CTRL_END

#define COLOR_NONE L"\e[0m"
