#pragma once

class DlgFake :
    public WindowImplBase {
public:
    DlgFake();
    ~DlgFake();
    void LoadURL(const std::wstring &url);
protected:
    virtual CDuiString GetSkinFile() override;
    virtual LPCTSTR GetWindowClassName(void) const override;
    void InitWindow() override;
    void OnFinalMessage(HWND hWnd) override;
private:
    CCefUI* web_;
};