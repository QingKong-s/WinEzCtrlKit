#pragma once
#include "CRegKey.h"
#include "CEnumFile2.h"
#include "ComPtr.h"
#include "CIniExt.h"
#include "SystemHelper.h"

ECK_NAMESPACE_BEGIN
enum class ShmSource : BYTE
{
	// 以下来源均有Shell和ShellEx
	AllFiles,				// HKCR\*						所有文件
	Folder,					// HKCR\Folder					文件夹和驱动器
	Directory,				// HKCR\Directory				文件夹
	Drive,					// HKCR\Drive					驱动器
	AllFileSystemObjects,	// HKCR\AllFileSystemObjects	所有对象
	DirectoryBackground,	// HKCR\Directory\Background	文件夹背景
	DesktopBackground,		// HKCR\DesktopBackground		桌面背景
	MyComputer,				// HKCR\CLSID\...				我的电脑
	RecycleBin,				// HKCR\CLSID\...				回收站
	CustomType,

	SendTo,					// %APPDATA%\Microsoft\Windows\SendTo
	// 新建类型：HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\
	// Explorer\Discardable\PostSetup\ShellNew
	// 此项的Classes值（REG_MULTI_SZ）定义新建文件类型
	// 每个类中ShellNew项定义新建文件命令
	New,
	StartMenu,				// %LocalAppData%\Microsoft\Windows\WinX

	MaxSource,
	Invalid,

	Priv_SsBegin = AllFiles,
	Priv_SsEnd = RecycleBin,
};

enum class ShmFlags :UINT
{
	None = 0u,
	ShellAndShellEx = 1u << 0,	// 该项含Shell和ShellEx
	Com = 1u << 1,				// 该项为COM处理程序
	Uwp = 1u << 2,				// 该项为UWP应用
	FileType = 1u << 3,			// 该项由文件类型定义
	Hidden = 1u << 4,			// 隐藏
	NeverDefault = 1u << 5,		// 绝不用作默认项
	HasSubMenu = 1u << 6,		// 含子菜单
	OnlyInBrowserWindow = 1u << 7,		// 只在资源管理器窗口显示
	HasLuaShield = 1u << 8,				// 显示盾牌图标
	ShowAsDisabledIfHidden = 1u << 8,	// 隐藏时显示为禁用
	NoWorkingDirectory = 1u << 9,		// 不设置工作目录
	Extended = 1u << 10,		// 扩展菜单项（按Shift显示）
	File = 1u << 11,			// 该项为文件而非注册表项
};
ECK_ENUM_BIT_FLAGS(ShmFlags);

namespace Priv
{
	constexpr inline struct
	{
		RegRoot eKey;
		std::wstring_view svSubKey;
	}
	ShmSource[]
	{
		{ RegRoot::ClassesRoot,LR"(*\)"sv },
		{ RegRoot::ClassesRoot,LR"(Folder\)"sv },
		{ RegRoot::ClassesRoot,LR"(Directory\)"sv },
		{ RegRoot::ClassesRoot,LR"(Drive\)"sv },
		{ RegRoot::ClassesRoot,LR"(AllFileSystemObjects\)"sv },
		{ RegRoot::ClassesRoot,LR"(Directory\Background\)"sv },
		{ RegRoot::ClassesRoot,LR"(DesktopBackground\)"sv },
		{ RegRoot::ClassesRoot,LR"(CLSID\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\)"sv },
		{ RegRoot::ClassesRoot,LR"(CLSID\{645FF040-5081-101B-9F08-00AA002F954E}\)"sv },
	};
	constexpr inline std::wstring_view ShmCtxMenuHnd{ L"shellex\\ContextMenuHandlers"sv };

