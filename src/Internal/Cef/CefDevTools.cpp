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
#include "StdAfx.h"
#include "CefDevTools.h"
#include "CefUtil.h"

namespace DuiLib {
    namespace Internal {
        CefDevToolsWnd::CefDevToolsWnd(CefRefPtr<CefBrowser> targetBrowser, float fScaleFactor) :
            m_pTargetBrowser(targetBrowser)
            , m_iViewMemoryBitmapWidth(0)
            , m_iViewMemoryBitmapHeight(0)
            , m_iPopupMemoryBitmapWidth(0)
            , m_iPopupMemoryBitmapHeight(0)
            , m_pViewBuffer(NULL)
            , m_hViewBitmap(NULL)
            , m_hViewMemoryDC(NULL)
            , m_pPopupBuffer(NULL)
            , m_hPopupBitmap(NULL)
            , m_hPopupMemoryDC(NULL)
            , m_bClosed(false)
            , m_fScaleFactor(fScaleFactor) {
            m_hViewMemoryDC = CreateCompatibleDC(NULL);
            m_hPopupMemoryDC = CreateCompatibleDC(NULL);
        }

        CefDevToolsWnd::~CefDevToolsWnd() {
            if (m_hViewMemoryDC) {
                DeleteDC(m_hViewMemoryDC);
                m_hViewMemoryDC = NULL;
            }

            if (m_hPopupMemoryDC) {
                DeleteDC(m_hPopupMemoryDC);
                m_hPopupMemoryDC = NULL;
            }
        }

        UINT CefDevToolsWnd::GetClassStyle() const {
            return CS_HREDRAW | CS_VREDRAW;
        }

        LPCTSTR CefDevToolsWnd::GetWindowClassName() const {
            return TEXT("CefDevToolsWnd891$322@");
        }

        void CefDevToolsWnd::OnFinalMessage(HWND hWnd) {
            CWindowWnd::OnFinalMessage(hWnd);
            delete this;
        }

        LRESULT CefDevToolsWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
            switch (uMsg) {
                case WM_CREATE: {
                    if (m_pTargetBrowser) {
                        CefBrowserSettings browser_settings;
                        browser_settings.windowless_frame_rate = 30;

                        CefWindowInfo windowInfo;
                        windowInfo.SetAsWindowless(m_hWnd, false);
                        windowInfo.ex_style |= WS_EX_NOACTIVATE;

                        m_ClientHandler = new ClientHandlerOsr(this);

                        m_pTargetBrowser->GetHost()->ShowDevTools(windowInfo, m_ClientHandler,
                                browser_settings, CefPoint());
                    }
                    break;
                }
                case WM_PAINT: {
                    PAINTSTRUCT ps;
                    BeginPaint(m_hWnd, &ps);

                    RECT controlRC;
                    GetClientRect(m_hWnd, &controlRC);

                    m_csViewBuf.Enter();
                    ASSERT(m_hViewMemoryDC);
                    BitBlt(ps.hdc, controlRC.left, controlRC.top,
                           controlRC.right - controlRC.left, controlRC.bottom - controlRC.top,
                           m_hViewMemoryDC, 0, 0, SRCCOPY);
                    m_csViewBuf.Leave();

                    if (!m_PopupRect.IsEmpty()) {
                        m_csPopupBuf.Enter();
                        BitBlt(ps.hdc, m_PopupRect.x + controlRC.left, m_PopupRect.y + controlRC.top,
                               m_PopupRect.width, m_PopupRect.height,
                               m_hPopupMemoryDC, 0, 0, SRCCOPY);
                        m_csPopupBuf.Leave();
                    }
                    EndPaint(m_hWnd, &ps);

                    if (!m_PopupRect.IsEmpty()) {
                        m_browser->GetHost()->Invalidate(PET_POPUP);
                    }
                    break;
                }
                case WM_SYSCHAR:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_CHAR: {
                    OnKeyEvent(uMsg, wParam, lParam);
                    break;
                }
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_MOUSEMOVE:
                case WM_MOUSELEAVE:
                case WM_MOUSEWHEEL: {
                    OnMouseEvent(uMsg, wParam, lParam);
                    break;
                }
                case WM_SIZE: {
                    OnSize();
                    break;
                }
                case WM_SETFOCUS:
                case WM_KILLFOCUS: {
                    OnFocus(uMsg == WM_SETFOCUS);
                    break;
                }
                case WM_DESTROY: {
                    if (m_browser) {
                        m_bClosed = true;
                        m_browser->GetHost()->CloseBrowser(true);
                    }

                    break;
                }
                case WM_MOVING:
                case WM_MOVE: {
                    if (m_browser && m_browser->GetHost())
                        m_browser->GetHost()->NotifyMoveOrResizeStarted();
                    return 0;
                }
                case WM_CLOSE:
                {
                  if (!m_bClosed) {
                    if (m_browser && m_browser->GetHost()) {
                      m_browser->GetHost()->CloseBrowser(false);
                    }
                    return 0;
                  }
                  break;
                }
                default:
                    break;
            }
            return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
        }


