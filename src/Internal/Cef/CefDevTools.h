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
#ifndef DUILIB_CEF_DEVTOOLS_H_
#define DUILIB_CEF_DEVTOOLS_H_
#pragma once
#ifdef UILIB_WITH_CEF
#include "CefHandler.h"

namespace DuiLib {
    namespace Internal {
        class CefDevToolsWnd :
            public CWindowWnd
            , public ClientHandlerOsr::OsrDelegate {
          public:
            CefDevToolsWnd(CefRefPtr<CefBrowser> targetBrowser, float fScaleFactor);
            ~CefDevToolsWnd();
            UINT GetClassStyle() const;
            LPCTSTR GetWindowClassName() const;
            void OnFinalMessage(HWND hWnd);

            LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

            void OnBeforeContextMenu(CefRefPtr<CefMenuModel> model) OVERRIDE;

            void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
            void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
            bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE;
            bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE;
            bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int &screenX, int &screenY) OVERRIDE;
            bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo &screen_info) OVERRIDE;
            void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE;
            void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect &rect) OVERRIDE;
            void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height) OVERRIDE;
            void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList &dirtyRects, void *share_handle) OVERRIDE;
            void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CefRenderHandler::CursorType type, const CefCursorInfo &custom_cursor_info) OVERRIDE;
            bool StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y) OVERRIDE;
            void UpdateDragCursor(CefRefPtr<CefBrowser> browser, CefRenderHandler::DragOperation operation) OVERRIDE;
            void UpdateAccessibilityTree(CefRefPtr<CefValue> value) OVERRIDE;
            void OnBrowserClosing(CefRefPtr<CefBrowser> browser) OVERRIDE;
            void OnSetAddress(const std::string &url) OVERRIDE;
            void OnSetTitle(const std::string &title) OVERRIDE;
            bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString &text) OVERRIDE;
            void OnSetFullscreen(bool fullscreen) OVERRIDE;
            void OnAutoResize(const CefSize &new_size) OVERRIDE;
            void OnSetLoadingState(bool isLoading, bool canGoBack, bool canGoForward) OVERRIDE;
            void OnSetDraggableRegions(const std::vector<CefDraggableRegion> &regions) OVERRIDE;
            void OnJSNotify(const CefRefPtr<CefListValue> &value_list) OVERRIDE;
            bool OnBeforePopup(const std::string &target_url) OVERRIDE;
            void OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam);
            void OnKeyEvent(UINT message, WPARAM wParam, LPARAM lParam);
            void OnFocus(bool setFocus);
            void OnSize();
            void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl);
            void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode);
          private:
            CefRect GetPopupRectInWebView(const CefRect &original_rect);
            bool IsOverPopupWidget(int x, int y) const;
            int GetPopupXOffset() const;
            int GetPopupYOffset() const;
            void ApplyPopupOffset(int &x, int &y) const;
          protected:
            CefRefPtr<CefBrowser> m_pTargetBrowser;
            float m_fScaleFactor;

            // View
            HDC m_hViewMemoryDC;
            HBITMAP m_hViewBitmap;
            void *m_pViewBuffer;
            int m_iViewMemoryBitmapWidth;
            int m_iViewMemoryBitmapHeight;
            int m_iViewWidth;
            int m_iViewHeight;
            CriticalSection m_csPopupBuf;


            // Popup
            HDC m_hPopupMemoryDC;
            HBITMAP m_hPopupBitmap;
            void *m_pPopupBuffer;
            int m_iPopupMemoryBitmapWidth;
            int m_iPopupMemoryBitmapHeight;
            CefRect m_OriginPopupRect;
            CefRect m_PopupRect;
            CriticalSection m_csViewBuf;

            // Mouse state tracking.
            POINT last_mouse_pos_;
            POINT current_mouse_pos_;
            bool mouse_rotation_;
            bool mouse_tracking_;
            int last_click_x_;
            int last_click_y_;
            CefBrowserHost::MouseButtonType last_click_button_;
            int last_click_count_;
            double last_click_time_;
            bool last_mouse_down_on_view_;

            CefRefPtr<Internal::ClientHandlerOsr> m_ClientHandler;
            CefRefPtr<CefBrowser> m_browser;

            bool m_bClosed;
        };
    }
}
#endif
#endif