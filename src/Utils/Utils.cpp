#include "StdAfx.h"
#include "Utils.h"

namespace DuiLib {

    /////////////////////////////////////////////////////////////////////////////////////
    //
    //

    CDuiPoint::CDuiPoint() {
        x = y = 0;
    }

    CDuiPoint::CDuiPoint(const POINT &src) {
        x = src.x;
        y = src.y;
    }

    CDuiPoint::CDuiPoint(int _x, int _y) {
        x = _x;
        y = _y;
    }

    CDuiPoint::CDuiPoint(LPARAM lParam) {
        x = GET_X_LPARAM(lParam);
        y = GET_Y_LPARAM(lParam);
    }


    /////////////////////////////////////////////////////////////////////////////////////
    //
    //

    CDuiSize::CDuiSize() {
        cx = cy = 0;
    }

    CDuiSize::CDuiSize(const SIZE &src) {
        cx = src.cx;
        cy = src.cy;
    }

    CDuiSize::CDuiSize(const RECT rc) {
        cx = rc.right - rc.left;
        cy = rc.bottom - rc.top;
    }

    CDuiSize::CDuiSize(int _cx, int _cy) {
        cx = _cx;
        cy = _cy;
    }


    /////////////////////////////////////////////////////////////////////////////////////
    //
    //

    CDuiRect::CDuiRect() {
        left = top = right = bottom = 0;
    }

    CDuiRect::CDuiRect(const RECT &src) {
        left = src.left;
        top = src.top;
        right = src.right;
        bottom = src.bottom;
    }

    CDuiRect::CDuiRect(int iLeft, int iTop, int iRight, int iBottom) {
        left = iLeft;
        top = iTop;
        right = iRight;
        bottom = iBottom;
    }

    int CDuiRect::GetWidth() const {
        return right - left;
    }

    int CDuiRect::GetHeight() const {
        return bottom - top;
    }

    void CDuiRect::Empty() {
        left = top = right = bottom = 0;
    }

    bool CDuiRect::IsNull() const {
        return (left == 0 && right == 0 && top == 0 && bottom == 0);
    }

    void CDuiRect::Join(const RECT &rc) {
        if( rc.left < left ) left = rc.left;

        if( rc.top < top ) top = rc.top;

        if( rc.right > right ) right = rc.right;

        if( rc.bottom > bottom ) bottom = rc.bottom;
    }

    void CDuiRect::ResetOffset() {
        ::OffsetRect(this, -left, -top);
    }

    void CDuiRect::Normalize() {
        if( left > right ) {
            int iTemp = left;
            left = right;
            right = iTemp;
        }

        if( top > bottom ) {
            int iTemp = top;
            top = bottom;
            bottom = iTemp;
        }
    }

    void CDuiRect::Offset(int cx, int cy) {
        ::OffsetRect(this, cx, cy);
    }

    void CDuiRect::Inflate(int cx, int cy) {
        ::InflateRect(this, cx, cy);
    }

    void CDuiRect::Deflate(int cx, int cy) {
        ::InflateRect(this, -cx, -cy);
    }

    void CDuiRect::Union(CDuiRect &rc) {
        ::UnionRect(this, this, &rc);
    }


    /////////////////////////////////////////////////////////////////////////////////////
    //
    //

    CStdPtrArray::CStdPtrArray(int iPreallocSize) : m_ppVoid(NULL), m_nCount(0), m_nAllocated(iPreallocSize) {
        ASSERT(iPreallocSize >= 0);

        if( iPreallocSize > 0 ) 
			m_ppVoid = static_cast<LPVOID *>(malloc(iPreallocSize * sizeof(LPVOID)));
    }

    CStdPtrArray::CStdPtrArray(const CStdPtrArray &src) : m_ppVoid(NULL), m_nCount(0), m_nAllocated(0) {
        for(int i = 0; i < src.GetSize(); i++)
            Add(src.GetAt(i));
    }

    CStdPtrArray::~CStdPtrArray() {
        if( m_ppVoid != NULL ) 
			free(m_ppVoid);
    }

    void CStdPtrArray::Empty() {
        if( m_ppVoid != NULL ) 
			free(m_ppVoid);

        m_ppVoid = NULL;
        m_nCount = m_nAllocated = 0;
    }

