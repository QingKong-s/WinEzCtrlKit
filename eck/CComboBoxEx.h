/*
* WinEzCtrlKit Library
*
* CComboBoxEx.h ： 标准组合框Ex
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CComboBox.h"
#include "CRefBinStream.h"
#include "CStreamView.h"

ECK_NAMESPACE_BEGIN
inline constexpr int CDV_COMBOBOXEX_1 = CDV_COMBOBOX_1 + 1;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_COMBOBOXEX
{
	struct ITEM
	{
		int cchText;
		int idxImage;
		int idxSelImage;
		int idxOverlay;
		int iIndent;
		LPARAM lParam;
		// WCHAR szText[];// 长度cchText + 1
	};

	CTRLDATA_COMBOBOX Base;// 无项目数据
	DWORD cbImageList;
	// ITEM Items[];// 长度CTRLDATA_COMBOBOX::cItem
};
#pragma pack(pop)

#define ECK_CWNDPROP_CBE_STYLE(Name, Style)				\
	ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
	BOOL StyleGet##Name() const							\
	{													\
		if constexpr (Style == 0)						\
			return !!GetExtendedStyle();				\
		else											\
			return IsBitSet(GetExtendedStyle(), Style);	\
	}													\
	void StyleSet##Name(BOOL b) const					\
	{													\
		SetExtendedStyle(b ? Style : 0, Style);			\
	}

class CComboBoxEx :public CComboBox
{
public:
	ECK_RTTI(CComboBoxEx);
	ECK_CWND_NOSINGLEOWNER(CComboBoxEx);
	ECK_CWND_CREATE_CLS(WC_COMBOBOXEXW);

	ECK_CWNDPROP_CBE_STYLE(CaseSensitive, CBES_EX_CASESENSITIVE);
	ECK_CWNDPROP_CBE_STYLE(NoEditImage, CBES_EX_NOEDITIMAGE);
	ECK_CWNDPROP_CBE_STYLE(NoEditImageIndent, CBES_EX_NOEDITIMAGEINDENT);
	ECK_CWNDPROP_CBE_STYLE(NoSizeLimit, CBES_EX_NOSIZELIMIT);
	ECK_CWNDPROP_CBE_STYLE(PathWordBreakProc, CBES_EX_PATHWORDBREAKPROC);
	ECK_CWNDPROP_CBE_STYLE(TextEndEllipsis, CBES_EX_TEXTENDELLIPSIS);

	[[nodiscard]] EckInline constexpr static PCVOID SkipBaseData(PCVOID p)
	{
		return CComboBox::SkipBaseData(p);
	}

	void SerializeData(CRefBin& rb, const SERIALIZE_OPT* pOpt = nullptr) override
	{
		SERIALIZE_OPT Opt{ pOpt ? *pOpt : SERIALIZE_OPT{} };
		Opt.uFlags |= SERF_NO_COMBO_ITEM;
		CComboBox::SerializeData(rb, &Opt);
		rb.PushBack(sizeof(CTRLDATA_COMBOBOXEX) - sizeof(CTRLDATA_COMBOBOX));
		auto pBase = (CTRLDATA_COMBOBOX*)CWnd::SkipBaseData(rb.Data());
		const auto ocbBase = (PCBYTE)pBase - rb.Data();
		pBase->iVer = CDV_COMBOBOXEX_1;
		CRefStrW rs{ (pOpt && pOpt->cchTextBuf) ? pOpt->cchTextBuf : MAX_PATH };
		COMBOBOXEXITEMW cbei;
		cbei.mask = CBEIF_IMAGE | CBEIF_INDENT | CBEIF_LPARAM | CBEIF_OVERLAY |
			CBEIF_SELECTEDIMAGE | CBEIF_TEXT;
		const auto cItem = pBase->cItem;
		pBase->cItem = -pBase->cItem;// 标记为ComboxEx项目
		EckCounter(cItem, i)
		{
			cbei.pszText = rs.Data();
			cbei.cchTextMax = rs.Size();
			cbei.iItem = i;
			GetItem(&cbei);
			const auto cchText = (int)wcslen(cbei.pszText);
			const auto pItem = (CTRLDATA_COMBOBOXEX::ITEM*)
				rb.PushBack(sizeof(CTRLDATA_COMBOBOXEX::ITEM) + (cchText + 1) * sizeof(WCHAR));
			pItem->cchText = cchText;
			pItem->idxImage = cbei.iImage;
			pItem->idxSelImage = cbei.iSelectedImage;
			pItem->idxOverlay = cbei.iOverlay;
			pItem->iIndent = cbei.iIndent;
			pItem->lParam = cbei.lParam;
			wmemcpy(PWSTR(pItem + 1), cbei.pszText, cchText + 1);
		}
		if (HIMAGELIST hIL = GetImageList(); hIL &&
			(!pOpt || !(pOpt->uFlags & SERF_EXCLUDE_IMAGELIST)))
		{
			const auto ocb = rb.Size();
			const auto pStream = new CRefBinStream{ rb };
			pStream->Seek(ToLi(0), STREAM_SEEK_END, nullptr);
			if (SUCCEEDED(ImageList_WriteEx(hIL,ILP_NORMAL, pStream)))
				((CTRLDATA_WND*)rb.Data())->uFlags |= SERDF_IMAGELIST;
			pStream->LeaveRelease();

			pBase = (CTRLDATA_COMBOBOX*)(rb.Data() + ocbBase);
			((CTRLDATA_COMBOBOXEX*)pBase)->cbImageList = (DWORD)(rb.Size() - ocb);
		}
		else
		{
			pBase = (CTRLDATA_COMBOBOX*)(rb.Data() + ocbBase);
			((CTRLDATA_COMBOBOXEX*)pBase)->cbImageList = 0;
		}
		pBase->cbSize = DWORD(rb.Size() - ocbBase);
	}

	void PostDeserialize(PCVOID pData) override
	{
		const auto pBase = (CTRLDATA_COMBOBOXEX*)CWnd::SkipBaseData(pData);
		if (pBase->Base.iVer < CDV_COMBOBOXEX_1)
			return;
		if ((((CTRLDATA_WND*)pData)->uFlags & SERDF_IMAGELIST) && pBase->cbImageList)
		{
			const auto pStream = new CStreamView
			{ (PCBYTE)SkipBaseData(pData) - pBase->cbImageList, pBase->cbImageList };
			IImageList* pIL;
			if (SUCCEEDED(ImageList_ReadEx(ILP_NORMAL, pStream, IID_PPV_ARGS(&pIL))))
				SetImageList((HIMAGELIST)pIL);
			pStream->LeaveRelease();
		}

		auto pItem = (CTRLDATA_COMBOBOXEX::ITEM*)PtrStepCb(pBase, sizeof(CTRLDATA_COMBOBOXEX) +
			(pBase->Base.cchCueBanner + 1) * sizeof(WCHAR));
		COMBOBOXEXITEMW cbei;
		cbei.mask = CBEIF_IMAGE | CBEIF_INDENT | CBEIF_LPARAM | CBEIF_OVERLAY |
			CBEIF_SELECTEDIMAGE | CBEIF_TEXT;
		cbei.iItem = -1;
		EckCounter(-pBase->Base.cItem, i)
		{
			cbei.pszText = PWSTR(pItem + 1);
			cbei.iImage = pItem->idxImage;
			cbei.iSelectedImage = pItem->idxSelImage;
			cbei.iOverlay = pItem->idxOverlay;
			cbei.iIndent = pItem->iIndent;
			cbei.lParam = pItem->lParam;
			auto a = InsertItem(&cbei);
			pItem = (CTRLDATA_COMBOBOXEX::ITEM*)PtrStepCb(pItem, sizeof(CTRLDATA_COMBOBOXEX::ITEM) +
				(pItem->cchText + 1) * sizeof(WCHAR));
		}
		CComboBox::PostDeserialize(pData);
	}

	/// <summary>
	/// 删除项目
	/// </summary>
	/// <param name="idx">索引</param>
	/// <returns>成功返回剩余项数，失败返回CB_ERR</returns>
	EckInline int DeleteItem(int idx) const
	{
		return (int)SendMsg(CBEM_DELETEITEM, idx, 0);
	}

	EckInline HWND GetComboBoxControl() const
	{
		return (HWND)SendMsg(CBEM_GETCOMBOCONTROL, 0, 0);
	}

	EckInline HWND GetEditControl() const
	{
		return (HWND)SendMsg(CBEM_GETEDITCONTROL, 0, 0);
	}

	EckInline DWORD GetExtendedStyle() const
	{
		return (DWORD)SendMsg(CBEM_GETEXTENDEDSTYLE, 0, 0);
	}

	EckInline HIMAGELIST GetImageList() const
	{
		return (HIMAGELIST)SendMsg(CBEM_GETIMAGELIST, 0, 0);
	}

	EckInline BOOL GetItem(COMBOBOXEXITEMW* pcbei) const
	{
		return (BOOL)SendMsg(CBEM_GETITEMW, 0, (LPARAM)pcbei);
	}

	EckInline BOOL GetItemText(int idx, CRefStrW& rs) const
	{
		rs.ReSize(MAX_PATH);
		COMBOBOXEXITEMW cbei;
		cbei.iItem = idx;
		cbei.mask = CBEIF_TEXT;
		cbei.pszText = rs.Data();
		cbei.cchTextMax = rs.Size();
		const auto bRet = GetItem(&cbei);
		if (bRet)
		{
			if (cbei.pszText == rs.Data())
				rs.ReCalcLen();
			else
				rs = cbei.pszText;
		}
		return bRet;
	}

	EckInline CRefStrW GetItemText(int idx) const
	{
		CRefStrW rs;
		GetItemText(idx, rs);
		return rs;
	}

	EckInline BOOL HasEditChanged() const
	{
		return (BOOL)SendMsg(CBEM_HASEDITCHANGED, 0, 0);
	}

	EckInline int InsertItem(COMBOBOXEXITEMW* pcbei) const
	{
		return (int)SendMsg(CBEM_INSERTITEMW, 0, (LPARAM)pcbei);
	}

	int InsertItem(PCWSTR pszText, int idx = -1, int iImage = -1,
		int idxSelImage = -1, LPARAM lParam = 0) const
	{
		COMBOBOXEXITEMW cbei;
		cbei.mask = CBEIF_IMAGE | CBEIF_LPARAM | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
		cbei.iItem = idx;
		cbei.pszText = (PWSTR)pszText;
		cbei.iImage = iImage;
		cbei.iSelectedImage = idxSelImage < 0 ? iImage : idxSelImage;
		cbei.lParam = lParam;
		return InsertItem(&cbei);
	}

	/// <summary>
	/// 置扩展样式
	/// </summary>
	/// <param name="dwStyle">样式</param>
	/// <param name="dwMask">掩码，若为0则修改所有样式</param>
	/// <returns>返回先前样式</returns>
	EckInline DWORD SetExtendedStyle(DWORD dwStyle, DWORD dwMask = 0u) const
	{
		return (DWORD)SendMsg(CBEM_SETEXTENDEDSTYLE, dwMask, dwStyle);
	}

	EckInline HIMAGELIST SetImageList(HIMAGELIST hImageList) const
	{
		return (HIMAGELIST)SendMsg(CBEM_SETIMAGELIST, 0, (LPARAM)hImageList);
	}

	EckInline IImageList* SetImageList(IImageList* pImageList) const
	{
		return (IImageList*)SetImageList((HIMAGELIST)pImageList);
	}

	EckInline BOOL SetItem(COMBOBOXEXITEMW* pcbei) const
	{
		return (BOOL)SendMsg(CBEM_SETITEMW, 0, (LPARAM)pcbei);
	}

	EckInline BOOL SetItemText(int idx, PCWSTR pszText) const
	{
		COMBOBOXEXITEMW cbei;
		cbei.iItem = idx;
		cbei.mask = CBEIF_TEXT;
		cbei.pszText = (PWSTR)pszText;
		return SetItem(&cbei);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CComboBoxEx, CComboBox);
ECK_NAMESPACE_END