#pragma once
#include "Utility.h"
#include "CRefStr.h"
#include "CRefBin.h"

#include <functional>

ECK_NAMESPACE_BEGIN
class CRegKey
{
private:
	HKEY m_hKey = nullptr;
public:
	static HKEY AnalyzeFullPath(PCWSTR pszPath, int& cchSkip)
	{
#ifdef _DEBUG
		if (wcsncmp(pszPath, L"HKU", 3u) == 0)
			EckAssert(wcslen(pszPath) > 4);
		else
			EckAssert(wcslen(pszPath) > 5);
#endif
		HKEY hKeyRoot;
		cchSkip = 5;
		if (wcsncmp(pszPath, L"HKLM", 4u) == 0)
			hKeyRoot = HKEY_LOCAL_MACHINE;
		else if (wcsncmp(pszPath, L"HKCU", 4u) == 0)
			hKeyRoot = HKEY_CURRENT_USER;
		else if (wcsncmp(pszPath, L"HKCR", 4u) == 0)
			hKeyRoot = HKEY_CLASSES_ROOT;
		else if (wcsncmp(pszPath, L"HKU", 3u) == 0)
		{
			hKeyRoot = HKEY_USERS;
			cchSkip = 4;
		}
		else if (wcsncmp(pszPath, L"HKCC", 4u) == 0)
			hKeyRoot = HKEY_CURRENT_CONFIG;
		else
		{
			hKeyRoot = nullptr;
			cchSkip = 0;
		}
		return hKeyRoot;
	}

	ECK_DISABLE_COPY_DEF_CONS(CRegKey);

	/// <summary>
	/// 构造自路径
	/// </summary>
	/// <param name="pszKey">注册项路径，应当以HKLM、HKCU、HKCR、HKU、HKCC五者其中之一开头</param>
	/// <param name="dwAccess">访问权限</param>
	CRegKey(PCWSTR pszPath, REGSAM dwAccess = KEY_READ | KEY_WRITE, LSTATUS* plStatus = nullptr)
	{
		if (!pszPath)
			return;
		int cchSkip = 0;
		if (const auto hKeyRoot = AnalyzeFullPath(pszPath, cchSkip))
		{
			auto r = Open(hKeyRoot, pszPath + cchSkip, dwAccess);
			if (plStatus)
				*plStatus = r;
		}
		else
		{
			if (plStatus)
				*plStatus = ERROR_INVALID_PARAMETER;
		}
	}

	CRegKey(HKEY hKey, PCWSTR pszKey, REGSAM dwAccess = KEY_READ | KEY_WRITE, LSTATUS* plStatus = nullptr)
	{
		const auto r = Open(hKey, pszKey, dwAccess);
		if (plStatus)
			*plStatus = r;
	}

	CRegKey(HKEY hKey) :m_hKey{ hKey } {}

	~CRegKey()
	{
		Close();
	}

	constexpr CRegKey(CRegKey&& x) noexcept :m_hKey{ x.m_hKey } { x.m_hKey = nullptr; }

	constexpr CRegKey& operator=(CRegKey&& x) noexcept
	{
		std::swap(m_hKey, x.m_hKey);
		return *this;
	}

	[[nodiscard]] EckInline HKEY Attach(HKEY hKey)
	{
		std::swap(m_hKey, hKey);
		return hKey;
	}

	[[nodiscard]] EckInline HKEY Detach() { return Attach(nullptr); }

	[[nodiscard]] EckInline HKEY GetHKey() const { return m_hKey; }

	EckInline LSTATUS Close()
	{
		const auto r = RegCloseKey(m_hKey);
		m_hKey = nullptr;
		return r;
	}

	EckInline LSTATUS Open(HKEY hKey, PCWSTR pszSubKey, REGSAM dwAccess = KEY_READ | KEY_WRITE, DWORD dwOption = 0u)
	{
		HKEY hKeyNew;
		const auto ls = RegOpenKeyExW(hKey, pszSubKey, dwOption, dwAccess, &hKeyNew);
		Close();
		m_hKey = hKeyNew;
		return ls;
	}

