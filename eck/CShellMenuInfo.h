#pragma once
#include "CRegKey.h"
#include "CEnumFile2.h"
#include "ComPtr.h"

ECK_NAMESPACE_BEGIN
constexpr UINT ShMenuBit = 8;
// 定义菜单项目的来源信息
enum class ShMenu :UINT
{
	// 低8位：来源ID，必须与ShellMenuInfoSource对应
	// 高24位：位标志
	AllSource = 0xFF,
	None = 0,
	// 以下来源均有Shell和ShellEx
	AllFiles,				// HKCR\*						所有文件
	Folder,					// HKCR\Folder					文件夹和驱动器
	Directory,				// HKCR\Directory				文件夹
	Drive,					// HKCR\Drive					驱动器
	AllFileSystemObjects,	// HKCR\AllFileSystemObjects	所有对象
	DirectoryBackground,	// HKCR\Directory\Background	文件夹背景
	DesktopBackground,		// HKCR\DesktopBackground		桌面背景
	Priv_ClsidStart,
	MyComputer = Priv_ClsidStart,	// HKCR\CLSID\...		我的电脑
	RecycleBin,						// HKCR\CLSID\...		回收站
	// 以下来源需要特判
	SendTo,					// %APPDATA%\Microsoft\Windows\SendTo
	//// 新建类型：HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\
	//// Explorer\Discardable\PostSetup\ShellNew
	//// 此项的Classes值（REG_MULTI_SZ）定义新建文件类型
	//// 每个类中ShellNew项定义新建文件命令
	//New,
	StartMenu,				// %LocalAppData%\Microsoft\Windows\WinX
	MaxSource,

	ShellAndShellEx = 1u << ShMenuBit,	// 该项含Shell和ShellEx
	Com = 1u << (ShMenuBit + 1),		// 该项为COM处理程序
	Uwp = 1u << (ShMenuBit + 2),		// 该项为UWP应用
	FileType = 1u << (ShMenuBit + 3),	// 该项由文件类型定义
	Hidden = 1u << (ShMenuBit + 4),		// 隐藏
	NeverDefault = 1u << (ShMenuBit + 5),			// 绝不用作默认项
	HasSubMenu = 1u << (ShMenuBit + 6),				// 含子菜单
	OnlyInBrowserWindow = 1u << (ShMenuBit + 7),	// 只在资源管理器窗口显示
	HasLuaShield = 1u << (ShMenuBit + 8),			// 显示盾牌图标
	ShowAsDisabledIfHidden = 1u << (ShMenuBit + 8),	// 隐藏时显示为禁用
	NoWorkingDirectory = 1u << (ShMenuBit + 9),		// 不设置工作目录
	Extended = 1u << (ShMenuBit + 10),	// 扩展菜单项
	File = 1u << (ShMenuBit + 11),		// 该项为文件而非注册表项
};
ECK_ENUM_BIT_FLAGS(ShMenu);

class CShellMenuInfo
{
private:
	constexpr static struct
	{
		RegRoot eKey;
		PCWSTR pszSubKey;
		int cchSubKey;
		ShMenu uFlags;
	}
	Source[]
	{
		{},
		{ RegRoot::ClassesRoot,EckStrAndLen(LR"(*\)"),					ShMenu::ShellAndShellEx | ShMenu::FileType },
		{ RegRoot::ClassesRoot,EckStrAndLen(LR"(Folder\)"),				ShMenu::ShellAndShellEx },
		{ RegRoot::ClassesRoot,EckStrAndLen(LR"(Directory\)"),			ShMenu::ShellAndShellEx },
		{ RegRoot::ClassesRoot,EckStrAndLen(LR"(Drive\)"),				ShMenu::ShellAndShellEx },
		{ RegRoot::ClassesRoot,EckStrAndLen(LR"(AllFileSystemObjects\)"),ShMenu::ShellAndShellEx },
		{ RegRoot::ClassesRoot,EckStrAndLen(LR"(Directory\Background\)"),ShMenu::ShellAndShellEx },
		{ RegRoot::ClassesRoot,EckStrAndLen(LR"(DesktopBackground\)"),	ShMenu::ShellAndShellEx },
		{ RegRoot::ClassesRoot,EckStrAndLen(LR"(CLSID\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\)"),	ShMenu::ShellAndShellEx },
		{ RegRoot::ClassesRoot,EckStrAndLen(LR"(CLSID\{645FF040-5081-101B-9F08-00AA002F954E}\)"),	ShMenu::ShellAndShellEx },
	};
	constexpr static size_t MaxSourceBuf = 48/*最大*/ + 36/*CtxMenuHnd*/;