    void CStdPtrArray::Resize(int iSize) {
        Empty();
        m_ppVoid = static_cast<LPVOID *>(malloc(iSize * sizeof(LPVOID)));
        ::ZeroMemory(m_ppVoid, iSize * sizeof(LPVOID));
        m_nAllocated = iSize;
        m_nCount = iSize;
    }

    bool CStdPtrArray::IsEmpty() const {
        return m_nCount == 0;
    }

    bool CStdPtrArray::Add(LPVOID pData) {
        if( ++m_nCount >= m_nAllocated) {
            int nAllocated = m_nAllocated * 2;

            if( nAllocated == 0 ) 
				nAllocated = 11;

            LPVOID *ppVoid = static_cast<LPVOID *>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));

            if( ppVoid != NULL ) {
                m_nAllocated = nAllocated;
                m_ppVoid = ppVoid;
            } else {
                --m_nCount;
                return false;
            }
        }

        m_ppVoid[m_nCount - 1] = pData;
        return true;
    }

    bool CStdPtrArray::InsertAt(int iIndex, LPVOID pData) {
        if( iIndex == m_nCount ) 
			return Add(pData);

        if( iIndex < 0 || iIndex > m_nCount ) 
			return false;

        if( ++m_nCount >= m_nAllocated) {
            int nAllocated = m_nAllocated * 2;

            if( nAllocated == 0 ) 
				nAllocated = 11;

            LPVOID *ppVoid = static_cast<LPVOID *>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));

            if( ppVoid != NULL ) {
                m_nAllocated = nAllocated;
                m_ppVoid = ppVoid;
            } else {
                --m_nCount;
                return false;
            }
        }

        memmove(&m_ppVoid[iIndex + 1], &m_ppVoid[iIndex], (m_nCount - iIndex - 1) * sizeof(LPVOID));
        m_ppVoid[iIndex] = pData;
        return true;
    }

    bool CStdPtrArray::SetAt(int iIndex, LPVOID pData) {
        if( iIndex < 0 || iIndex >= m_nCount ) 
			return false;

        m_ppVoid[iIndex] = pData;
        return true;
    }

    bool CStdPtrArray::Remove(int iIndex) {
        if( iIndex < 0 || iIndex >= m_nCount ) 
			return false;

        if( iIndex < --m_nCount ) 
			::CopyMemory(m_ppVoid + iIndex, m_ppVoid + iIndex + 1, (m_nCount - iIndex) * sizeof(LPVOID));

        return true;
    }

    int CStdPtrArray::Find(LPVOID pData) const {
        for( int i = 0; i < m_nCount; i++ ) 
			if( m_ppVoid[i] == pData ) 
				return i;

        return -1;
    }

    int CStdPtrArray::GetSize() const {
        return m_nCount;
    }

    LPVOID *CStdPtrArray::GetData() {
        return m_ppVoid;
    }

    LPVOID CStdPtrArray::GetAt(int iIndex) const {
        if( iIndex < 0 || iIndex >= m_nCount ) 
			return NULL;

        return m_ppVoid[iIndex];
    }

    LPVOID CStdPtrArray::operator[] (int iIndex) const {
        ASSERT(iIndex >= 0 && iIndex < m_nCount);
        return m_ppVoid[iIndex];
    }


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CDuiString::CDuiString() : m_pstr(m_szBuffer)
	{
		m_szBuffer[0] = _T('\0');
	}

	CDuiString::CDuiString(const TCHAR ch) : m_pstr(m_szBuffer)
	{
		m_szBuffer[0] = ch;
		m_szBuffer[1] = _T('\0');
	}

	CDuiString::CDuiString(LPCTSTR lpsz, int nLen) : m_pstr(m_szBuffer)
	{
		ASSERT(!::IsBadStringPtr(lpsz, -1) || lpsz == NULL);
		m_szBuffer[0] = _T('\0');
		Assign(lpsz, nLen);
	}

	CDuiString::CDuiString(const CDuiString& src) : m_pstr(m_szBuffer)
	{
		m_szBuffer[0] = _T('\0');
		Assign(src.m_pstr);
	}

	CDuiString::~CDuiString()
	{
		if (m_pstr != m_szBuffer) free(m_pstr);
	}

	CDuiString CDuiString::ToString()
	{
		return m_pstr;
	}

	int CDuiString::GetLength() const
	{
		return (int)_tcslen(m_pstr);
	}

	CDuiString::operator LPCTSTR() const
	{
		return m_pstr;
	}

	void CDuiString::Append(LPCTSTR pstr)
	{
		int nNewLength = GetLength() + (int)_tcslen(pstr);
		if (nNewLength >= MAX_LOCAL_STRING_LEN) {
			if (m_pstr == m_szBuffer) {
				m_pstr = static_cast<LPTSTR>(malloc((nNewLength + 1) * sizeof(TCHAR)));
				_tcscpy(m_pstr, m_szBuffer);
				_tcscat(m_pstr, pstr);
			}
			else {
				m_pstr = static_cast<LPTSTR>(realloc(m_pstr, (nNewLength + 1) * sizeof(TCHAR)));
				_tcscat(m_pstr, pstr);
			}
		}
		else {
			if (m_pstr != m_szBuffer) {
				free(m_pstr);
				m_pstr = m_szBuffer;
			}
			_tcscat(m_szBuffer, pstr);
		}
	}

	void CDuiString::Assign(LPCTSTR pstr, int cchMax)
	{
		if (pstr == NULL) pstr = _T("");
		cchMax = (cchMax < 0 ? (int)_tcslen(pstr) : cchMax);
		if (cchMax < MAX_LOCAL_STRING_LEN) {
			if (m_pstr != m_szBuffer) {
				free(m_pstr);
				m_pstr = m_szBuffer;
			}
		}
		else if (cchMax > GetLength() || m_pstr == m_szBuffer) {
			if (m_pstr == m_szBuffer) m_pstr = NULL;
			m_pstr = static_cast<LPTSTR>(realloc(m_pstr, (cchMax + 1) * sizeof(TCHAR)));
		}
		_tcsncpy(m_pstr, pstr, cchMax);
		m_pstr[cchMax] = _T('\0');
	}

	bool CDuiString::IsEmpty() const
	{
		return m_pstr[0] == _T('\0');
	}

	void CDuiString::Empty()
	{
		if (m_pstr != m_szBuffer) free(m_pstr);
		m_pstr = m_szBuffer;
		m_szBuffer[0] = _T('\0');
	}

	LPCTSTR CDuiString::GetData() const
	{
		return m_pstr;
	}

	TCHAR CDuiString::GetAt(int nIndex) const
	{
		return m_pstr[nIndex];
	}

	TCHAR CDuiString::operator[] (int nIndex) const
	{
		return m_pstr[nIndex];
	}

	const CDuiString& CDuiString::operator=(const CDuiString& src)
	{
		Assign(src);
		return *this;
	}

	const CDuiString& CDuiString::operator=(LPCTSTR lpStr)
	{
		if (lpStr)
		{
			ASSERT(!::IsBadStringPtr(lpStr, -1));
			Assign(lpStr);
		}
		else
		{
			Empty();
		}
		return *this;
	}