	BOOL ShmpQueryComDisplayName(_In_reads_(38) PCWCH pszClsid, CRefStrW& rsDisplayName)
	{
		WCHAR szPath[38 + 10];
		EckCopyConstStringW(szPath, L"CLSID\\");
		TcsCopyLen(szPath + 6, pszClsid, 38 + 1);

		CRegKey Key{ HKEY_CLASSES_ROOT,szPath,KEY_READ };
		constexpr PCWSTR DisplayName[]
		{
			L"LocalizedString",
			L"InfoTip",
			nullptr
		};
		for (const auto psz : DisplayName)
		{
			if (Key.QueryValueStr(rsDisplayName, psz) == ERROR_SUCCESS)
				return TRUE;
		}
		return FALSE;
	}

	void ShmpLoadIndirectString(CRefStrW& rs)
	{
		// XXX: 
		rs.Reserve(64);
		SHLoadIndirectString(rs.Data(), rs.Data(),
			rs.Capacity(), nullptr);
		rs.ReCalcLen();
	}
}

struct ShmItem
{
	ShmFlags uFlags{};
	RegRoot eRegRoot{};			// 注册表根
	CRefStrW rsRegPath{};		// 注册表全路径
	CRefStrW rsDisplayName{};	// 显示名称
	CRefStrW rsClsidOrCmd{};	// 若有ShmFlags::Com，则为CLSID，否则为命令行
	CRefStrW rsFile{};			// 关联的文件
	CRefStrW rsIcon{};			// 图标，若未显式指定则为空
};

// 含有shell和shellex的来源
class CShmEnumClass
{
private:
	CRegKey m_RegKey{};
	DWORD m_idxEnum{};
	BYTE m_idxSource{};
	BOOLEAN m_bCurrIsShellEx{};
	BOOLEAN m_bFileType{};
	CRefStrW m_rsFileType{};
	CRefStrW m_rsTmp{};

	void ReserveTempBuffer()
	{
		DWORD cchMaxBuf;
		m_RegKey.QueryInfo(nullptr, nullptr, nullptr, &cchMaxBuf);
		m_rsTmp.Reserve(cchMaxBuf * 3 / 2);
	}

	LSTATUS InitShell()
	{
		m_idxEnum = 0;
		GetCurrentRegPath(m_rsTmp);
		m_rsTmp.PushBack(EckStrAndLen(L"shell"));
		const auto ls = m_RegKey.Open(RegRootToKey(Priv::ShmSource[m_idxSource].eKey),
			m_rsTmp.Data(), KEY_READ);
		if (ls != ERROR_SUCCESS)
			return ls;
		ReserveTempBuffer();
		m_bCurrIsShellEx = FALSE;
		return ERROR_SUCCESS;
	}

	LSTATUS InitShellEx()
	{
		m_idxEnum = 0;
		GetCurrentRegPath(m_rsTmp);
		m_rsTmp.PushBack(Priv::ShmCtxMenuHnd);
		const auto ls = m_RegKey.Open(RegRootToKey(Priv::ShmSource[m_idxSource].eKey),
			m_rsTmp.Data(), KEY_READ);
		if (ls != ERROR_SUCCESS)
			return ls;
		ReserveTempBuffer();
		m_bCurrIsShellEx = TRUE;
		return ERROR_SUCCESS;
	}

