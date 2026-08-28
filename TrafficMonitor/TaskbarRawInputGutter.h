#pragma once

#include <windows.h>
#include <vector>
#include <algorithm>
#include "TaskbarVolumeControl.h"

// Extends TrafficMonitor's taskbar mouse interaction into the otherwise-unused
// taskbar margin without creating any overlay window on top of Explorer.
//
// Raw Input is used only as an observation source. It is NOT a global mouse
// hook, does not inject into Explorer, and does not suppress normal Windows
// input. We only act when the cursor is inside the taskbar and inside the
// TrafficMonitor taskbar window's span on the parallel axis, while being
// outside the real TrafficMonitor HWND on the perpendicular axis.
//
// Geometry is resolved from GetWindowRect() for both HWNDs for every relevant
// mouse event. There is deliberately no cached x/y/width/height binding, so
// icon-count movement, DPI changes, taskbar resizing and monitor changes are
// followed automatically.
class CTaskbarRawInputGutter final
{
public:
    static void RegisterTarget(HWND taskbar_hwnd, HWND target_hwnd, bool horizontal_taskbar)
    {
        if (!::IsWindow(taskbar_hwnd) || !::IsWindow(target_hwnd))
            return;

        auto& targets = Targets();
        CleanupTargets();

        for (auto& target : targets)
        {
            if (target.target_hwnd == target_hwnd)
            {
                target.taskbar_hwnd = taskbar_hwnd;
                target.horizontal_taskbar = horizontal_taskbar;
                EnsureRawInputRegistration();
                return;
            }
        }

        targets.push_back({taskbar_hwnd, target_hwnd, horizontal_taskbar});
        EnsureRawInputRegistration();
    }

    static void UnregisterTarget(HWND target_hwnd)
    {
        auto& targets = Targets();
        targets.erase(
            std::remove_if(
                targets.begin(),
                targets.end(),
                [target_hwnd](const TargetInfo& target)
                {
                    return target.target_hwnd == target_hwnd;
                }),
            targets.end());

        if (LastClickTarget() == target_hwnd)
            ResetDoubleClickState();

        if (targets.empty())
            RemoveRawInputRegistration();
    }

private:
    struct TargetInfo
    {
        HWND taskbar_hwnd{};
        HWND target_hwnd{};
        bool horizontal_taskbar{true};
    };

    static std::vector<TargetInfo>& Targets()
    {
        static std::vector<TargetInfo> targets;
        return targets;
    }

    static HWND& InputSinkHwnd()
    {
        static HWND hwnd{};
        return hwnd;
    }

    static bool& IsRawInputRegistered()
    {
        static bool registered{};
        return registered;
    }

    static DWORD& LastClickTime()
    {
        static DWORD value{};
        return value;
    }

    static POINT& LastClickPoint()
    {
        static POINT value{};
        return value;
    }

    static HWND& LastClickTarget()
    {
        static HWND value{};
        return value;
    }

    static const wchar_t* SinkClassName()
    {
        return L"TrafficMonitorTaskbarRawInputGutterSink";
    }

    static bool EnsureSinkClass()
    {
        static const bool registered = []() -> bool
        {
            WNDCLASSEXW wc{};
            wc.cbSize = sizeof(wc);
            wc.lpfnWndProc = &CTaskbarRawInputGutter::SinkWindowProc;
            wc.hInstance = ::GetModuleHandleW(nullptr);
            wc.lpszClassName = SinkClassName();

            if (::RegisterClassExW(&wc) != 0)
                return true;

            return ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
        }();

        return registered;
    }

    static bool EnsureSinkWindow()
    {
        HWND& hwnd = InputSinkHwnd();
        if (::IsWindow(hwnd))
            return true;

        if (!EnsureSinkClass())
            return false;

        // A message-only window never participates in z-order, painting or
        // taskbar layout. It exists only as the documented Raw Input sink.
        hwnd = ::CreateWindowExW(
            0,
            SinkClassName(),
            L"",
            0,
            0,
            0,
            0,
            0,
            HWND_MESSAGE,
            nullptr,
            ::GetModuleHandleW(nullptr),
            nullptr);

        return ::IsWindow(hwnd);
    }

