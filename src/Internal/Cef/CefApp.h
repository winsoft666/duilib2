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
#ifndef PPX_CEF_CEF_APP_H_
#define PPX_CEF_CEF_APP_H_
#pragma once
#ifdef UILIB_WITH_CEF
#include "include/cef_app.h"
#include "include/cef_v8.h"

namespace DuiLib {
    namespace Internal {

        class ClientApp : public CefApp {
          public:
            ClientApp() {

            }
            DISALLOW_COPY_AND_ASSIGN(ClientApp);
        };

        class ClientAppOther : public ClientApp {
          public:
            ClientAppOther() {
            }

          private:
            IMPLEMENT_REFCOUNTING(ClientAppOther);
            DISALLOW_COPY_AND_ASSIGN(ClientAppOther);
        };



        class ClientAppBrowser : public ClientApp, public CefBrowserProcessHandler {
          public:
            ClientAppBrowser();

          private:
            // CefApp methods.
            void OnBeforeCommandLineProcessing(
                const CefString &process_type,
                CefRefPtr<CefCommandLine> command_line) OVERRIDE;

            CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE {return this;}

            // CefBrowserProcessHandler methods.
            void OnContextInitialized() OVERRIDE;
            void OnBeforeChildProcessLaunch(
                CefRefPtr<CefCommandLine> command_line) OVERRIDE;
            void OnRenderProcessThreadCreated(
                CefRefPtr<CefListValue> extra_info) OVERRIDE;
            CefRefPtr<CefPrintHandler> print_handler_;

            IMPLEMENT_REFCOUNTING(ClientAppBrowser);
            DISALLOW_COPY_AND_ASSIGN(ClientAppBrowser);
        };


        typedef std::map <
        std::pair<std::string, int>,
            std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value> > >
            CallbackMap;

        class ClientAppRenderer : public ClientApp, public CefRenderProcessHandler, public CefV8Handler {
          public:
            ClientAppRenderer();

          private:
            CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this;}

            // CefRenderProcessHandler methods.
            void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;
            void OnWebKitInitialized() OVERRIDE;
            void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
            void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE;
            CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;
            void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) OVERRIDE;
            void OnContextReleased(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefV8Context> context) OVERRIDE;
            bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) OVERRIDE;

            bool Execute(const CefString &name,
                         CefRefPtr<CefV8Value> object,
                         const CefV8ValueList &arguments,
                         CefRefPtr<CefV8Value> &retval,
                         CefString &exception) OVERRIDE;
          private:
            CallbackMap callback_map_;
            IMPLEMENT_REFCOUNTING(ClientAppRenderer);
            DISALLOW_COPY_AND_ASSIGN(ClientAppRenderer);
        };
    }
}
#endif
#endif