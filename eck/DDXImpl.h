/*
* WinEzCtrlKit Library
*
* DDXImpl.h : 对话框数据交换默认实现
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "DDX.h"

#include "CButton.h"

ECK_NAMESPACE_BEGIN
class CDdxCheckButton :public CDdx
{
protected:
	CCheckButton* m_pCtrl{};

	void OnDataChanged() override
	{
		if (m_Data.eType == DDXType::Bool)
			SetInt(GetBool());
		else if (m_Data.eType == DDXType::Int)
			m_pCtrl->SetCheckState(GetInt());
		else
			EckAssert(FALSE && L"Unsupported DDXType");
	}

	void SetupEventHandler()
	{
		const auto pParent = CWndFromHWND(GetParent(m_pCtrl->HWnd));
		EckAssert(pParent && L"Parent window not found");
		pParent->GetSignal().Connect([this](HWND, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)->LRESULT
			{
				if (uMsg == WM_COMMAND && LOWORD(wParam) == BN_CLICKED && m_pCtrl->HWnd == (HWND)lParam)
				{
					const auto i = m_pCtrl->GetCheckState();
					if (GetInt() != i)
						IntSet(i);
				}
				return 0;
			}, (UINT_PTR)this);
	}

	void RevokeEventHandler()
	{
		if (!m_pCtrl)
			return;
		const auto pParent = CWndFromHWND(GetParent(m_pCtrl->HWnd));
		EckAssert(pParent && L"Parent window not found");
		pParent->GetSignal().Disconnect((UINT_PTR)this);
	}
public:
	CDdxCheckButton()
	{
		m_Data.eType = DDXType::Int;
		m_Data.Data = 0;
	}

	CDdxCheckButton(CCheckButton* pCtrl) :m_pCtrl{ pCtrl }
	{
		m_Data.eType = DDXType::Int;
		m_Data.Data = 0;
		SetupEventHandler();
	}

	void Attach(CCheckButton* pCtrl)
	{
		RevokeEventHandler();
		m_pCtrl = pCtrl;
		SetupEventHandler();
	}

	void Detach()
	{
		RevokeEventHandler();
		m_pCtrl = nullptr;
	}
};

class CDdxRadioButton :public CDdx
{
protected:
	std::vector<CCheckButton*> m_vCtrl{};

	void OnDataChanged() override
	{
		int idxSel = GetInt();
		int i{};
		for (; i < idxSel; ++i)
			m_vCtrl[i]->SetCheckState(BST_UNCHECKED);
		m_vCtrl[idxSel]->SetCheckState(BST_CHECKED);
		++i;
		for (; i < m_vCtrl.size(); ++i)
			m_vCtrl[i]->SetCheckState(BST_UNCHECKED);
	}

	auto FindCtrl(HWND hWnd)
	{
		return std::find_if(m_vCtrl.begin(), m_vCtrl.end(), [hWnd](const CCheckButton* pCtrl)->bool { return pCtrl->HWnd == hWnd; });
	}

	void SetupEventHandler()
	{
		const auto pParent = CWndFromHWND(GetParent(m_vCtrl.front()->HWnd));
		EckAssert(pParent && L"Parent window not found");
		pParent->GetSignal().Connect([this](HWND, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)->LRESULT
			{
				if (uMsg == WM_COMMAND && LOWORD(wParam) == BN_CLICKED)
				{
					const auto it = FindCtrl((HWND)lParam);
					if (it != m_vCtrl.end())
					{
						const auto i = (int)std::distance(m_vCtrl.begin(), it);
						if (GetInt() != i)
							IntSet(i);
					}
				}
				return 0;
			}, (UINT_PTR)this);

		for (int i{}; const auto e : m_vCtrl)
		{
			if (e->GetCheckState())
			{
				m_Data.Data = i;
				return;
			}
			++i;
		}
		m_Data.Data = -1;
	}

	void RevokeEventHandler()
	{
		if (m_vCtrl.empty())
			return;
		const auto pParent = CWndFromHWND(GetParent(m_vCtrl.front()->HWnd));
		EckAssert(pParent && L"Parent window not found");
		pParent->GetSignal().Disconnect((UINT_PTR)this);
	}
public:
	CDdxRadioButton() = default;

	template<class... Args>
	CDdxRadioButton(CCheckButton* pCtrl, Args... args) :m_vCtrl{ pCtrl, args... } { SetupEventHandler(); }

	template<class... Args>
	void Attach(CCheckButton* pCtrl, Args... args)
	{
		RevokeEventHandler();
		m_vCtrl = { pCtrl, args... };
		SetupEventHandler();
	}

	void Detach()
	{
		RevokeEventHandler();
		m_vCtrl.clear();
	}
};
ECK_NAMESPACE_END