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
#include "RequestContextHandler.h"

namespace DuiLib {
    namespace Internal {
        RequestContextHandler::RequestContextHandler() {
        }

        bool RequestContextHandler::OnBeforePluginLoad(const CefString &mime_type, const CefString &plugin_url, const CefString &top_origin_url, CefRefPtr<CefWebPluginInfo> plugin_info, PluginPolicy *plugin_policy) {
            // Always allow the PDF plugin to load.
            if (*plugin_policy != PLUGIN_POLICY_ALLOW && mime_type == "application/pdf") {
                *plugin_policy = PLUGIN_POLICY_ALLOW;
                return true;
            }

            return false;
        }

    }
}
#endif