	LSTATUS NextShell(ShmItem& e)
	{
		e.uFlags = ShmFlags::None;
		LSTATUS ls;
		DWORD cchBuf{ (DWORD)m_rsTmp.Capacity() };
		ls = m_RegKey.EnumKey(m_idxEnum++, m_rsTmp.Data(), &cchBuf);
		if (ls != ERROR_SUCCESS)
			return ls;
		// m_rsTmp现在是项目名
		CRegKey Key{ m_RegKey.GetHKey(),m_rsTmp.Data(),KEY_READ };// 当前项目
		// 注册表路径
		e.rsRegPath.Reserve(cchBuf + 10);
		e.rsRegPath.DupString(L"shell\\");
		e.rsRegPath.PushBack(m_rsTmp.Data(), cchBuf);
		// CLSID/命令行
		e.rsClsidOrCmd.Clear();
		BOOL bClsid{};
		ls = Key.QueryValueStr(e.rsClsidOrCmd, L"ExplorerCommandHandler");
		if (ls == ERROR_SUCCESS)// 有ExplorerCommandHandler
		{
			e.uFlags |= ShmFlags::Com;
			bClsid = TRUE;
		}
		else// 检查是否有command\DelegateExecute
		{
			ls = Key.GetValueStr(e.rsClsidOrCmd, L"command", nullptr);
			if (ls != ERROR_SUCCESS)
			{
				ls = Key.GetValueStr(e.rsClsidOrCmd,
					L"command", L"DelegateExecute");
				if (ls == ERROR_SUCCESS)
				{
					e.uFlags |= ShmFlags::Com;
					bClsid = TRUE;
				}
				else// 没有DelegateExecute，检查默认值
					Key.GetValueStr(e.rsClsidOrCmd, L"command", nullptr);
			}
		}
		// 显示名称
		ls = Key.QueryValueStr(e.rsDisplayName, L"MUIVerb");
		if (ls != ERROR_SUCCESS)
		{
			ls = Key.QueryValueStr(e.rsDisplayName, nullptr);
			if (ls != ERROR_SUCCESS)
			{
				if (!bClsid || !Priv::ShmpQueryComDisplayName(
					e.rsClsidOrCmd.Data(), e.rsDisplayName))
					e.rsDisplayName.DupString(m_rsTmp.Data(), cchBuf);
			}
		}
		if (!e.rsDisplayName.IsEmpty() &&
			e.rsDisplayName.Front() == L'@')
			Priv::ShmpLoadIndirectString(e.rsDisplayName);
		// 属性
		if (Key.IsValueExists(L"NeverDefault"))
			e.uFlags |= ShmFlags::NeverDefault;
		if (Key.IsValueExists(L"OnlyInBrowserWindow"))
			e.uFlags |= ShmFlags::OnlyInBrowserWindow;
		if (Key.IsValueExists(L"ShowAsDisabledIfHidden"))
			e.uFlags |= ShmFlags::ShowAsDisabledIfHidden;
		if (Key.IsValueExists(L"NoWorkingDirectory"))
			e.uFlags |= ShmFlags::NoWorkingDirectory;
		if (Key.IsValueExists(L"Extended"))
			e.uFlags |= ShmFlags::Extended;
		return ERROR_SUCCESS;
	}

	LSTATUS NextShellEx(ShmItem& e)
	{
		LSTATUS ls;
		DWORD cchBuf{ (DWORD)m_rsTmp.Capacity() };
		ls = m_RegKey.EnumKey(m_idxEnum++, m_rsTmp.Data(), &cchBuf);
		if (ls != ERROR_SUCCESS)
			return ls;
		// m_rsTmp现在是项目名
		CRegKey Key{ m_RegKey.GetHKey(),m_rsTmp.Data(),KEY_READ };// 当前项目
		// 注册表路径
		e.rsRegPath.Reserve(cchBuf + 36);
		e.rsRegPath.DupString(Priv::ShmCtxMenuHnd);
		e.rsRegPath.PushBackChar(L'\\');
		e.rsRegPath.PushBack(m_rsTmp.Data(), cchBuf);
		// CLSID
		BOOL bClsid{};
		ls = Key.QueryValueStr(e.rsClsidOrCmd, nullptr);
		if (ls == ERROR_SUCCESS && !e.rsClsidOrCmd.IsEmpty())
		{
			if (e.rsClsidOrCmd.Front() != L'{')// 默认值并非CLSID...
			{
				const auto t{ std::move(e.rsClsidOrCmd) };
				e.rsClsidOrCmd.DupString(m_rsTmp.Data(), cchBuf);
				m_rsTmp.DupString(t.Data(), t.Size());
			}
			e.uFlags |= ShmFlags::Com;
			bClsid = TRUE;
		}
		// 显示名称
		ls = Key.QueryValueStr(e.rsDisplayName, L"MUIVerb");
		if (ls != ERROR_SUCCESS)
		{
			if (!bClsid || !Priv::ShmpQueryComDisplayName(
				e.rsClsidOrCmd.Data(), e.rsDisplayName))
				e.rsDisplayName.DupString(m_rsTmp.Data(), cchBuf);
		}
		if (e.rsDisplayName.Front() == L'@')
			Priv::ShmpLoadIndirectString(e.rsDisplayName);
		return ERROR_SUCCESS;
	}
public:
	void Reset()
	{
		m_RegKey.Close();
		m_idxEnum = 0;
		m_idxSource = 0;
		m_bCurrIsShellEx = FALSE;
		m_bFileType = FALSE;
		m_rsFileType.Clear();
	}

