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
#include "CefApp.h"
#include "CefUtil.h"
#include "CefGloablContext.h"
#include <io.h>

namespace DuiLib {
    namespace Internal {
        ClientAppBrowser::ClientAppBrowser() {

        }

        void ClientAppBrowser::OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line) {
            if (!CefGlobalContext::Instance()->GetUsingProxyServer()) {
                command_line->AppendSwitch("no-proxy-server");
            }

            command_line->AppendSwitch("disable-extensions");

            if (!CefGlobalContext::Instance()->GetCefGPUEnabled()) {
              command_line->AppendSwitch("disable-surfaces");
              command_line->AppendSwitch("disable-gpu-shader-disk-cache");
              command_line->AppendSwitch("disable-gpu");
              command_line->AppendSwitch("disable-gpu-compositing");
            }

            //command_line->AppendSwitch("enable-begin-frame-scheduling");

            CDuiString flashDllPath = CPaintManagerUI::GetInstancePath() + TEXT("pepperflash\\26.0.0.126\\pepflashplayer.dll");
            if (_waccess(flashDllPath.GetData(), 0) == 0) {
              command_line->AppendSwitchWithValue("ppapi-flash-path", "pepperflash/26.0.0.126/pepflashplayer.dll");
              command_line->AppendSwitchWithValue("ppapi-flash-version", "26.0.0.126");
            }
        }

        void ClientAppBrowser::OnContextInitialized() {

        }

        void ClientAppBrowser::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) {

        }

        void ClientAppBrowser::OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info) {

        }




        ClientAppRenderer::ClientAppRenderer() {

        }

        void ClientAppRenderer::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) {

        }

        void ClientAppRenderer::OnWebKitInitialized() {
            std::string extension_js_code =
                // Registered Javascript function, which will be called by CPP
                "var Duilib2App;"
                "if( !Duilib2App )"
                "   Duilib2App = {};"
                "(function() {"
                "   Duilib2App." + std::string(RegisterCppNotifyJSFunctionName) + " = function(name,callback) {"
                "       native function " + std::string(RegisterCppNotifyJSFunctionName) + "();"
                "       return " + std::string(RegisterCppNotifyJSFunctionName) + "(name, callback);"
                "   };"

                // Registered CPP Function, which will be called by JS
                "   Duilib2App." + std::string(JSNotifyCppFunctionName) + " = function() {"
                "       native function " + std::string(JSNotifyCppFunctionName) + "();"
                "       return " + std::string(JSNotifyCppFunctionName) + ".apply(this, Array.prototype.slice.call(arguments));"
                "   };"
                "})();";

            LOG(INFO) << extension_js_code;

            bool ret = CefRegisterExtension("v8/app", extension_js_code, this);
        }

        void ClientAppRenderer::OnBrowserCreated(CefRefPtr<CefBrowser> browser) {

        }

        void ClientAppRenderer::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) {

        }

        CefRefPtr<CefLoadHandler> ClientAppRenderer::GetLoadHandler() {
            CefRefPtr<CefLoadHandler> load_handler;
            return load_handler;
        }

        void ClientAppRenderer::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {

        }

        void ClientAppRenderer::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {
            // Remove any JavaScript callbacks registered for the context that has been released.
            if (!callback_map_.empty()) {
                CallbackMap::iterator it = callback_map_.begin();
                for (; it != callback_map_.end();) {
                    if (it->second.first->IsSame(context))
                        callback_map_.erase(it++);
                    else
                        ++it;
                }
            }
        }

        bool ClientAppRenderer::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {
            DCHECK_EQ(source_process, PID_BROWSER);
            bool handled = false;

            std::string msg_name = message->GetName();
            if (msg_name == BROWSER_2_RENDER_CPP_CALL_JS_MSG) {
                if (!callback_map_.empty()) {
                    CefRefPtr<CefListValue> arg_list = message->GetArgumentList();

                    std::string func_name = arg_list->GetString(0);
                    LOG(INFO) << "func_name=" << func_name;

                    CallbackMap::const_iterator it = callback_map_.find(
                                                         std::make_pair(func_name, browser->GetIdentifier())
                                                     );

                    if (it != callback_map_.end()) {
                        // Keep a local reference to the objects. The callback may remove itself from the callback map.
                        CefRefPtr<CefV8Context> context = it->second.first;
                        CefRefPtr<CefV8Value> callback = it->second.second;

                        // Enter the context.
                        context->Enter();

                        CefV8ValueList arguments;

                        int index = 0;
                        for (size_t i = 1; i < arg_list->GetSize(); i++) {
                            arguments.push_back(CefValueToCefV8Value(arg_list->GetValue(i)));
                        }

                        // Execute the callback.
                        CefRefPtr<CefV8Value> retval = callback->ExecuteFunctionWithContext(context, NULL, arguments);

                        if (retval.get()) {
                            handled = true;
                            //if (retval->IsBool())
                            //	handled = retval->GetBoolValue();
                        }

                        // Exit the context.
                        context->Exit();
                    }
                }
            }

            return handled;
        }

        bool ClientAppRenderer::Execute(const CefString &name, CefRefPtr<CefV8Value> object, const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval, CefString &exception) {
            std::string message_name = name;

            CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
            CefRefPtr<CefBrowser> browser = context->GetBrowser();

            if (message_name == RegisterCppNotifyJSFunctionName) { // register
                if (arguments.size() == 2 && arguments[0]->IsString() && arguments[1]->IsFunction()) {
                    std::string func_name = arguments[0]->GetStringValue();
                    int browser_id = context->GetBrowser()->GetIdentifier();
                    callback_map_.insert(
                        std::make_pair(std::make_pair(func_name, browser_id), std::make_pair(context, arguments[1]))
                    );

                    LOG(INFO) << "RegisterCppNotifyJSFunc func_name=" << func_name << ", browser_id=" << browser_id;
                } else {
                    LOG(INFO) << "RegisterCppNotifyJSFunc function " << RegisterCppNotifyJSFunctionName << " parameter error";
                }
            } else {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(RENDER_2_BROWSER_JS_CALL_CPP_MSG);
                CefRefPtr<CefListValue> args = message->GetArgumentList();

                size_t args_index = 0;
                args->SetString(args_index++, name);
                for (size_t i = 0; i < arguments.size(); i++) {
                    args->SetValue(args_index++, CefV8ValueToCefValue(arguments[i]));
                }

                browser->SendProcessMessage(PID_BROWSER, message);
            }

            return true;
        }
    }
}
#endif