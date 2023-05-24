#include "StdAfx.h"
#include "Utils/Task.h"

namespace DuiLib {
    bool IsInUIThread() {
        return GetCurrentThreadId() == CPaintManagerUI::GetUIThreadId();
    }
}