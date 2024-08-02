#include "StdAfx.h"

namespace DuiLib {
    class CComboBoxWnd : public CWindowWnd, public INotifyUI {
      public:
        CComboBoxWnd();
        void Init(CComboBoxUI *pOwner);
        LPCTSTR GetWindowClassName() const;
        void OnFinalMessage(HWND hWnd);

        LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
        void Notify(TNotifyUI &msg) override;

#if(_WIN32_WINNT >= 0x0501)
        virtual UINT GetClassStyle() const;
#endif
        bool IsHitItem(POINT ptMouse);

        int GetCount() const;
        bool Add(CControlUI *pControl);
        bool AddAt(CControlUI *pControl, int iIndex);
        bool Remove(CControlUI *pControl);
        bool RemoveAt(int iIndex);
        void RemoveAll();

        void Show(bool bShow);
        CDuiString GetText();
        void SetText(const CDuiString &str);
      public:
        CPaintManagerUI m_pm;
        CComboBoxUI *m_pOwner;
        CVerticalLayoutUI *m_pLayout;
        bool m_bHitItem;
        CListUI *m_pList;
        std::vector<CControlUI *> m_vControls;
    };

    CComboBoxWnd::CComboBoxWnd() {
        m_pOwner = NULL;
        m_pLayout = NULL;
        m_pList = NULL;
        m_bHitItem = false;

        m_pList = new CListUI();
        m_pList->SetManager(&m_pm, NULL, true);
    }


    void CComboBoxWnd::Notify(TNotifyUI &msg) {
        if (msg.sType == DUI_MSGTYPE_WINDOWINIT) {

        } else if(msg.sType == DUI_MSGTYPE_CLICK) {
            CDuiString sName = msg.pSender->GetName();
            CControlUI *pCtrl = msg.pSender;

            while(pCtrl != NULL) {
                IListItemUI *pListItem = (IListItemUI *)pCtrl->GetInterface(DUI_CTR_LISTITEM);

                if(pListItem != NULL ) {
                    SetText(pCtrl->GetText());
                    break;
                }

                pCtrl = pCtrl->GetParent();
            }

            if( m_pOwner->GetManager() != NULL )
                m_pOwner->GetManager()->SendNotify(msg.pSender, DUI_MSGTYPE_CLICK, 0, 0);
        } else if (msg.sType == DUI_MSGTYPE_ITEMSELECT) {
            if (m_pList) {
                SetText(m_pList->GetItemAt(msg.wParam)->GetText());
            }

            if (m_pOwner && m_pOwner->m_pManager)
                m_pOwner->m_pManager->SendNotify(msg);
        }
    }

    void CComboBoxWnd::Init(CComboBoxUI *pOwner) {
        if (!pOwner)
            return;

        m_pOwner = pOwner;

        Create(pOwner->GetManager()->GetPaintWindow(), NULL, WS_POPUP, WS_EX_TOOLWINDOW, 0, 0, 0, 0);
        // HACK: Don't deselect the parent's caption
        HWND hWndParent = m_hWnd;

        while( ::GetParent(hWndParent) != NULL )
            hWndParent = ::GetParent(hWndParent);

        ::ShowWindow(m_hWnd, SW_HIDE);
        ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
    }

    LPCTSTR CComboBoxWnd::GetWindowClassName() const {
        return _T("ComboWnd");
    }

    void CComboBoxWnd::OnFinalMessage(HWND hWnd) {
        m_pOwner->m_pWindow = NULL;
        m_pOwner->m_uButtonState &= ~ UISTATE_PUSHED;
        m_pOwner->Invalidate();
        delete this;
    }

    bool CComboBoxWnd::IsHitItem(POINT ptMouse) {
        CControlUI *pControl = m_pm.FindControl(ptMouse);

        if(pControl != NULL) {
            LPVOID pInterface = pControl->GetInterface(DUI_CTR_SCROLLBAR);

            if(pInterface) return false;

            while(pControl != NULL) {
                IListItemUI *pListItem = (IListItemUI *)pControl->GetInterface(DUI_CTR_LISTITEM);

                if(pListItem != NULL ) {
                    return true;
                }

                pControl = pControl->GetParent();
            }
        }

        return false;
    }

