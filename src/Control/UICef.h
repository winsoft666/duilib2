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
#ifndef __UICEF_H__
#define __UICEF_H__
#pragma once
#ifdef UILIB_WITH_CEF
#include <functional>

#pragma comment(lib, "opengl32.lib")

namespace DuiLib {
    class UILIB_API CCefUI : public CContainerUI {
        DECLARE_DUICONTROL(CCefUI)
      public:
        typedef std::function<void(const std::string &url, int response_status)> ResourceResponseCallback;
        typedef std::function<void(const std::string &businessName, const std::vector<CLiteVariant>& vars)> JSCallback;
        typedef std::function<void(const std::string &url, int terminateCode)> RenderProcessTerminatedCallback;
        typedef std::function<void(bool isMainFrame, const std::string &url, int errorCode, const std::string &errorText)> 
          LoadErrorCallback;

        CCefUI();
        ~CCefUI();

        LPCTSTR GetClass() const override;
        LPVOID GetInterface(LPCTSTR pstrName) override;
        void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

        void SetPos(RECT rc, bool bNeedInvalidate /* = true */) override;
        void DoInit() override;
        bool DoPaint(HDC hDC, const RECT &rcPaint, CControlUI *pStopControl) override;
        void DoEvent(TEventUI &event) override;

        void SetVisible(bool bVisible = true) override;
        void SetInternVisible(bool bVisible = true) override;

        void SetResourceResponseCallback(ResourceResponseCallback cb);
        ResourceResponseCallback GetResourceResponseCallback() const;

        void SetRenderProcessTerminatedCallback(RenderProcessTerminatedCallback cb);
        RenderProcessTerminatedCallback GetRenderProcessTerminatedCallback() const;

        void SetJSCallback(JSCallback cb);
        JSCallback GetJSCallback() const;

        void SetLoadErrorCallback(LoadErrorCallback cb);
        LoadErrorCallback GetLoadErrorCallback() const;

        void SetAllowProtocols(const std::vector<std::string> vAllowProtocols);
        std::vector<std::string> GetAllowProtocols() const;

        void SetUrl(const CDuiString &url);
        CDuiString GetUrl() const;

        bool GetBkTransparent() const;
        int GetFPS() const;
        void SetFPS(int fps);
        DWORD GetCefBkColor() const;

        void SetUsingOpenGL(bool b);
        bool IsUsingOpenGL() const;

        void SetUsingOSR(bool b);
        bool IsUsingOSR() const;

        void GoBack();
        void GoForward();
        void Reload(bool bIgnoreCache = false);
        bool IsLoading();
        void StopLoad();
        void ShowDevTools();
        void CloseDevTools();
        bool CallJavascriptFunction(const std::string &strFuncName, const std::vector<CLiteVariant> &args);
      protected:
        CDuiString m_strUrl;
        bool m_hCreated;
        ResourceResponseCallback m_ResourceRspCB;
        JSCallback m_JSCB;
        RenderProcessTerminatedCallback m_RenderProcessTerminatedCB;
        LoadErrorCallback m_LoadErrorCB;
        class CCefUIImpl;
        CCefUIImpl *m_pImpl;
    };
}
#endif
#endif // __UICEF_H__
