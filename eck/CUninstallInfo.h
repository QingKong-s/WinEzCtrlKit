#pragma once
#include "CRegKey.h"

#include <RegStr.h>
#include <Msi.h>

ECK_NAMESPACE_BEGIN
enum class AppwizFlags : UINT
{
	None = 0u,
	NoRemove = 1u << 0,	// 未提供卸载选项
	NoModify = 1u << 1,	// 未提供修改选项
	NoRepair = 1u << 2,	// 未提供修复选项
	WindowsInstaller = 1u << 3,	// 由MSI安装程序安装，且从注册表获取
	SystemComponent = 1u << 4,	// 系统组件
	RegCurrentUser = 1u << 5,	// 注册表在HKEY_CURRENT_USER下
	RegLocalMachine = 1u << 6,	// 注册表在HKEY_LOCAL_MACHINE下
	RegWow64 = 1u << 7,			// 仅_WIN64平台有效，表示该卸载项为Wow64程序
	Patch = 1u << 8,			// 补丁
	Msi = 1u << 9,				// MSI安装程序安装，且从MSIAPI获取
	NoReg = 1u << 10,			// 仅用于MSI安装程序，指示查询注册表失败。调用方一般不使用此值
};
ECK_ENUM_BIT_FLAGS(AppwizFlags);

enum class AppwizStr
{
	ProductID,	// 手动获取
	RegKeyName,	// 手动获取

	MinImportant,

	DisplayName = MinImportant,
	InstallLocation,
	UninstallString,
	Publisher,
	KBNumber,

	MaxImportant,

	DisplayIcon = MaxImportant,
	DisplayVersion,
	HelpLink,
	HelpTelephone,
	InstallDate,
	InstallSource,
	URLInfoAbout,
	URLUpdateInfo,
	Contact,
	Readme,
	RegOwner,
	RegCompany,
	Comments,
	QuietUninstallString,
	ModifyPath,

	Max
};

namespace Priv
{
	constexpr inline PCWSTR AppwizStrs[size_t(AppwizStr::Max)]
	{
		L"ProductID",
		L"RegKeyName",

		L"DisplayName",
		L"InstallLocation",
		L"UninstallString",
		L"Publisher",
		L"KBNumber",

		L"DisplayIcon",
		L"DisplayVersion",
		L"HelpLink",
		L"HelpTelephone",
		L"InstallDate",
		L"InstallSource",
		L"URLInfoAbout",
		L"URLUpdateInfo",
		L"Contact",
		L"Readme",
		L"RegOwner",
		L"RegCompany",
		L"Comments",
		L"QuietUninstallString",
		L"ModifyPath",
	};

	constexpr inline PCWSTR AppwizStrsMsi[size_t(AppwizStr::Max)]
	{
		L"ProductID",
		L"RegKeyName",// 与ProductID相同

		INSTALLPROPERTY_PRODUCTNAME,// 注意：INSTALLPROPERTY_DISPLAYNAME用于补丁
		INSTALLPROPERTY_INSTALLLOCATION,
		L"/UninstallString",
		INSTALLPROPERTY_PUBLISHER,
		L"/KBNumber",

		INSTALLPROPERTY_PRODUCTICON,
		INSTALLPROPERTY_VERSIONSTRING,
		INSTALLPROPERTY_HELPLINK,
		INSTALLPROPERTY_HELPTELEPHONE,
		INSTALLPROPERTY_INSTALLDATE,
		INSTALLPROPERTY_INSTALLSOURCE,
		INSTALLPROPERTY_URLINFOABOUT,
		INSTALLPROPERTY_URLUPDATEINFO,
		L"/Contact",
		L"/Readme",
		L"RegOwner",
		L"RegCompany",
		L"/Comments",
		L"/QuietUninstallString",
		L"/ModifyPath",
	};

