#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
// 部件
enum class Part
{
	Invalid = -1,

	Button,
	RadioButton,
	CheckButton,
	GroupBox,

	Edit,

	List,
	ListItem,

	UserBegin = 0x1000,
};
// 状态
enum class State
{
	Normal,
	Hot,
	Pressed,
	Disabled,
	Checked,
	Focused,
	Selected,
	CheckedFocused,
	CheckedSelected,
	MaxState
};

constexpr inline UINT ThemeFileMagic = 'EKDT';

// 文件头
struct THEME_FILE_HEADER
{
	UINT Magic;
	int iVer;
	UINT uFlags;
	DWORD ocbAtlas;
};

class CTheme : public IUnknown
{
private:
	struct THEME_INFO
	{
		int cPart;
	};

	struct STATE
	{
		State eState;
		D2D1_RECT_F rcInAtlas;
		D2D1_RECT_F rc9Grid;
	};

	struct PART
	{
		Part ePart;
		int cState;
	};

	ULONG m_cRef{ 1 };

public:
	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	STDMETHODIMP_(ULONG) Release()
	{
		ULONG cRef = InterlockedDecrement(&m_cRef);
		if (cRef == 0)
		{
			delete this;
		}
		return cRef;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject)
	{
		EckDbgBreak();
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	HRESULT Open(_In_z_ PCWSTR pszFileName)
	{

	}

	HRESULT Open(_In_reads_bytes_(cbData) PCVOID pData, SIZE_T cbData)
	{

	}

	HRESULT DrawBackground(ID2D1DeviceContext* pDC, Part ePart, State eState,
		const D2D1_RECT_F& rc, _In_opt_ const D2D1_RECT_F* prcClip)
	{

	}

	BOOL IsPartValid(Part ePart, State eState) const
	{

	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END