	W32ERR Init(ShmSource eSource, _In_reads_z_(cchFileType)
		PCWSTR pszFileType = nullptr, int cchFileType = -1)
	{
		Reset();
		m_idxSource = (BYTE)eSource;
		if (eSource == ShmSource::CustomType)
		{
			m_rsFileType.DupString(pszFileType, cchFileType);
			if (m_rsFileType.IsEmpty())
				return ERROR_INVALID_PARAMETER;
			if (m_rsFileType.Back() != L'\\')
				m_rsFileType.PushBackChar(L'\\');
		}

		m_bFileType = (eSource == ShmSource::CustomType ||
			eSource == ShmSource::AllFiles);
		InitShell();
		return ERROR_SUCCESS;
	}

	// 仅当函数返回ERROR_SUCCESS时，e才有效
	W32ERR Next(ShmItem& e)
	{
		if (!m_bCurrIsShellEx)
		{
			if (NextShell(e) != ERROR_SUCCESS)
			{
				// 若枚举失败，则尝试切换到ShellEx
				if (InitShellEx() != ERROR_SUCCESS)
					return ERROR_NO_MORE_ITEMS;// 切换失败
				return NextShellEx(e);
			}
			return ERROR_SUCCESS;
		}
		else
			return NextShellEx(e);
	}

	// 末尾带反斜杠
	void GetCurrentRegPath(CRefStrW& rs) const
	{
		if (m_idxSource == (BYTE)ShmSource::CustomType)
			rs = m_rsFileType;
		else
			rs.DupString(Priv::ShmSource[m_idxSource].svSubKey);
	}
};

class CShmEnumSendTo
{
private:
	CEnumFile2 m_EnumFile{};
	PWSTR m_pszSendToPath{};// CoTaskMemFree
	int m_cchSendToPath{};
	CIniExtMut m_IniDesktop{};
public:
	void Reset()
	{
		m_EnumFile.Close();
		CoTaskMemFree(m_pszSendToPath);
		m_cchSendToPath = 0;
		m_IniDesktop.Clear();
	}