    int CComboBoxWnd::GetCount() const {
        if (m_pList)
            return m_pList->GetCount();
        return 0;
    }

    bool CComboBoxWnd::Add(CControlUI *pControl) {
        if (m_pList) {
            return m_pList->Add(pControl);
        }
        m_vControls.push_back(pControl);
        return true;
    }

    bool CComboBoxWnd::AddAt(CControlUI *pControl, int iIndex) {
        if (m_pList) {
            return m_pList->AddAt(pControl, iIndex);
        }
        return false;
    }

    bool CComboBoxWnd::Remove(CControlUI *pControl) {
        if (m_pList) {
            return m_pList->Remove(pControl);
        }
        return false;
    }

    bool CComboBoxWnd::RemoveAt(int iIndex) {
        if (m_pList) {
            return m_pList->RemoveAt(iIndex);
        }
        return false;
    }

    void CComboBoxWnd::RemoveAll() {
        if (m_pList) {
            m_pList->RemoveAll();
        }
    }

    void CComboBoxWnd::Show(bool bShow) {
        if (bShow) {
            HWND hWndParent = m_hWnd;
            while (::GetParent(hWndParent) != NULL)
                hWndParent = ::GetParent(hWndParent);

            // Position the popup window in absolute space
            SIZE szDrop = m_pOwner->GetDropBoxSize();
            szDrop = m_pm.GetDPIObj()->Scale(szDrop);

            RECT rc = m_pOwner->GetPos();
            rc.top = rc.bottom;     // 父窗口left、bottom位置作为弹出窗口起点

            POINT ptWnd = { rc.left, rc.top };
            ::ClientToScreen(hWndParent, &ptWnd);

            if (szDrop.cx > 0)
                rc.right = rc.left + szDrop.cx; // 计算弹出窗口宽度

            SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
            int cyFixed = 0;

            for (int i = 0; i < m_pList->GetCount(); i++) {
                CControlUI *pControl = static_cast<CControlUI *>(m_pList->GetItemAt(i));
                if (!pControl->IsVisible())
                    continue;

                SIZE sz = pControl->EstimateSize(szAvailable);
                cyFixed += sz.cy;
            }

            cyFixed += (m_pList->GetInset().top + m_pList->GetInset().bottom);

            rc.bottom = rc.top + MIN(cyFixed, szDrop.cy);

            ::SetWindowPos(m_hWnd, NULL, ptWnd.x, ptWnd.y, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
        } else {
            ::ShowWindow(m_hWnd, SW_HIDE);
        }
    }

    CDuiString CComboBoxWnd::GetText() {
        if (m_pList && m_pList->GetCount() > 0 && m_pList->GetCurSel() >= 0) {
            return m_pList->GetItemAt(m_pList->GetCurSel())->GetText();
        }
        return TEXT("");
    }

    void CComboBoxWnd::SetText(const CDuiString &str) {
        if(m_pOwner)
            m_pOwner->SetText(str);
    }

    LRESULT CComboBoxWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if( uMsg == WM_CREATE ) {
            m_pm.SetForceUseSharedRes(true);
            m_pm.Init(m_hWnd);
            // The trick is to add the items to the new container. Their owner gets
            // reassigned by this operation - which is why it is important to reassign
            // the items back to the righfull owner/manager when the window closes.
            m_pLayout = new CVerticalLayoutUI;
            m_pLayout->SetManager(&m_pm, NULL, true);

            m_pList->SetInset(CDuiRect(1, 1, 1, 1));
            m_pList->SetBkColor(0xFFFFFFFF);
            m_pList->SetBorderColor(0xFFC6C7D2);
            m_pList->SetBorderSize(CDuiRect(1, 1, 1, 1));
            m_pList->EnableScrollBar(true, true);
            if (m_pList->GetHeader())
                m_pList->GetHeader()->SetVisible(false);
            m_pList->ApplyAttributeList(m_pOwner->GetDropBoxAttributeList());

            m_pLayout->Add(m_pList);

            CShadowUI *pShadow = m_pOwner->GetManager()->GetShadow();
            if(pShadow != NULL && m_pOwner != NULL) {
                pShadow->CopyShadow(m_pm.GetShadow());
                m_pm.GetShadow()->ShowShadow(m_pOwner->IsShowShadow());
            }

            m_pm.AttachDialog(m_pLayout);
            m_pm.AddNotifier(this);

            m_pm.SetDPI(m_pOwner->GetManager()->GetDPIObj()->GetDPI());

            // Add Control to list
            for (size_t i = 0; i < m_vControls.size(); i++) {
                CControlUI *pControl = static_cast<CControlUI *>(m_vControls[i]);
                if (pControl) {
                    if (m_pList)
                        m_pList->Add(pControl);
                }
            }
            m_vControls.clear();

            return 0;
        } else if( uMsg == WM_CLOSE ) {
            m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
            RECT rcNull = { 0 };

            for( int i = 0; i < m_pOwner->GetCount(); i++ )
                static_cast<CControlUI *>(m_pOwner->GetItemAt(i))->SetPos(rcNull);

            m_pOwner->SetFocus();
        } else if( uMsg == WM_LBUTTONDOWN ) {
            POINT pt = { 0 };
            ::GetCursorPos(&pt);
            ::ScreenToClient(m_pm.GetPaintWindow(), &pt);
            m_bHitItem = IsHitItem(pt);
        } else if( uMsg == WM_LBUTTONUP ) {
            POINT pt = { 0 };
            ::GetCursorPos(&pt);
            ::ScreenToClient(m_pm.GetPaintWindow(), &pt);

            if(m_bHitItem && IsHitItem(pt)) {
                PostMessage(WM_KILLFOCUS);
            }

            m_bHitItem = false;
        } else if (uMsg == WM_KEYDOWN) {
            switch (wParam) {
                case VK_ESCAPE:
                case VK_RETURN:
                    PostMessage(WM_KILLFOCUS);
                    break;
            }
        } else if( uMsg == WM_KILLFOCUS ) {
            if (m_hWnd != (HWND)wParam)
                ShowWindow(false);
        }

        LRESULT lRes = 0;
        if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) )
            return lRes;

        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }


#if(_WIN32_WINNT >= 0x0501)
    UINT CComboBoxWnd::GetClassStyle() const {
        if(m_pOwner->IsShowShadow()) {
            return __super::GetClassStyle();

        } else {
            return __super::GetClassStyle() | CS_DROPSHADOW;
        }
    }
#endif
    ////////////////////////////////////////////////////////
    IMPLEMENT_DUICONTROL(CComboBoxUI)

    CComboBoxUI::CComboBoxUI() :
        m_uTextStyle(DT_VCENTER | DT_SINGLELINE)
        , m_dwTextColor(0)
        , m_dwDisabledTextColor(0)
        , m_iFont(-1)
        , m_bShowHtml(false)
        , m_pWindow(NULL)
        , m_uButtonState(0) {
        m_szDropBox = CDuiSize(0, 150);
        ::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
        m_pWindow = new CComboBoxWnd();
    }

    LPCTSTR CComboBoxUI::GetClass() const {
        return DUI_CTR_COMBOBOX;
    }

    LPVOID CComboBoxUI::GetInterface(LPCTSTR pstrName) {
        if( _tcsicmp(pstrName, DUI_CTR_COMBOBOX) == 0 ) return static_cast<CComboBoxUI *>(this);

        if( _tcsicmp(pstrName, _T("IListOwner")) == 0 ) return static_cast<IListOwnerUI *>(this);

        return CContainerUI::GetInterface(pstrName);
    }

    UINT CComboBoxUI::GetControlFlags() const {
        return UIFLAG_TABSTOP | UIFLAG_SETCURSOR;
    }

    void CComboBoxUI::DoInit() {
        m_pWindow->Init(this);
    }

    ListType CComboBoxUI::GetListType() {
        return LT_COMBO;
    }

    DuiLib::TListInfoUI *CComboBoxUI::GetListInfo() {
        return NULL;
    }

    int CComboBoxUI::GetCurSel() const {
        if (!m_pWindow || !m_pWindow->m_pList)
            return -1;
        return m_pWindow->m_pList->GetCurSel();
    }

    bool CComboBoxUI::SelectItem(int iIndex, bool bTakeFocus /*= false*/) {
        bool ret = false;
        if (m_pWindow && m_pWindow->m_pList) {
            ret = m_pWindow->m_pList->SelectItem(iIndex, bTakeFocus);
        }
        if (ret) {
            SetText(m_pWindow->m_pList->GetItemAt(iIndex)->GetText());
        }
        return ret;
    }

    bool CComboBoxUI::SelectMultiItem(int iIndex, bool bTakeFocus) {
        return SelectItem(iIndex, bTakeFocus);
    }

    bool CComboBoxUI::UnSelectItem(int iIndex, bool bOthers) {
        return false;
    }

    bool CComboBoxUI::SetItemIndex(CControlUI *pControl, int iIndex) {
        return true;
    }

    int CComboBoxUI::GetCount() const {
        if (m_pWindow)
            return m_pWindow->GetCount();
        return 0;
    }

    bool CComboBoxUI::Add(CControlUI *pControl) {
        IListItemUI *pListItem = static_cast<IListItemUI *>(pControl->GetInterface(DUI_CTR_LISTITEM));
        if (!pListItem || !m_pWindow)
            return false;

        if(m_pWindow)
            return m_pWindow->Add(pControl);

        return true;
    }

    bool CComboBoxUI::AddAt(CControlUI *pControl, int iIndex) {
        IListItemUI *pListItem = static_cast<IListItemUI *>(pControl->GetInterface(DUI_CTR_LISTITEM));
        if (!pListItem)
            return false;

        if(m_pWindow)
            return m_pWindow->AddAt(pControl, iIndex);
        return true;
    }

    bool CComboBoxUI::Remove(CControlUI *pControl) {
        IListItemUI *pListItem = static_cast<IListItemUI *>(pControl->GetInterface(DUI_CTR_LISTITEM));
        if (!pListItem || !m_pWindow)
            return false;

        return m_pWindow->Remove(pControl);
    }

    bool CComboBoxUI::RemoveAt(int iIndex) {
        if (!m_pWindow)
            return false;

        return m_pWindow->RemoveAt(iIndex);
    }

    void CComboBoxUI::RemoveAll() {
        if (!m_pWindow)
            return;

        return m_pWindow->RemoveAll();
    }

    void CComboBoxUI::DoEvent(TEventUI &event) {
        if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
            if( m_pParent != NULL ) m_pParent->DoEvent(event);
            else CContainerUI::DoEvent(event);

            return;
        }

        if( event.Type == UIEVENT_SETFOCUS ) {
            Invalidate();
        } else if( event.Type == UIEVENT_KILLFOCUS ) {
            Invalidate();
        }

        else if( event.Type == UIEVENT_BUTTONDOWN ) {
            if( IsEnabled() ) {
                Activate();
                m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
            }
        } else if( event.Type == UIEVENT_BUTTONUP ) {
            if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
                m_uButtonState &= ~ UISTATE_CAPTURED;
                Invalidate();
            }
        } else if( event.Type == UIEVENT_MOUSEMOVE ) {
            return;
        } else if( event.Type == UIEVENT_KEYDOWN ) {
            switch( event.chKey ) {
                case VK_F4:
                    Activate();
                    return;
            }
        } else if( event.Type == UIEVENT_CONTEXTMENU ) {
            return;
        } else if( event.Type == UIEVENT_MOUSEENTER ) {
            if( ::PtInRect(&m_rcItem, event.ptMouse ) ) {
                if( (m_uButtonState & UISTATE_HOT) == 0  )
                    m_uButtonState |= UISTATE_HOT;

                Invalidate();
            }
        } else if( event.Type == UIEVENT_MOUSELEAVE ) {
            if( (m_uButtonState & UISTATE_HOT) != 0 ) {
                m_uButtonState &= ~UISTATE_HOT;
                Invalidate();
            }
        } else {
            CControlUI::DoEvent(event);
        }
    }

    SIZE CComboBoxUI::EstimateSize(SIZE szAvailable) {
        if( m_cxyFixed.cy == 0 )
            return CDuiSize(m_pManager->GetDPIObj()->Scale(m_cxyFixed.cx), m_pManager->GetDefaultFontInfo()->tm.tmHeight + 12);

        return CControlUI::EstimateSize(szAvailable);
    }

    bool CComboBoxUI::Activate() {
        if( !CControlUI::Activate() )
            return false;

        if( m_pManager != NULL )
            m_pManager->SendNotify(this, DUI_MSGTYPE_PREDROPDOWN);

        if (m_pWindow)
            m_pWindow->Show(true);

        if( m_pManager != NULL )
            m_pManager->SendNotify(this, DUI_MSGTYPE_DROPDOWN);

        Invalidate();
        return true;
    }

    CDuiString CComboBoxUI::GetText() const {
        if (m_pWindow)
            return m_pWindow->GetText();
        return TEXT("");
    }

    void CComboBoxUI::SetEnabled(bool bEnable) {
        CContainerUI::SetEnabled(bEnable);

        if( !IsEnabled() )
            m_uButtonState = 0;
    }

    CDuiString CComboBoxUI::GetDropBoxAttributeList() {
        return m_sDropBoxAttributes;
    }

    void CComboBoxUI::SetDropBoxAttributeList(LPCTSTR pstrList) {
        m_sDropBoxAttributes = pstrList;
    }

    SIZE CComboBoxUI::GetDropBoxSize() const {
        return m_szDropBox;
    }

    void CComboBoxUI::SetDropBoxSize(SIZE szDropBox) {
        m_szDropBox = szDropBox;
    }

    void CComboBoxUI::SetTextStyle(UINT uStyle) {
        m_uTextStyle = uStyle;
        Invalidate();
    }

    UINT CComboBoxUI::GetTextStyle() const {
        return m_uTextStyle;
    }

    void CComboBoxUI::SetTextColor(DWORD dwTextColor) {
        m_dwTextColor = dwTextColor;
        Invalidate();
    }

    DWORD CComboBoxUI::GetTextColor() const {
        return m_dwTextColor;
    }

    void CComboBoxUI::SetDisabledTextColor(DWORD dwTextColor) {
        m_dwDisabledTextColor = dwTextColor;
        Invalidate();
    }

    DWORD CComboBoxUI::GetDisabledTextColor() const {
        return m_dwDisabledTextColor;
    }

    void CComboBoxUI::SetFont(int index) {
        m_iFont = index;
        Invalidate();
    }

    int CComboBoxUI::GetFont() const {
        return m_iFont;
    }

    RECT CComboBoxUI::GetTextPadding() const {
        return m_rcTextPadding;
    }

    void CComboBoxUI::SetTextPadding(RECT rc) {
        m_rcTextPadding = rc;
        Invalidate();
    }

    bool CComboBoxUI::IsShowHtml() {
        return m_bShowHtml;
    }

    void CComboBoxUI::SetShowHtml(bool bShowHtml) {
        if( m_bShowHtml == bShowHtml ) return;

        m_bShowHtml = bShowHtml;
        Invalidate();
    }

    bool CComboBoxUI::IsShowShadow() {
        return m_bShowShadow;
    }

    void CComboBoxUI::SetShowShadow(bool bShow) {
        if( m_bShowShadow == bShow ) return;

        m_bShowShadow = bShow;
        Invalidate();
    }

    LPCTSTR CComboBoxUI::GetNormalImage() const {
        return m_sNormalImage;
    }

    void CComboBoxUI::SetNormalImage(LPCTSTR pStrImage) {
        m_sNormalImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CComboBoxUI::GetHotImage() const {
        return m_sHotImage;
    }

    void CComboBoxUI::SetHotImage(LPCTSTR pStrImage) {
        m_sHotImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CComboBoxUI::GetPushedImage() const {
        return m_sPushedImage;
    }

    void CComboBoxUI::SetPushedImage(LPCTSTR pStrImage) {
        m_sPushedImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CComboBoxUI::GetFocusedImage() const {
        return m_sFocusedImage;
    }

    void CComboBoxUI::SetFocusedImage(LPCTSTR pStrImage) {
        m_sFocusedImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CComboBoxUI::GetDisabledImage() const {
        return m_sDisabledImage;
    }

    void CComboBoxUI::SetDisabledImage(LPCTSTR pStrImage) {
        m_sDisabledImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CComboBoxUI::GetArrowNormalImage() const {
        return m_sArrowNormalImage;
    }

    void CComboBoxUI::SetArrowNormalImage(LPCTSTR pStrImage) {
        m_sArrowNormalImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CComboBoxUI::GetArrowHotImage() const {
        return m_sArrowHotImage;
    }

    void CComboBoxUI::SetArrowHotImage(LPCTSTR pStrImage) {
        m_sArrowHotImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CComboBoxUI::GetArrowPushedImage() const {
        return m_sArrowPushedImage;
    }

    void CComboBoxUI::SetArrowPushedImage(LPCTSTR pStrImage) {
        m_sArrowPushedImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CComboBoxUI::GetArrowFocusedImage() const {
        return m_sArrowFocusedImage;
    }

    void CComboBoxUI::SetArrowFocusedImage(LPCTSTR pStrImage) {
        m_sArrowFocusedImage = pStrImage;
        Invalidate();
    }

    LPCTSTR CComboBoxUI::GetArrowDisabledImage() const {
        return m_sArrowDisabledImage;
    }

    void CComboBoxUI::SetArrowDisabledImage(LPCTSTR pStrImage) {
        m_sArrowDisabledImage = pStrImage;
        Invalidate();
    }

    void CComboBoxUI::SetPos(RECT rc, bool bNeedInvalidate) {
        if(!::EqualRect(&rc, &m_rcItem)) {
            // 隐藏下拉窗口
            if (m_pWindow && ::IsWindow(m_pWindow->GetHWND()))
                m_pWindow->Show(false);

            //// 所有元素大小置为0
            //RECT rcNull = { 0 };

            //for( int i = 0; i < m_items.GetSize(); i++ )
            //    static_cast<CControlUI *>(m_items[i])->SetPos(rcNull);

            // 调整位置
            CControlUI::SetPos(rc, bNeedInvalidate);
        }
    }

    void CComboBoxUI::Move(SIZE szOffset, bool bNeedInvalidate) {
        CControlUI::Move(szOffset, bNeedInvalidate);
    }

    void CComboBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) {
        if( _tcsicmp(pstrName, _T("align")) == 0 ) {
            if( _tcsstr(pstrValue, _T("left")) != NULL ) {
                m_uTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_SINGLELINE);
                m_uTextStyle |= DT_LEFT;
            }

            if( _tcsstr(pstrValue, _T("center")) != NULL ) {
                m_uTextStyle &= ~(DT_LEFT | DT_RIGHT );
                m_uTextStyle |= DT_CENTER;
            }

            if( _tcsstr(pstrValue, _T("right")) != NULL ) {
                m_uTextStyle &= ~(DT_LEFT | DT_CENTER | DT_SINGLELINE);
                m_uTextStyle |= DT_RIGHT;
            }
        } else if( _tcsicmp(pstrName, _T("valign")) == 0 ) {
            if( _tcsstr(pstrValue, _T("top")) != NULL ) {
                m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER);
                m_uTextStyle |= (DT_TOP | DT_SINGLELINE);
            }

            if( _tcsstr(pstrValue, _T("vcenter")) != NULL ) {
                m_uTextStyle &= ~(DT_TOP | DT_BOTTOM );
                m_uTextStyle |= (DT_VCENTER | DT_SINGLELINE);
            }

            if( _tcsstr(pstrValue, _T("bottom")) != NULL ) {
                m_uTextStyle &= ~(DT_TOP | DT_VCENTER);
                m_uTextStyle |= (DT_BOTTOM | DT_SINGLELINE);
            }
        } else if( _tcsicmp(pstrName, _T("endellipsis")) == 0 ) {
            if( _tcsicmp(pstrValue, _T("true")) == 0 )
                m_uTextStyle |= DT_END_ELLIPSIS;
            else
                m_uTextStyle &= ~DT_END_ELLIPSIS;
        } else if( _tcsicmp(pstrName, _T("wordbreak")) == 0 ) {
            if( _tcsicmp(pstrValue, _T("true")) == 0 ) {
                m_uTextStyle &= ~DT_SINGLELINE;
                m_uTextStyle |= DT_WORDBREAK | DT_EDITCONTROL;
            } else {
                m_uTextStyle &= ~DT_WORDBREAK & ~DT_EDITCONTROL;
                m_uTextStyle |= DT_SINGLELINE;
            }
        } else if( _tcsicmp(pstrName, _T("font")) == 0 )
            SetFont(_ttoi(pstrValue));
        else if( _tcsicmp(pstrName, _T("textcolor")) == 0 ) {
            if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);

            LPTSTR pstr = NULL;
            DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
            SetTextColor(clrColor);
        } else if( _tcsicmp(pstrName, _T("disabledtextcolor")) == 0 ) {
            if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);

            LPTSTR pstr = NULL;
            DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
            SetDisabledTextColor(clrColor);
        } else if( _tcsicmp(pstrName, _T("textpadding")) == 0 ) {
            RECT rcTextPadding = { 0 };
            LPTSTR pstr = NULL;
            rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);
            ASSERT(pstr);
            rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);
            ASSERT(pstr);
            rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);
            ASSERT(pstr);
            rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10);
            ASSERT(pstr);
            SetTextPadding(rcTextPadding);
        } else if( _tcsicmp(pstrName, _T("showhtml")) == 0 )
            SetShowHtml(_tcsicmp(pstrValue, _T("true")) == 0);
        else if( _tcsicmp(pstrName, _T("showshadow")) == 0 )
            SetShowShadow(_tcsicmp(pstrValue, _T("true")) == 0);
        else if( _tcsicmp(pstrName, _T("normalimage")) == 0 )
            SetNormalImage(pstrValue);
        else if( _tcsicmp(pstrName, _T("hotimage")) == 0 )
            SetHotImage(pstrValue);
        else if( _tcsicmp(pstrName, _T("pushedimage")) == 0 )
            SetPushedImage(pstrValue);
        else if( _tcsicmp(pstrName, _T("focusedimage")) == 0 )
            SetFocusedImage(pstrValue);
        else if( _tcsicmp(pstrName, _T("disabledimage")) == 0 )
            SetDisabledImage(pstrValue);
        else if( _tcsicmp(pstrName, _T("dropboxattribute")) == 0 )
            SetDropBoxAttributeList(pstrValue);
        else if( _tcsicmp(pstrName, _T("dropboxsize")) == 0) {
            SIZE szDropBoxSize = { 0 };
            LPTSTR pstr = NULL;
            szDropBoxSize.cx = _tcstol(pstrValue, &pstr, 10);
            ASSERT(pstr);
            szDropBoxSize.cy = _tcstol(pstr + 1, &pstr, 10);
            ASSERT(pstr);
            SetDropBoxSize(szDropBoxSize);
        } else if (_tcsicmp(pstrName, _T("arrownormalimage")) == 0)
            SetArrowNormalImage(pstrValue);
        else if (_tcsicmp(pstrName, _T("arrowhotimage")) == 0)
            SetArrowHotImage(pstrValue);
        else if (_tcsicmp(pstrName, _T("arrowpushedimage")) == 0)
            SetArrowPushedImage(pstrValue);
        else if (_tcsicmp(pstrName, _T("arrowfocusedimage")) == 0)
            SetArrowFocusedImage(pstrValue);
        else if (_tcsicmp(pstrName, _T("arrowdisabledimage")) == 0)
            SetArrowDisabledImage(pstrValue);
        else if (_tcsstr(pstrName, TEXT("item")) == pstrName) { // begin with "item"
            if (m_pWindow && m_pWindow->m_pList)
                m_pWindow->m_pList->SetAttribute(pstrName, pstrValue);
        } else {
            CContainerUI::SetAttribute(pstrName, pstrValue);
        }
    }

    bool CComboBoxUI::DoPaint(HDC hDC, const RECT &rcPaint, CControlUI *pStopControl) {
        return CControlUI::DoPaint(hDC, rcPaint, pStopControl);
    }

    void CComboBoxUI::PaintStatusImage(HDC hDC) {
        if( IsFocused() )
            m_uButtonState |= UISTATE_FOCUSED;
        else
            m_uButtonState &= ~ UISTATE_FOCUSED;

        if( !IsEnabled() )
            m_uButtonState |= UISTATE_DISABLED;
        else
            m_uButtonState &= ~ UISTATE_DISABLED;

        bool bDrawSuccessed = false;
        if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
            if( !m_sDisabledImage.IsEmpty() ) {
                bDrawSuccessed = DrawImage(hDC, (LPCTSTR)m_sDisabledImage);
            }
        } else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
            if( !m_sPushedImage.IsEmpty() ) {
                bDrawSuccessed = DrawImage(hDC, (LPCTSTR)m_sPushedImage);
            }
        } else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
            if( !m_sHotImage.IsEmpty() ) {
                bDrawSuccessed = DrawImage(hDC, (LPCTSTR)m_sHotImage);
            }
        } else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
            if( !m_sFocusedImage.IsEmpty() ) {
                bDrawSuccessed = DrawImage(hDC, (LPCTSTR)m_sFocusedImage);
            }
        }

        if(!bDrawSuccessed && !m_sNormalImage.IsEmpty() ) {
            if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) {}
        }

        // Draw Arrow
        //
        if ((m_uButtonState & UISTATE_DISABLED) != 0) {
            if (!m_sArrowDisabledImage.IsEmpty()) {
                if (!DrawImage(hDC, (LPCTSTR)m_sArrowDisabledImage)) {}
                else return;
            }
        } else if ((m_uButtonState & UISTATE_PUSHED) != 0) {
            if (!m_sArrowPushedImage.IsEmpty()) {
                if (!DrawImage(hDC, (LPCTSTR)m_sArrowPushedImage)) {}
                else return;
            }
        } else if ((m_uButtonState & UISTATE_HOT) != 0) {
            if (!m_sArrowHotImage.IsEmpty()) {
                if (!DrawImage(hDC, (LPCTSTR)m_sArrowHotImage)) {}
                else return;
            }
        } else if ((m_uButtonState & UISTATE_FOCUSED) != 0) {
            if (!m_sArrowFocusedImage.IsEmpty()) {
                if (!DrawImage(hDC, (LPCTSTR)m_sArrowFocusedImage)) {}
                else return;
            }
        }

        if (!m_sArrowNormalImage.IsEmpty()) {
            if (!DrawImage(hDC, (LPCTSTR)m_sArrowNormalImage)) {}
        }
    }

    void CComboBoxUI::PaintText(HDC hDC) {
        if( m_dwTextColor == 0 )
            m_dwTextColor = m_pManager->GetDefaultFontColor();

        if( m_dwDisabledTextColor == 0 )
            m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

        RECT rc = m_rcItem;
        rc.left += m_rcTextPadding.left;
        rc.right -= m_rcTextPadding.right;
        rc.top += m_rcTextPadding.top;
        rc.bottom -= m_rcTextPadding.bottom;

        CDuiString sText = GetText();

        if( sText.IsEmpty() )
            return;

        int nLinks = 0;

        if( IsEnabled() ) {
            if( m_bShowHtml )
                CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, m_dwTextColor, NULL, NULL, nLinks, m_iFont, m_uTextStyle);
            else
                CRenderEngine::DrawText(hDC, m_pManager, rc, sText, m_dwTextColor, m_iFont, m_uTextStyle);
        } else {
            if( m_bShowHtml )
                CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, m_dwDisabledTextColor, NULL, NULL, nLinks, m_iFont, m_uTextStyle);
            else
                CRenderEngine::DrawText(hDC, m_pManager, rc, sText, m_dwDisabledTextColor, m_iFont, m_uTextStyle);
        }
    }

} // namespace DuiLib