#ifdef _UNICODE

	const CDuiString& CDuiString::operator=(LPCSTR lpStr)
	{
		if (lpStr)
		{
			ASSERT(!::IsBadStringPtrA(lpStr, -1));
			int cchStr = (int)strlen(lpStr) + 1;
			LPWSTR pwstr = (LPWSTR)_alloca(cchStr);
			if (pwstr != NULL) ::MultiByteToWideChar(::GetACP(), 0, lpStr, -1, pwstr, cchStr);
			Assign(pwstr);
		}
		else
		{
			Empty();
		}
		return *this;
	}

	const CDuiString& CDuiString::operator+=(LPCSTR lpStr)
	{
		if (lpStr)
		{
			ASSERT(!::IsBadStringPtrA(lpStr, -1));
			int cchStr = (int)strlen(lpStr) + 1;
			LPWSTR pwstr = (LPWSTR)_alloca(cchStr);
			if (pwstr != NULL) ::MultiByteToWideChar(::GetACP(), 0, lpStr, -1, pwstr, cchStr);
			Append(pwstr);
		}

		return *this;
	}

#else

	const CDuiString& CDuiString::operator=(LPCWSTR lpwStr)
	{
		if (lpwStr)
		{
			ASSERT(!::IsBadStringPtrW(lpwStr, -1));
			int cchStr = ((int)wcslen(lpwStr) * 2) + 1;
			LPSTR pstr = (LPSTR)_alloca(cchStr);
			if (pstr != NULL) ::WideCharToMultiByte(::GetACP(), 0, lpwStr, -1, pstr, cchStr, NULL, NULL);
			Assign(pstr);
		}
		else
		{
			Empty();
		}

		return *this;
	}

	const CDuiString& CDuiString::operator+=(LPCWSTR lpwStr)
	{
		if (lpwStr)
		{
			ASSERT(!::IsBadStringPtrW(lpwStr, -1));
			int cchStr = ((int)wcslen(lpwStr) * 2) + 1;
			LPSTR pstr = (LPSTR)_alloca(cchStr);
			if (pstr != NULL) ::WideCharToMultiByte(::GetACP(), 0, lpwStr, -1, pstr, cchStr, NULL, NULL);
			Append(pstr);
		}

		return *this;
	}