	constexpr static std::wstring_view CtxMenuHnd[]
	{
		{ EckStrAndLen(L"shellex\\ContextMenuHandlers") },
		{ EckStrAndLen(L"shellex\\-ContextMenuHandlers") },
	};
public:
	struct CMenu
	{
		ShMenu uFlags{};
		CRefStrW rsRegPath{};
		CRefStrW rsDisplayName{};
		CRefStrW rsClsidOrCmd{};
		CRefStrW rsFile{};
		CRefStrW rsIcon{};

		EckInlineNdCe ShMenu GetSource() const noexcept { return uFlags & ShMenu::AllSource; }
	};
private:
	CRegKey m_RegKey{};
	CEnumFile2 m_EnumFile{};
	CRefStrW m_rsBuf{};
	DWORD m_idxEnum{};
	BYTE m_idxSource{ 1 };
	BOOLEAN m_bSingleSource{};
	// ShMenu::ShellAndShellEx状态
	BOOLEAN m_bCurrIsShellEx{};
	BYTE m_idxShellExSrc{};// 下一次枚举的源
	// ShMenu::SendTo状态
	PWSTR m_pszSendToPath{};// CoTaskMemFree
	int m_cchSendToPath{};
	// ShMenu::StartMenu状态
	PWSTR m_pszWinXPath{};// RtlFreeHeap
	int m_cchWinXPath{};
	CEnumFile2 m_EnumStartMenu{};

	void LoadIndirectString(CRefStrW& rs)
	{
		// XXX: 
		rs.Reserve(64);
		SHLoadIndirectString(rs.Data(), rs.Data(),
			rs.Capacity(), nullptr);
		rs.ReCalcLen();
	}

	BOOL QueryComDisplayName(_In_reads_z_(38) PCWSTR pszClsid, CRefStrW& rsDisplayName)
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
	//===================================

	LSTATUS SsShellInitShell()
	{
		m_idxEnum = 0;
		WCHAR szBuf[MaxSourceBuf];
		TcsCopyLen(szBuf, Source[m_idxSource].pszSubKey,
			Source[m_idxSource].cchSubKey);
		TcsCopyLen(szBuf + Source[m_idxSource].cchSubKey, L"shell", 6);
		const auto ls = m_RegKey.Open(RegRootToKey(Source[m_idxSource].eKey),
			szBuf, KEY_READ);
		if (ls != ERROR_SUCCESS)
			return ls;
		DWORD cchMaxBuf;
		m_RegKey.QueryInfo(nullptr, nullptr, nullptr, &cchMaxBuf);
		m_rsBuf.Reserve(cchMaxBuf);
		m_bCurrIsShellEx = FALSE;
		m_idxShellExSrc = 0;
		return ERROR_SUCCESS;
	}

	LSTATUS SsShellInitShellEx()
	{
		m_idxEnum = 0;
		WCHAR szBuf[MaxSourceBuf];
		TcsCopyLen(szBuf, Source[m_idxSource].pszSubKey,
			Source[m_idxSource].cchSubKey);
		LSTATUS ls;
		DWORD cchMaxBuf;
		for (int i = m_idxShellExSrc; i < ARRAYSIZE(CtxMenuHnd); ++i)
		{
			const auto cmh = CtxMenuHnd[i];
			TcsCopyLen(szBuf + Source[m_idxSource].cchSubKey,
				cmh.data(), cmh.size() + 1);
			ls = m_RegKey.Open(RegRootToKey(Source[m_idxSource].eKey),
				szBuf, KEY_READ);
			if (ls == ERROR_SUCCESS)
			{
				m_RegKey.QueryInfo(nullptr, nullptr, nullptr, &cchMaxBuf);
				m_rsBuf.Reserve(cchMaxBuf);
				m_bCurrIsShellEx = TRUE;
				return ERROR_SUCCESS;
			}
		}
		return ERROR_NO_MORE_ITEMS;
	}

