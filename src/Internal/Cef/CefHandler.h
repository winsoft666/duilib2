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
#ifndef DUILIB_CEF_HANDLER_H_
#define DUILIB_CEF_HANDLER_H_
#pragma once
#ifdef UILIB_WITH_CEF
#include "include/cef_client.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_message_router.h"
#include "include/wrapper/cef_resource_manager.h"

namespace DuiLib {
    namespace Internal {
        class ClientHandlerOsr :
            public CefClient,
            public CefContextMenuHandler,
            public CefDisplayHandler,
            public CefDownloadHandler,
            public CefDragHandler,
            public CefFocusHandler,
            public CefKeyboardHandler,
            public CefLifeSpanHandler,
            public CefLoadHandler,
            public CefRequestHandler,
            public CefRenderHandler {
          public:
            class OsrDelegate {
              public:
                // Called when the browser is closing.
                virtual void OnBrowserClosing(CefRefPtr<CefBrowser> browser) = 0;

                // Set the window URL address.
                virtual void OnSetAddress(const std::string &url) = 0;

                // Set the window title.
                virtual void OnSetTitle(const std::string &title) = 0;

                virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString &text) = 0;

                // Set fullscreen mode.
                virtual void OnSetFullscreen(bool fullscreen) = 0;

                // Auto-resize contents.
                virtual void OnAutoResize(const CefSize &new_size) = 0;

                // Set the loading state.
                virtual void OnSetLoadingState(bool isLoading,
                                               bool canGoBack,
                                               bool canGoForward) = 0;

                virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, 
                    CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) = 0;

                virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) = 0;

                // Set the draggable regions.
                virtual void OnSetDraggableRegions(
                    const std::vector<CefDraggableRegion> &regions) = 0;

                // Set focus to the next/previous control.
                virtual void OnTakeFocus(bool next) {}

                // Called on the UI thread before a context menu is displayed.
                virtual void OnBeforeContextMenu(CefRefPtr<CefMenuModel> model) = 0;


                // These methods match the CefLifeSpanHandler interface.
                virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) = 0;
                virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) = 0;

                // These methods match the CefRenderHandler interface.
                virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect &rect) = 0;
#if CEFVER == 3626
                virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) = 0;
                virtual void OnImeCompositionRangeChanged(
                    CefRefPtr<CefBrowser> browser,
                    const CefRange &selection_range,
                    const CefRenderHandler::RectList &character_bounds) = 0;
#else
                virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) = 0;
#endif
                virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
                                            int viewX,
                                            int viewY,
                                            int &screenX,
                                            int &screenY) = 0;
                virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
                                           CefScreenInfo &screen_info) = 0;
                virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) = 0;
                virtual void OnPopupSize(CefRefPtr<CefBrowser> browser,
                                         const CefRect &rect) = 0;
                virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                                     CefRenderHandler::PaintElementType type,
                                     const CefRenderHandler::RectList &dirtyRects,
                                     const void *buffer,
                                     int width,
                                     int height) = 0;
                virtual void OnAcceleratedPaint(
                    CefRefPtr<CefBrowser> browser,
                    CefRenderHandler::PaintElementType type,
                    const CefRenderHandler::RectList &dirtyRects,
                    void *share_handle) {
                }
                virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
                                            CefCursorHandle cursor,
                                            CefRenderHandler::CursorType type,
                                            const CefCursorInfo &custom_cursor_info) = 0;
                virtual bool StartDragging(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefDragData> drag_data,
                                           CefRenderHandler::DragOperationsMask allowed_ops,
                                           int x,
                                           int y) = 0;
                virtual void UpdateDragCursor(
                    CefRefPtr<CefBrowser> browser,
                    CefRenderHandler::DragOperation operation) = 0;

                virtual void UpdateAccessibilityTree(CefRefPtr<CefValue> value) = 0;

                virtual void OnJSNotify(const CefRefPtr<CefListValue> &value_list) = 0;

                virtual bool OnBeforePopup(const std::string &target_url) = 0;

                virtual void OnResourceResponse(const std::string url, int rsp_status) {}

                virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) {};

                virtual void OnDraggableRegionsChanged(
                  CefRefPtr<CefBrowser> browser,
                  const std::vector<CefDraggableRegion>& regions) {
                }
              protected:
                virtual ~OsrDelegate() {}
            };

            ClientHandlerOsr(OsrDelegate *delegate);
            virtual ~ClientHandlerOsr();

            void DetachDelegate();

            void SetAllowProtocols(std::vector<std::string> vAllowProtocols);

            // CefClient methods.
            CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE { return this; }
            //CefRefPtr<CefAccessibilityHandler> GetAccessibilityHandler() OVERRIDE { return this; }
            CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE { return this; }
            CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
            CefRefPtr<CefDownloadHandler> GetDownloadHandler() OVERRIDE { return this; }
            CefRefPtr<CefDragHandler> GetDragHandler() OVERRIDE { return this; }
            CefRefPtr<CefFocusHandler> GetFocusHandler() OVERRIDE { return this; }
            CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE { return this; }
            CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
            CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }
            CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }

            bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE;

            // CefDisplayHandler methods
            void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString &url) OVERRIDE;
            void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString &title) OVERRIDE;
            void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen) OVERRIDE;
            bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString &text) OVERRIDE;
