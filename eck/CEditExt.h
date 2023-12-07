#pragma once
#include "CEdit.h"
#include "CSubclassMgr.h"

ECK_NAMESPACE_BEGIN
inline constexpr int
DATAVER_EDITEXT_1 = 1;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CREATEDATA_EDITEXT
{
	int iVer;
	COLORREF crText;
	COLORREF crTextBK;
	COLORREF crBK;
	ECKENUM iInputMode;
	BITBOOL bMultiLine : 1;
	BITBOOL bAutoWrap : 1;
};
#pragma pack(pop)

class CEditExt :public CEdit
{
public:
	enum class InputMode
	{
		Normal = 0,
		ReadOnly = 1,
		Password = 2,
		NeedFilterKey = 2,// 仅供内部使用
		IntText = 3,
		RealText = 4,
		Byte = 5,
		Short = 6,
		Int = 7,
		LongLong = 8,
		Float = 9,
		Double = 10,
		DateTime = 11,
	};
private:
	SUBCLASS_MGR_DECL(CEditExt);
	SUBCLASS_REF_MGR_DECL(CEditExt, ObjRecorderRefPlaceholder);
private:
	COLORREF m_crText;			// 文本颜色
	COLORREF m_crTextBK;		// 文本背景色
	COLORREF m_crBK;			// 编辑框背景色
	InputMode m_iInputMode = InputMode::Normal;	// 输入方式
	BITBOOL m_bMultiLine : 1 = FALSE;			// 多行
	BITBOOL m_bAutoWrap : 1 = TRUE;				// 自动换行
	WCHAR m_chMask = 0;			// 掩码字符

	HBRUSH m_hbrEditBK = NULL;	// 背景画刷
	int m_cyText = 0;			// 文本高度
	RECT m_rcMargins{};			// 边距
	HWND m_hParent = NULL;		// 父窗口

	void UpdateTextInfo();

	static LRESULT CALLBACK SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
public:
	CEditExt()
	{
		m_crTextBK = m_crBK = GetSysColor(COLOR_WINDOW);
		m_crText = CLR_DEFAULT;
	}

	virtual ~CEditExt()
	{
		DeleteObject(m_hbrEditBK);
	}

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;

	void SerializeData(CRefBin& rb) override
	{
		const SIZE_T cbSize = sizeof(CREATEDATA_EDITEXT);
		CEdit::SerializeData(rb);
		((CREATEDATA_EDIT*)CWnd::SkipBaseData(rb.Data()))->chPassword = GetPasswordChar();

		CMemWriter w(rb.PushBack(cbSize), cbSize);
		CREATEDATA_EDITEXT* p;
		w.SkipPointer(p);
		p->iVer = DATAVER_EDITEXT_1;
		p->crText = GetClr(0);
		p->crTextBK = GetClr(1);
		p->crBK = GetClr(2);
		p->iInputMode = (ECKENUM)GetInputMode();
		p->bMultiLine = GetMultiLine();
		p->bAutoWrap = GetAutoWrap();
	}

	/// <summary>
	/// 置颜色
	/// </summary>
	/// <param name="iType">0 - 文本  1 - 文本背景  2 - 背景</param>
	/// <param name="cr">颜色</param>
	void SetClr(int iType, COLORREF cr);

	/// <summary>
	/// 取颜色
	/// </summary>
	/// <param name="iType">0 - 文本  1 - 文本背景  2 - 背景</param>
	EckInline COLORREF GetClr(int iType)
	{
		switch (iType)
		{
		case 0:return m_crText;
		case 1:return m_crTextBK;
		case 2:return m_crBK;
		}
		assert(FALSE);
		return 0;
	}

	EckInline void SetMultiLine(BOOL bMultiLine)
	{
		m_bMultiLine = bMultiLine;
	}

	EckInline BOOL GetMultiLine()
	{
		return IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), ES_MULTILINE);
	}

	EckInline void SetAutoWrap(BOOL bAutoWrap)
	{
		m_bAutoWrap = bAutoWrap;
	}

	EckInline BOOL GetAutoWrap()
	{
		return m_bAutoWrap;
	}

	EckInline void SetInputMode(InputMode iInputMode)
	{
		m_iInputMode = iInputMode;
		SendMsg(EM_SETREADONLY, iInputMode == InputMode::ReadOnly, 0);
		SendMsg(EM_SETPASSWORDCHAR, (iInputMode == InputMode::Password ? m_chMask : 0), 0);
	}

	EckInline InputMode GetInputMode()
	{
		return m_iInputMode;
	}

	EckInline void SetPasswordChar(WCHAR chMask)
	{
		m_chMask = chMask;
		if (m_iInputMode == InputMode::Password)
			CEdit::SetPasswordChar(m_chMask);
	}

	EckInline WCHAR GetPasswordChar()
	{
		return m_chMask;
	}
};

ECK_NAMESPACE_END