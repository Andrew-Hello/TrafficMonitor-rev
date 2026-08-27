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

    // Accumulates high-resolution wheel deltas and emits one native Windows
    // volume media-key press for each complete WHEEL_DELTA step.
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

        const WORD volume_key = steps > 0 ? VK_VOLUME_UP : VK_VOLUME_DOWN;
        int step_count = steps > 0 ? steps : -steps;

        // zDelta is a short, so this is mainly a guard against malformed input
        // causing an excessive burst of synthetic key events.
        if (step_count > 16)
            step_count = 16;

        for (int i = 0; i < step_count; ++i)
            SendVolumeKey(volume_key);

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

    static bool SendVolumeKey(WORD virtual_key)
    {
        INPUT input[2]{};

        input[0].type = INPUT_KEYBOARD;
        input[0].ki.wVk = virtual_key;

        input[1].type = INPUT_KEYBOARD;
        input[1].ki.wVk = virtual_key;
        input[1].ki.dwFlags = KEYEVENTF_KEYUP;

        return ::SendInput(2, input, sizeof(INPUT)) == 2;
    }
};
