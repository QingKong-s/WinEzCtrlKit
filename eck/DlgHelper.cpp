#include "DlgHelper.h"
#include "WndHelper.h"


ECK_NAMESPACE_BEGIN
HWND GetFirstTabStopCtrl(HWND hWnd)
{
	HWND hCtrl = NULL;
	EnumChildWindows(hWnd,
		[](HWND hWnd, LPARAM lParam)->BOOL
		{
			if (IsWindowVisible(hWnd) &&
				IsWindowEnabled(hWnd) &&
				IsBitSet(GetWindowLongPtrW(hWnd, GWL_STYLE), WS_TABSTOP))
			{
				*(HWND*)lParam = hWnd;
				return FALSE;
			}
			return TRUE;
		}, (LPARAM)&hCtrl);
	return hCtrl;
}

CRefBin SerializeDialogTemplate(const DLGTDLG& Dlg, const std::vector<DLGTITEM>& Items)
{
	using namespace eck::Literals;
	if (Dlg.wVer != 1_us || Dlg.wSignature != 0xFFFF_us || (SIZE_T)Dlg.cDlgItems != Items.size())
		return {};

	SIZE_T cbTotal =
		sizeof(DLGTHEADER) /*头*/ +
		sizeof(WORD) * 3 /*菜单、窗口类、标题三个数组的第一个元素*/;

	if (Dlg.Menu)
	{
		auto& Val = *Dlg.Menu;
		if (Val.index() == 0)// 资源ID
			cbTotal += sizeof(WORD);
		else// 名称字符串
			cbTotal += ((std::get<1>(Val).Size() + 1) * sizeof(WCHAR));
	}

	if (Dlg.Class)
	{
		auto& Val = *Dlg.Class;
		if (Val.index() == 0)// 类原子
			cbTotal += sizeof(ATOM);
		else// 类名字符串
			cbTotal += ((std::get<1>(Val).Size() + 1) * sizeof(WCHAR));
	}

	if (Dlg.Caption)
	{
		auto& Val = *Dlg.Caption;
		cbTotal += ((Val.Size() + 1) * sizeof(WCHAR));
	}

	BOOL bHasFontStru = IsBitSet(Dlg.dwStyle, DS_SETFONT) || IsBitSet(Dlg.dwStyle, DS_SHELLFONT);
	if (bHasFontStru)
	{
		cbTotal += sizeof(Dlg.Font);
		cbTotal += ((Dlg.rsFontName.Size() + 1) * sizeof(WCHAR));
	}

	cbTotal += CalcNextAlignBoundaryDistance(NULL, (PCVOID)cbTotal, sizeof(DWORD));

	cbTotal += (Items.size() * (
		sizeof(DLGITEMTHEADER) /*头*/ +
		sizeof(WORD) * 3 /*窗口类、标题两个数组的第一个元素和附加数据大小*/));
	for (auto& x : Items)
	{
		if (x.Class.index() == 0)// 类原子
			cbTotal += sizeof(ATOM);
		else// 类名字符串
			cbTotal += ((std::get<1>(x.Class).Size() + 1) * sizeof(WCHAR));

		if (x.Caption.index() == 0)// 资源ID
			cbTotal += sizeof(WORD);
		else// 标题字符串
			cbTotal += ((std::get<1>(x.Caption).Size() + 1) * sizeof(WCHAR));

		if (x.rbExtra.Size())
			cbTotal += AlignMemSize(x.rbExtra.Size(), 2);

		cbTotal += CalcNextAlignBoundaryDistance(NULL, (PCVOID)cbTotal, sizeof(DWORD));
	}

	CRefBin rb;
	rb.ReSize(cbTotal);

	CMemWriter w(rb.Data());
	w.Write(&Dlg, sizeof(DLGTHEADER));

	if (Dlg.Menu)
	{
		auto& Val = *Dlg.Menu;
		if (Val.index() == 0)// 资源ID
			w << 0xFFFF_us << std::get<0>(Val);
		else// 名称字符串
			w << std::get<1>(Val);
	}
	else
		w << 0_us;

	if (Dlg.Class)
	{
		auto& Val = *Dlg.Class;
		if (Val.index() == 0)// 类原子
			w << 0xFFFF_us << std::get<0>(Val);
		else// 类名字符串
			w << std::get<1>(Val);
	}
	else
		w << 0_us;

	if (Dlg.Caption)
		w << *Dlg.Caption;
	else
		w << 0_us;

	if (bHasFontStru)
		w << Dlg.Font << Dlg.rsFontName;

	w += CalcNextAlignBoundaryDistance(rb.Data(), w, sizeof(DWORD));

	for (auto& x : Items)
	{
		w.Write(&x, sizeof(DLGITEMTHEADER));

		if (x.Class.index() == 0)// 类原子
			w << 0xFFFF_us << std::get<0>(x.Class);
		else// 类名字符串
			w << std::get<1>(x.Class);

		if (x.Caption.index() == 0)// 资源ID
			w << 0xFFFF_us << std::get<0>(x.Caption);
		else// 标题字符串
			w << std::get<1>(x.Caption);

		w << (WORD)x.rbExtra.Size();
		if (x.rbExtra.Size())
		{
			memcpy(w, x.rbExtra.Data(), x.rbExtra.Size());
			w += AlignMemSize(x.rbExtra.Size(), 2);
		}

		w += CalcNextAlignBoundaryDistance(rb.Data(), w, sizeof(DWORD));
	}

	return rb;
}

