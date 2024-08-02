#include "StdAfx.h"
#include "UIFlash.h"
#include <atlcomcli.h>

#define DISPID_FLASHEVENT_FLASHCALL  ( 0x00C5 )
#define DISPID_FLASHEVENT_FSCOMMAND  ( 0x0096 )
#define DISPID_FLASHEVENT_ONPROGRESS    ( 0x07A6 )

namespace DuiLib {
    IMPLEMENT_DUICONTROL(CFlashUI)
#define FLASH_ACTIVEX_CLSID TEXT("{D27CDB6E-AE6D-11CF-96B8-444553540000}")

    CFlashUI::CFlashUI(void)
        : m_dwRef(0)
        , m_dwCookie(0)
        , m_pFlash(NULL) {
        OLECHAR szCLSID[100] = { 0 };
        _tcsncpy(szCLSID, FLASH_ACTIVEX_CLSID, lengthof(szCLSID) - 1);
        ::CLSIDFromString(szCLSID, &m_clsid);
    }

    CFlashUI::~CFlashUI(void) {
        ReleaseControl();
    }

    bool CFlashUI::IsFlashActiveXInstalled() {
        CLSID clsid = IID_NULL;
        OLECHAR szCLSID[100] = { 0 };
        _tcsncpy(szCLSID, FLASH_ACTIVEX_CLSID, lengthof(szCLSID) - 1);
        ::CLSIDFromString(szCLSID, &clsid);

        IOleControl *pOleControl = NULL;
        HRESULT hr = ::CoCreateInstance(clsid, NULL, CLSCTX_ALL, IID_IOleControl, (LPVOID *)&pOleControl);
        if (FAILED(hr))
            return false;
        if (!pOleControl)
            return false;
        pOleControl->Release();
        return true;
    }

    void CFlashUI::SetActionScriptCallback(ActionScriptCallback cb) {
        m_ActionScriptCB = cb;
    }

    bool CFlashUI::CallActionScriptFunction(const std::wstring &strRequest, std::wstring &strResponse) {
        if (!m_pFlash)
            return false;
        if (m_pFlash != NULL) {
            BSTR response;
            HRESULT hr = m_pFlash->CallFunction((BSTR)strRequest.c_str(), &response);
            if (SUCCEEDED(hr)) {
                strResponse = response;
                return true;
            }
        }
        return false;
    }

    void CFlashUI::SetFlashPath(const CDuiString &strFlashPath) {
        m_strFlashPath = strFlashPath;

        if (m_pFlash) {
            LoadSWF(m_pFlash, m_strFlashPath.GetData(), m_strResType, NULL);
        }
    }

    CDuiString CFlashUI::GetFlashPath() const {
        return m_strFlashPath;
    }

    void CFlashUI::SetFlashResType(const CDuiString &strResType) {
        m_strResType = strResType;

        if (m_pFlash) {
            LoadSWF(m_pFlash, m_strFlashPath.GetData(), m_strResType, NULL);
        }
    }

    CDuiString CFlashUI::GetFlashResType() const {
        return m_strResType;
    }

    LPCTSTR CFlashUI::GetClass() const {
        return DUI_CTR_FLASH;
    }

    LPVOID CFlashUI::GetInterface( LPCTSTR pstrName ) {
        if( _tcsicmp(pstrName, DUI_CTR_FLASH) == 0 )
            return static_cast<CFlashUI *>(this);

        return CActiveXUI::GetInterface(pstrName);
    }

