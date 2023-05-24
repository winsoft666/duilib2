#ifndef __THREAD_UI_TASK_H__
#define __THREAD_UI_TASK_H__
#pragma once

#include <future>
#include <functional>

namespace DuiLib {
    UILIB_API bool IsInUIThread();

    template<class F, class... Args>
    auto PostTaskToUIThread(F&& f, Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(CPaintManagerUI::UIWorksMutex);
            CPaintManagerUI::UIWorks.emplace([task]() { (*task)(); });
            ::PostThreadMessage(CPaintManagerUI::GetUIThreadId(), WM_USER + 1234, WM_USER + 1234, WM_USER + 1234);
        }
        return res;
    }

}

#endif // !__THREAD_UI_TASK_H__