	LSTATUS WkiNextShell(CMenu& e)
	{
		LSTATUS ls;
		DWORD cchBuf{ (DWORD)m_rsBuf.Capacity() };
		ls = m_RegKey.EnumKey(m_idxEnum++, m_rsBuf.Data(), &cchBuf);
		if (ls != ERROR_SUCCESS)
			return ls;
		// m_rsBuf现在是项目名
		CRegKey Key{ m_RegKey.GetHKey(),m_rsBuf.Data(),KEY_READ };// 当前项目
		// 注册表路径
		e.rsRegPath.Reserve(cchBuf + 16);
		e.rsRegPath.PushBack(L"shell\\");
		e.rsRegPath.PushBack(m_rsBuf.Data(), cchBuf);
		// CLSID
		BOOL bClsid{};
		ls = Key.QueryValueStr(e.rsClsidOrCmd, L"ExplorerCommandHandler");
		if (ls == ERROR_SUCCESS)
		{
			e.uFlags |= ShMenu::Com;
			bClsid = TRUE;
		}
		else
		{
			ls = Key.GetValueStr(e.rsClsidOrCmd, L"command", nullptr);
			if (ls != ERROR_SUCCESS)
			{
				ls = Key.GetValueStr(e.rsClsidOrCmd,
					L"command", L"DelegateExecute");
				if (ls == ERROR_SUCCESS)
				{
					e.uFlags |= ShMenu::Com;
					bClsid = TRUE;
				}
			}
		}
		// 显示名称
		ls = Key.QueryValueStr(e.rsDisplayName, L"MUIVerb");
		if (ls != ERROR_SUCCESS)
		{
			ls = Key.QueryValueStr(e.rsDisplayName, nullptr);
			if (ls != ERROR_SUCCESS)
			{
				if (!bClsid ||
					!QueryComDisplayName(e.rsClsidOrCmd.Data(), e.rsDisplayName))
					e.rsDisplayName.DupString(m_rsBuf.Data(), cchBuf);
			}
		}
		if (e.rsDisplayName.Front() == L'@')
			LoadIndirectString(e.rsDisplayName);
		// 属性
		if (Key.IsValueExists(L"NeverDefault"))
			e.uFlags |= ShMenu::NeverDefault;
		if (Key.IsValueExists(L"OnlyInBrowserWindow"))
			e.uFlags |= ShMenu::OnlyInBrowserWindow;
		if (Key.IsValueExists(L"ShowAsDisabledIfHidden"))
			e.uFlags |= ShMenu::ShowAsDisabledIfHidden;
		if (Key.IsValueExists(L"NoWorkingDirectory"))
			e.uFlags |= ShMenu::NoWorkingDirectory;
		if (Key.IsValueExists(L"Extended"))
			e.uFlags |= ShMenu::Extended;
		return ERROR_SUCCESS;
	}

	LSTATUS WkiNextShellEx(CMenu& e, std::wstring_view svParent)
	{
		LSTATUS ls;
		DWORD cchBuf{ (DWORD)m_rsBuf.Capacity() };
		ls = m_RegKey.EnumKey(m_idxEnum++, m_rsBuf.Data(), &cchBuf);
		if (ls != ERROR_SUCCESS)
			return ls;
		// m_rsBuf现在是项目名
		CRegKey Key{ m_RegKey.GetHKey(),m_rsBuf.Data(),KEY_READ };// 当前项目
		// 注册表路径
		e.rsRegPath.Reserve(cchBuf + 36);
		e.rsRegPath
			.PushBack(svParent)
			.PushBackChar(L'\\')
			.PushBack(m_rsBuf.Data(), cchBuf);
		// CLSID
		BOOL bClsid{};
		ls = Key.QueryValueStr(e.rsClsidOrCmd, nullptr);
		if (ls == ERROR_SUCCESS && !e.rsClsidOrCmd.IsEmpty())
		{
			if (e.rsClsidOrCmd.Front() != L'{')// 默认值并非CLSID...
			{
				const auto t{ std::move(e.rsClsidOrCmd) };
				e.rsClsidOrCmd.DupString(m_rsBuf.Data(), cchBuf);
				m_rsBuf.DupString(t.Data(), t.Size());
			}
			e.uFlags |= ShMenu::Com;
			bClsid = TRUE;
		}
		// 显示名称
		ls = Key.QueryValueStr(e.rsDisplayName, L"MUIVerb");
		if (ls != ERROR_SUCCESS)
		{
			if (!bClsid ||
				!QueryComDisplayName(e.rsClsidOrCmd.Data(), e.rsDisplayName))
				e.rsDisplayName.DupString(m_rsBuf.Data(), cchBuf);
		}
		if (e.rsDisplayName.Front() == L'@')
			LoadIndirectString(e.rsDisplayName);
		return ERROR_SUCCESS;
	}