	EckInline LSTATUS Open(PCWSTR pszPath, REGSAM dwAccess = KEY_READ | KEY_WRITE, DWORD dwOption = 0u)
	{
		int cchSkip = 0;
		auto hKeyRoot = AnalyzeFullPath(pszPath, cchSkip);
		if (!hKeyRoot)
			return ERROR_INVALID_PARAMETER;
		return Open(hKeyRoot, pszPath + cchSkip, dwAccess, dwOption);
	}

	EckInline LSTATUS CopyTree(PCWSTR pszSubKey, HKEY hKeyDst)
	{
		return RegCopyTreeW(m_hKey, pszSubKey, hKeyDst);
	}

	EckInline LSTATUS Create(HKEY hKey, PCWSTR pszSubKey, REGSAM dwAccess = KEY_READ | KEY_WRITE,
		DWORD dwOptions = REG_OPTION_NON_VOLATILE, DWORD* pdwDisposition = nullptr,
		PWSTR pszClass = nullptr, SECURITY_ATTRIBUTES* psa = nullptr)
	{
		HKEY hKeyNew;
		const auto ls = RegCreateKeyExW(hKey, pszSubKey, 0, pszClass, dwOptions,
			dwAccess, psa, &hKeyNew, pdwDisposition);
		Close();
		m_hKey = hKeyNew;
		return ls;
	}

	EckInline LSTATUS Create(PCWSTR pszPath, REGSAM dwAccess = KEY_READ | KEY_WRITE,
		DWORD dwOptions = REG_OPTION_NON_VOLATILE, DWORD* pdwDisposition = nullptr,
		PWSTR pszClass = nullptr, SECURITY_ATTRIBUTES* psa = nullptr)
	{
		int cchSkip = 0;
		auto hKeyRoot = AnalyzeFullPath(pszPath, cchSkip);
		if (!hKeyRoot)
			return ERROR_INVALID_PARAMETER;
		return Create(hKeyRoot, pszPath + cchSkip, dwAccess, dwOptions, pdwDisposition, pszClass, psa);
	}

	EckInline LSTATUS Delete(PCWSTR pszSubKey, DWORD dwAccess)
	{
		return RegDeleteKeyExW(m_hKey, pszSubKey, dwAccess, 0);
	}

	EckInline LSTATUS DeleteValue(PCWSTR pszSubKey, PCWSTR pszValue)
	{
		return RegDeleteKeyValueW(m_hKey, pszSubKey, pszValue);
	}

	EckInline LSTATUS DeleteTree(PCWSTR pszSubKey)
	{
		return RegDeleteTreeW(m_hKey, pszSubKey);
	}

	EckInline LSTATUS DeleteValue(PCWSTR pszValue)
	{
		return RegDeleteValueW(m_hKey, pszValue);
	}

	EckInline LSTATUS EnumKey(DWORD idx, PWSTR pszKeyNameBuf, DWORD* pcchKeyNameBuf,
		PWSTR pszClassNameBuf = nullptr, DWORD* pcchClassName = nullptr, FILETIME* pftLastWrite = nullptr)
	{
		return RegEnumKeyExW(m_hKey, idx, pszKeyNameBuf, pcchKeyNameBuf, nullptr,
			pszClassNameBuf, pcchClassName, pftLastWrite);
	}

	int EnumKey(std::invocable<CRefStrW&> auto&& Fn,
		_Out_opt_ LSTATUS* plStatus = nullptr)
	{
		if (plStatus)
			*plStatus = ERROR_SUCCESS;
		DWORD cchMaxSub = 0;
		QueryInfo(nullptr, nullptr, nullptr, &cchMaxSub);
		DWORD idx = 0;
		CRefStrW rs(cchMaxSub);
		DWORD dw = cchMaxSub + 1;
		LSTATUS lStatus;
		while ((lStatus = EnumKey(idx, rs.Data(), &dw)) != ERROR_NO_MORE_ITEMS)
		{
			++idx;
			if (lStatus != ERROR_SUCCESS)
			{
				if (plStatus)
					*plStatus = lStatus;
				return idx;
			}
			rs.ReSize((int)dw);
			EckCanCallbackContinue(Fn(rs))
				break;
			dw = cchMaxSub + 1;
		}
		return (int)idx;
	}