#endif // _UNICODE

	const CDuiString& CDuiString::operator=(const TCHAR ch)
	{
		Empty();
		m_szBuffer[0] = ch;
		m_szBuffer[1] = _T('\0');
		return *this;
	}

	CDuiString CDuiString::operator+(const CDuiString& src) const
	{
		CDuiString sTemp = *this;
		sTemp.Append(src);
		return sTemp;
	}

	CDuiString CDuiString::operator+(LPCTSTR lpStr) const
	{
		if (lpStr)
		{
			ASSERT(!::IsBadStringPtr(lpStr, -1));
			CDuiString sTemp = *this;
			sTemp.Append(lpStr);
			return sTemp;
		}

		return *this;
	}

	const CDuiString& CDuiString::operator+=(const CDuiString& src)
	{
		Append(src);
		return *this;
	}

	const CDuiString& CDuiString::operator+=(LPCTSTR lpStr)
	{
		if (lpStr)
		{
			ASSERT(!::IsBadStringPtr(lpStr, -1));
			Append(lpStr);
		}

		return *this;
	}

	const CDuiString& CDuiString::operator+=(const TCHAR ch)
	{
		TCHAR str[] = { ch, _T('\0') };
		Append(str);
		return *this;
	}

	bool CDuiString::operator == (LPCTSTR str) const { return (Compare(str) == 0); };
	bool CDuiString::operator != (LPCTSTR str) const { return (Compare(str) != 0); };
	bool CDuiString::operator <= (LPCTSTR str) const { return (Compare(str) <= 0); };
	bool CDuiString::operator <  (LPCTSTR str) const { return (Compare(str) < 0); };
	bool CDuiString::operator >= (LPCTSTR str) const { return (Compare(str) >= 0); };
	bool CDuiString::operator >  (LPCTSTR str) const { return (Compare(str) > 0); };

	void CDuiString::SetAt(int nIndex, TCHAR ch)
	{
		ASSERT(nIndex >= 0 && nIndex < GetLength());
		m_pstr[nIndex] = ch;
	}

	int CDuiString::Compare(LPCTSTR lpsz) const
	{
		return _tcscmp(m_pstr, lpsz);
	}

	int CDuiString::CompareNoCase(LPCTSTR lpsz) const
	{
		return _tcsicmp(m_pstr, lpsz);
	}

	void CDuiString::MakeUpper()
	{
		_tcsupr(m_pstr);
	}

	void CDuiString::MakeLower()
	{
		_tcslwr(m_pstr);
	}

	CDuiString CDuiString::Left(int iLength) const
	{
		if (iLength < 0) iLength = 0;
		if (iLength > GetLength()) iLength = GetLength();
		return CDuiString(m_pstr, iLength);
	}

	CDuiString CDuiString::Mid(int iPos, int iLength) const
	{
		if (iLength < 0) iLength = GetLength() - iPos;
		if (iPos + iLength > GetLength()) iLength = GetLength() - iPos;
		if (iLength <= 0) return CDuiString();
		return CDuiString(m_pstr + iPos, iLength);
	}

	CDuiString CDuiString::Right(int iLength) const
	{
		int iPos = GetLength() - iLength;
		if (iPos < 0) {
			iPos = 0;
			iLength = GetLength();
		}
		return CDuiString(m_pstr + iPos, iLength);
	}

	int CDuiString::Find(TCHAR ch, int iPos /*= 0*/) const
	{
		ASSERT(iPos >= 0 && iPos <= GetLength());
		if (iPos != 0 && (iPos < 0 || iPos >= GetLength())) return -1;
		LPCTSTR p = _tcschr(m_pstr + iPos, ch);
		if (p == NULL) return -1;
		return (int)(p - m_pstr);
	}

	int CDuiString::Find(LPCTSTR pstrSub, int iPos /*= 0*/) const
	{
		ASSERT(!::IsBadStringPtr(pstrSub, -1));
		ASSERT(iPos >= 0 && iPos <= GetLength());
		if (iPos != 0 && (iPos < 0 || iPos > GetLength())) return -1;
		LPCTSTR p = _tcsstr(m_pstr + iPos, pstrSub);
		if (p == NULL) return -1;
		return (int)(p - m_pstr);
	}

	int CDuiString::ReverseFind(TCHAR ch) const
	{
		LPCTSTR p = _tcsrchr(m_pstr, ch);
		if (p == NULL) return -1;
		return (int)(p - m_pstr);
	}

	int CDuiString::Replace(LPCTSTR pstrFrom, LPCTSTR pstrTo)
	{
		CDuiString sTemp;
		int nCount = 0;
		int iPos = Find(pstrFrom);
		if (iPos < 0) return 0;
		int cchFrom = (int)_tcslen(pstrFrom);
		int cchTo = (int)_tcslen(pstrTo);
		while (iPos >= 0) {
			sTemp = Left(iPos);
			sTemp += pstrTo;
			sTemp += Mid(iPos + cchFrom);
			Assign(sTemp);
			iPos = Find(pstrFrom, iPos + cchTo);
			nCount++;
		}
		return nCount;
	}

	int CDuiString::Format(LPCTSTR pstrFormat, ...)
	{
		LPTSTR szSprintf = NULL;
		va_list argList;
		int nLen;
		va_start(argList, pstrFormat);
		nLen = _vsntprintf(NULL, 0, pstrFormat, argList);
		szSprintf = (TCHAR*)malloc((nLen + 1) * sizeof(TCHAR));
		ZeroMemory(szSprintf, (nLen + 1) * sizeof(TCHAR));
		int iRet = _vsntprintf(szSprintf, nLen + 1, pstrFormat, argList);
		va_end(argList);
		Assign(szSprintf);
		free(szSprintf);
		return iRet;
	}

	int CDuiString::SmallFormat(LPCTSTR pstrFormat, ...)
	{
		CDuiString sFormat = pstrFormat;
		TCHAR szBuffer[64] = { 0 };
		va_list argList;
		va_start(argList, pstrFormat);
		int iRet = ::wvsprintf(szBuffer, sFormat, argList);
		va_end(argList);
		Assign(szBuffer);
		return iRet;
	}



    /////////////////////////////////////////////////////////////////////////////
    //
    //
    CStdStringPtrMap::CStdStringPtrMap() {
    }

    CStdStringPtrMap::~CStdStringPtrMap() {
    }

    void CStdStringPtrMap::RemoveAll() {
		m_Map.clear();
    }

    LPVOID CStdStringPtrMap::Find(LPCTSTR key) const {
		std::map<CDuiString, LPVOID>::const_iterator it = m_Map.find(key);
		if (it == m_Map.end())
			return NULL;
		const LPVOID value = m_Map.at(key);
		return value;
    }

    bool CStdStringPtrMap::Insert(LPCTSTR key, LPVOID pData) {
		m_Map[key] = pData;
        return true;
    }

    LPVOID CStdStringPtrMap::Set(LPCTSTR key, LPVOID pData) {
		LPVOID old = Find(key);
		m_Map[key] = pData;
        return old;
    }

    bool CStdStringPtrMap::Remove(LPCTSTR key) {
		std::map<CDuiString, LPVOID>::const_iterator it = m_Map.find(key);
		if (it == m_Map.end())
			return false;

		m_Map.erase(key);

        return true;
    }

    int CStdStringPtrMap::GetSize() const {
        return m_Map.size();
    }


	std::map<CDuiString, LPVOID>::iterator CStdStringPtrMap::Begin() {
		return m_Map.begin();
	}

	std::map<CDuiString, LPVOID>::iterator CStdStringPtrMap::End() {
		return m_Map.end();
	}

    bool StringIsInVector(const std::vector<CDuiString> &v, const CDuiString& str, bool bIgnoreCase) {
        for (size_t i = 0; i < v.size(); i++) {
            if (bIgnoreCase) {
                if (v[i].CompareNoCase(str.GetData()) == 0) {
                    return true;
                }
            }
            else {
                if (v[i].Compare(str.GetData()) == 0) {
                    return true;
                }
            }
        }
        return false;
    }

	bool IsDevtoolResourceExist() {
		wchar_t szFolderPath[MAX_PATH + 2] = { 0 };
		GetModuleFileNameW(NULL, szFolderPath, MAX_PATH);
		PathRemoveFileSpecW(szFolderPath);
		PathAddBackslashW(szFolderPath);
		PathCombineW(szFolderPath, szFolderPath, L"devtools_resources.pak");
		if (_waccess(szFolderPath, 0) == 0)
			return true;
		return false;
	}

	CLiteVariant::CLiteVariant() :
		m_DT(DataType::DT_UNKNOWN)
		, m_iData(0)
		, m_fData(0.f) {

	}

	CLiteVariant::CLiteVariant(const CLiteVariant& other) {
		this->m_DT = other.GetType();
		this->m_iData = other.GetInt();
		this->m_fData = other.GetDouble();
		this->m_strData = other.GetString();
	}

	CLiteVariant::CLiteVariant(int i) {
		SetInt(i);
	}

	CLiteVariant::CLiteVariant(double f) {
		SetDouble(f);
	}

	CLiteVariant::CLiteVariant(const std::string &s) {
		SetString(s);
	}

	CLiteVariant::~CLiteVariant() {
		this->m_strData.clear();
		this->m_iData = 0;
		this->m_fData = 0.f;
	}

	void CLiteVariant::SetType(DataType dt) {
		m_DT = dt;
	}

	void CLiteVariant::SetInt(int i) {
		m_iData = i;
		m_DT = DataType::DT_INT;
	}

	void CLiteVariant::SetDouble(double f) {
		m_fData = f;
		m_DT = DataType::DT_DOUBLE;
	}

	void CLiteVariant::SetString(const std::string &s) {
		m_strData = s;
		m_DT = DataType::DT_STRING;
	}

	bool CLiteVariant::IsInt() const {
		return m_DT == DataType::DT_INT;
	}

	bool CLiteVariant::IsDouble() const {
		return m_DT == DataType::DT_DOUBLE;
	}

	bool CLiteVariant::IsString() const {
		return m_DT == DataType::DT_STRING;
	}

	CLiteVariant::DataType CLiteVariant::GetType() const {
		return m_DT;
	}

	int CLiteVariant::GetInt() const {
		return m_iData;
	}

	double CLiteVariant::GetDouble() const {
		return m_fData;
	}

	std::string CLiteVariant::GetString() const {
		return m_strData;
	}

	const CLiteVariant& CLiteVariant::operator=(const CLiteVariant& other) {
		this->m_DT = other.GetType();
		this->m_iData = other.GetInt();
		this->m_fData = other.GetDouble();
		this->m_strData = other.GetString();
		return *this;
	}


    std::string Unicode2Ansi(const std::wstring &str, unsigned int code_page /*= 0*/) {
        std::string strRes;
        int iSize = ::WideCharToMultiByte(code_page, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

        if (iSize == 0)
            return strRes;

        char *szBuf = new (std::nothrow) char[iSize];

        if (!szBuf)
            return strRes;

        memset(szBuf, 0, iSize);

        ::WideCharToMultiByte(code_page, 0, str.c_str(), -1, szBuf, iSize, NULL, NULL);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    std::wstring Ansi2Unicode(const std::string &str, unsigned int code_page /*= 0*/) {
        std::wstring strRes;

        int iSize = ::MultiByteToWideChar(code_page, 0, str.c_str(), -1, NULL, 0);

        if (iSize == 0)
            return strRes;

        wchar_t *szBuf = new (std::nothrow) wchar_t[iSize];

        if (!szBuf)
            return strRes;

        memset(szBuf, 0, iSize * sizeof(wchar_t));

        ::MultiByteToWideChar(code_page, 0, str.c_str(), -1, szBuf, iSize);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    std::string Unicode2Utf8(const std::wstring &str) {
        std::string strRes;

        int iSize = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

        if (iSize == 0)
            return strRes;

        char *szBuf = new (std::nothrow) char[iSize];

        if (!szBuf)
            return strRes;

        memset(szBuf, 0, iSize);

        ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, szBuf, iSize, NULL, NULL);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    std::string Unicode2Utf8BOM(const std::wstring &str) {
        std::string strRes;

        int iSize = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

        if (iSize == 0)
            return strRes;

        char *szBuf = new (std::nothrow) char[iSize + 3];

        if (!szBuf)
            return strRes;

        memset(szBuf, 0, iSize + 3);

        ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &szBuf[3], iSize, NULL, NULL);
        szBuf[0] = 0xef;
        szBuf[1] = 0xbb;
        szBuf[2] = 0xbf;

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }


    std::wstring Utf82Unicode(const std::string &str) {
        std::wstring strRes;
        int iSize = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

        if (iSize == 0)
            return strRes;

        wchar_t *szBuf = new (std::nothrow) wchar_t[iSize];

        if (!szBuf)
            return strRes;

        memset(szBuf, 0, iSize * sizeof(wchar_t));
        ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, szBuf, iSize);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    std::string Ansi2Utf8(const std::string &str, unsigned int code_page /*= 0*/) {
        return Unicode2Utf8(Ansi2Unicode(str, code_page));
    }

    std::string Ansi2Utf8BOM(const std::string &str, unsigned int code_page /* = 0*/) {
        return Unicode2Utf8BOM(Ansi2Unicode(str, code_page));
    }

    std::string Utf82Ansi(const std::string &str, unsigned int code_page /*= 0*/) {
        return Unicode2Ansi(Utf82Unicode(str), code_page);
    }


    CriticalSection::CriticalSection() {
        InitializeCriticalSection(&crit_);
    }
    CriticalSection::~CriticalSection() {
        DeleteCriticalSection(&crit_);
    }
    void CriticalSection::Enter() const {
        EnterCriticalSection(&crit_);
    }
    void CriticalSection::Leave() const {
        LeaveCriticalSection(&crit_);
    }

    bool CriticalSection::TryEnter() const {
        return TryEnterCriticalSection(&crit_) != FALSE;
    }

    CritScope::CritScope(const CriticalSection *pCS) : crit_(pCS) {
        crit_->Enter();
    }

    CritScope::~CritScope() {
        crit_->Leave();
    }

    std::wstring GetCurrentProcessDirectoryW() {
        wchar_t szPath[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, szPath, MAX_PATH);
        PathRemoveFileSpecW(szPath);
        PathAddBackslashW(szPath);
        return szPath;
    }

    std::string GetCurrentProcessDirectoryA() {
        char szPath[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, szPath, MAX_PATH);
        PathRemoveFileSpecA(szPath);
        PathAddBackslashA(szPath);
        return szPath;
    }

    void TraceMsgW(const wchar_t *lpFormat, ...) {
        if (!lpFormat)
            return;

        wchar_t *pMsgBuffer = NULL;
        unsigned int iMsgBufCount = 0;

        va_list arglist;
        va_start(arglist, lpFormat);
        HRESULT hr = STRSAFE_E_INSUFFICIENT_BUFFER;

        while (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
            iMsgBufCount += 1024;

            if (pMsgBuffer) {
                free(pMsgBuffer);
                pMsgBuffer = NULL;
            }

            pMsgBuffer = (wchar_t *)malloc(iMsgBufCount * sizeof(wchar_t));

            if (!pMsgBuffer) {
                break;
            }

            hr = StringCchVPrintfW(pMsgBuffer, iMsgBufCount, lpFormat, arglist);
        }

        va_end(arglist);

        if (hr == S_OK) {
            if (pMsgBuffer)
                OutputDebugStringW(pMsgBuffer);
        }

        if (pMsgBuffer) {
            free(pMsgBuffer);
            pMsgBuffer = NULL;
        }
    }

    void TraceMsgA(const char *lpFormat, ...) {
        if (!lpFormat)
            return;

        char *pMsgBuffer = NULL;
        unsigned int iMsgBufCount = 0;

        va_list arglist;
        va_start(arglist, lpFormat);
        HRESULT hr = STRSAFE_E_INSUFFICIENT_BUFFER;

        while (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
            iMsgBufCount += 1024;

            if (pMsgBuffer) {
                free(pMsgBuffer);
                pMsgBuffer = NULL;
            }

            pMsgBuffer = (char *)malloc(iMsgBufCount * sizeof(char));

            if (!pMsgBuffer) {
                break;
            }

            hr = StringCchVPrintfA(pMsgBuffer, iMsgBufCount, lpFormat, arglist);
        }

        va_end(arglist);

        if (hr == S_OK) {
            if (pMsgBuffer)
                OutputDebugStringA(pMsgBuffer);
        }

        if (pMsgBuffer) {
            free(pMsgBuffer);
            pMsgBuffer = NULL;
        }
    }


    BOOL UIPIMsgFilter(HWND hWnd, UINT uMessageID, BOOL bAllow) {
        OSVERSIONINFO VersionTmp;
        VersionTmp.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&VersionTmp);
        BOOL res = FALSE;

        if (VersionTmp.dwMajorVersion >= 6) { // vista above.
            BOOL(WINAPI * pfnChangeMessageFilterEx)(HWND, UINT, DWORD, PCHANGEFILTERSTRUCT);
            BOOL(WINAPI * pfnChangeMessageFilter)(UINT, DWORD);

            CHANGEFILTERSTRUCT filterStatus;
            filterStatus.cbSize = sizeof(CHANGEFILTERSTRUCT);

            HINSTANCE hlib = LoadLibrary(_T("user32.dll"));

            if (hlib != NULL) {
                (FARPROC &)pfnChangeMessageFilterEx = GetProcAddress(hlib, "ChangeWindowMessageFilterEx");

                if (pfnChangeMessageFilterEx != NULL && hWnd != NULL) {
                    res = pfnChangeMessageFilterEx(hWnd, uMessageID, (bAllow ? MSGFLT_ADD : MSGFLT_REMOVE), &filterStatus);
                }

                // If failed, try again.
                if (!res) {
                    (FARPROC &)pfnChangeMessageFilter = GetProcAddress(hlib, "ChangeWindowMessageFilter");

                    if (pfnChangeMessageFilter != NULL) {
                        res = pfnChangeMessageFilter(uMessageID, (bAllow ? MSGFLT_ADD : MSGFLT_REMOVE));
                    }
                }
            }

            if (hlib != NULL) {
                FreeLibrary(hlib);
            }
        } else {
            res = TRUE;
        }

        return res;
    }


    TimerBase::TimerBase() {
        m_hTimer = NULL;
        m_pTimer = NULL;
    }

    TimerBase::~TimerBase() {

    }

    void CALLBACK TimerBase::TimerProc(void *param, BOOLEAN timerCalled) {
        UNREFERENCED_PARAMETER(timerCalled);
        TimerBase *timer = static_cast<TimerBase *>(param);

        timer->OnTimedEvent();
    }

    BOOL TimerBase::Start(DWORD ulInterval,  // ulInterval in ms
        BOOL bImmediately,
        BOOL bOnce,
        ULONG dwFlags /* = WT_EXECUTELONGFUNCTION */) {
        BOOL bRet = FALSE;

        if (!m_hTimer) {
            bRet = CreateTimerQueueTimer(&m_hTimer,
                NULL,
                TimerProc,
                (PVOID)this,
                bImmediately ? 0 : ulInterval,
                bOnce ? 0 : ulInterval,
                dwFlags);
        }

        return bRet;
    }

    void TimerBase::Stop(bool bWait) {
        if (m_hTimer) {
            DeleteTimerQueueTimer(NULL, m_hTimer, bWait ? INVALID_HANDLE_VALUE : NULL);
            m_hTimer = NULL;
        }
    }

    void TimerBase::OnTimedEvent() {
    }
} // namespace DuiLib