BOOL DeserializeDialogTemplate(PCVOID pTemplate, DLGTDLG& Dlg, std::vector<DLGTITEM>& Items)
{
	using namespace eck::Literals;
	CMemReader r(pTemplate);
	r.Read(&Dlg, sizeof(DLGTHEADER));
	if (Dlg.wVer != 1_us || Dlg.wSignature != 0xFFFF_us)
		return FALSE;

	WORD us;
	CRefStrW rs;

	r >> us;
	switch (us)
	{
	case 0_us:
		break;
	case 0xFFFF_us:
		r >> us;
		Dlg.Menu = us;
		break;
	default:
	{
		r -= sizeof(WORD);
		r >> rs;
		Dlg.Menu = std::move(rs);
	}
	break;
	}

	r >> us;
	switch (us)
	{
	case 0_us:
		break;
	case 0xFFFF_us:
		r >> us;
		Dlg.Class = us;
		break;
	default:
	{
		r -= sizeof(WORD);
		r >> rs;
		Dlg.Class = std::move(rs);
	}
	break;
	}

	r >> us;
	if (us)
	{
		r -= sizeof(WORD);
		r >> rs;
		Dlg.Caption = std::move(rs);
	}

	if (IsBitSet(Dlg.dwStyle, DS_SETFONT) || IsBitSet(Dlg.dwStyle, DS_SHELLFONT))
	{
		r >> Dlg.Font;
		r >> rs;
		Dlg.rsFontName = std::move(rs);
	}

	r += CalcNextAlignBoundaryDistance(pTemplate, r, sizeof(DWORD));

	Items.resize(Dlg.cDlgItems);
	for (auto& x : Items)
	{
		r.Read(&x, sizeof(DLGITEMTHEADER));

		r >> us;
		if (us == 0xFFFF_us)
		{
			r >> us;
			x.Class = us;
		}
		else
		{
			r -= sizeof(WCHAR);
			r >> rs;
			x.Class = std::move(rs);
		}

		r >> us;
		if (us == 0xFFFF_us)
		{
			r >> us;
			x.Caption = us;
		}
		else
		{
			r -= sizeof(WCHAR);
			r >> rs;
			x.Caption = std::move(rs);
		}

		r >> us;
		if (us)
		{
			CRefBin rb;
			rb.ReSize(us);
			r.Read(rb.Data(), rb.Size());
			x.rbExtra = std::move(rb);
		}

		r += CalcNextAlignBoundaryDistance(pTemplate, r, sizeof(DWORD));
	}

	return TRUE;
}