	EckInline LSTATUS EnumValue(DWORD idx, PWSTR pszValueNameBuf, DWORD* pcchValueNameBuf,
		void* pDataBuf = nullptr, DWORD* pcbDataBuf = nullptr, DWORD* pdwType = nullptr)
	{
		return RegEnumValueW(m_hKey, idx, pszValueNameBuf, pcchValueNameBuf,
			nullptr, pdwType, (BYTE*)pDataBuf, pcbDataBuf);
	}

	int EnumValue(const std::function<BOOL(CRefStrW& rsName, DWORD dwType)>& fn, LSTATUS* plStatus = nullptr)
	{
		if (plStatus)
			*plStatus = ERROR_SUCCESS;
		constexpr int c_cchMax = 32770 / 2;
		DWORD idx = 0;
		CRefStrW rs(c_cchMax);
		DWORD dw = c_cchMax;
		DWORD dwType;
		LSTATUS lStatus;
		while ((lStatus = EnumValue(idx, rs.Data(), &dw, nullptr, nullptr, &dwType)) != ERROR_NO_MORE_ITEMS)
		{
			++idx;
			if (lStatus != ERROR_SUCCESS)
			{
				if (plStatus)
					*plStatus = lStatus;
				return idx;
			}
			rs.ReSize((int)dw);
			if (fn(rs, dwType))
				break;
			dw = c_cchMax;
		}
		return (int)idx;
	}

	EckInline LSTATUS Flush()
	{
		return RegFlushKey(m_hKey);
	}

	EckInline LSTATUS GetValue(PCWSTR pszSubKey, PCWSTR pszValue, void* pBuf, DWORD* pcbBuf,
		DWORD dwFlags = 0u, DWORD* pdwType = nullptr)
	{
		return RegGetValueW(m_hKey, pszSubKey, pszValue, dwFlags, pdwType, pBuf, pcbBuf);
	}

	/// <summary>
	/// 取字节集注册项。
	/// 绝对不能在当前句柄为HKEY_PERFORMANCE_DATA调用此方法
	/// </summary>
	/// <param name="pszSubKey">子项名称</param>
	/// <param name="pszValue">值名称</param>
	/// <param name="dwFlags">标志，本函数将自动添加类型限制标志</param>
	/// <param name="plStatus">接收错误码变量的可选指针</param>
	/// <returns>数据</returns>
	[[nodiscard]] CRefBin GetValueBin(PCWSTR pszSubKey, PCWSTR pszValue, DWORD dwFlags = 0u, LSTATUS* plStatus = nullptr)
	{
		EckAssert(m_hKey != HKEY_PERFORMANCE_DATA);
		dwFlags |= RRF_RT_REG_BINARY;
		if (plStatus)
			*plStatus = ERROR_SUCCESS;
		DWORD cb = 0u;
		LSTATUS lStatus = GetValue(pszSubKey, pszValue, nullptr, &cb, dwFlags);
		if (lStatus == ERROR_SUCCESS)
		{
			if (cb)
			{
				CRefBin rb(cb);
				GetValue(pszSubKey, pszValue, rb.Data(), &cb, dwFlags);
				return rb;
			}
			else
				return {};
		}
		if (plStatus)
			*plStatus = lStatus;
		return {};
	}

	[[nodiscard]] DWORD GetValueDword(PCWSTR pszSubKey, PCWSTR pszValue, DWORD dwFlags = 0u, LSTATUS* plStatus = nullptr)
	{
		EckAssert(m_hKey != HKEY_PERFORMANCE_DATA);
		dwFlags |= RRF_RT_REG_DWORD;
		if (plStatus)
			*plStatus = ERROR_SUCCESS;
		DWORD dwData, cb = sizeof(DWORD);
		LSTATUS lStatus = GetValue(pszSubKey, pszValue, &dwData, &cb, dwFlags);
		if (lStatus == ERROR_SUCCESS)
			return dwData;
		if (plStatus)
			*plStatus = lStatus;
		return 0u;
	}

	[[nodiscard]] ULONGLONG GetValueQword(PCWSTR pszSubKey, PCWSTR pszValue, DWORD dwFlags = 0u, LSTATUS* plStatus = nullptr)
	{
		EckAssert(m_hKey != HKEY_PERFORMANCE_DATA);
		dwFlags |= RRF_RT_REG_QWORD;
		if (plStatus)
			*plStatus = ERROR_SUCCESS;
		ULONGLONG ullData;
		DWORD cb = sizeof(DWORD);
		LSTATUS lStatus = GetValue(pszSubKey, pszValue, &ullData, &cb, dwFlags);
		if (lStatus == ERROR_SUCCESS)
			return ullData;
		if (plStatus)
			*plStatus = lStatus;
		return 0u;
	}

