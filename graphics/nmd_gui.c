#include "nmd_common.h"

#ifdef _WIN32
LRESULT nmd_win32_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!_nmd_context.hWnd && uMsg == WM_MOUSEMOVE)
    {
        _nmd_context.io.mouse_pos.x = ((int)(short)LOWORD(lParam));
        _nmd_context.io.mouse_pos.y = ((int)(short)HIWORD(lParam));
    }

	/* Handle raw input */
    if (uMsg == WM_INPUT) 
    {
        RAWINPUT ri;
        UINT sz = sizeof(RAWINPUT);
        if (GetRawInputData((HRAWINPUT)lParam, RID_HEADER, &ri, &sz, sizeof(RAWINPUTHEADER)) > 0)
        {
            if (ri.header.dwType == RIM_TYPEMOUSE)
            {
                if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &ri, &sz, sizeof(RAWINPUTHEADER)) > 0)
                {
                    if (ri.data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
                    {
                        _nmd_context.io.mouse_pos.x += (float)ri.data.mouse.lLastX;
                        _nmd_context.io.mouse_pos.y += (float)ri.data.mouse.lLastY;

                        if (ri.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
                            _nmd_context.io.mouse_down[0] = true, _nmd_context.io.mouse_clicked_pos[0] = _nmd_context.io.mouse_pos;
                        else if (ri.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
                            _nmd_context.io.mouse_down[0] = false, _nmd_context.io.mouse_released[0] = true;
                        if (ri.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
                            _nmd_context.io.mouse_down[1] = true, _nmd_context.io.mouse_clicked_pos[1] = _nmd_context.io.mouse_pos;
                        else if (ri.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
                            _nmd_context.io.mouse_down[1] = false, _nmd_context.io.mouse_released[1] = true;
                        if (ri.data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                            _nmd_context.io.mouse_down[2] = true, _nmd_context.io.mouse_clicked_pos[2] = _nmd_context.io.mouse_pos;
                        else if (ri.data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
                            _nmd_context.io.mouse_down[2] = false, _nmd_context.io.mouse_released[2] = true;
                    }
                }
            }
        }
    }

    switch (uMsg)
    {
    /* Handle keyboard keys */
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if(wParam < 256)
            _nmd_context.io.keys_down[wParam] = true;
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wParam < 256)
            _nmd_context.io.keys_down[wParam] = false;
        return 0;

    /* Handle mouse buttons */
    case WM_LBUTTONDOWN: _nmd_context.io.mouse_down[0] = true; _nmd_context.io.mouse_clicked_pos[0] = _nmd_context.io.mouse_pos; return 0;
    case WM_LBUTTONUP: _nmd_context.io.mouse_down[0] = false; _nmd_context.io.mouse_released[0] = true; return 0;

    case WM_RBUTTONDOWN: _nmd_context.io.mouse_down[1] = true; _nmd_context.io.mouse_clicked_pos[1] = _nmd_context.io.mouse_pos; return 0;
    case WM_RBUTTONUP: _nmd_context.io.mouse_down[1] = false; _nmd_context.io.mouse_released[1] = true; return 0;

    case WM_MBUTTONDOWN: _nmd_context.io.mouse_down[2] = true; _nmd_context.io.mouse_clicked_pos[2] = _nmd_context.io.mouse_pos; return 0;
    case WM_MBUTTONUP: _nmd_context.io.mouse_down[2] = false;  _nmd_context.io.mouse_released[2] = true; return 0;

    case WM_XBUTTONDOWN: _nmd_context.io.mouse_down[GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 3 : 4] = true; return 0;
    case WM_XBUTTONUP: _nmd_context.io.mouse_down[GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 3 : 4] = false; return 0;
    }

    return 0;
}
#endif /* _WIN32 */

/* This is a very simple hash function which takes a string and returns a 32-bit number which should be unique for the string */
uint32_t _nmd_hash_string_to_uint32(const char* string)
{
    if (!*string)
        return 0;

    /* Starting hash */
    uint32_t hash = 0xDEADC0DE + *string;

    /* This 2-byte pointer is used to constantly shift between different parts of 'hash' */
    uint16_t* p;

    for (size_t i = 1; string[i]; i++)
    {
        /*
        If 'i' modulo 3 is 0, then point to the first and second bytes of 'hash'
        If 'i' modulo 3 is 1, then point to the second and third bytes of 'hash'
        If 'i' modulo 3 is 2, then point to the third and fourth bytes of 'hash'
        */
        p = (uint16_t*)((uint8_t*)&hash+(i%3));

        /* Do some random XOR between the two bytes pointed by 'p' and string[i] and string[i -1] */
        *p = *p ^ ((string[i - 1] << 8) | string[i]);
    }

    return hash;
}

/* Iterates through all windows in the global context and return the window that has the ID equal to 'window_hash' */
nmd_window* _nmd_find_window_by_hash(uint32_t window_hash)
{
    for (size_t i = 0; i < _nmd_context.gui.num_windows; i++)
    {
        if (_nmd_context.gui.windows[i].id == window_hash)
            return _nmd_context.gui.windows + i;
    }

    return 0;
}

/* Helper function. Wrapper around '_nmd_find_window_by_hash' but it takes a string */
nmd_window* _nmd_find_window_by_name(const char* window_name)
{
    return _nmd_find_window_by_hash(_nmd_hash_string_to_uint32(window_name));
}

/* Specifies the beginning of the window. Widgets can be added after calling this function. Returns true if the window is not minimized */
bool nmd_begin(const char* window_name)
{
    nmd_window* window = _nmd_find_window_by_name(window_name);
    if (!window)
    {
        /* Add window */

        /* Check if we need to resize the buffer */
        if (_nmd_context.gui.num_windows == _nmd_context.gui.windows_capacity)
        {
            _nmd_context.gui.windows_capacity *= 2;
            void* mem = NMD_MALLOC(_nmd_context.gui.windows_capacity);
            if (!mem)
                return false;
            NMD_MEMCPY(mem, _nmd_context.gui.windows, _nmd_context.gui.num_windows);
            NMD_FREE(_nmd_context.gui.windows);
        }

        window = &_nmd_context.gui.windows[_nmd_context.gui.num_windows++];
        window->id = _nmd_hash_string_to_uint32(window_name);
        window->rect.p0 = _nmd_context.gui.window_pos;
        window->rect.p1.x = window->rect.p0.x + 250;
        window->rect.p1.y = window->rect.p0.y + 230;
        window->visible = true;
        window->collapsed = false;
        window->allow_close = _nmd_context.gui.num_windows == 0 ? false : true;
        window->allow_collapse = true;
        window->allow_move_title_bar = true;
        window->allow_move_body = true;
        window->allow_resize = true;
        window->moving = false;
    }

    _nmd_context.gui.window = window;

    if (!window->visible)
        return false;

    if (window->moving)
    {
        if (_nmd_context.io.mouse_down[0])
        {
            /* Move window */
            const int width = window->rect.p1.x - window->rect.p0.x;
            const int height = window->rect.p1.y - window->rect.p0.y;

            window->rect.p0.x = _nmd_context.io.mouse_pos.x - _nmd_context.io.window_move_delta.x;
            window->rect.p0.y = _nmd_context.io.mouse_pos.y - _nmd_context.io.window_move_delta.y;
            window->rect.p1.x = window->rect.p0.x + width;
            window->rect.p1.y = window->rect.p0.y + height;
        }
        else
            window->moving = false;
    }

    /* Add top bar's background filled rect */
    nmd_add_rect_filled(window->rect.p0.x, window->rect.p0.y, window->rect.p1.x, window->rect.p0.y + 18.0f, NMD_COLOR_GUI_MAIN, 5.0f, window->collapsed ? NMD_CORNER_ALL : NMD_CORNER_TOP);
    
    /* Add window name */
    nmd_add_text(&_nmd_context.draw_list.default_atlas, window->rect.p0.x + (window->allow_collapse ? 20 : 0), window->rect.p0.y + 13, window_name, 0, NMD_COLOR_WHITE);

    /* Check if window can be collapsed and if the mouse is over the collapse/expand triangle */
    if (window->allow_collapse && _nmd_context.io.mouse_pos.x >= window->rect.p0.x + 1 && _nmd_context.io.mouse_pos.x < window->rect.p0.x + 17 && _nmd_context.io.mouse_pos.y >= window->rect.p0.y + 1 && _nmd_context.io.mouse_pos.y < window->rect.p0.y + 17)
    {
        /* Check if we properly clicked the triangle */
        if (_nmd_context.io.mouse_released[0])
        {
            if (_nmd_context.io.mouse_clicked_pos[0].x >= window->rect.p0.x + 1 && _nmd_context.io.mouse_clicked_pos[0].x < window->rect.p0.x + 17 && _nmd_context.io.mouse_clicked_pos[0].y >= window->rect.p0.y + 1 && _nmd_context.io.mouse_clicked_pos[0].y < window->rect.p0.y + 17)
                window->collapsed = !window->collapsed;
        }
        
        /* Add filled circle behind triangle */
        nmd_add_circle_filled(window->rect.p0.x + 9.0f, window->rect.p0.y + 9.0f, 7.5f, _nmd_context.io.mouse_down[0] ? NMD_COLOR_GUI_PRESSED : NMD_COLOR_GUI_HOVER, 12);
    } 
    /* Check if the window can be closed and if the mouse is over the close button */
    else if (window->allow_close && _nmd_context.io.mouse_pos.x >= window->rect.p1.x - 17 && _nmd_context.io.mouse_pos.x < window->rect.p1.x - 1 && _nmd_context.io.mouse_pos.y >= window->rect.p0.y + 1 && _nmd_context.io.mouse_pos.y < window->rect.p0.y + 17)
    {
        /* Check if we properly clicked the close button */
        if (_nmd_context.io.mouse_released[0])
        {
            if (_nmd_context.io.mouse_clicked_pos[0].x >= window->rect.p1.x - 10 && _nmd_context.io.mouse_clicked_pos[0].x < window->rect.p1.x - 1 && _nmd_context.io.mouse_clicked_pos[0].y >= window->rect.p0.y + 1 && _nmd_context.io.mouse_clicked_pos[0].y < window->rect.p0.y + 17)
                window->visible = false;
        }

        /* Add filled circle behind triangle */
        nmd_add_circle_filled(window->rect.p1.x - 9.5f, window->rect.p0.y + 9.0f, 7.5f, _nmd_context.io.mouse_down[0] ? NMD_COLOR_GUI_PRESSED : NMD_COLOR_GUI_HOVER, 12);
    }
    /* Check if the window's title bar can be dragged and if the mouse is over the title bar */
    else if (window->allow_move_title_bar && _nmd_context.io.mouse_pos.x >= window->rect.p0.x && _nmd_context.io.mouse_pos.x < window->rect.p1.x && _nmd_context.io.mouse_pos.y >= window->rect.p0.y && _nmd_context.io.mouse_pos.y < window->rect.p0.y + 18.0f && !window->moving)
    {
        /* Determine if the window is being moved */
        window->moving = _nmd_context.io.mouse_down[0];
        if (window->moving)
        {
            _nmd_context.io.window_move_delta.x = _nmd_context.io.mouse_pos.x - window->rect.p0.x;
            _nmd_context.io.window_move_delta.y = _nmd_context.io.mouse_pos.y - window->rect.p0.y;
        }
    }
    
    if (window->allow_close)
    {
        /* Add close cross("button)" to the top right corner */
        nmd_add_line(window->rect.p1.x - 13, window->rect.p0.y + 5.0f, window->rect.p1.x - 5.9f, window->rect.p0.y + 12.1f, NMD_COLOR_WHITE, 1.0f);
        nmd_add_line(window->rect.p1.x - 13, window->rect.p0.y + 12.0f, window->rect.p1.x - 5.9f, window->rect.p0.y + 4.9f, NMD_COLOR_WHITE, 1.0f);
    }

    /* Return 'false' if the window is mimized, this tells the user he shouldn't call any widgets */
    if (window->collapsed)
    {
        /* Add triangle facing right */
        nmd_add_triangle_filled(window->rect.p0.x + 6, window->rect.p0.y + 5, window->rect.p0.x + 15, window->rect.p0.y + 9, window->rect.p0.x + 6, window->rect.p0.y + 13, NMD_COLOR_WHITE);
        return false;
    }
    else
    {
        if (window->allow_collapse)
        {
            /* Add triangle facing bottom */
            nmd_add_triangle_filled(window->rect.p0.x + 5, window->rect.p0.y + 5, window->rect.p0.x + 13, window->rect.p0.y + 5, window->rect.p0.x + 9, window->rect.p0.y + 14, NMD_COLOR_WHITE);
        }
    }

    /* Add body's background filled rect */
    nmd_add_rect_filled(window->rect.p0.x, window->rect.p0.y + 18.0f, window->rect.p1.x, window->rect.p1.y, NMD_COLOR_GUI_BACKGROUND, 5.0f, NMD_CORNER_BOTTOM);

    window->y_offset = window->rect.p0.y + 18 + 6;
    
    return true;
}

/* Specifies the end of the window. Widgets won't be added after calling this function */
void nmd_end()
{
    _nmd_context.gui.window = 0;
}

void nmd_text(const char* fmt, ...)
{
    /* Make sure we can draw this widget */
    nmd_window* window = _nmd_context.gui.window;
    if (!window->visible || window->collapsed)
        return;

    va_list args;
    va_start(args, fmt);
    const int size = NMD_VSPRINTF(_nmd_context.gui.fmt_buffer, fmt, args);
    va_end(args);

    nmd_add_text(&_nmd_context.draw_list.default_atlas, window->rect.p0.x + 6, window->y_offset + 10, _nmd_context.gui.fmt_buffer, _nmd_context.gui.fmt_buffer + size, NMD_COLOR_WHITE);

    window->y_offset += 10 + 5;
}

bool nmd_button(const char* label)
{
    /* Make sure we can draw this widget */
    nmd_window* window = _nmd_context.gui.window;
    if (!window->visible || window->collapsed)
        return false;

    nmd_vec2 size;
    nmd_get_text_size(&_nmd_context.draw_list.default_atlas, label, 0, &size);

    const bool is_mouse_hovering = _nmd_context.io.mouse_pos.x >= window->rect.p0.x + 6 && _nmd_context.io.mouse_pos.x < window->rect.p0.x + 6 + size.x + 8 && _nmd_context.io.mouse_pos.y >= window->y_offset && _nmd_context.io.mouse_pos.y < window->y_offset + 16;

    const bool clicked_button = is_mouse_hovering && _nmd_context.io.mouse_released[0] && _nmd_context.io.mouse_clicked_pos[0].x >= window->rect.p0.x + 6 && _nmd_context.io.mouse_clicked_pos[0].x < window->rect.p0.x + 6 + size.x + 8 && _nmd_context.io.mouse_clicked_pos[0].y >= window->y_offset && _nmd_context.io.mouse_clicked_pos[0].y < window->y_offset + 16;

    /* Add background filled rect */
    nmd_add_rect_filled(window->rect.p0.x + 6, window->y_offset, window->rect.p0.x + 6 + size.x + 8, window->y_offset + 16, is_mouse_hovering ? NMD_COLOR_GUI_BUTTON_HOVER : NMD_COLOR_GUI_BUTTON_BACKGROUND, 0, 0);
    
    nmd_add_text(&_nmd_context.draw_list.default_atlas, window->rect.p0.x + 6 + 4, window->y_offset + 11, label, 0, NMD_COLOR_WHITE);

    window->y_offset += 16 + 5;

    return clicked_button;
}

bool nmd_checkbox(const char* label, bool* checked)
{
    /* Make sure we can draw this widget */
    nmd_window* window = _nmd_context.gui.window;
    if (!window->visible || window->collapsed)
        return false;

    const bool is_mouse_hovering = _nmd_context.io.mouse_pos.x >= window->rect.p0.x + 6 && _nmd_context.io.mouse_pos.x < window->rect.p0.x + 6 + 16 && _nmd_context.io.mouse_pos.y >= window->y_offset && _nmd_context.io.mouse_pos.y < window->y_offset + 16;
    
    bool state_changed = false;

    if (is_mouse_hovering && _nmd_context.io.mouse_released[0] && _nmd_context.io.mouse_clicked_pos[0].x >= window->rect.p0.x + 6 && _nmd_context.io.mouse_clicked_pos[0].x < window->rect.p0.x + 6 + 16 && _nmd_context.io.mouse_clicked_pos[0].y >= window->y_offset && _nmd_context.io.mouse_clicked_pos[0].y < window->y_offset + 16)
    {
        *checked = !*checked;
        state_changed = true;
    }

    /* Add background filled rect */
    nmd_add_rect_filled(window->rect.p0.x + 6, window->y_offset, window->rect.p0.x + 6 + 16, window->y_offset + 16, is_mouse_hovering ? NMD_COLOR_GUI_WIDGET_HOVER : NMD_COLOR_GUI_WIDGET_BACKGROUND, 0, 0);
    
    /* Add filled square if 'state' is true */
    if (*checked)
        nmd_add_rect_filled(window->rect.p0.x + 6 + 3, window->y_offset + 3, window->rect.p0.x + 6 + 16 - 3, window->y_offset + 16 - 3, NMD_COLOR_GUI_ACTIVE, 0, 0);

    window->y_offset += 16 + 4;

    nmd_add_text(&_nmd_context.draw_list.default_atlas, window->rect.p0.x + 6 + 16 + 4, window->y_offset - 9, label, 0, NMD_COLOR_WHITE);

    return state_changed;
}

#define _NMD_SLIDER_WIDTH 160
bool nmd_slider_float(const char* label, float* value, float min_value, float max_value)
{
    /* Make sure we can draw this widget */
    nmd_window* window = _nmd_context.gui.window;
    if (!window->visible || window->collapsed || *value < min_value || *value > max_value)
        return false;

    float offset;

    bool value_changed = false;

    /* Check if the user is sliding the slider */
    if (_nmd_context.io.mouse_down[0] && _nmd_context.io.mouse_clicked_pos[0].x >= window->rect.p0.x + 6 && _nmd_context.io.mouse_clicked_pos[0].x < window->rect.p0.x + 6 + _NMD_SLIDER_WIDTH && _nmd_context.io.mouse_clicked_pos[0].y >= window->y_offset && _nmd_context.io.mouse_clicked_pos[0].y < window->y_offset + 16)
    {
        const float last_value = *value;

        /* Calculte the new offset */
        if (_nmd_context.io.mouse_pos.x < window->rect.p0.x + 6 + 6)
        {
            offset = 0;
            *value = min_value;
        }
        else if (_nmd_context.io.mouse_pos.x >= window->rect.p0.x + 6 + _NMD_SLIDER_WIDTH - 6)
        {
            offset = _NMD_SLIDER_WIDTH - 12;
            *value = max_value;
        }
        else
        {
            offset = _nmd_context.io.mouse_pos.x - (window->rect.p0.x + 6) - 6;
            *value = (offset / (_NMD_SLIDER_WIDTH -6)) * (max_value - min_value) + min_value;
        }

        if (*value != last_value)
            value_changed = true;
    }
    else
    {
        if (*value == max_value)
            offset = _NMD_SLIDER_WIDTH - 12;
        else
            offset = ((*value - min_value) / (max_value - min_value)) * (_NMD_SLIDER_WIDTH - 6);
    }
    //offset = (*value / max_value) * (120-12) - min_value;

    const bool is_mouse_hovering = _nmd_context.io.mouse_pos.x >= window->rect.p0.x + 6 && _nmd_context.io.mouse_pos.x < window->rect.p0.x + 6 + _NMD_SLIDER_WIDTH && _nmd_context.io.mouse_pos.y >= window->y_offset && _nmd_context.io.mouse_pos.y < window->y_offset + 16;

    /* Add background filled rect */
    nmd_add_rect_filled(window->rect.p0.x + 6, window->y_offset, window->rect.p0.x + 6 + _NMD_SLIDER_WIDTH, window->y_offset + 16, is_mouse_hovering ? NMD_COLOR_GUI_WIDGET_HOVER : NMD_COLOR_GUI_WIDGET_BACKGROUND, 0, 0);

    /* Add indicator bar */
    nmd_add_rect_filled(window->rect.p0.x + 6 + 2 + offset, window->y_offset + 2, window->rect.p0.x + 6 + 2 + offset + 8, window->y_offset + 16 - 2, NMD_COLOR_GUI_ACTIVE, 0, 0);

    /* Add value text */
    const int size = NMD_SPRINTF(_nmd_context.gui.fmt_buffer, "%.3f", *value);
    nmd_add_text(&_nmd_context.draw_list.default_atlas, window->rect.p0.x + 6 + (_NMD_SLIDER_WIDTH/2-15), window->y_offset + 12, _nmd_context.gui.fmt_buffer, _nmd_context.gui.fmt_buffer + size, NMD_COLOR_WHITE);

    /* Add label text */
    nmd_add_text(&_nmd_context.draw_list.default_atlas, window->rect.p0.x + 6 + _NMD_SLIDER_WIDTH + 4, window->y_offset + 12, label, 0, NMD_COLOR_WHITE);

    window->y_offset += 16 + 5;

    return value_changed;
}