    static bool EnsureRawInputRegistration()
    {
        if (!EnsureSinkWindow())
            return false;

        RAWINPUTDEVICE device{};
        device.usUsagePage = 0x01; // Generic Desktop Controls
        device.usUsage = 0x02;     // Mouse
        device.dwFlags = RIDEV_INPUTSINK;
        device.hwndTarget = InputSinkHwnd();

        // Reassert the registration whenever a taskbar target is registered or
        // updated. TrafficMonitor core does not otherwise register raw mouse
        // input, and this also makes reopen/recreate cycles self-healing.
        const bool ok = ::RegisterRawInputDevices(&device, 1, sizeof(device)) != FALSE;
        IsRawInputRegistered() = ok;
        return ok;
    }

    static void RemoveRawInputRegistration()
    {
        if (!IsRawInputRegistered())
            return;

        RAWINPUTDEVICE device{};
        device.usUsagePage = 0x01;
        device.usUsage = 0x02;
        device.dwFlags = RIDEV_REMOVE;
        device.hwndTarget = nullptr;
        ::RegisterRawInputDevices(&device, 1, sizeof(device));
        IsRawInputRegistered() = false;
    }

    static void CleanupTargets()
    {
        auto& targets = Targets();
        targets.erase(
            std::remove_if(
                targets.begin(),
                targets.end(),
                [](const TargetInfo& target)
                {
                    return !::IsWindow(target.taskbar_hwnd) ||
                           !::IsWindow(target.target_hwnd);
                }),
            targets.end());
    }

    static bool PointInRectInclusiveExclusive(const RECT& rect, const POINT& point)
    {
        return point.x >= rect.left && point.x < rect.right &&
               point.y >= rect.top && point.y < rect.bottom;
    }

    static LONG MaxLong(LONG a, LONG b)
    {
        return a > b ? a : b;
    }

    static LONG MinLong(LONG a, LONG b)
    {
        return a < b ? a : b;
    }

    static bool IsTaskbarActuallyUnderPoint(HWND taskbar_hwnd, const POINT& point)
    {
        HWND hit = ::WindowFromPoint(point);
        if (hit == nullptr)
            return false;

        return hit == taskbar_hwnd || ::IsChild(taskbar_hwnd, hit) != FALSE;
    }

    static bool IsPointInGutter(const TargetInfo& target, const POINT& point)
    {
        if (!::IsWindowVisible(target.taskbar_hwnd) ||
            !::IsWindowVisible(target.target_hwnd))
            return false;

        RECT taskbar_rect{};
        RECT target_rect{};
        if (!::GetWindowRect(target.taskbar_hwnd, &taskbar_rect) ||
            !::GetWindowRect(target.target_hwnd, &target_rect))
            return false;

        if (!PointInRectInclusiveExclusive(taskbar_rect, point))
            return false;

        if (!IsTaskbarActuallyUnderPoint(target.taskbar_hwnd, point))
            return false;

        if (target.horizontal_taskbar)
        {
            // Parallel axis: exactly TrafficMonitor's current REAL width,
            // clamped to the current taskbar. Perpendicular axis: taskbar
            // margin only, excluding the real TrafficMonitor HWND itself.
            const LONG left = MaxLong(taskbar_rect.left, target_rect.left);
            const LONG right = MinLong(taskbar_rect.right, target_rect.right);
            if (right <= left || point.x < left || point.x >= right)
                return false;

            return point.y < target_rect.top || point.y >= target_rect.bottom;
        }

        const LONG top = MaxLong(taskbar_rect.top, target_rect.top);
        const LONG bottom = MinLong(taskbar_rect.bottom, target_rect.bottom);
        if (bottom <= top || point.y < top || point.y >= bottom)
            return false;

        return point.x < target_rect.left || point.x >= target_rect.right;
    }

    static HWND FindGutterTarget(const POINT& point)
    {
        CleanupTargets();

        for (const auto& target : Targets())
        {
            if (IsPointInGutter(target, point))
                return target.target_hwnd;
        }

        return nullptr;
    }

    static UINT CurrentMouseKeyState()
    {
        UINT state{};
        if ((::GetKeyState(VK_LBUTTON) & 0x8000) != 0)
            state |= MK_LBUTTON;
        if ((::GetKeyState(VK_RBUTTON) & 0x8000) != 0)
            state |= MK_RBUTTON;
        if ((::GetKeyState(VK_MBUTTON) & 0x8000) != 0)
            state |= MK_MBUTTON;
        if ((::GetKeyState(VK_SHIFT) & 0x8000) != 0)
            state |= MK_SHIFT;
        if ((::GetKeyState(VK_CONTROL) & 0x8000) != 0)
            state |= MK_CONTROL;
        return state;
    }

