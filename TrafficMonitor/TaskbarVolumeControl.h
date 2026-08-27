#pragma once

#include "SettingsHelper.h"

// Handles system volume adjustment from mouse wheel messages received by
// TrafficMonitor's own taskbar window. This intentionally does not install
// any global hook and does not inject code into Explorer.
class CTaskbarVolumeControl final
{
public:
    static bool IsEnabled()
    {
        return EnabledCache();
    }

    static void SetEnabled(bool enabled)
    {
        EnabledCache() = enabled;

        CSettingsHelper ini;
        ini.WriteBool(L"task_bar", L"volume_control_by_mouse_wheel", enabled);
        ini.Save();
    }

    // Accumulates high-resolution wheel deltas and emits one Windows shell
    // application command for each complete WHEEL_DELTA step.
    //
    // WM_APPCOMMAND is deliberately sent to TrafficMonitor's own top-level
    // window rather than to the current foreground window. If the application
    // does not handle the command, DefWindowProc forwards it to the Windows
    // shell hook (HSHELL_APPCOMMAND). This avoids SendInput and therefore does
    // not depend on the integrity level of the current foreground process.
    //
    // Returns true whenever the feature is enabled, meaning the wheel message
    // should be consumed by the TrafficMonitor taskbar window.
    static bool HandleMouseWheel(short zDelta, int& accumulated_delta)
    {
        if (!IsEnabled())
        {
            accumulated_delta = 0;
            return false;
        }

        if (zDelta == 0)
            return true;

        // A direction reversal starts a new gesture. This avoids a small
        // opposite-direction wheel movement being cancelled by old residue.
        if ((zDelta > 0 && accumulated_delta < 0) ||
            (zDelta < 0 && accumulated_delta > 0))
        {
            accumulated_delta = 0;
        }

        accumulated_delta += zDelta;
        const int steps = accumulated_delta / WHEEL_DELTA;
        if (steps == 0)
            return true;

        accumulated_delta -= steps * WHEEL_DELTA;

        const int app_command = steps > 0 ? APPCOMMAND_VOLUME_UP : APPCOMMAND_VOLUME_DOWN;
        int step_count = steps > 0 ? steps : -steps;

        // zDelta is a short, so this is mainly a guard against malformed input
        // causing an excessive burst of shell application commands.
        if (step_count > 16)
            step_count = 16;

        for (int i = 0; i < step_count; ++i)
            SendVolumeAppCommand(app_command);

        return true;
    }

private:
    static bool& EnabledCache()
    {
        static bool enabled = LoadEnabled();
        return enabled;
    }

    static bool LoadEnabled()
    {
        CSettingsHelper ini;
        return ini.GetBool(L"task_bar", L"volume_control_by_mouse_wheel", false);
    }

    static bool SendVolumeAppCommand(int app_command)
    {
        CWnd* main_window = AfxGetMainWnd();
        if (main_window == nullptr)
            return false;

        HWND hwnd = main_window->GetSafeHwnd();
        if (hwnd == nullptr || !::IsWindow(hwnd))
            return false;

        const LPARAM command_lparam = static_cast<LPARAM>(app_command) << 16;
        ::SendMessage(hwnd, WM_APPCOMMAND, reinterpret_cast<WPARAM>(hwnd), command_lparam);
        return true;
    }
};
