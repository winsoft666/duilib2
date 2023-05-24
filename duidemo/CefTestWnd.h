#pragma once
#ifdef UILIB_WITH_CEF
class CefTestWnd : 
    public WindowImplBase {
public:
    CefTestWnd();
    ~CefTestWnd();

    CDuiString GetSkinFile() override {
        return TEXT("cef.xml");
    }

    LPCTSTR GetWindowClassName() const override {
        return TEXT("DlgCEFTESTBA9B2BC0");
    }

    void Notify(TNotifyUI& msg) override;
    void OnFinalMessage(HWND hWnd) override;
    LRESULT ResponseDefaultKeyEvent(WPARAM wParam) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
private:
    void OnWindowInit();
    void Web2CallJS();
private:

	CCefUI* web1_;
	CCefUI* web2_;

	CButtonUI* btn_min_;
	CButtonUI* btn_close_;

	CButtonUI* btn_go_back_1_;
	CButtonUI* btn_go_foward_1_;
	CButtonUI* btn_refresh_1_;
	CButtonUI* btn_go_1_;
	CButtonUI* btn_open_devtools_1_;
	CButtonUI* btn_close_devtools_1_;
	CButtonUI* btn_call_js_1_;
	CEditUI*   edt_url_1_;

	CButtonUI* btn_go_back_2_;
	CButtonUI* btn_go_foward_2_;
	CButtonUI* btn_refresh_2_;
	CButtonUI* btn_go_2_;
	CButtonUI* btn_open_devtools_2_;
	CButtonUI* btn_close_devtools_2_;
	CButtonUI* btn_call_js_2_;
	CEditUI*   edt_url_2_;
};
#endif
