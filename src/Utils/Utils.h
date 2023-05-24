#ifndef __UTILS_H__
#define __UTILS_H__

#pragma once
#include "OAIdl.h"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>
#include "UIlibExport.h"

namespace DuiLib {

    class UILIB_API STRINGorID {
      public:
        STRINGorID(LPCTSTR lpString) : m_lpstr(lpString) { }
        STRINGorID(UINT nID) : m_lpstr(MAKEINTRESOURCE(nID)) { }
        LPCTSTR m_lpstr;
    };

    class UILIB_API CDuiPoint : public tagPOINT {
      public:
        CDuiPoint();
        CDuiPoint(const POINT &src);
        CDuiPoint(int x, int y);
        CDuiPoint(LPARAM lParam);
    };

    class UILIB_API CDuiSize : public tagSIZE {
      public:
        CDuiSize();
        CDuiSize(const SIZE &src);
        CDuiSize(const RECT rc);
        CDuiSize(int cx, int cy);
    };

    class UILIB_API CDuiRect : public tagRECT {
      public:
        CDuiRect();
        CDuiRect(const RECT &src);
        CDuiRect(int iLeft, int iTop, int iRight, int iBottom);

        int GetWidth() const;
        int GetHeight() const;
        void Empty();
        bool IsNull() const;
        void Join(const RECT &rc);
        void ResetOffset();
        void Normalize();
        void Offset(int cx, int cy);
        void Inflate(int cx, int cy);
        void Deflate(int cx, int cy);
        void Union(CDuiRect &rc);
    };

    class UILIB_API CStdPtrArray {
      public:
        CStdPtrArray(int iPreallocSize = 0);
        CStdPtrArray(const CStdPtrArray &src);
        ~CStdPtrArray();

        void Empty();
        void Resize(int iSize);
        bool IsEmpty() const;
        int Find(LPVOID iIndex) const;
        bool Add(LPVOID pData);
        bool SetAt(int iIndex, LPVOID pData);
        bool InsertAt(int iIndex, LPVOID pData);
        bool Remove(int iIndex);
        int GetSize() const;
        LPVOID *GetData();

        LPVOID GetAt(int iIndex) const;
        LPVOID operator[] (int nIndex) const;

      protected:
        LPVOID *m_ppVoid;
        int m_nCount;
        int m_nAllocated;
    };

    class UILIB_API CDuiString {
      public:
        enum { MAX_LOCAL_STRING_LEN = 63 };

        CDuiString();
        CDuiString(const TCHAR ch);
        CDuiString(const CDuiString &src);
        CDuiString(LPCTSTR lpsz, int nLen = -1);
        ~CDuiString();
        CDuiString ToString();

        void Empty();
        int GetLength() const;
        bool IsEmpty() const;
        TCHAR GetAt(int nIndex) const;
        void Append(LPCTSTR pstr);
        void Assign(LPCTSTR pstr, int nLength = -1);
        LPCTSTR GetData() const;

        void SetAt(int nIndex, TCHAR ch);
        operator LPCTSTR() const;

        TCHAR operator[] (int nIndex) const;
        const CDuiString &operator=(const CDuiString &src);
        const CDuiString &operator=(const TCHAR ch);
        const CDuiString &operator=(LPCTSTR pstr);
#ifdef _UNICODE
        const CDuiString &operator=(LPCSTR lpStr);
        const CDuiString &operator+=(LPCSTR lpStr);
#else
        const CDuiString &operator=(LPCWSTR lpwStr);
        const CDuiString &operator+=(LPCWSTR lpwStr);
#endif
        CDuiString operator+(const CDuiString &src) const;
        CDuiString operator+(LPCTSTR pstr) const;
        const CDuiString &operator+=(const CDuiString &src);
        const CDuiString &operator+=(LPCTSTR pstr);
        const CDuiString &operator+=(const TCHAR ch);

        bool operator == (LPCTSTR str) const;
        bool operator != (LPCTSTR str) const;
        bool operator <= (LPCTSTR str) const;
        bool operator <  (LPCTSTR str) const;
        bool operator >= (LPCTSTR str) const;
        bool operator >  (LPCTSTR str) const;