HWND CreateWindowFromDialogTemplate(DLGTDLG& Dlg, std::vector<DLGTITEM>& Items,
	WNDPROC pfnWndProc, HWND hParent, HINSTANCE hInstance, void* pParam, HMENU* phMenu, HFONT* phFont, UINT uFlags)
{
	using namespace eck::Literals;
	if (Dlg.wVer != 1_us || Dlg.wSignature != 0xFFFF_us || (SIZE_T)Dlg.cDlgItems != Items.size())
		return NULL;
	PCWSTR pszClass, pszCaption, pszMenu;
	if (Dlg.Class)
	{
		auto& Val = *Dlg.Class;
		if (Val.index() == 0)// 类原子
			pszClass = MAKEINTATOMW(std::get<0>(Val));
		else// 类名字符串
			pszClass = std::get<1>(Val).Data();
	}
	else
		pszClass = MAKEINTATOMW(32770);

	if (Dlg.Caption)
		pszCaption = (*Dlg.Caption).Data();
	else
		pszCaption = NULL;

	HMENU hMenu;
	if (Dlg.Menu)
	{
		auto& Val = *Dlg.Menu;
		if (Val.index() == 0)// 资源ID
			pszMenu = MAKEINTRESOURCEW(std::get<0>(Val));
		else// 名称字符串
			pszMenu = std::get<1>(Val).Data();
		hMenu = LoadMenuW(hInstance, pszMenu);
	}
	else
		hMenu = NULL;

	HFONT hFont;
	BOOL bHasFontStru = IsBitSet(Dlg.dwStyle, DS_SETFONT) || IsBitSet(Dlg.dwStyle, DS_SHELLFONT);
	if (bHasFontStru)
		hFont = EzFont(Dlg.rsFontName.Data(), Dlg.Font.wPoint, Dlg.Font.wWeight,
			Dlg.Font.bItalic, FALSE, FALSE, NULL, Dlg.Font.byCharSet);
	else
		hFont = NULL;

	RECT rc{ Dlg.x,Dlg.y,Dlg.cx,Dlg.cy };
	int xBaseUnit, yBaseUnit;
	if (!IsBitSet(uFlags, CWFDT_USEPIXELUNIT))
	{
		if (IsBitSet(Dlg.dwStyle, DS_SETFONT) || IsBitSet(Dlg.dwStyle, DS_SHELLFONT))
		{
			HDC hCDC = CreateCompatibleDC(NULL);
			SelectObject(hCDC, hFont);
			SIZE size;
			GetTextExtentPoint32W(hCDC, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size);
			xBaseUnit = ((size.cx / 26) + 1) / 2;
			yBaseUnit = size.cy;
			DeleteDC(hCDC);
		}
		else
		{
			long lRet = GetDialogBaseUnits();
			xBaseUnit = LOWORD(lRet);
			yBaseUnit = HIWORD(lRet);
		}
		/*
		* 公式：
		* xPixel = xDLU * xBaseUnit / 4
		* yPixel = yDLU * yBaseUnit / 8
		*/
		rc.left = rc.left * xBaseUnit / 4;
		rc.top = rc.top * yBaseUnit / 8;
		rc.right = rc.right * xBaseUnit / 4;
		rc.bottom = rc.bottom * yBaseUnit / 8;
	}

	int x = rc.left, y = rc.top;
	rc.right += rc.left;
	rc.bottom += rc.top;
#if _WIN32_WINNT >= _WIN32_WINNT_WIN10
	AdjustWindowRectExForDpi(&rc, Dlg.dwStyle, hMenu != NULL, Dlg.dwExStyle, GetDpi(hParent));
#else
	AdjustWindowRectEx(&rc, Dlg.dwStyle, hMenu != NULL, Dlg.dwExStyle);
#endif // _WIN32_WINNT >= _WIN32_WINNT_WIN10
	rc.right -= rc.left;
	rc.bottom -= rc.top;
	rc.left = x;
	rc.top = y;
	HWND hWnd = CreateWindowExW(Dlg.dwExStyle, pszClass, pszCaption, Dlg.dwStyle & ~WS_VISIBLE,
		rc.left, rc.top, rc.right, rc.bottom, hParent, hMenu, hInstance, pParam);
	if (pfnWndProc)
		SetWindowProc(hWnd, pfnWndProc);
	SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, FALSE);

	HWND hCtrl;
	for (auto& x : Items)
	{
		if (x.Class.index() == 0)// 类原子
			pszClass = MAKEINTATOMW(std::get<0>(x.Class));
		else// 类名字符串
			pszClass = std::get<1>(x.Class).Data();

		if (x.Caption.index() == 0)// 类原子
			pszCaption = MAKEINTATOMW(std::get<0>(x.Caption));
		else// 类名字符串
			pszCaption = std::get<1>(x.Caption).Data();

		rc = { x.x,x.y,x.cx,x.cy };
		if (!IsBitSet(uFlags, CWFDT_USEPIXELUNIT))
		{
			rc.left = rc.left * xBaseUnit / 4;
			rc.top = rc.top * yBaseUnit / 8;
			rc.right = rc.right * xBaseUnit / 4;
			rc.bottom = rc.bottom * yBaseUnit / 8;
		}
		hCtrl = CreateWindowExW(x.dwExStyle, pszClass, pszCaption, x.dwStyle,
			rc.left, rc.top, rc.right, rc.bottom, hWnd, i32ToP<HMENU>(x.id), hInstance, x.rbExtra.Data());
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)hFont, FALSE);
	}

	HWND hFirstCtrl = GetFirstTabStopCtrl(hWnd);
	if (SendMessageW(hWnd, WM_INITDIALOG, (WPARAM)hFirstCtrl, (LPARAM)pParam))
		SetFocus(hFirstCtrl);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	if (phMenu)
		*phMenu = hMenu;
	if (phFont)
		*phFont = hFont;
	return hWnd;
}
ECK_NAMESPACE_END