        void CefDevToolsWnd::OnBeforeContextMenu(CefRefPtr<CefMenuModel> model) {

        }

        //////////////////////////////////////////////////////////////////////////
        // ClientHandlerOsr::OsrDelegate methods.
        void CefDevToolsWnd::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
            m_browser = browser;
        }

        void CefDevToolsWnd::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
            if (m_ClientHandler)
                m_ClientHandler->DetachDelegate();
            m_browser = nullptr;
            ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        }

        bool CefDevToolsWnd::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect &rect) {
            CEF_REQUIRE_UI_THREAD();
            return false;
        }
#if CEFVER == 3626
        void CefDevToolsWnd::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect);
#else
        bool CefDevToolsWnd::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) {
            CEF_REQUIRE_UI_THREAD();
            if (m_bClosed)
                return false;
            RECT pos;
            GetClientRect(m_hWnd, &pos);
            int width = pos.right - pos.left;
            int height = pos.bottom - pos.top;

            rect.x = 0;
            rect.y = 0;
            rect.width = DeviceToLogical(width, m_fScaleFactor);
            if (rect.width == 0)
                rect.width = 1;

            rect.height = DeviceToLogical(height, m_fScaleFactor);
            if (rect.height == 0)
                rect.height = 1;
            return true;
        }