	LSTATUS GetValueStr(CRefStrW& rs, PCWSTR pszSubKey, PCWSTR pszValue, DWORD dwFlags = 0u)
	{
		EckAssert(m_hKey != HKEY_PERFORMANCE_DATA);
		dwFlags |= RRF_RT_REG_SZ;
		DWORD cb{};
		LSTATUS ls = GetValue(pszSubKey, pszValue, nullptr, &cb, dwFlags);
		if (ls == ERROR_SUCCESS)
		{
			rs.Reserve(cb / sizeof(WCHAR));
			ls = GetValue(pszSubKey, pszValue, rs.Data(), &cb, dwFlags);
			if (cb)
				rs.ReSize(cb / sizeof(WCHAR) - 1);
		}
		return ls;
	}

	EckInline LSTATUS QueryInfo(PWSTR pszClassBuf = nullptr, DWORD* pcchClassBuf = nullptr, DWORD* pcSubKeys = nullptr,
		DWORD* pcchMaxSubKeyLen = nullptr, DWORD* pcchMaxClassLen = nullptr, DWORD* pcValues = nullptr,
		DWORD* pcchMaxValueNameLen = nullptr, DWORD* pcbMaxValueLen = nullptr, DWORD* pcbSecurityDescriptor = nullptr,
		FILETIME* pftLastWriteTime = nullptr)
	{
		return RegQueryInfoKeyW(m_hKey, pszClassBuf, pcchClassBuf, nullptr, pcSubKeys,
			pcchMaxSubKeyLen, pcchMaxClassLen, pcValues, pcchMaxValueNameLen,
			pcbMaxValueLen, pcbSecurityDescriptor, pftLastWriteTime);
	}

	EckInline LSTATUS QueryValue(PCWSTR pszValue, void* pBuf, DWORD* pcbBuf, DWORD* pdwType = nullptr)
	{
		return RegQueryValueExW(m_hKey, pszValue, nullptr, pdwType, (BYTE*)pBuf, pcbBuf);
	}

	[[nodiscard]] CRefBin QueryValueBin(PCWSTR pszValue, LSTATUS* plStatus = nullptr)
	{
		EckAssert(m_hKey != HKEY_PERFORMANCE_DATA);
		if (plStatus)
			*plStatus = ERROR_SUCCESS;
		DWORD cb = 0u;
		LSTATUS lStatus = QueryValue(pszValue, nullptr, &cb);
		if (lStatus == ERROR_SUCCESS)
		{
			if (cb)
			{
				CRefBin rb(cb);
				QueryValue(pszValue, rb.Data(), &cb);
				return rb;
			}
			else
				return {};
		}
		if (plStatus)
			*plStatus = lStatus;
		return {};
	}

	[[nodiscard]] DWORD QueryValueDword(PCWSTR pszValue, LSTATUS* plStatus = nullptr)
	{
		EckAssert(m_hKey != HKEY_PERFORMANCE_DATA);
		if (plStatus)
			*plStatus = ERROR_SUCCESS;
		DWORD dwData, cb = sizeof(DWORD);
		LSTATUS lStatus = QueryValue(pszValue, &dwData, &cb);
		if (lStatus == ERROR_SUCCESS)
			return dwData;
		if (plStatus)
			*plStatus = lStatus;
		return 0u;
	}

	[[nodiscard]] ULONGLONG QueryValueQword(PCWSTR pszValue, LSTATUS* plStatus = nullptr)
	{
		EckAssert(m_hKey != HKEY_PERFORMANCE_DATA);
		if (plStatus)
			*plStatus = ERROR_SUCCESS;
		ULONGLONG ullData;
		DWORD cb = sizeof(DWORD);
		LSTATUS lStatus = QueryValue(pszValue, &ullData, &cb);
		if (lStatus == ERROR_SUCCESS)
			return ullData;
		if (plStatus)
			*plStatus = lStatus;
		return 0u;
	}

