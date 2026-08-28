#pragma once

#include <windows.h>

// Extends the mouse hit area of TrafficMonitor's taskbar window into the
// otherwise-unused taskbar margin above/below (or left/right for a vertical
// taskbar) without changing the visible TrafficMonitor window size.
//
// The gutter windows are siblings of the real TrafficMonitor taskbar window.
// Their geometry is always derived from the real HWND via GetWindowRect(), so
// they follow taskbar icon movement, width changes, DPI changes and display
// changes instead of relying on cached layout coordinates.
class CTaskbarInteractionGutters final
{
public:
    CTaskbarInteractionGutters() = default;
    ~CTaskbarInteractionGutters()
    {
        Destroy();
    }

    CTaskbarInteractionGutters(const CTaskbarInteractionGutters&) = delete;
    CTaskbarInteractionGutters& operator=(const CTaskbarInteractionGutters&) = delete;

    void Sync(HWND taskbar_hwnd, HWND target_hwnd, bool horizontal_taskbar)
    {
        m_target_hwnd = target_hwnd;

        if (!::IsWindow(taskbar_hwnd) || !::IsWindow(target_hwnd))
        {
            Hide();
            return;
        }

        HWND parent_hwnd = ::GetParent(target_hwnd);
        if (!::IsWindow(parent_hwnd))
        {
            Hide();
            return;
        }

        // The interaction windows must live in the same taskbar hierarchy as
        // the real TrafficMonitor window. If embedding failed, do not create a
        // desktop-level overlay as a fallback.
        if (parent_hwnd != taskbar_hwnd && !::IsChild(taskbar_hwnd, parent_hwnd))
        {
            Hide();
            return;
        }

        // A taskbar/display change may move TrafficMonitor into another parent
        // container. Recreate the gutters under that exact new parent.
        if (m_parent_hwnd != nullptr && m_parent_hwnd != parent_hwnd)
            DestroyWindows();
        m_parent_hwnd = parent_hwnd;

        if (!EnsureWindows())
        {
            Hide();
            return;
        }

        RECT taskbar_rect{};
        RECT target_rect{};
        if (!::GetWindowRect(taskbar_hwnd, &taskbar_rect) ||
            !::GetWindowRect(target_hwnd, &target_rect))
        {
            Hide();
            return;
        }

        // Clamp the interactive strip to the taskbar itself. Only the axis
        // perpendicular to the taskbar is expanded; TrafficMonitor's real
        // horizontal/vertical span is never widened into neighbouring icons.
        if (horizontal_taskbar)
        {
            const LONG left = MaxLong(taskbar_rect.left, target_rect.left);
            const LONG right = MinLong(taskbar_rect.right, target_rect.right);
            if (right <= left)
            {
                Hide();
                return;
            }

            RECT before_rect{
                left,
                taskbar_rect.top,
                right,
                MinLong(taskbar_rect.bottom, target_rect.top)
            };
            RECT after_rect{
                left,
                MaxLong(taskbar_rect.top, target_rect.bottom),
                right,
                taskbar_rect.bottom
            };

            PositionWindow(m_before_hwnd, before_rect);
            PositionWindow(m_after_hwnd, after_rect);
        }
        else
        {
            const LONG top = MaxLong(taskbar_rect.top, target_rect.top);
            const LONG bottom = MinLong(taskbar_rect.bottom, target_rect.bottom);
            if (bottom <= top)
            {
                Hide();
                return;
            }

            RECT before_rect{
                taskbar_rect.left,
                top,
                MinLong(taskbar_rect.right, target_rect.left),
                bottom
            };
            RECT after_rect{
                MaxLong(taskbar_rect.left, target_rect.right),
                top,
                taskbar_rect.right,
                bottom
            };

            PositionWindow(m_before_hwnd, before_rect);
            PositionWindow(m_after_hwnd, after_rect);
        }
    }

    void Destroy()
    {
        DestroyWindows();
        m_target_hwnd = nullptr;
        m_parent_hwnd = nullptr;
    }

private:
    static const wchar_t* WindowClassName()
    {
        return L"TrafficMonitorTaskbarInteractionGutter";
    }

    static LONG MaxLong(LONG a, LONG b)
    {
        return a > b ? a : b;
    }

    static LONG MinLong(LONG a, LONG b)
    {
        return a < b ? a : b;
    }

    static bool EnsureWindowClass()
    {
        static const bool registered = []() -> bool
        {
            WNDCLASSEXW wc{};
            wc.cbSize = sizeof(wc);
            wc.style = CS_DBLCLKS;
            wc.lpfnWndProc = &CTaskbarInteractionGutters::WindowProc;
            wc.hInstance = ::GetModuleHandleW(nullptr);
            wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
            wc.hbrBackground = nullptr;
            wc.lpszClassName = WindowClassName();

            if (::RegisterClassExW(&wc) != 0)
                return true;

            return ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
        }();

        return registered;
    }