        int Compare(LPCTSTR pstr) const;
        int CompareNoCase(LPCTSTR pstr) const;

        void MakeUpper();
        void MakeLower();

        CDuiString Left(int nLength) const;
        CDuiString Mid(int iPos, int nLength = -1) const;
        CDuiString Right(int nLength) const;

        int Find(TCHAR ch, int iPos = 0) const;
        int Find(LPCTSTR pstr, int iPos = 0) const;
        int ReverseFind(TCHAR ch) const;
        int Replace(LPCTSTR pstrFrom, LPCTSTR pstrTo);

        int __cdecl Format(LPCTSTR pstrFormat, ...);
        int __cdecl SmallFormat(LPCTSTR pstrFormat, ...);

      protected:
        LPTSTR m_pstr;
        TCHAR m_szBuffer[MAX_LOCAL_STRING_LEN + 1];
    };

    UILIB_API bool StringIsInVector(const std::vector<CDuiString> &v, const CDuiString &str, bool bIgnoreCase);

    class UILIB_API CStdStringPtrMap {
      public:
        CStdStringPtrMap();
        ~CStdStringPtrMap();

        LPVOID Find(LPCTSTR key) const;
        bool Insert(LPCTSTR key, LPVOID pData);
        LPVOID Set(LPCTSTR key, LPVOID pData);
        bool Remove(LPCTSTR key);
        void RemoveAll();
        int GetSize() const;
        std::map<CDuiString, LPVOID>::iterator Begin();
        std::map<CDuiString, LPVOID>::iterator End();
      protected:
        std::map<CDuiString, LPVOID> m_Map;
    };

    class UILIB_API CDuiVariant : public VARIANT {
      public:
        CDuiVariant() {
            VariantInit(this);
        }
        CDuiVariant(int i) {
            VariantInit(this);
            this->vt = VT_I4;
            this->intVal = i;
        }
        CDuiVariant(float f) {
            VariantInit(this);
            this->vt = VT_R4;
            this->fltVal = f;
        }
        CDuiVariant(LPOLESTR s) {
            VariantInit(this);
            this->vt = VT_BSTR;
            this->bstrVal = s;
        }
        CDuiVariant(IDispatch *disp) {
            VariantInit(this);
            this->vt = VT_DISPATCH;
            this->pdispVal = disp;
        }

        ~CDuiVariant() {
            VariantClear(this);
        }
    };

    UILIB_API bool IsDevtoolResourceExist();

    class UILIB_API CLiteVariant {
      public:
        enum class DataType {
            DT_UNKNOWN = 0,
            DT_INT,
            DT_DOUBLE,
            DT_STRING
        };
        CLiteVariant();
        CLiteVariant(const CLiteVariant &other);
        CLiteVariant(int i);
        CLiteVariant(double f);
        CLiteVariant(const std::string &s);
        ~CLiteVariant();

        void SetType(DataType dt);
        void SetInt(int i);
        void SetDouble(double f);
        void SetString(const std::string &s);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsString() const;

        DataType GetType() const;
        int GetInt() const;
        double GetDouble() const;
        std::string GetString() const;

        const CLiteVariant &operator=(const CLiteVariant &other);
      private:
        DataType m_DT;
        int m_iData;
        double m_fData;
        std::string m_strData;
    };


   UILIB_API std::string Unicode2Ansi(const std::wstring &str, unsigned int code_page = 0);
   UILIB_API std::wstring Ansi2Unicode(const std::string &str, unsigned int code_page = 0);
   UILIB_API std::string Unicode2Utf8(const std::wstring &str);
   UILIB_API std::wstring Utf82Unicode(const std::string &str);
   UILIB_API std::string Ansi2Utf8(const std::string &str, unsigned int code_page = 0);
   UILIB_API std::string Utf82Ansi(const std::string &str, unsigned int code_page = 0);
   UILIB_API std::string Unicode2Utf8BOM(const std::wstring &str);
   UILIB_API std::string Ansi2Utf8BOM(const std::string &str, unsigned int code_page = 0);