	LSTATUS WkNextShellAndShellEx(CMenu& e)
	{
		WCHAR szBuf[MaxSourceBuf];
		TcsCopyLen(szBuf, Source[m_idxSource].pszSubKey,
			Source[m_idxSource].cchSubKey);
		const auto pszSub = szBuf + Source[m_idxSource].cchSubKey;
		DWORD idx{}, cchMaxBuf, cchBuf;

		e.uFlags |= Source[m_idxSource].uFlags;
		if (!m_bCurrIsShellEx)
		{
			if (WkiNextShell(e) != ERROR_SUCCESS)
			{
				// 若枚举失败，则尝试切换到ShellEx
				if (SsShellInitShellEx() != ERROR_SUCCESS)
					return ERROR_NO_MORE_ITEMS;// 切换失败
				return WkNextShellAndShellEx(e);
			}
			return ERROR_SUCCESS;
		}
		else
			return WkiNextShellEx(e, CtxMenuHnd[m_idxShellExSrc]);
	}
	//===================================

	LSTATUS SsSendToInit()
	{
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SendTo,
			SHGFP_TYPE_CURRENT, nullptr, &m_pszSendToPath)))
		{
			m_cchSendToPath = (int)TcsLen(m_pszSendToPath);
			return WIN32_FROM_NTSTATUS(m_EnumFile.Open(m_pszSendToPath));
		}
		return ERROR_ACCESS_DENIED;
	}

	LSTATUS WkNextSendTo(CMenu& e)
	{
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

		auto rsIniPath{ e.rsFile };

		const auto posFileName = e.rsFile.Size();
		e.rsFile.PushBack(pInfo->FileName, (int)pInfo->FileNameLength / 2);
		if (pInfo->FileAttributes & FILE_ATTRIBUTE_HIDDEN)
			e.uFlags |= ShMenu::Hidden;

		// XXX: 修改INI实现
		rsIniPath.PushBack(L"desktop.ini");
		e.rsDisplayName.Reserve(64);
		const auto cchDispName = (int)GetPrivateProfileStringW(L"LocalizedFileNames",
			e.rsFile.Data() + posFileName,
			nullptr,
			e.rsDisplayName.Data(),
			e.rsDisplayName.Capacity(),
			rsIniPath.Data());
		if (cchDispName)
			e.rsDisplayName.ReSize(cchDispName);
		else
		{
			e.rsDisplayName.DupString(pInfo->FileName, (int)pInfo->FileNameLength);
			const auto pszExt = PathFindExtensionW(e.rsDisplayName.Data());
			e.rsDisplayName.ReSize(int(pszExt - e.rsDisplayName.Data()));
		}

		if (e.rsDisplayName.Front() == L'@')
			LoadIndirectString(e.rsDisplayName);

		e.rsIcon.Clear();
		if (e.rsFile.IsEndOfI(EckStrAndLen(L".lnk")))
		{
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
			const auto pszExt = PathFindExtensionW(e.rsFile.Data());
			if (!*pszExt)
				goto NotLnkOrCom;
			CRegKey Key{ HKEY_CLASSES_ROOT,pszExt,KEY_READ };
			Key.QueryValueStr(e.rsIcon, nullptr);// CLSID路径
			if (e.rsIcon.IsEmpty())
				goto NotLnkOrCom;

			const auto posClsid = e.rsIcon.FindChar(L'{');
			e.rsClsidOrCmd.DupString(e.rsIcon.Data() + posClsid, 38);
			e.uFlags |= ShMenu::Com;

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
	//===================================

	LSTATUS SsStartMenuInit()
	{
		UNICODE_STRING usPath
			RTL_CONSTANT_STRING(LR"(%LocalAppData%\Microsoft\Windows\WinX)");
		ULONG cbBuffer{ (47 + 30/*用户名*/ + 5) * sizeof(WCHAR) };
		m_pszWinXPath = (PWSTR)RtlAllocateHeap(RtlProcessHeap(), 0, cbBuffer);
		UNICODE_STRING usExpand;
		usExpand.Buffer = m_pszWinXPath;
		usExpand.Length = 0;
		usExpand.MaximumLength = cbBuffer;
		auto nts = RtlExpandEnvironmentStrings_U(
			nullptr, &usPath, &usExpand, &cbBuffer);
		if (!NT_SUCCESS(nts))
			if (nts != STATUS_BUFFER_TOO_SMALL)
			{
				RtlFreeHeap(RtlProcessHeap(), 0, m_pszWinXPath);
				m_pszWinXPath = (PWSTR)RtlAllocateHeap(RtlProcessHeap(), 0, cbBuffer);
				usExpand.Buffer = m_pszWinXPath;
				usExpand.Length = 0;
				usExpand.MaximumLength = cbBuffer;
				if (!NT_SUCCESS(nts = RtlExpandEnvironmentStrings_U(
					nullptr, &usPath, &usExpand, &cbBuffer)))
					return WIN32_FROM_NTSTATUS(nts);
			}
			else
				return WIN32_FROM_NTSTATUS(nts);
		m_cchWinXPath = (int)usExpand.Length / 2;
		return WIN32_FROM_NTSTATUS(m_EnumFile.Open(m_pszWinXPath));
	}

	LSTATUS WkNextStartMenu(CMenu& e)
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
				m_rsBuf.DupString(L"\\", 1);
				m_rsBuf.DupString(pInfo->FileName, (int)pInfo->FileNameLength / 2);
				nts = m_EnumStartMenu.Open(m_rsBuf.Data(), m_EnumFile.GetHDir());
				if (NT_SUCCESS(nts))
					break;// 直到找到第一个有效目录
			}
			return WkNextStartMenu(e);
		}
		EckLoop()
		{
			nts = m_EnumStartMenu.Next(nullptr, 0, pInfo);
			if (!NT_SUCCESS(nts))
			{
				m_EnumStartMenu.Close();
				if (nts == STATUS_NO_MORE_FILES)
					return WkNextStartMenu(e);
				else
					return WIN32_FROM_NTSTATUS(nts);
			}
			if (*pInfo->FileName == L'.')
				continue;

			e.rsFile.DupString(m_pszWinXPath, m_cchWinXPath);
			e.rsFile.PushBackChar(L'\\');
			e.rsFile.PushBack(pInfo->FileName, (int)pInfo->FileNameLength / 2);
			if (pInfo->FileAttributes & FILE_ATTRIBUTE_HIDDEN)
				e.uFlags |= ShMenu::Hidden;
			e.rsDisplayName.DupString(pInfo->FileName, (int)pInfo->FileNameLength / 2);
			if (e.rsDisplayName.Front() == L'@')
				LoadIndirectString(e.rsDisplayName);
			e.rsIcon.Clear();
		}
		return ERROR_SUCCESS;
	}

	LSTATUS SsPostSwitch()
	{
		if (m_idxSource < ARRAYSIZE(Source))
		{
			if (SsShellInitShell() != ERROR_SUCCESS)
				if (SsShellInitShellEx() != ERROR_SUCCESS)
					return SsNext();
			return ERROR_SUCCESS;
		}
		else if (m_idxSource == (BYTE)ShMenu::SendTo)
		{
			if (SsSendToInit() != ERROR_SUCCESS)
				return SsNext();
			return ERROR_SUCCESS;
		}
		else if (m_idxSource == (BYTE)ShMenu::StartMenu)
		{
			if (SsStartMenuInit() != ERROR_SUCCESS)
				return SsNext();
			return ERROR_SUCCESS;
		}
		else
			return ERROR_NO_MORE_ITEMS;
	}

	LSTATUS SsNext()
	{
		if (m_bSingleSource)
			return ERROR_NO_MORE_ITEMS;
		m_idxEnum = 0;
		++m_idxSource;
		return SsPostSwitch();
	}
public:
	LSTATUS Next(CMenu& e)
	{
		LSTATUS ls{};
		e.uFlags = (ShMenu)m_idxSource;
		if (m_idxSource < ARRAYSIZE(Source))
			ls = WkNextShellAndShellEx(e);
		else if (m_idxSource == (BYTE)ShMenu::SendTo)
			ls = WkNextSendTo(e);
		else if (m_idxSource == (BYTE)ShMenu::StartMenu)
			ls = WkNextStartMenu(e);

		if (ls != ERROR_SUCCESS)
		{
			ls = SsNext();
			if (ls == ERROR_SUCCESS)
				return Next(e);
		}
		return ls;
	}

	LSTATUS Reset(ShMenu e, BOOL bSingle = FALSE) noexcept
	{
		m_bSingleSource = bSingle;
		if (e == ShMenu::AllSource)
			m_idxSource = 1;
		else
			m_idxSource = BYTE(e);
		SsPostSwitch();
		return ERROR_INVALID_PARAMETER;
	}

	EckInlineCe void SetSingleSource(BOOL bSingle) noexcept { m_bSingleSource = bSingle; }
};
ECK_NAMESPACE_END