	W32ERR Init()
	{
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SendTo,
			SHGFP_TYPE_CURRENT, nullptr, &m_pszSendToPath)))
		{
			m_cchSendToPath = (int)TcsLen(m_pszSendToPath);
			return WIN32_FROM_NTSTATUS(m_EnumFile.Open(m_pszSendToPath));
		}
		return ERROR_ACCESS_DENIED;
	}

	LSTATUS Next(ShmItem& e)
	{
		e.uFlags = ShmFlags::None;
		CEnumFile2::TDefInfo* pInfo;
		NTSTATUS nts;
		EckLoop()
		{
			if (!NT_SUCCESS(nts = m_EnumFile.Next(nullptr, 0, pInfo)))
				return WIN32_FROM_NTSTATUS(nts);
			// 跳过desktop.ini
			constexpr WCHAR DesktopIni[]{ L"desktop.ini" };
			if (pInfo->FileNameLength == sizeof(DesktopIni) - sizeof(WCHAR) &&
				TcsEqualLenI(pInfo->FileName, DesktopIni,
					pInfo->FileNameLength / sizeof(WCHAR)))
				continue;
			// 跳过.和..
			if (*pInfo->FileName == L'.')
				continue;
			break;
		}
		e.rsFile.DupString(m_pszSendToPath, m_cchSendToPath);
		e.rsFile.PushBackChar(L'\\');

		if (m_IniDesktop.IsEmpty())
		{
			auto rsIniPath{ e.rsFile };
			rsIniPath.PushBack(EckStrAndLen(L"desktop.ini"));
			const auto rb = ReadInFile(rsIniPath.Data());
			m_IniDesktop.Load((PCWSTR)rb.Data(),
				int(rb.Size() / sizeof(WCHAR)), INIE_IF_DISABLE_EXT);
		}

		const auto posFileName = e.rsFile.Size();
		e.rsFile.PushBack(pInfo->FileName, int(pInfo->FileNameLength / 2));
		if (pInfo->FileAttributes & FILE_ATTRIBUTE_HIDDEN)
			e.uFlags |= ShmFlags::Hidden;
		const auto Sec = m_IniDesktop.GetSection(L"LocalizedFileNames"sv);
		const auto Kv = m_IniDesktop.GetKeyValue(Sec, e.rsFile.Data() + posFileName);
		if (Kv)
			e.rsDisplayName = Kv->rsValue;
		{
			e.rsDisplayName.DupString(pInfo->FileName, (int)pInfo->FileNameLength);
			e.rsDisplayName.PazRemoveExtension();
		}

		if (e.rsDisplayName.Front() == L'@')
			Priv::ShmpLoadIndirectString(e.rsDisplayName);

		e.rsIcon.Clear();
		if (e.rsFile.IsEndOfI(EckStrAndLen(L".lnk")))
		{
			e.rsClsidOrCmd.Clear();
			ComPtr<IShellLinkW> psl;
			ComPtr<IPersistFile> ppf;
			if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr,
				CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&psl)))
			{
				psl.As(ppf);
				if (FAILED(ppf->Load(e.rsFile.Data(), STGM_READ)))
					goto Fail;

				e.rsIcon.Reserve(MAX_PATH);
				int idxIcon{};
				if (FAILED(psl->GetIconLocation(
					e.rsIcon.Data(), e.rsIcon.Capacity(), &idxIcon)))
					goto Fail;
				e.rsIcon.ReCalcLen();
				e.rsIcon.AppendFormat(L",%d", idxIcon);
			}
		Fail:;
		}
		else// 不为快捷方式，则项目的类注册一个处理程序
		{
			const auto posExt = e.rsFile.PazFindExtension();
			if (posExt < 0)
				goto NotLnkOrCom;
			CRegKey Key{ HKEY_CLASSES_ROOT,e.rsFile.Data() + posExt,KEY_READ };
			Key.QueryValueStr(e.rsIcon, nullptr);// CLSID路径
			if (e.rsIcon.IsEmpty())
				goto NotLnkOrCom;

			const auto posClsid = e.rsIcon.FindChar(L'{');
			e.rsClsidOrCmd.DupString(e.rsIcon.Data() + posClsid, 38);
			e.uFlags |= ShmFlags::Com;

			if (e.rsIcon.Back() != L'\\')
				e.rsIcon.PushBackChar(L'\\');
			e.rsIcon.PushBack(EckStrAndLen(L"DefaultIcon"));
			if (Key.Open(HKEY_CLASSES_ROOT, e.rsIcon.Data(), KEY_READ) == ERROR_SUCCESS)
				Key.QueryValueStr(e.rsIcon, nullptr);
			else
				e.rsIcon.Clear();
		}
	NotLnkOrCom:;
		return ERROR_SUCCESS;
	}

	void InvalidateDesktopIniCache() noexcept { m_IniDesktop.Clear(); }
};

class CShmEnumWinX
{
private:
	CEnumFile2 m_EnumFile{};
	CEnumFile2 m_EnumStartMenu{};
	CRefStrW m_rsWinXPath{};
	CRefStrW m_rsCurrGroup{};
	int m_idxGroup{};
public:
	void Reset()
	{
		m_EnumFile.Close();
		m_EnumStartMenu.Close();
		m_rsWinXPath.Clear();
		m_rsCurrGroup.Clear();
		m_idxGroup = 0;
	}