#endif
        bool CefDevToolsWnd::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int &screenX, int &screenY) {
            CEF_REQUIRE_UI_THREAD();
            if (m_bClosed)
                return false;
            // Convert the point from view coordinates to actual screen coordinates.
            POINT screen_pt = { Internal::LogicalToDevice(viewX, m_fScaleFactor),
                                Internal::LogicalToDevice(viewY, m_fScaleFactor)
                              };
            ClientToScreen(m_hWnd, &screen_pt);
            screenX = screen_pt.x;
            screenY = screen_pt.y;
            return true;
        }

        bool CefDevToolsWnd::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo &screen_info) {
            CEF_REQUIRE_UI_THREAD();
            if (m_bClosed)
                return false;
            CefRect view_rect;
            GetViewRect(browser, view_rect);

            screen_info.device_scale_factor = m_fScaleFactor;
            screen_info.rect = view_rect;
            screen_info.available_rect = view_rect;
            return true;
        }

        void CefDevToolsWnd::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) {
            if (m_bClosed)
                return;
            if (!show) {
                m_PopupRect.Reset();
                browser->GetHost()->Invalidate(PET_VIEW);
            }
        }

        void CefDevToolsWnd::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect &rect) {
            if (m_bClosed)
                return;
            CefRect newRect = Internal::LogicalToDevice(rect, m_fScaleFactor);
            if (newRect.width <= 0 || newRect.height <= 0)
                return;
            m_OriginPopupRect = newRect;
            m_PopupRect = GetPopupRectInWebView(newRect);
        }

        CefRect CefDevToolsWnd::GetPopupRectInWebView(const CefRect &original_rect) {
            CefRect rc(original_rect);
            // if x or y are negative, move them to 0.
            if (rc.x < 0)
                rc.x = 0;
            if (rc.y < 0)
                rc.y = 0;
            // if popup goes outside the view, try to reposition origin
            if (rc.x + rc.width > m_iViewWidth)
                rc.x = m_iViewWidth - rc.width;
            if (rc.y + rc.height > m_iViewHeight)
                rc.y = m_iViewHeight - rc.height;
            // if x or y became negative, move them to 0 again.
            if (rc.x < 0)
                rc.x = 0;
            if (rc.y < 0)
                rc.y = 0;
            return rc;
        }

        bool CefDevToolsWnd::IsOverPopupWidget(int x, int y) const {
            const CefRect &rc = m_PopupRect;
            int popup_right = rc.x + rc.width;
            int popup_bottom = rc.y + rc.height;
            return (x >= rc.x) && (x < popup_right) &&
                   (y >= rc.y) && (y < popup_bottom);
        }

        int CefDevToolsWnd::GetPopupXOffset() const {
            return m_OriginPopupRect.x - m_PopupRect.x;
        }

        int CefDevToolsWnd::GetPopupYOffset() const {
            return m_OriginPopupRect.y - m_PopupRect.y;
        }

        void CefDevToolsWnd::ApplyPopupOffset(int &x, int &y) const {
            if (IsOverPopupWidget(x, y)) {
                x += GetPopupXOffset();
                y += GetPopupYOffset();
            }
        }

        void CefDevToolsWnd::OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type,
                                     const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height) {
            if (m_bClosed)
                return;
            if (type == CefRenderHandler::PaintElementType::PET_VIEW) {
                m_csViewBuf.Enter();

                m_iViewWidth = width;
                m_iViewHeight = height;

                if (width != m_iViewMemoryBitmapWidth || height != m_iViewMemoryBitmapHeight) {
                    BITMAPINFO bi;
                    memset(&bi, 0, sizeof(bi));
                    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    bi.bmiHeader.biWidth = int(width);
                    bi.bmiHeader.biHeight = -int(height);
                    bi.bmiHeader.biPlanes = 1;
                    bi.bmiHeader.biBitCount = 32;
                    bi.bmiHeader.biCompression = BI_RGB;

                    HBITMAP hbmp = ::CreateDIBSection(0, &bi, DIB_RGB_COLORS, &m_pViewBuffer, NULL, 0);
                    ::SelectObject(m_hViewMemoryDC, hbmp);
                    if (m_hViewBitmap)
                        ::DeleteObject(m_hViewBitmap);

                    m_hViewBitmap = hbmp;

                    m_iViewMemoryBitmapWidth = width;
                    m_iViewMemoryBitmapHeight = height;
                }

                memcpy(m_pViewBuffer, buffer, width * height * 4);
                m_csViewBuf.Leave();
            } else if (type == CefRenderHandler::PaintElementType::PET_POPUP) {
                m_csPopupBuf.Enter();

                if (width != m_iPopupMemoryBitmapWidth || height != m_iPopupMemoryBitmapHeight) {
                    BITMAPINFO bi;
                    memset(&bi, 0, sizeof(bi));
                    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    bi.bmiHeader.biWidth = int(width);
                    bi.bmiHeader.biHeight = -int(height);
                    bi.bmiHeader.biPlanes = 1;
                    bi.bmiHeader.biBitCount = 32;
                    bi.bmiHeader.biCompression = BI_RGB;

                    HBITMAP hbmp = ::CreateDIBSection(0, &bi, DIB_RGB_COLORS, &m_pPopupBuffer, NULL, 0);
                    ::SelectObject(m_hPopupMemoryDC, hbmp);
                    if (m_hPopupBitmap)
                        ::DeleteObject(m_hPopupBitmap);

                    m_hPopupBitmap = hbmp;

                    m_iPopupMemoryBitmapWidth = width;
                    m_iPopupMemoryBitmapHeight = height;
                }
                memcpy(m_pPopupBuffer, buffer, width * height * 4);
                m_csPopupBuf.Leave();
            }

            RECT rc;
            GetClientRect(m_hWnd, &rc);
            ::InvalidateRect(m_hWnd, &rc, FALSE);
        }

        void CefDevToolsWnd::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                                CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList &dirtyRects, void *share_handle) {
        }

        void CefDevToolsWnd::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CefRenderHandler::CursorType type, const CefCursorInfo &custom_cursor_info) {
            CEF_REQUIRE_UI_THREAD();
            if (!m_hWnd || !::IsWindow(m_hWnd))
                return;

            // Change the plugin window's cursor.
            SetClassLongPtr(m_hWnd, GCLP_HCURSOR, static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
            ::SetCursor(cursor);
        }

        bool CefDevToolsWnd::StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y) {
            CEF_REQUIRE_UI_THREAD();
            // Cancel the drag. The dragging implementation requires ATL support.
            return false;
        }

        void CefDevToolsWnd::UpdateDragCursor(CefRefPtr<CefBrowser> browser,
                                              CefRenderHandler::DragOperation operation) {
        }
#if CEFVER == 3626
        void OnImeCompositionRangeChanged(
            CefRefPtr<CefBrowser> browser,
            const CefRange &selection_range,
            const CefRenderHandler::RectList &character_bounds);