#if CEFVER == 3626
            bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level, const CefString &message, const CefString &source, int line) OVERRIDE;

            bool OnAutoResize(CefRefPtr<CefBrowser> browser, const CefSize &new_size) OVERRIDE;

            void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE;

            void OnImeCompositionRangeChanged(
                CefRefPtr<CefBrowser> browser, const CefRange &selection_range, const CefRenderHandler::RectList &character_bounds) OVERRIDE;

            void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                    CefRenderHandler::PaintElementType type,
                                    const CefRenderHandler::RectList &dirtyRects,
                                    void *share_handle) OVERRIDE;
#else
            bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE;
#endif
            // CefDownloadHandler methods
            void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDownloadItem> download_item,
                                  const CefString &suggested_name,
                                  CefRefPtr<CefBeforeDownloadCallback> callback) OVERRIDE;
            void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefDownloadItem> download_item,
                                   CefRefPtr<CefDownloadItemCallback> callback) OVERRIDE;

            // CefContextMenuHandler methods
            void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model) OVERRIDE;

            // CefLifeSpanHandler methods
            bool OnBeforePopup(
                CefRefPtr<CefBrowser> browser,
                CefRefPtr<CefFrame> frame,
                const CefString &target_url,
                const CefString &target_frame_name,
                CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                bool user_gesture,
                const CefPopupFeatures &popupFeatures,
                CefWindowInfo &windowInfo,
                CefRefPtr<CefClient> &client,
                CefBrowserSettings &settings,
                bool *no_javascript_access) OVERRIDE;

            void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
            bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
            void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

            void OnLoadError(CefRefPtr<CefBrowser> browser,
                CefRefPtr<CefFrame> frame,
                ErrorCode errorCode,
                const CefString& errorText,
                const CefString& failedUrl) OVERRIDE;

            void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                CefRefPtr<CefFrame> frame,
                int httpStatusCode) OVERRIDE;

            // CefRequestHandler methods
            void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) OVERRIDE;
#if CEFVER == 3626
            bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect) OVERRIDE;
#else
            bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect) OVERRIDE;
#endif
            void OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString &url, bool &allow_os_execution) OVERRIDE;

            bool OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response) OVERRIDE;

            CefRequestHandler::ReturnValue OnBeforeResourceLoad(
                CefRefPtr<CefBrowser> browser,
                CefRefPtr<CefFrame> frame,
                CefRefPtr<CefRequest> request,
                CefRefPtr<CefRequestCallback> callback) OVERRIDE;

            // CefRenderHandler methods.
            bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE;

            bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int &screenX, int &screenY) OVERRIDE;
            bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo &screen_info) OVERRIDE;
            void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE;
            void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect &rect) OVERRIDE;
            void OnPaint(CefRefPtr<CefBrowser> browser,
                         CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height) OVERRIDE;

            void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo &custom_cursor_info) OVERRIDE;

            // CefDragHandler
            void OnDraggableRegionsChanged(CefRefPtr<CefBrowser> browser,
              const std::vector<CefDraggableRegion>& regions) OVERRIDE;
          private:
            OsrDelegate *delegate_;

            std::vector<std::string> allow_protocols_;

            IMPLEMENT_REFCOUNTING(ClientHandlerOsr);
            DISALLOW_COPY_AND_ASSIGN(ClientHandlerOsr);
        };
    }
}
#endif
#endif // !DUILIB_CEF_HANDLER_H_