	const inline struct
	{
		HKEY hRoot;
		PCWSTR pszSubKey;
		int cchSubKey;
	}
	UninstallInfoSource[]
	{
		{ HKEY_LOCAL_MACHINE, EckStrAndLen(REGSTR_PATH_UNINSTALL) },
		{ HKEY_CURRENT_USER,  EckStrAndLen(REGSTR_PATH_UNINSTALL) },
#ifdef _WIN64
		{ HKEY_LOCAL_MACHINE, EckStrAndLen(L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall") },
		{ HKEY_CURRENT_USER,  EckStrAndLen(L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall") },
#endif
	};
}

class CUninstallInfo
{
private:
	CRegKey m_Reg{};
	CRefStrW m_rsBuffer{};
	DWORD m_idxCurr{};
	BYTE m_idxCurrSource{};
	BITBOOL m_bAutoCompleteAppInfo : 1{ TRUE };
	BITBOOL m_bSkipWindowsInstaller : 1{ TRUE };
	BITBOOL m_bEnumMsi : 1{ 1 };

	LSTATUS NextSource()
	{
		if (m_idxCurrSource < ARRAYSIZE(Priv::UninstallInfoSource))
		{
			auto& ui = Priv::UninstallInfoSource[m_idxCurrSource++];
			auto ls = m_Reg.Open(ui.hRoot, ui.pszSubKey, KEY_READ);
			if (ls == ERROR_SUCCESS)
			{
				DWORD cchMaxSub;
				if ((ls = m_Reg.QueryInfo(nullptr,
					nullptr, nullptr, &cchMaxSub)) != ERROR_SUCCESS)
					return ls;
				m_rsBuffer.Reserve(cchMaxSub);
				m_idxCurr = 0;
				return ERROR_SUCCESS;
			}
			return NextSource();
		}
		return ERROR_NO_MORE_ITEMS;
	}

	constexpr static AppwizFlags GetRegFlags(size_t idxSrc)
	{
		switch (idxSrc)
		{
		case 0:
			return AppwizFlags::RegLocalMachine;
			break;
		case 1:
			return AppwizFlags::RegCurrentUser;
			break;
		case 2:
			return (AppwizFlags::RegWow64 | AppwizFlags::RegLocalMachine);
			break;
		case 3:
			return (AppwizFlags::RegWow64 | AppwizFlags::RegCurrentUser);
			break;
		default:
			ECK_UNREACHABLE;
		}
	}
public:
	struct CApp
	{
		friend class CUninstallInfo;

		CRegKey Reg{};
		CRefStrW StrBuffer{};
		DWORD EstimatedSize{};
		AppwizFlags Flags{};
		DWORD MajorVersion{};
		DWORD MinorVersion{};
		struct StringSpan
		{
			int idx;
			int cch;
		} Str[size_t(AppwizStr::Max)]{};
	private:
		LSTATUS GetStringValueReg(PCWSTR pszValue, _Out_ StringSpan& spResult)
		{
			LSTATUS ls;
			DWORD cbBuf{};
			if ((ls = Reg.QueryValue(pszValue, nullptr, &cbBuf)) != ERROR_SUCCESS)
			{
				spResult = {};
				return ls;
			}
			if (cbBuf)
			{
				const auto cchAdded = cbBuf / sizeof(WCHAR) + 1;
				const auto psBuf = StrBuffer.PushBack((int)cchAdded);
				if ((ls = Reg.QueryValue(pszValue, psBuf, &cbBuf)) != ERROR_SUCCESS ||
					!cbBuf)
				{
					StrBuffer.PopBack((int)cchAdded);
					spResult = {};
					return ls;
				}
				*(psBuf + cchAdded - 1) = 0;
				spResult = { int(psBuf - StrBuffer.Data()),int(cchAdded - 1) };
			}
			else
				spResult = {};
			return ERROR_SUCCESS;
		}

		LSTATUS GetStringValueMsi(PCWSTR pszValue, _Out_ StringSpan& spResult)
		{
			LSTATUS ls;
			DWORD cchBuf{};// 不含结尾NULL
			ls = (LSTATUS)MsiGetProductInfoW(StrBuffer.Data()/*首部总为ProductID*/,
				pszValue, nullptr, &cchBuf);
			if (!cchBuf)
			{
				spResult = {};
				return ls;
			}
			++cchBuf;
			const auto pszBuf = StrBuffer.PushBack(cchBuf);
			const auto cchAdded = cchBuf;
			ls = (LSTATUS)MsiGetProductInfoW(StrBuffer.Data()/*首部总为ProductID*/,
				pszValue, pszBuf, &cchBuf);
			if (ls != ERROR_SUCCESS || !cchBuf)
			{
				StrBuffer.PopBack(cchAdded);
				spResult = {};
				return ls;
			}
			spResult = { int(pszBuf - StrBuffer.Data()),int(cchAdded - 1) };
			return ERROR_SUCCESS;
		}

		BOOL AcquireBasicInfo(BOOL bSkipWindowsInstaller,
			PCWSTR pszKeyOrId, int cchKeyOrId, BOOL bMsi)
		{
			DWORD dwBuf{}, cbBuf{ sizeof(dwBuf) };
			if (bMsi)
			{
				Flags = AppwizFlags::Msi;// Certainly...
				// 测试位于注册表的哪个源
				EckCounter(ARRAYSIZE(Priv::UninstallInfoSource), i)
				{
					const auto& Src = Priv::UninstallInfoSource[i];
					StrBuffer.DupString(Src.pszSubKey, Src.cchSubKey);
					StrBuffer.PushBackChar(L'\\');
					StrBuffer.PushBack(pszKeyOrId, cchKeyOrId);
					if (Reg.Open(Src.hRoot, StrBuffer.Data(), KEY_READ) == ERROR_SUCCESS)
					{
						Flags |= GetRegFlags(i);
						break;
					}
				}
				if (!Reg.GetHKey())
					Flags |= AppwizFlags::NoReg;

				StrBuffer.DupString(pszKeyOrId, cchKeyOrId + 1/* For null terminator */);
				Str[size_t(AppwizStr::ProductID)] = { 0,38 };
			}
			else
			{
				Reg.QueryValue(L"WindowsInstaller", &dwBuf, &cbBuf);
				if (dwBuf)
				{
					// 若有需要，调用方使用此标志判断是否跳过Windows Installer应用
					Flags |= AppwizFlags::WindowsInstaller;
					if (bSkipWindowsInstaller)
						return TRUE;// 停止获取信息
				}
				dwBuf = 0;
			}
			const auto posKey = StrBuffer.Size();
			StrBuffer.PushBack(pszKeyOrId, cchKeyOrId + 1);
			Str[size_t(AppwizStr::RegKeyName)] = { posKey,StrBuffer.Size() - posKey - 1 };

			Reg.QueryValue(L"NoRemove", &dwBuf, &cbBuf);
			if (dwBuf)
				Flags |= AppwizFlags::NoRemove;
			dwBuf = 0;
			Reg.QueryValue(L"NoModify", &dwBuf, &cbBuf);
			if (dwBuf)
				Flags |= AppwizFlags::NoModify;
			dwBuf = 0;
			Reg.QueryValue(L"NoRepair", &dwBuf, &cbBuf);
			if (dwBuf)
				Flags |= AppwizFlags::NoRepair;
			dwBuf = 0;
			Reg.QueryValue(L"SystemComponent", &dwBuf, &cbBuf);
			if (dwBuf)
				Flags |= AppwizFlags::SystemComponent;

			if (bMsi)
			{
				for (size_t i = size_t(AppwizStr::MinImportant);
					i < size_t(AppwizStr::MaxImportant); ++i)
				{
					if (Priv::AppwizStrsMsi[i][0] == '/')
						GetStringValueReg(Priv::AppwizStrsMsi[i] + 1, Str[i]);
					else
						GetStringValueMsi(Priv::AppwizStrsMsi[i], Str[i]);
				}
			}
			else
			{
				for (size_t i = size_t(AppwizStr::MinImportant);
					i < size_t(AppwizStr::MaxImportant); ++i)
				{
					GetStringValueReg(Priv::AppwizStrs[i], Str[i]);
				}

				if (GetStr(AppwizStr::KBNumber).cch)
					Flags |= AppwizFlags::Patch;
			}
			return FALSE;
		}

		static DWORD VersionFromStr(PCWSTR pszVer, DWORD dwType)
		{
			if (dwType == REG_DWORD)
				return *(DWORD*)pszVer;
			if (dwType == REG_SZ)
				return _wtoi(pszVer);
			return 0;
		}

		W32ERR CreateProcessForExec(StringSpan spCmd)
		{
			if (!spCmd.cch)
			{
				STARTUPINFO si{};
				PROCESS_INFORMATION pi{};
				si.cb = sizeof(si);
				const auto pszBuf = (PWSTR)_malloca((spCmd.cch + 1) * sizeof(WCHAR));
				EckCheckMem(pszBuf);
				TcsCopyLen(pszBuf, StrBuffer.Data() + spCmd.idx, spCmd.cch + 1);
				const auto b = CreateProcessW(nullptr, pszBuf,
					nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
				_freea(pszBuf);
				if (b)
				{
					NtClose(pi.hProcess);
					NtClose(pi.hThread);
					return ERROR_SUCCESS;
				}
				else
					return NtCurrentTeb()->LastErrorValue;
			}
			else
				return ERROR_NOT_SUPPORTED;
		}
	public:
		EckInlineNdCe BOOL TestFlag(AppwizFlags f) const noexcept
		{
			return (Flags & f) != AppwizFlags::None;
		}

		EckInlineNdCe BOOL IsWindowsInstaller() const noexcept
		{
			return TestFlag(AppwizFlags::WindowsInstaller);
		}

		// 测试是否为一般意义上的可操作项
		EckInlineNdCe BOOL IsValid() const noexcept
		{
			if (!Str[size_t(AppwizStr::DisplayName)].cch ||
				Str[size_t(AppwizStr::KBNumber)].cch)
				return FALSE;
			if (TestFlag(AppwizFlags::Msi))
				return !TestFlag(AppwizFlags::NoReg);
			else
				return !!Str[size_t(AppwizStr::UninstallString)].cch;
		}

		// 提供与 控制面板/程序和功能 相似的过滤条件
		EckInlineNdCe BOOL IsNormalApp() const noexcept
		{
			return IsValid() && !TestFlag(
				AppwizFlags::WindowsInstaller | AppwizFlags::SystemComponent);
		}

		EckInlineNdCe StringSpan GetStr(AppwizStr e) const noexcept
		{
			return Str[size_t(e)];
		}

		EckInlineNdCe std::wstring_view GetStrView(AppwizStr e) const noexcept
		{
			const auto f = GetStr(e);
			if (f.cch)
				return { StrBuffer.Data() + f.idx,(size_t)f.cch };
			else
				return {};
		}

		void Clear()
		{
			Reg.Close();
			StrBuffer.Clear();
			EstimatedSize = 0;
			Flags = AppwizFlags::None;
			MajorVersion = 0;
			MinorVersion = 0;
			ZeroMemory(Str, sizeof(Str));
		}

		void AcquireAllInfo()
		{
			for (size_t i = size_t(AppwizStr::MaxImportant); i < size_t(AppwizStr::Max); ++i)
				GetStringValueReg(Priv::AppwizStrs[i], Str[i]);
			// 补全非字符串信息
			EstimatedSize = 0;
			DWORD cbBuf{ sizeof(EstimatedSize) };
			Reg.QueryValue(L"EstimatedSize", &EstimatedSize, &cbBuf);

			// MajorVersion、MinorVersion、VersionMajor、VersionMinor
			// 某些程序填写为字符串，因此使用字符串缓冲区接收
			WCHAR szBuf[CchI32ToStrBufNoRadix2];
			DWORD dwType;
			BOOL bMajorRead{}, bMinorRead{};
			cbBuf = sizeof(szBuf);
			if (Reg.QueryValue(L"MajorVersion", szBuf, &cbBuf, &dwType) == ERROR_SUCCESS)
			{
				MajorVersion = VersionFromStr(szBuf, dwType);
				bMajorRead = !MajorVersion;
			}
			cbBuf = sizeof(szBuf);
			if (Reg.QueryValue(L"MinorVersion", szBuf, &cbBuf, &dwType) == ERROR_SUCCESS)
			{
				MinorVersion = VersionFromStr(szBuf, dwType);
				bMinorRead = !MinorVersion;
			}
			if (!bMajorRead)
			{
				cbBuf = sizeof(szBuf);
				if (Reg.QueryValue(L"VersionMajor", szBuf, &cbBuf, &dwType) == ERROR_SUCCESS)
					MajorVersion = VersionFromStr(szBuf, dwType);
			}
			if (!bMinorRead)
			{
				cbBuf = sizeof(szBuf);
				if (Reg.QueryValue(L"VersionMinor", szBuf, &cbBuf, &dwType) == ERROR_SUCCESS)
					MinorVersion = VersionFromStr(szBuf, dwType);
			}
		}

		W32ERR Repair()
		{
			if (TestFlag(AppwizFlags::NoRepair))
				return ERROR_NOT_SUPPORTED;
			if (TestFlag(AppwizFlags::Msi))
			{
				return MsiReinstallProductW(GetStrView(AppwizStr::ProductID).data(),
					REINSTALLMODE_USERDATA | REINSTALLMODE_MACHINEDATA |
					REINSTALLMODE_SHORTCUT | REINSTALLMODE_FILEOLDERVERSION |
					REINSTALLMODE_FILEVERIFY | REINSTALLMODE_PACKAGE);
			}
			return ERROR_NOT_SUPPORTED;
		}

		W32ERR Uninstall()
		{
			if (TestFlag(AppwizFlags::NoRemove))
				return ERROR_NOT_SUPPORTED;
			if (TestFlag(AppwizFlags::Msi))
				return MsiConfigureProductW(GetStrView(AppwizStr::ProductID).data(),
					INSTALLSTATE_ABSENT, INSTALLSTATE_ABSENT);
			else
				return CreateProcessForExec(GetStr(AppwizStr::UninstallString));
		}

		W32ERR Modify()
		{
			if (TestFlag(AppwizFlags::NoModify))
				return ERROR_NOT_SUPPORTED;
			if (TestFlag(AppwizFlags::Msi))
			{
				const auto uOld = MsiSetInternalUI(INSTALLUILEVEL_FULL, nullptr);
				const auto r = MsiConfigureProductW(GetStrView(AppwizStr::ProductID).data(),
					INSTALLSTATE_DEFAULT, INSTALLSTATE_DEFAULT);
				MsiSetInternalUI(uOld, nullptr);
				return r;
			}
			else
				return CreateProcessForExec(GetStr(AppwizStr::ModifyPath));
		}
	};

	LSTATUS Init()
	{
		m_idxCurrSource = 0;
		return NextSource();
	}

	LSTATUS Next(_Inout_ CApp& App)
	{
		LSTATUS ls;
		if (m_idxCurrSource < ARRAYSIZE(Priv::UninstallInfoSource))
		{
			DWORD cbBuf{ (DWORD)m_rsBuffer.ByteCapacity() };
			// 枚举注册表
			if ((ls = m_Reg.EnumKey(m_idxCurr++, m_rsBuffer.Data(), &cbBuf)) != ERROR_SUCCESS)
			{
				if (ls == ERROR_NO_MORE_ITEMS)
				{
					if ((ls = NextSource()) != ERROR_SUCCESS)
					{
						if (m_idxCurrSource >= ARRAYSIZE(Priv::UninstallInfoSource))
						{
							m_idxCurr = 0;
							m_rsBuffer.Reserve(40);
						}
						return ls;
					}
					return Next(App);
				}
				return ls;
			}
			if ((ls = App.Reg.Open(m_Reg.GetHKey(), m_rsBuffer.Data(), KEY_READ)) != ERROR_SUCCESS)
				return ls;
			App.StrBuffer.Clear();
			App.StrBuffer.Reserve(MAX_PATH);
			App.Flags = GetRegFlags(m_idxCurrSource);
			if (App.AcquireBasicInfo(m_bSkipWindowsInstaller,
				m_rsBuffer.Data(), cbBuf / sizeof(WCHAR), FALSE))
				return ERROR_SUCCESS;
		}
		else
		{
			if (!m_bEnumMsi)
				return ERROR_NO_MORE_ITEMS;
			// 枚举MSI
			if ((ls = MsiEnumProductsW(m_idxCurr++, m_rsBuffer.Data())) != ERROR_SUCCESS)
				return ls;
			App.AcquireBasicInfo(FALSE/* Not used */, m_rsBuffer.Data(), 38, TRUE);
		}
		if (m_bAutoCompleteAppInfo)
			App.AcquireAllInfo();
		return ERROR_SUCCESS;
	}

	// 是否在获取每个应用的基本信息时自动获取所有信息，
	// 若仅需基本信息，可设置为FALSE以提高性能
	EckInlineCe void SetAutoCompleteAppInfo(BOOL bAutoComplete) noexcept
	{
		m_bAutoCompleteAppInfo = bAutoComplete;
	}
	EckInlineNdCe BOOL GetAutoCompleteAppInfo() const noexcept
	{
		return m_bAutoCompleteAppInfo;
	}

	// 若调用方使用MsiEnumProductsW枚举MSI应用信息，
	// 则可设为TRUE以跳过这些应用，此时仅获取WindowsInstaller标志
	EckInlineCe void SetSkipWindowsInstaller(BOOL bSkip) noexcept
	{
		m_bSkipWindowsInstaller = bSkip;
	}
	EckInlineNdCe BOOL GetSkipWindowsInstaller() const noexcept
	{
		return m_bSkipWindowsInstaller;
	}

	// 是否使用MSIAPI枚举应用信息
	EckInlineCe void SetEnumMsi(BOOL bEnum) noexcept { m_bEnumMsi = bEnum; }
	EckInlineNdCe BOOL GetEnumMsi() const noexcept { return m_bEnumMsi; }
};
ECK_NAMESPACE_END