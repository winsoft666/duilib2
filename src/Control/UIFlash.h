#ifndef __UIFLASH_H__
#define __UIFLASH_H__
#pragma once

#include <functional>
#include "Utils/flash11.tlh"

class CActiveXCtrl;

namespace DuiLib {
    class UILIB_API CFlashUI
        : public CActiveXUI
        , public _IShockwaveFlashEvents
        , public ITranslateAccelerator {
        DECLARE_DUICONTROL(CFlashUI)
      public:
        typedef std::function<void(const std::wstring &request)> ActionScriptCallback;
        CFlashUI(void);
        virtual ~CFlashUI(void);

        static bool IsFlashActiveXInstalled();

        void SetActionScriptCallback(ActionScriptCallback cb);
        bool CallActionScriptFunction(const std::wstring &strRequest, std::wstring &strResponse);

        void SetFlashPath(const CDuiString &strFlashPath);
        CDuiString GetFlashPath() const;

        void SetFlashResType(const CDuiString &strResType);
        CDuiString GetFlashResType() const;
      protected:
        virtual bool DoCreateControl();
        virtual void OnShowActiveX();
        virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
        bool LoadSWF(IShockwaveFlash *pFlash, STRINGorID swf, CDuiString type, HINSTANCE instance);
      private:
        virtual LPCTSTR GetClass() const;
        virtual LPVOID GetInterface( LPCTSTR pstrName );

        virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount( __RPC__out UINT *pctinfo );
        virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( UINT iTInfo, LCID lcid, __RPC__deref_out_opt ITypeInfo **ppTInfo );
        virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames( __RPC__in REFIID riid, __RPC__in_ecount_full(cNames ) LPOLESTR *rgszNames, UINT cNames, LCID lcid, __RPC__out_ecount_full(cNames) DISPID *rgDispId);
        virtual HRESULT STDMETHODCALLTYPE Invoke( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr );

        virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppvObject );
        virtual ULONG STDMETHODCALLTYPE AddRef( void );
        virtual ULONG STDMETHODCALLTYPE Release( void );

        virtual void ReleaseControl();
        HRESULT RegisterEventHandler(BOOL inAdvise);

        virtual LRESULT TranslateAccelerator( MSG *pMsg );


      private:
        LONG m_dwRef;
        DWORD m_dwCookie;
        IShockwaveFlash *m_pFlash;
        CDuiString m_strFlashPath;
        CDuiString m_strResType;
        ActionScriptCallback m_ActionScriptCB;
    };
}

#endif // __UIFLASH_H__
