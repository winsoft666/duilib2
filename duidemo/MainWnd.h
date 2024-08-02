#pragma once
#include "PopupWnd.h"
#include "ShlObj.h"
#include "MsgWnd.h"
#include "ControlEx.h"
#include "SkinManager.h"
#include "DlgFake.h"

class MainPageWnd : public CNotifyPump {
public:
	MainPageWnd();

public:
	void SetPaintMagager(CPaintManagerUI *pPaintMgr);

	DUI_DECLARE_MESSAGE_MAP()
		virtual void OnSelectChanged(TNotifyUI &msg);
	virtual void OnItemClick(TNotifyUI &msg);

private:
	CPaintManagerUI *m_pPaintManager;
};

//////////////////////////////////////////////////////////////////////////
///

class MainWnd :
    public WindowImplBase,
    public CWebBrowserEventHandler,
    public SkinChangedReceiver {
  public:
    MainWnd();
    ~MainWnd();

    void OnClick(TNotifyUI &msg);
  public:
    CDuiString GetSkinFile();
    LPCTSTR GetWindowClassName() const;
    UINT GetClassStyle() const;
    void InitWindow();
    void OnFinalMessage(HWND hWnd);

  public:
    CControlUI *CreateControl(LPCTSTR pstrClass);
    virtual BOOL Receive(SkinChangedParam param);
    LPCTSTR QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType);
    virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam) override;
  public:
    void Notify(TNotifyUI &msg);
    DUI_DECLARE_MESSAGE_MAP()
  public:
    LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);
    LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

  public:// WebBrowser
    virtual HRESULT STDMETHODCALLTYPE UpdateUI(void);
    virtual HRESULT STDMETHODCALLTYPE GetHostInfo(CWebBrowserUI *pWeb, DOCHOSTUIINFO __RPC_FAR *pInfo);
    virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(CWebBrowserUI *pWeb, DWORD dwID, POINT __RPC_FAR *ppt,
            IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved);
  private:
    void UpdateText(const CDuiString &str);
    void AddListItem(int iCount);
	void TaskTest();
  private:
    CButtonUI *m_pBtnMax;
    CButtonUI *m_pBtnRestore;
    CButtonUI *m_pBtnMin;
    CButtonUI *m_pBtnSkin;
    CHorizontalLayoutUI *m_pHrlTitle;
    CMenuWnd *m_pMenu;
    CStdStringPtrMap m_MenuInfos;
    CSystemTrayIcon m_trayIcon;
    CListUI* m_pList;
    CEditUI* m_pEditH;
    CEditUI* m_pEditS;
    CEditUI* m_pEditL;
	CActiveXUI *m_pActiveXFlash;
	CFlashUI* m_pFlash;
    CComboBoxUI* m_pCombox;
	HANDLE m_hTaskStartEvent;

    UINT m_hTaskCreatedMsg;

#ifdef UILIB_WITH_CEF
    DlgFake* m_pDlgFake;
#endif
  public:
    MainPageWnd m_MainPage;
};