   class UILIB_API CriticalSection {
   public:
       CriticalSection();
       ~CriticalSection();
       void Enter() const;
       void Leave() const;
       bool TryEnter() const;
   private:
       CriticalSection(const CriticalSection &refCritSec);
       CriticalSection &operator=(const CriticalSection &refCritSec);
       mutable CRITICAL_SECTION crit_;
   };

   class UILIB_API CritScope {
   public:
       explicit CritScope(const CriticalSection *pCS);
       ~CritScope();
   private:
       const CriticalSection *const crit_;
       CritScope(const CritScope&) = delete;
       void operator=(const CritScope&) = delete;
   };

   UILIB_API std::wstring GetCurrentProcessDirectoryW();
   UILIB_API std::string GetCurrentProcessDirectoryA();
#if defined(UNICODE) || defined(_UNICODE)
#define GetCurrentProcessDirectory DuiLib::GetCurrentProcessDirectoryW
#else
#define GetCurrentProcessDirectory DuiLib::GetCurrentProcessDirectoryA
#endif


   UILIB_API void TraceMsgW(const wchar_t *lpFormat, ...);
   UILIB_API void TraceMsgA(const char *lpFormat, ...);

   UILIB_API BOOL UIPIMsgFilter(HWND hWnd, UINT uMessageID, BOOL bAllow);


   template<class T>
   class Singleton {
   public:
       static T *Instance();
       static void Release();
   protected:
       Singleton() {}
       Singleton(const Singleton &) {}
       Singleton &operator=(const Singleton &) {}
   private:
       static T *this_;
       static std::mutex m_;
   };


   template<class T>
   T  *Singleton<T>::this_ = nullptr;

   template<class T>
   std::mutex Singleton<T>::m_;


   template<class T>
   T *Singleton<T>::Instance(void) {
       //double-check
       if (this_ == nullptr) {
           std::lock_guard<std::mutex> lg(m_);
           if (this_ == nullptr) {
               this_ = new T;
           }
       }
       return this_;
   }

   template<class T>
   void Singleton<T>::Release(void) {
       if (this_) {
           delete this_;
       }
   }

#define SINGLETON_CLASS_DECLARE(class_name)	\
    friend class DuiLib::Singleton<##class_name>;

   class UILIB_API TimerBase {
   public:
       TimerBase();
       virtual ~TimerBase();
       static void CALLBACK TimerProc(void *param, BOOLEAN timerCalled);

       // About dwFlags, see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms682485(v=vs.85).aspx
       //
       BOOL Start(DWORD ulInterval,  // ulInterval in ms
           BOOL bImmediately,
           BOOL bOnce,
           ULONG dwFlags = WT_EXECUTELONGFUNCTION);
       void Stop(bool bWait);
       virtual void OnTimedEvent();
   private:
       HANDLE m_hTimer;
       PTP_TIMER m_pTimer;
   };

   template <class T>
   class TTimer : public TimerBase {
   public:
       typedef private void (T::*POnTimer)(void);

       TTimer() {
           m_pClass = NULL;
           m_pfnOnTimer = NULL;
       }

       void SetTimedEvent(T *pClass, POnTimer pFunc) {
           m_pClass = pClass;
           m_pfnOnTimer = pFunc;
       }

   protected:
       void OnTimedEvent() override {
           if (m_pfnOnTimer && m_pClass) {
               (m_pClass->*m_pfnOnTimer)();
           }
       }

   private:
       T *m_pClass;
       POnTimer m_pfnOnTimer;
   };

   class UILIB_API Timer : public TimerBase {
   public:
       typedef std::function<void()> FN_CB;
       Timer() {

       }

       Timer(FN_CB cb) {
           SetTimedEvent(cb);
       }

       void SetTimedEvent(FN_CB cb) {
           m_cb = cb;
       }

   protected:
       void OnTimedEvent() override {
           if (m_cb) {
               m_cb();
           }
       }

   private:
       FN_CB m_cb;
   };
}// namespace DuiLib

#endif // __UTILS_H__