	W32ERR Init()
	{
		Reset();
		UNICODE_STRING usPath
			RTL_CONSTANT_STRING(LR"(%LocalAppData%\Microsoft\Windows\WinX)");
		ULONG cbBuffer{ (47 + 30/*用户名*/ + 5) * sizeof(WCHAR) };
		m_rsWinXPath.Reserve(int(cbBuffer / sizeof(WCHAR)));
		UNICODE_STRING usExpand{ m_rsWinXPath.ToNtString() };
		auto nts = RtlExpandEnvironmentStrings_U(
			nullptr, &usPath, &usExpand, &cbBuffer);
		if (!NT_SUCCESS(nts))
		{
			if (nts == STATUS_BUFFER_TOO_SMALL)
			{
				m_rsWinXPath.Reserve(int(cbBuffer / sizeof(WCHAR)));
				usExpand = m_rsWinXPath.ToNtString();
				if (!NT_SUCCESS(nts = RtlExpandEnvironmentStrings_U(
					nullptr, &usPath, &usExpand, &cbBuffer)))
					return WIN32_FROM_NTSTATUS(nts);
			}
			else
				return WIN32_FROM_NTSTATUS(nts);
		}
	Ok:
		m_rsWinXPath.ReSize(int(usExpand.Length / sizeof(WCHAR)));
		return WIN32_FROM_NTSTATUS(m_EnumFile.Open(m_rsWinXPath.Data()));
	}

	W32ERR Next(ShmItem& e)
	{
		NTSTATUS nts;
		CEnumFile2::TDefInfo* pInfo;
		if (!m_EnumStartMenu.GetHDir())
		{
			EckLoop()
			{
				nts = m_EnumFile.Next(nullptr, 0, pInfo);
				if (!NT_SUCCESS(nts))
					return WIN32_FROM_NTSTATUS(nts);
				if (*pInfo->FileName == L'.')
					continue;
				m_rsCurrGroup.DupString(pInfo->FileName,
					int(pInfo->FileNameLength / sizeof(WCHAR)));
				nts = m_EnumStartMenu.Open(m_rsCurrGroup.Data(), m_EnumFile.GetHDir());
				++m_idxGroup;
				if (NT_SUCCESS(nts))
					break;// 直到找到第一个有效目录
			}
		}
		EckLoop()
		{
			nts = m_EnumStartMenu.Next(nullptr, 0, pInfo);
			if (!NT_SUCCESS(nts))
			{
				m_EnumStartMenu.Close();
				if (nts == STATUS_NO_MORE_FILES)
					return Next(e);
				else
					return WIN32_FROM_NTSTATUS(nts);
			}
			const int cchFileName = int(pInfo->FileNameLength / sizeof(WCHAR));
			if (*pInfo->FileName == L'.' ||
				TcsEqualLen2I(pInfo->FileName, cchFileName, EckStrAndLen(L"desktop.ini")))
				continue;

			e.rsFile = m_rsWinXPath;
			e.rsFile.PushBackChar(L'\\');
			e.rsFile.PushBack(pInfo->FileName, cchFileName);
			if (pInfo->FileAttributes & FILE_ATTRIBUTE_HIDDEN)
				e.uFlags |= ShmFlags::Hidden;
			e.rsDisplayName.DupString(pInfo->FileName, cchFileName);
			if (e.rsDisplayName.Front() == L'@')
				Priv::ShmpLoadIndirectString(e.rsDisplayName);
			e.rsIcon.Clear();
			break;
		}
		return ERROR_SUCCESS;
	}

	EckInlineNdCe auto& GetWinXPath() const noexcept { return m_rsWinXPath; }
	// 取当前组文件夹名称
	EckInlineNdCe auto& GetCurrGroup() const noexcept { return m_rsCurrGroup; }
	// 开始枚举前，调用方将0作为初始无效值，每次调用Next后与该函数返回值比较，
	// 若不同，则已开始新组的枚举，否则继续当前组的枚举
	EckInlineNdCe int GetCurrGroupIndex() const noexcept { return m_idxGroup; }
};
ECK_NAMESPACE_END