    HRESULT STDMETHODCALLTYPE CFlashUI::GetTypeInfoCount( __RPC__out UINT *pctinfo ) {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE CFlashUI::GetTypeInfo( UINT iTInfo, LCID lcid, __RPC__deref_out_opt ITypeInfo **ppTInfo ) {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE CFlashUI::GetIDsOfNames( __RPC__in REFIID riid, __RPC__in_ecount_full(cNames ) LPOLESTR *rgszNames, UINT cNames, LCID lcid, __RPC__out_ecount_full(cNames) DISPID *rgDispId ) {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE CFlashUI::Invoke( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr ) {

        switch(dispIdMember) {
            case DISPID_FLASHEVENT_FLASHCALL: {
                if (pDispParams->cArgs != 1 || pDispParams->rgvarg[0].vt != VT_BSTR)
                    return E_INVALIDARG;

                if (m_ActionScriptCB) {
                    m_ActionScriptCB(pDispParams->rgvarg[0].bstrVal);
                }
                return S_OK;
            }

            case DISPID_FLASHEVENT_FSCOMMAND: {
                if (pDispParams && pDispParams->cArgs == 2) {
                    if (pDispParams->rgvarg[0].vt == VT_BSTR &&
                            pDispParams->rgvarg[1].vt == VT_BSTR) {
                        //return FSCommand(pDispParams->rgvarg[1].bstrVal, pDispParams->rgvarg[0].bstrVal);
                    } else {
                        return DISP_E_TYPEMISMATCH;
                    }
                } else {
                    return DISP_E_BADPARAMCOUNT;
                }
            }

            case DISPID_FLASHEVENT_ONPROGRESS: {
            }

            case DISPID_READYSTATECHANGE: {
            }
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CFlashUI::QueryInterface( REFIID riid, void **ppvObject ) {
        *ppvObject = NULL;

        if( riid == IID_IUnknown)
            *ppvObject = static_cast</*IOleWindow**/LPUNKNOWN>(this);
        else if( riid == IID_IDispatch)
            *ppvObject = static_cast<IDispatch *>(this);
        else if( riid ==  __uuidof(_IShockwaveFlashEvents))
            *ppvObject = static_cast<_IShockwaveFlashEvents *>(this);

        if( *ppvObject != NULL )
            AddRef();

        return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
    }

    ULONG STDMETHODCALLTYPE CFlashUI::AddRef( void ) {
        ::InterlockedIncrement(&m_dwRef);
        return m_dwRef;
    }

    ULONG STDMETHODCALLTYPE CFlashUI::Release( void ) {
        ::InterlockedDecrement(&m_dwRef);
        return m_dwRef;
    }

    void CFlashUI::ReleaseControl() {
        RegisterEventHandler(FALSE);

        if (m_pFlash) {
            m_pFlash->Release();
            m_pFlash = NULL;
        }
    }

    bool CFlashUI::DoCreateControl() {
        if (!CActiveXUI::DoCreateControl())
            return false;

        GetControl(__uuidof(IShockwaveFlash), (LPVOID *)&m_pFlash);
        RegisterEventHandler(TRUE);
        return true;
    }

    void CFlashUI::OnShowActiveX() {
        IShockwaveFlash *pFlash = NULL;
        HRESULT hr = GetControl(__uuidof(IShockwaveFlash), (void **)&pFlash);
        if (SUCCEEDED(hr) && pFlash) {
            pFlash->put_WMode(_bstr_t(_T("Transparent")));
            pFlash->put_Movie(_bstr_t(m_strFlashPath.GetData()));
            LoadSWF(pFlash, m_strFlashPath.GetData(), m_strResType, NULL);
            pFlash->DisableLocalSecurity();
            pFlash->put_AllowScriptAccess(L"always");
            pFlash->Release();
        }
    }

    void CFlashUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) {
        if (_tcscmp(pstrName, _T("path")) == 0) {
            SetFlashPath(pstrValue);
        } else if (_tcscmp(pstrName, _T("restype")) == 0) {
            SetFlashResType(pstrValue);
        } else {
            CActiveXUI::SetAttribute(pstrName, pstrValue);
        }
    }

#pragma pack(push, 1)

    typedef struct _FLASH_STREAM_HEADER {
        DWORD m_dwSignature;
        DWORD m_dwDataSize;
    } FLASH_STREAM_HEADER, *PFLASH_STREAM_HEADER;
#pragma pack(pop)

    bool CFlashUI::LoadSWF(IShockwaveFlash *pFlash, STRINGorID swf, CDuiString type, HINSTANCE instance) {
        LPBYTE pData = NULL;
        DWORD dwSize = 0;

        do {
            if (type.GetLength() == 0) {
                CDuiString sFile = CPaintManagerUI::GetResourcePath();

                if (CPaintManagerUI::GetResourceZip().IsEmpty()) {
                    sFile += swf.m_lpstr;
                    HANDLE hFile = ::CreateFile(sFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
                                                FILE_ATTRIBUTE_NORMAL, NULL);

                    if (hFile == INVALID_HANDLE_VALUE)
                        break;

                    dwSize = ::GetFileSize(hFile, NULL);

                    if (dwSize == 0)
                        break;

                    DWORD dwRead = 0;
                    pData = new BYTE[dwSize];
                    ::ReadFile(hFile, pData, dwSize, &dwRead, NULL);
                    ::CloseHandle(hFile);

                    if (dwRead != dwSize) {
                        delete[] pData;
                        pData = NULL;
                        break;
                    }
                } else {
                    sFile += CPaintManagerUI::GetResourceZip();
                    CDuiString sFilePwd = CPaintManagerUI::GetResourceZipPwd();
                    HZIP hz = NULL;

                    if (CPaintManagerUI::IsCachedResourceZip()) {
                        hz = (HZIP)CPaintManagerUI::GetResourceZipHandle();
                    } else {
#ifdef UNICODE
                        std::string pwd = Unicode2Ansi(sFilePwd.GetData());
                        hz = OpenZip(sFile, pwd.c_str());
#else
                        hz = OpenZip(sFile, sFilePwd);
#endif
                    }

                    if (hz == NULL)
                        break;

                    ZIPENTRY ze;
                    int i = 0;
                    CDuiString key = swf.m_lpstr;
                    key.Replace(_T("\\"), _T("/"));

                    if (FindZipItem(hz, key, true, &i, &ze) != 0)
                        break;

                    dwSize = ze.unc_size;

                    if (dwSize == 0)
                        break;

                    pData = new BYTE[dwSize];
                    int res = UnzipItem(hz, i, pData, dwSize);

                    if (res != 0x00000000 && res != 0x00000600) {
                        delete[] pData;
                        pData = NULL;

                        if (!CPaintManagerUI::IsCachedResourceZip())
                            CloseZip(hz);

                        break;
                    }

                    if (!CPaintManagerUI::IsCachedResourceZip())
                        CloseZip(hz);
                }
            } else {
                HINSTANCE dllinstance = NULL;

                if (instance) {
                    dllinstance = instance;
                } else {
                    dllinstance = CPaintManagerUI::GetResourceDll();
                }

                HRSRC hResource = ::FindResource(dllinstance, swf.m_lpstr, type.GetData());

                if (hResource == NULL)
                    break;

                HGLOBAL hGlobal = ::LoadResource(dllinstance, hResource);

                if (hGlobal == NULL) {
                    FreeResource(hResource);
                    break;
                }

                dwSize = ::SizeofResource(dllinstance, hResource);

                if (dwSize == 0) break;

                pData = new BYTE[dwSize];
                ::CopyMemory(pData, (LPBYTE)::LockResource(hGlobal), dwSize);
                ::FreeResource(hResource);
            }
        } while (0);

        while (!pData) {
            //读不到图片, 则直接去读取bitmap.m_lpstr指向的路径
            HANDLE hFile = ::CreateFile(swf.m_lpstr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
                                        FILE_ATTRIBUTE_NORMAL, NULL);

            if (hFile == INVALID_HANDLE_VALUE)
                break;

            dwSize = ::GetFileSize(hFile, NULL);

            if (dwSize == 0)
                break;

            DWORD dwRead = 0;
            pData = new BYTE[dwSize];
            ::ReadFile(hFile, pData, dwSize, &dwRead, NULL);
            ::CloseHandle(hFile);

            if (dwRead != dwSize) {
                delete[] pData;
                pData = NULL;
            }

            break;
        }

        if (!pData) {
            return false;
        }

        bool bret = false;
        do {
            ATL::CComPtr<IStream> spStream;
            HRESULT hResult = ::CreateStreamOnHGlobal(NULL, TRUE, &spStream);
            if (FAILED(hResult))
                break;

            FLASH_STREAM_HEADER fsh = { 0x55665566, dwSize };
            ULARGE_INTEGER uli = { sizeof(fsh) + dwSize };
            hResult = spStream->SetSize(uli);
            if (FAILED(hResult))
                break;

            hResult = spStream->Write(&fsh, sizeof(fsh), NULL);
            if (FAILED(hResult))
                break;

            hResult = spStream->Write(reinterpret_cast<void *>(pData), dwSize, NULL);
            if (FAILED(hResult))
                break;

            uli.QuadPart = 0;
            hResult = spStream->Seek(*reinterpret_cast<PLARGE_INTEGER>(&uli), STREAM_SEEK_SET, NULL);
            if (FAILED(hResult))
                break;

            ATL::CComPtr<IPersistStreamInit> spPersistStreamInit;
            hResult = pFlash->QueryInterface(&spPersistStreamInit);
            if (SUCCEEDED(hResult))
                hResult = spPersistStreamInit->Load(spStream);
            bret = SUCCEEDED(hResult);
        } while (false);

        delete[] pData;
        pData = NULL;

        return bret;
    }

    LRESULT CFlashUI::TranslateAccelerator(MSG *pMsg) {
        if(pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST)
            return S_FALSE;

        if( m_pFlash == NULL )
            return E_NOTIMPL;

        // 当前Web窗口不是焦点,不处理加速键
        BOOL bIsChild = FALSE;
        HWND hTempWnd = NULL;
        HWND hWndFocus = ::GetFocus();

        hTempWnd = hWndFocus;

        while(hTempWnd != NULL) {
            if(hTempWnd == m_hwndHost) {
                bIsChild = TRUE;
                break;
            }

            hTempWnd = ::GetParent(hTempWnd);
        }

        if(!bIsChild)
            return S_FALSE;

        CComPtr<IOleInPlaceActiveObject> pObj;

        if (FAILED(m_pFlash->QueryInterface(IID_IOleInPlaceActiveObject, (LPVOID *)&pObj)))
            return S_FALSE;

        HRESULT hResult = pObj->TranslateAccelerator(pMsg);
        return hResult;
    }

    HRESULT CFlashUI::RegisterEventHandler( BOOL inAdvise ) {
        if (m_pFlash == NULL)
            return S_FALSE;

        HRESULT hr = S_FALSE;
        CComPtr<IConnectionPointContainer>  pCPC;
        CComPtr<IConnectionPoint> pCP;

        hr = m_pFlash->QueryInterface(IID_IConnectionPointContainer, (void **)&pCPC);

        if (FAILED(hr))
            return hr;

        hr = pCPC->FindConnectionPoint(__uuidof(_IShockwaveFlashEvents), &pCP);

        if (FAILED(hr))
            return hr;

        if (inAdvise) {
            hr = pCP->Advise((IDispatch *)this, &m_dwCookie);
        } else {
            hr = pCP->Unadvise(m_dwCookie);
        }

        return hr;
    }

};