	LSTATUS QueryValueStr(CRefStrW& rs, PCWSTR pszValue, DWORD* pdwType = nullptr)
	{
		EckAssert(m_hKey != HKEY_PERFORMANCE_DATA);
		DWORD cb{}, dwType{};
		if (!pdwType)
			pdwType = &dwType;
		LSTATUS ls = QueryValue(pszValue, nullptr, &cb);
		if (ls == ERROR_SUCCESS)
		{
			rs.Reserve(cb / sizeof(WCHAR));
			ls = QueryValue(pszValue, rs.Data(), &cb, pdwType);
			const auto cch = int(cb / sizeof(WCHAR));
			if (cch)
			{
				if (*(rs.Data() + cch - 1) == 0)// 返回数据包含结尾NULL
					rs.ReSize(cch - 1);
				else
				{
					rs.ReSize(cch);
					if (*pdwType == REG_MULTI_SZ)
						rs.PushBackChar(0);
				}
			}
		}
		return ls;
	}

	EckInline LSTATUS RenameKey(PCWSTR pszSubKey, PCWSTR pszNewName)
	{
		return RegRenameKey(m_hKey, pszSubKey, pszNewName);
	}

	EckInline LSTATUS SetValue(PCWSTR pszSubKey, PCWSTR pszValue, PCVOID pData, DWORD cbData, DWORD dwType)
	{
		return RegSetKeyValueW(m_hKey, pszSubKey, pszValue, dwType, pData, cbData);
	}

	EckInline LSTATUS SetValue(PCWSTR pszSubKey, PCWSTR pszValue, DWORD dwData)
	{
		return SetValue(pszSubKey, pszValue, &dwData, sizeof(dwData), REG_DWORD);
	}

	EckInline LSTATUS SetValueQword(PCWSTR pszSubKey, PCWSTR pszValue, ULONGLONG ullData)
	{
		return SetValue(pszSubKey, pszValue, &ullData, sizeof(ullData), REG_QWORD);
	}

	EckInline LSTATUS SetValue(PCWSTR pszSubKey, PCWSTR pszValue, const CRefStrW& rs)
	{
		return SetValue(pszSubKey, pszValue, rs.Data(), (DWORD)rs.ByteSize(), REG_SZ);
	}

	EckInline LSTATUS SetValue(PCWSTR pszSubKey, PCWSTR pszValue, const CRefBin& rs)
	{
		return SetValue(pszSubKey, pszValue, rs.Data(), (DWORD)rs.Size(), REG_BINARY);
	}

	EckInline LSTATUS SetValue(PCWSTR pszValue, PCVOID pData, DWORD cbData, DWORD dwType)
	{
		return RegSetValueExW(m_hKey, pszValue, 0, dwType, (PCBYTE)pData, cbData);
	}

	EckInline LSTATUS SetValue(PCWSTR pszValue, DWORD dwData)
	{
		return SetValue(pszValue, &dwData, sizeof(dwData), REG_DWORD);
	}

	EckInline LSTATUS SetValueQword(PCWSTR pszValue, ULONGLONG ullData)
	{
		return SetValue(pszValue, &ullData, sizeof(ullData), REG_QWORD);
	}

	EckInline LSTATUS SetValue(PCWSTR pszValue, const CRefStrW& rs)
	{
		return SetValue(pszValue, rs.Data(), (DWORD)rs.ByteSize(), REG_SZ);
	}

	EckInline LSTATUS SetValue(PCWSTR pszValue, const CRefBin& rs)
	{
		return SetValue(pszValue, rs.Data(), (DWORD)rs.Size(), REG_BINARY);
	}

	EckInline LSTATUS LoadMuiString(PCWSTR pszValue, PWSTR pszBuf,
		DWORD cbBuf, DWORD* pcbOut, DWORD dwFlags = 0, PCWSTR pszDir = nullptr)
	{
		return RegLoadMUIStringW(m_hKey, pszValue, pszBuf, cbBuf, pcbOut, dwFlags, pszDir);
	}

	EckInline BOOL IsKeyExists(PCWSTR pszSubKey)
	{
		return GetValue(pszSubKey, nullptr, nullptr, nullptr) == ERROR_SUCCESS;
	}

	EckInline BOOL IsValueExists(PCWSTR pszValue)
	{
		return QueryValue(pszValue, nullptr, nullptr) == ERROR_SUCCESS;
	}
};
ECK_NAMESPACE_END