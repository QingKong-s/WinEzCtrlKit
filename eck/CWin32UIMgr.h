#pragma once
#include "CWnd.h"
#include "CIniExt.h"

ECK_NAMESPACE_BEGIN
/*
[>Form]

[>LinearLayoutV]
	[Default]
	Lyt.Flags=FixV,FixH

	[EditExt]
	Name=EDName
	CueBanner=Name

	[EditExt]
	Name=EDPassword
	CueBanner=Password

	[>LinearLayoutH]
		[Default]
		Lyt.Flags=FixV,FixH

		[Button]
		Name=BTOk
		Text=Ok

		[Button]
		Name=BTCancel
		Text=Cancel
	[<]
[<LinearLayoutV]

[<]

*/
class CWin32UIMgr
{
private:
	CRefStrW m_rsNamePool{};
	std::unordered_map<std::wstring_view, CObject*> m_Obj{};
public:
	HRESULT LoadLayout(PCWSTR pszIni, int cchIni, IniResult* pIniResult = nullptr)
	{
		CIniExtMut Ini{};
		const auto r = Ini.Load(pszIni, cchIni);
		if (r != IniResult::Ok)
		{
			if (pIniResult)
				*pIniResult = r;
			return E_INVALIDARG;
		}
		if (pIniResult)
			*pIniResult = IniResult::Ok;

		return S_OK;
	}
};
ECK_NAMESPACE_END