#pragma once
#include "CRegKey.h"
#include "ProcModule.h"

ECK_NAMESPACE_BEGIN
enum class ThreadingModel
{
	Apartment,
	Free,
	Both,
	Neutral,
	Unknown
};

struct CSH_REG_INPROC_SRV
{
	std::wstring_view svName;
	std::wstring_view svInfoTip;
	std::wstring_view svFileName;
	std::wstring_view svIcon;
	ThreadingModel eModel;
};

constexpr inline std::wstring_view RegThreadingModel[]
{
	L"Apartment"sv,L"Free"sv,L"Both"sv,L"Neutral"sv,{}
};

inline HRESULT RegisterInProcessComServer(REFCLSID clsid,
	const CSH_REG_INPROC_SRV& Params) noexcept
{
	std::wstring_view svModFile{};
	CRefStrW rsModFile{};
	if (Params.svFileName.empty())
	{
		GetModuleFile(NtCurrentImageBase(), rsModFile);
		svModFile = rsModFile.ToStringView();
	}
	else
		svModFile = Params.svFileName;

	WCHAR szTopPath[6 + 38 + 1]{ L"CLSID\\" };
	(void)StringFromGUID2(clsid, szTopPath + 6, 39);
	CRegKey Key{};
	const auto ls = Key.Create(HKEY_CLASSES_ROOT, szTopPath, KEY_WRITE);
	if (ls)
		return HRESULT_FROM_WIN32(ls);
	// CLSID\{GUID}\
	// - (Default)			名称
	// - InfoTip			提示信息
	// + InprocServer32
	//   - (Default)		模块文件路径
	//   - ThreadingModel	套间类型
	// + DefaultIcon
	//   - (Default)		图标路径
	Key.SetValue(nullptr, nullptr, Params.svName.data(),
		DWORD(Params.svName.size() * sizeof(WCHAR)), REG_SZ);
	if (!Params.svInfoTip.empty())
	{
		Key.SetValue(L"InfoTip", nullptr, Params.svInfoTip.data(),
			DWORD(Params.svInfoTip.size() * sizeof(WCHAR)), REG_SZ);
	}
	CRegKey Key2{};
	//
	Key2.Create(Key.GetHKey(), L"InprocServer32");
	Key2.SetValue(nullptr, nullptr, svModFile.data(),
		DWORD(svModFile.size() * sizeof(WCHAR)), REG_SZ);
	const auto& Model = RegThreadingModel[size_t(Params.eModel)];
	Key2.SetValue(nullptr, L"ThreadingModel", Model.data(),
		DWORD(Model.size() * sizeof(WCHAR)), REG_SZ);
	//
	if (!Params.svIcon.empty())
	{
		Key2.Create(Key.GetHKey(), L"DefaultIcon");
		Key2.SetValue(nullptr, nullptr, Params.svIcon.data(),
			DWORD(Params.svIcon.size() * sizeof(WCHAR)), REG_SZ);
	}
	return S_OK;
}

inline HRESULT UnregisterInProcessComServer(REFCLSID clsid) noexcept
{
	CRegKey Key{};
	const auto ls = Key.Open(HKEY_CLASSES_ROOT, L"CLSID", KEY_ENUMERATE_SUB_KEYS | DELETE);
	if (ls)
		return HRESULT_FROM_WIN32(ls);
	WCHAR szClsid[38 + 1];
	(void)StringFromGUID2(clsid, szClsid, 39);
	return HRESULT_FROM_WIN32(Key.DeleteTree(szClsid));
}
ECK_NAMESPACE_END