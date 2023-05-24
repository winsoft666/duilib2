/*******************************************************************************
* Copyright (C) 2018 - 2020, winsoft666, <winsoft666@outlook.com>.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
*
* Expect bugs
*
* Please use and enjoy. Please let me know of any bugs/improvements
* that you have found/implemented and I will fix/incorporate them into this
* file.
*******************************************************************************/
#ifdef UILIB_WITH_CEF
#include "CefUtil.h"
#include "include/base/cef_logging.h"

namespace DuiLib {
    namespace Internal {
        namespace {
            LARGE_INTEGER qi_freq_ = {};
        }  // namespace

        uint64_t GetTimeNow() {
            if (!qi_freq_.HighPart && !qi_freq_.LowPart) {
                QueryPerformanceFrequency(&qi_freq_);
            }
            LARGE_INTEGER t = {};
            QueryPerformanceCounter(&t);
            return static_cast<uint64_t>((t.QuadPart / double(qi_freq_.QuadPart)) *
                                         1000000);
        }

        void SetUserDataPtr(HWND hWnd, void *ptr) {
            SetLastError(ERROR_SUCCESS);
            LONG_PTR result =
                ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptr));
            CHECK(result != 0 || GetLastError() == ERROR_SUCCESS);
        }

        WNDPROC SetWndProcPtr(HWND hWnd, WNDPROC wndProc) {
            WNDPROC old =
                reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hWnd, GWLP_WNDPROC));
            CHECK(old != NULL);
            LONG_PTR result = ::SetWindowLongPtr(hWnd, GWLP_WNDPROC,
                                                 reinterpret_cast<LONG_PTR>(wndProc));
            CHECK(result != 0 || GetLastError() == ERROR_SUCCESS);
            return old;
        }

        int GetCefMouseModifiers(WPARAM wparam) {
            int modifiers = 0;
            if (wparam & MK_CONTROL)
                modifiers |= EVENTFLAG_CONTROL_DOWN;
            if (wparam & MK_SHIFT)
                modifiers |= EVENTFLAG_SHIFT_DOWN;
            if (IsKeyDown(VK_MENU))
                modifiers |= EVENTFLAG_ALT_DOWN;
            if (wparam & MK_LBUTTON)
                modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
            if (wparam & MK_MBUTTON)
                modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
            if (wparam & MK_RBUTTON)
                modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;

            // Low bit set from GetKeyState indicates "toggled".
            if (::GetKeyState(VK_NUMLOCK) & 1)
                modifiers |= EVENTFLAG_NUM_LOCK_ON;
            if (::GetKeyState(VK_CAPITAL) & 1)
                modifiers |= EVENTFLAG_CAPS_LOCK_ON;
            return modifiers;
        }

        int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) {
            int modifiers = 0;
            if (IsKeyDown(VK_SHIFT))
                modifiers |= EVENTFLAG_SHIFT_DOWN;
            if (IsKeyDown(VK_CONTROL))
                modifiers |= EVENTFLAG_CONTROL_DOWN;
            if (IsKeyDown(VK_MENU))
                modifiers |= EVENTFLAG_ALT_DOWN;

            // Low bit set from GetKeyState indicates "toggled".
            if (::GetKeyState(VK_NUMLOCK) & 1)
                modifiers |= EVENTFLAG_NUM_LOCK_ON;
            if (::GetKeyState(VK_CAPITAL) & 1)
                modifiers |= EVENTFLAG_CAPS_LOCK_ON;

            switch (wparam) {
                case VK_RETURN:
                    if ((lparam >> 16) & KF_EXTENDED)
                        modifiers |= EVENTFLAG_IS_KEY_PAD;
                    break;
                case VK_INSERT:
                case VK_DELETE:
                case VK_HOME:
                case VK_END:
                case VK_PRIOR:
                case VK_NEXT:
                case VK_UP:
                case VK_DOWN:
                case VK_LEFT:
                case VK_RIGHT:
                    if (!((lparam >> 16) & KF_EXTENDED))
                        modifiers |= EVENTFLAG_IS_KEY_PAD;
                    break;
                case VK_NUMLOCK:
                case VK_NUMPAD0:
                case VK_NUMPAD1:
                case VK_NUMPAD2:
                case VK_NUMPAD3:
                case VK_NUMPAD4:
                case VK_NUMPAD5:
                case VK_NUMPAD6:
                case VK_NUMPAD7:
                case VK_NUMPAD8:
                case VK_NUMPAD9:
                case VK_DIVIDE:
                case VK_MULTIPLY:
                case VK_SUBTRACT:
                case VK_ADD:
                case VK_DECIMAL:
                case VK_CLEAR:
                    modifiers |= EVENTFLAG_IS_KEY_PAD;
                    break;
                case VK_SHIFT:
                    if (IsKeyDown(VK_LSHIFT))
                        modifiers |= EVENTFLAG_IS_LEFT;
                    else if (IsKeyDown(VK_RSHIFT))
                        modifiers |= EVENTFLAG_IS_RIGHT;
                    break;
                case VK_CONTROL:
                    if (IsKeyDown(VK_LCONTROL))
                        modifiers |= EVENTFLAG_IS_LEFT;
                    else if (IsKeyDown(VK_RCONTROL))
                        modifiers |= EVENTFLAG_IS_RIGHT;
                    break;
                case VK_MENU:
                    if (IsKeyDown(VK_LMENU))
                        modifiers |= EVENTFLAG_IS_LEFT;
                    else if (IsKeyDown(VK_RMENU))
                        modifiers |= EVENTFLAG_IS_RIGHT;
                    break;
                case VK_LWIN:
                    modifiers |= EVENTFLAG_IS_LEFT;
                    break;
                case VK_RWIN:
                    modifiers |= EVENTFLAG_IS_RIGHT;
                    break;
            }
            return modifiers;
        }

        bool IsKeyDown(WPARAM wparam) {
            return (GetKeyState(wparam) & 0x8000) != 0;
        }

        float GetDeviceScaleFactor() {
            static float scale_factor = 1.0;
            static bool initialized = false;

            if (!initialized) {
                // This value is safe to cache for the life time of the app since the user
                // must logout to change the DPI setting. This value also applies to all
                // screens.
                HDC screen_dc = ::GetDC(NULL);
                int dpi_x = GetDeviceCaps(screen_dc, LOGPIXELSX);
                scale_factor = static_cast<float>(dpi_x) / 96.0f;
                ::ReleaseDC(NULL, screen_dc);
                initialized = true;
            }

            return scale_factor;
        }


        int LogicalToDevice(int value, float device_scale_factor) {
            float scaled_val = static_cast<float>(value) * device_scale_factor;
            return static_cast<int>(std::floor(scaled_val));
        }

        CefRect LogicalToDevice(const CefRect &value, float device_scale_factor) {
            return CefRect(LogicalToDevice(value.x, device_scale_factor),
                           LogicalToDevice(value.y, device_scale_factor),
                           LogicalToDevice(value.width, device_scale_factor),
                           LogicalToDevice(value.height, device_scale_factor));
        }

        int DeviceToLogical(int value, float device_scale_factor) {
            float scaled_val = static_cast<float>(value) / device_scale_factor;
            return static_cast<int>(std::floor(scaled_val));
        }

        void DeviceToLogical(CefMouseEvent &value, float device_scale_factor) {
            value.x = DeviceToLogical(value.x, device_scale_factor);
            value.y = DeviceToLogical(value.y, device_scale_factor);
        }

        CefRefPtr<CefV8Value> CefValueToCefV8Value(CefRefPtr<CefValue> value) {
            CefRefPtr<CefV8Value> result;
            switch (value->GetType()) {
                case VTYPE_INVALID: {
                    result = CefV8Value::CreateNull();
                }
                break;
                case VTYPE_NULL: {
                    result = CefV8Value::CreateNull();
                }
                break;
                case VTYPE_BOOL: {
                    result = CefV8Value::CreateBool(value->GetBool());
                }
                break;
                case VTYPE_INT: {
                    result = CefV8Value::CreateInt(value->GetInt());
                }
                break;
                case VTYPE_DOUBLE: {
                    result = CefV8Value::CreateDouble(value->GetDouble());
                }
                break;
                case VTYPE_STRING: {
                    result = CefV8Value::CreateString(value->GetString());
                }
                break;
                case VTYPE_BINARY: {
                    result = CefV8Value::CreateNull();
                }
                break;
                case VTYPE_DICTIONARY: {
#if CEFVER == 3626
                    result = CefV8Value::CreateObject(NULL, NULL);
#else
                    result = CefV8Value::CreateObject(NULL);
#endif
                    CefRefPtr<CefDictionaryValue> dict = value->GetDictionary();
                    CefDictionaryValue::KeyList keys;
                    dict->GetKeys(keys);
                    for (unsigned int i = 0; i < keys.size(); i++) {
                        CefString key = keys[i];
                        result->SetValue(key, CefValueToCefV8Value(dict->GetValue(key)), V8_PROPERTY_ATTRIBUTE_NONE);
                    }
                }
                break;
                case VTYPE_LIST: {
                    CefRefPtr<CefListValue> list = value->GetList();
                    int size = list->GetSize();
                    result = CefV8Value::CreateArray(size);
                    for (int i = 0; i < size; i++) {
                        result->SetValue(i, CefValueToCefV8Value(list->GetValue(i)));
                    }
                }
                break;
            }
            return result;
        }

        CefRefPtr<CefValue> CefV8ValueToCefValue(CefRefPtr<CefV8Value> value) {
            CefRefPtr<CefValue> result = CefValue::Create();

            if (!value->IsValid() || value->IsNull()) {
                result->SetNull();
            } else if (value->IsBool()) {
                result->SetBool(value->GetBoolValue());
            } else if (value->IsInt()) {
                result->SetInt(value->GetIntValue());
            } else if (value->IsDouble()) {
                result->SetDouble(value->GetDoubleValue());
            } else if (value->IsString()) {
                result->SetString(value->GetStringValue());
            }


            return result;
        }
    }
}
#endif