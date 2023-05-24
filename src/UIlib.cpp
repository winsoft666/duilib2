// Copyright (c) 2010-2011, duilib develop team(www.duilib.com).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or
// without modification, are permitted provided that the
// following conditions are met.
//
// Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials
// provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
// DirectUI - UI Library
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2006-2007 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. These
// source files may be redistributed by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name is included.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//
//

#include "StdAfx.h"
#ifdef UILIB_WITH_CEF
    #include "include/cef_app.h"
    #include "Internal/Cef/CefApp.h"
#endif

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID /*lpReserved*/) {
    switch( dwReason ) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            ::DisableThreadLibraryCalls((HMODULE)hModule);
            break;
    }

    return TRUE;
}

namespace DuiLib {
#ifdef UILIB_WITH_CEF
    namespace {
        enum ProcessType {
            BrowserProcess,
            RendererProcess,
            ZygoteProcess,
            OtherProcess,
        };

        const char kProcessType[] = "type";
        const char kRendererProcess[] = "renderer";

        static ProcessType GetProcessType(CefRefPtr<CefCommandLine> command_line) {
            if (!command_line->HasSwitch(kProcessType))
                return BrowserProcess;

            const std::string &process_type = command_line->GetSwitchValue(kProcessType);
            if (process_type == kRendererProcess)
                return RendererProcess;

            return OtherProcess;
        }
    }
#endif

    bool Initialize(HINSTANCE hInstance, bool bInitCef, bool bEnableCefCache, bool bUsingCefProxy, bool bCefGPUEnabled) {
        ::CoInitialize(NULL);
        CPaintManagerUI::SetInstance(hInstance);
#ifdef UILIB_WITH_CEF
        CefGlobalContext::Instance()->SetWithCef(bInitCef);
        CefGlobalContext::Instance()->SetUsingProxyServer(bUsingCefProxy);
        CefGlobalContext::Instance()->SetCefGPUEnabled(bCefGPUEnabled);

        if (!bInitCef)
            return true;

        CefGlobalContext::Instance()->SetCefCache(bEnableCefCache);


        wchar_t szFolderPath[MAX_PATH + 2] = { 0 };
        std::wstring strCachePath;
        GetModuleFileNameW(NULL, szFolderPath, MAX_PATH);
        PathRemoveFileSpecW(szFolderPath);
        PathAddBackslashW(szFolderPath);
        strCachePath = szFolderPath;
        strCachePath += L"PPXCEF_TEMP"; // can't end with "\"

        CefSettings settings;
        settings.command_line_args_disabled = 1; // disable command line
        settings.multi_threaded_message_loop = true;
        settings.no_sandbox = true;
#ifdef _DEBUG
        settings.log_severity = LOGSEVERITY_VERBOSE;
#else
        settings.log_severity = LOGSEVERITY_ERROR;
#endif
        settings.windowless_rendering_enabled = true;
        settings.persist_session_cookies = bEnableCefCache ? 1 : 0;
        settings.ignore_certificate_errors = 1;
        CefString(&settings.locale).FromWString(L"zh-CN");
        CefString(&settings.accept_language_list).FromWString(L"zh-CN,en-US,en");
        CefString(&settings.user_data_path).FromWString(strCachePath);
        if (bEnableCefCache)
            CefString(&settings.cache_path).FromWString(strCachePath);

        CefMainArgs main_args(hInstance);
        if (!CefInitialize(main_args, settings, CefGlobalContext::Instance()->GetCefApp(), nullptr)) {
            return false;
        }
#else
        if (bInitCef)
            return false;
#endif
        return true;
    }

    void UnInitialize() {
#ifdef UILIB_WITH_CEF
        if (CefGlobalContext::Instance()->GetWithCef()) {
            CefShutdown();
        }
#endif
        CPaintManagerUI::Term();
        ::CoUninitialize();
    }

#ifdef UILIB_WITH_CEF
    bool CefProcessTypeCheck(HINSTANCE instance) {
        CefEnableHighDPISupport();
        CefMainArgs main_args(instance);

        CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
        command_line->InitFromString(::GetCommandLineW());

        ProcessType process_type = GetProcessType(command_line);

        if (process_type == BrowserProcess)
            CefGlobalContext::Instance()->SetCefApp(new Internal::ClientAppBrowser());
        else if (process_type == RendererProcess)
            CefGlobalContext::Instance()->SetCefApp(new Internal::ClientAppRenderer());
        else if (process_type == OtherProcess)
            CefGlobalContext::Instance()->SetCefApp(new Internal::ClientAppOther());

        int exit_code = CefExecuteProcess(main_args, CefGlobalContext::Instance()->GetCefApp(), nullptr);
        if (exit_code >= 0) {
            return false;
        }

        return true;
    }
#endif
}