#endif
        void CefDevToolsWnd::UpdateAccessibilityTree(CefRefPtr<CefValue> value) {
        }

        void CefDevToolsWnd::OnBrowserClosing(CefRefPtr<CefBrowser> browser) {
          m_bClosed = true;
        }

        void CefDevToolsWnd::OnSetAddress(const std::string &url) {
        }

        void CefDevToolsWnd::OnSetTitle(const std::string &title) {
        }


        bool CefDevToolsWnd::OnTooltip(CefRefPtr<CefBrowser> browser, CefString &text) {
            return false;
        }

        void CefDevToolsWnd::OnSetFullscreen(bool fullscreen) {
        }

        void CefDevToolsWnd::OnAutoResize(const CefSize &new_size) {
        }

        void CefDevToolsWnd::OnSetLoadingState(bool isLoading, bool canGoBack, bool canGoForward) {
        }

        void CefDevToolsWnd::OnSetDraggableRegions(const std::vector<CefDraggableRegion> &regions) {
        }

        void CefDevToolsWnd::OnJSNotify(const CefRefPtr<CefListValue> &value_list) {

        }

        bool CefDevToolsWnd::OnBeforePopup(const std::string &target_url) {
            return true;
        }

        void CefDevToolsWnd::OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) {
            if (m_bClosed)
                return;
            CefRefPtr<CefBrowserHost> browser_host;
            if (m_browser)
                browser_host = m_browser->GetHost();

            LONG currentTime = 0;
            bool cancelPreviousClick = false;

            if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
                    message == WM_MBUTTONDOWN || message == WM_MOUSEMOVE ||
                    message == WM_MOUSELEAVE) {
                currentTime = GetMessageTime();
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                cancelPreviousClick =
                    (abs(last_click_x_ - x) > (GetSystemMetrics(SM_CXDOUBLECLK) / 2)) ||
                    (abs(last_click_y_ - y) > (GetSystemMetrics(SM_CYDOUBLECLK) / 2)) ||
                    ((currentTime - last_click_time_) > GetDoubleClickTime());
                if (cancelPreviousClick &&
                        (message == WM_MOUSEMOVE || message == WM_MOUSELEAVE)) {
                    last_click_count_ = 0;
                    last_click_x_ = 0;
                    last_click_y_ = 0;
                    last_click_time_ = 0;
                }
            }

            switch (message) {
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN: {
                    ::SetCapture(m_hWnd);
                    ::SetFocus(m_hWnd);
                    int x = GET_X_LPARAM(lParam);
                    int y = GET_Y_LPARAM(lParam);
                    if (wParam & MK_SHIFT) {
                        // Start rotation effect.
                        last_mouse_pos_.x = current_mouse_pos_.x = x;
                        last_mouse_pos_.y = current_mouse_pos_.y = y;
                        mouse_rotation_ = true;
                    } else {
                        CefBrowserHost::MouseButtonType btnType =
                            (message == WM_LBUTTONDOWN
                             ? MBT_LEFT
                             : (message == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
                        if (!cancelPreviousClick && (btnType == last_click_button_)) {
                            ++last_click_count_;
                        } else {
                            last_click_count_ = 1;
                            last_click_x_ = x;
                            last_click_y_ = y;
                        }
                        last_click_time_ = currentTime;
                        last_click_button_ = btnType;

                        if (browser_host) {
                            CefMouseEvent mouse_event;
                            mouse_event.x = x;
                            mouse_event.y = y;
                            last_mouse_down_on_view_ = !IsOverPopupWidget(x, y);
                            ApplyPopupOffset(mouse_event.x, mouse_event.y);
                            Internal::DeviceToLogical(mouse_event, m_fScaleFactor);
                            mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
                            browser_host->SendMouseClickEvent(mouse_event, btnType, false,
                                                              last_click_count_);
                        }
                    }
                }
                break;

                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                    if (GetCapture() == m_hWnd)
                        ReleaseCapture();
                    if (mouse_rotation_) {
                        // End rotation effect.
                        mouse_rotation_ = false;
                    } else {
                        int x = GET_X_LPARAM(lParam);
                        int y = GET_Y_LPARAM(lParam);
                        CefBrowserHost::MouseButtonType btnType =
                            (message == WM_LBUTTONUP
                             ? MBT_LEFT
                             : (message == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));
                        if (browser_host) {
                            CefMouseEvent mouse_event;
                            mouse_event.x = x;
                            mouse_event.y = y;
                            if (last_mouse_down_on_view_ && IsOverPopupWidget(x, y) &&
                                    (GetPopupXOffset() || GetPopupYOffset())) {
                                break;
                            }
                            ApplyPopupOffset(mouse_event.x, mouse_event.y);
                            Internal::DeviceToLogical(mouse_event, m_fScaleFactor);
                            mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
                            browser_host->SendMouseClickEvent(mouse_event, btnType, true,
                                                              last_click_count_);
                        }
                    }
                    break;

                case WM_MOUSEMOVE: {
                    int x = GET_X_LPARAM(lParam);
                    int y = GET_Y_LPARAM(lParam);
                    if (mouse_rotation_) {
                        // Apply rotation effect.
                        current_mouse_pos_.x = x;
                        current_mouse_pos_.y = y;
                        last_mouse_pos_.x = current_mouse_pos_.x;
                        last_mouse_pos_.y = current_mouse_pos_.y;
                    } else {
                        if (!mouse_tracking_) {
                            // Start tracking mouse leave. Required for the WM_MOUSELEAVE event to be generated.
                            TRACKMOUSEEVENT tme;
                            tme.cbSize = sizeof(TRACKMOUSEEVENT);
                            tme.dwFlags = TME_LEAVE;
                            tme.hwndTrack = m_hWnd;
                            TrackMouseEvent(&tme);
                            mouse_tracking_ = true;
                        }

                        if (browser_host) {
                            CefMouseEvent mouse_event;
                            mouse_event.x = x;
                            mouse_event.y = y;
                            ApplyPopupOffset(mouse_event.x, mouse_event.y);
                            Internal::DeviceToLogical(mouse_event, m_fScaleFactor);
                            mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
                            browser_host->SendMouseMoveEvent(mouse_event, false);
                        }
                    }
                    break;
                }

                case WM_MOUSELEAVE: {
                    if (mouse_tracking_) {
                        // Stop tracking mouse leave.
                        TRACKMOUSEEVENT tme;
                        tme.cbSize = sizeof(TRACKMOUSEEVENT);
                        tme.dwFlags = TME_LEAVE & TME_CANCEL;
                        tme.hwndTrack = m_hWnd;
                        TrackMouseEvent(&tme);
                        mouse_tracking_ = false;
                    }

                    if (browser_host) {
                        // Determine the cursor position in screen coordinates.
                        POINT p;
                        ::GetCursorPos(&p);
                        ::ScreenToClient(m_hWnd, &p);

                        CefMouseEvent mouse_event;
                        mouse_event.x = p.x;
                        mouse_event.y = p.y;
                        Internal::DeviceToLogical(mouse_event, m_fScaleFactor);
                        mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
                        browser_host->SendMouseMoveEvent(mouse_event, true);
                    }
                }
                break;

                case WM_MOUSEWHEEL:
                    if (browser_host) {
                        POINT screen_point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                        HWND scrolled_wnd = ::WindowFromPoint(screen_point);
                        if (scrolled_wnd != m_hWnd)
                            break;

                        ScreenToClient(m_hWnd, &screen_point);
                        int delta = GET_WHEEL_DELTA_WPARAM(wParam);

                        CefMouseEvent mouse_event;
                        mouse_event.x = screen_point.x;
                        mouse_event.y = screen_point.y;
                        ApplyPopupOffset(mouse_event.x, mouse_event.y);
                        Internal::DeviceToLogical(mouse_event, m_fScaleFactor);
                        mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
                        browser_host->SendMouseWheelEvent(mouse_event,
                                                          Internal::IsKeyDown(VK_SHIFT) ? delta : 0,
                                                          !Internal::IsKeyDown(VK_SHIFT) ? delta : 0);
                    }
                    break;
            }
        }

        void CefDevToolsWnd::OnKeyEvent(UINT message, WPARAM wParam, LPARAM lParam) {
            if (m_bClosed)
                return;
            if (!m_browser)
                return;

            CefKeyEvent event;
            event.windows_key_code = wParam;
            event.native_key_code = lParam;
            event.is_system_key = (message == WM_SYSCHAR || message == WM_SYSKEYDOWN ||
                                   message == WM_SYSKEYUP);

            if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
                event.type = KEYEVENT_RAWKEYDOWN;
            else if (message == WM_KEYUP || message == WM_SYSKEYUP)
                event.type = KEYEVENT_KEYUP;
            else
                event.type = KEYEVENT_CHAR;
            event.modifiers = Internal::GetCefKeyboardModifiers(wParam, lParam);

            m_browser->GetHost()->SendKeyEvent(event);
        }

        void CefDevToolsWnd::OnFocus(bool setFocus) {
            if (m_browser)
                m_browser->GetHost()->SendFocusEvent(setFocus);
        }

        void CefDevToolsWnd::OnSize() {
            if (m_browser)
                m_browser->GetHost()->WasResized();
        }

        void CefDevToolsWnd::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) {

        }

        void CefDevToolsWnd::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) {

        }

    }
}
#endif