#include "stdafx.h"
#include "DlgFake.h"

#ifdef UILIB_WITH_CEF
DlgFake::DlgFake() : web_(NULL) {

}

DlgFake::~DlgFake() {

}

void DlgFake::LoadURL(const std::wstring &url) {
    if (web_) {
        web_->SetUrl(url.c_str());
    }
}

DuiLib::CDuiString DlgFake::GetSkinFile() {
    return TEXT("fake.xml");
}

LPCTSTR DlgFake::GetWindowClassName(void) const {
    return TEXT("DLG_FAKE_23434");
}

void DlgFake::InitWindow() {
    SetAutoCloseCefWhenWindowCloseMsg(true, 0, 0);
    web_ = static_cast<CCefUI*>(m_PaintManager.FindControl(TEXT("webMain")));
}

void DlgFake::OnFinalMessage(HWND hWnd) {
    delete this;
}
#endif