    static void ForwardWheel(HWND target_hwnd, SHORT wheel_delta, const POINT& point)
    {
        if (!::IsWindow(target_hwnd) || wheel_delta == 0)
            return;

        // Do nothing in the gutter when the feature is disabled. This avoids
        // creating an extra default WM_MOUSEWHEEL path that did not exist
        // before this feature.
        if (!CTaskbarVolumeControl::IsEnabled())
            return;

        const WPARAM w_param = MAKEWPARAM(
            CurrentMouseKeyState(),
            static_cast<WORD>(wheel_delta));
        const LPARAM l_param = MAKELPARAM(
            static_cast<SHORT>(point.x),
            static_cast<SHORT>(point.y));

        // WM_MOUSEWHEEL uses screen coordinates. The existing derived
        // taskbar handler converts them to client coordinates, sees that the
        // point is outside all plugin item rectangles, then reuses the already
        // tested WM_APPCOMMAND volume path.
        ::SendMessageW(target_hwnd, WM_MOUSEWHEEL, w_param, l_param);
    }

    static void ResetDoubleClickState()
    {
        LastClickTime() = 0;
        LastClickPoint() = POINT{};
        LastClickTarget() = nullptr;
    }

    static void ProcessLeftButtonDown(HWND target_hwnd, const POINT& point)
    {
        const DWORD now = ::GetTickCount();
        const int half_width = max(1, ::GetSystemMetrics(SM_CXDOUBLECLK) / 2);
        const int half_height = max(1, ::GetSystemMetrics(SM_CYDOUBLECLK) / 2);

        const bool same_target = LastClickTarget() == target_hwnd;
        const bool within_time = LastClickTime() != 0 &&
            static_cast<DWORD>(now - LastClickTime()) <= ::GetDoubleClickTime();
        const bool within_position =
            abs(point.x - LastClickPoint().x) <= half_width &&
            abs(point.y - LastClickPoint().y) <= half_height;

        if (same_target && within_time && within_position)
        {
            POINT client_point = point;
            ::ScreenToClient(target_hwnd, &client_point);

            // Intentionally forward a client coordinate that can be outside
            // the real HWND. OnLButtonDblClk() then finds no plugin item and
            // executes the user's existing generic double-click action.
            const LPARAM l_param = MAKELPARAM(
                static_cast<SHORT>(client_point.x),
                static_cast<SHORT>(client_point.y));
            ::SendMessageW(
                target_hwnd,
                WM_LBUTTONDBLCLK,
                CurrentMouseKeyState(),
                l_param);

            ResetDoubleClickState();
            return;
        }

        LastClickTime() = now;
        LastClickPoint() = point;
        LastClickTarget() = target_hwnd;
    }

    static void HandleRawInput(HRAWINPUT raw_input_handle)
    {
        RAWINPUT raw{};
        UINT size = sizeof(raw);
        const UINT result = ::GetRawInputData(
            raw_input_handle,
            RID_INPUT,
            &raw,
            &size,
            sizeof(RAWINPUTHEADER));

        if (result == static_cast<UINT>(-1) || raw.header.dwType != RIM_TYPEMOUSE)
            return;

        const USHORT button_flags = raw.data.mouse.usButtonFlags;
        if ((button_flags & (RI_MOUSE_WHEEL | RI_MOUSE_LEFT_BUTTON_DOWN)) == 0)
            return;

        POINT point{};
        if (!::GetCursorPos(&point))
            return;

        HWND target_hwnd = FindGutterTarget(point);
        if (target_hwnd == nullptr)
        {
            if ((button_flags & RI_MOUSE_LEFT_BUTTON_DOWN) != 0)
                ResetDoubleClickState();
            return;
        }

        if ((button_flags & RI_MOUSE_WHEEL) != 0)
        {
            const SHORT wheel_delta = static_cast<SHORT>(raw.data.mouse.usButtonData);
            ForwardWheel(target_hwnd, wheel_delta, point);
        }

        if ((button_flags & RI_MOUSE_LEFT_BUTTON_DOWN) != 0)
            ProcessLeftButtonDown(target_hwnd, point);
    }

    static LRESULT CALLBACK SinkWindowProc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
    {
        if (message == WM_INPUT)
        {
            HandleRawInput(reinterpret_cast<HRAWINPUT>(l_param));

            // Required by the Raw Input contract for foreground RIM_INPUT
            // cleanup; harmless for RIM_INPUTSINK delivery as well.
            return ::DefWindowProcW(hwnd, message, w_param, l_param);
        }

        return ::DefWindowProcW(hwnd, message, w_param, l_param);
    }
};