    bool EnsureWindows()
    {
        if (!EnsureWindowClass() || !::IsWindow(m_parent_hwnd))
            return false;

        if (!::IsWindow(m_before_hwnd))
            m_before_hwnd = CreateGutterWindow();
        if (!::IsWindow(m_after_hwnd))
            m_after_hwnd = CreateGutterWindow();

        return ::IsWindow(m_before_hwnd) && ::IsWindow(m_after_hwnd);
    }

    HWND CreateGutterWindow()
    {
        // WS_EX_TRANSPARENT changes paint ordering, not hit testing. With no
        // background brush and no drawing of our own, the Explorer taskbar
        // remains visually unchanged while this child still receives mouse
        // input in the otherwise-unused margin.
        return ::CreateWindowExW(
            WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_NOPARENTNOTIFY,
            WindowClassName(),
            L"",
            WS_CHILD,
            0,
            0,
            0,
            0,
            m_parent_hwnd,
            nullptr,
            ::GetModuleHandleW(nullptr),
            this);
    }

    void PositionWindow(HWND hwnd, const RECT& screen_rect)
    {
        if (!::IsWindow(hwnd))
            return;

        const LONG width = screen_rect.right - screen_rect.left;
        const LONG height = screen_rect.bottom - screen_rect.top;
        if (width <= 0 || height <= 0)
        {
            ::ShowWindow(hwnd, SW_HIDE);
            return;
        }

        // Convert from the authoritative screen-space rectangles to the
        // current taskbar parent only at the last moment. This avoids stale
        // offsets when Explorer moves containers or DPI changes.
        POINT origin{screen_rect.left, screen_rect.top};
        if (::MapWindowPoints(HWND_DESKTOP, m_parent_hwnd, &origin, 1) == 0 &&
            ::GetLastError() != ERROR_SUCCESS)
        {
            ::ShowWindow(hwnd, SW_HIDE);
            return;
        }

        ::SetWindowPos(
            hwnd,
            HWND_TOP,
            origin.x,
            origin.y,
            width,
            height,
            SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }

    void Hide()
    {
        if (::IsWindow(m_before_hwnd))
            ::ShowWindow(m_before_hwnd, SW_HIDE);
        if (::IsWindow(m_after_hwnd))
            ::ShowWindow(m_after_hwnd, SW_HIDE);
    }

    void DestroyWindows()
    {
        if (::IsWindow(m_before_hwnd))
            ::DestroyWindow(m_before_hwnd);
        if (::IsWindow(m_after_hwnd))
            ::DestroyWindow(m_after_hwnd);

        m_before_hwnd = nullptr;
        m_after_hwnd = nullptr;
    }

    LRESULT ForwardClientMouseMessage(HWND source_hwnd, UINT message, WPARAM w_param, LPARAM l_param)
    {
        if (!::IsWindow(m_target_hwnd))
            return 0;

        POINT point{
            static_cast<SHORT>(LOWORD(l_param)),
            static_cast<SHORT>(HIWORD(l_param))
        };
        ::ClientToScreen(source_hwnd, &point);
        ::ScreenToClient(m_target_hwnd, &point);

        const LPARAM target_l_param = MAKELPARAM(
            static_cast<WORD>(static_cast<SHORT>(point.x)),
            static_cast<WORD>(static_cast<SHORT>(point.y)));
        return ::SendMessageW(m_target_hwnd, message, w_param, target_l_param);
    }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
    {
        if (message == WM_NCCREATE)
        {
            const auto* create = reinterpret_cast<const CREATESTRUCTW*>(l_param);
            ::SetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(create->lpCreateParams));
        }

        auto* self = reinterpret_cast<CTaskbarInteractionGutters*>(
            ::GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        if (self != nullptr)
        {
            switch (message)
            {
            case WM_MOUSEWHEEL:
                // WM_MOUSEWHEEL already carries screen coordinates, so forward
                // it verbatim. This reaches the derived taskbar handler and
                // therefore reuses the existing plugin-first/volume logic.
                if (::IsWindow(self->m_target_hwnd))
                    return ::SendMessageW(self->m_target_hwnd, message, w_param, l_param);
                return 0;

            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONUP:
                return self->ForwardClientMouseMessage(hwnd, message, w_param, l_param);

            case WM_MOUSEACTIVATE:
                return MA_NOACTIVATE;

            case WM_NCHITTEST:
                return HTCLIENT;

            case WM_ERASEBKGND:
                return TRUE;

            case WM_PAINT:
            {
                PAINTSTRUCT ps{};
                ::BeginPaint(hwnd, &ps);
                ::EndPaint(hwnd, &ps);
                return 0;
            }

            case WM_NCDESTROY:
                ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
                break;

            default:
                break;
            }
        }

        return ::DefWindowProcW(hwnd, message, w_param, l_param);
    }

private:
    HWND m_target_hwnd{};
    HWND m_parent_hwnd{};
    HWND m_before_hwnd{};
    HWND m_after_hwnd{};
};
