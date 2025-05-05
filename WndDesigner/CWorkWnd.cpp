#include "pch.h"

//LRESULT CWorkWndBk::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch (uMsg)
//	{
//	case WM_PAINT:
//	{
//		PAINTSTRUCT ps;
//		BeginPaint(hWnd, &ps);
//		FillRect(ps.hdc, &ps.rcPaint, GetSysColorBrush(COLOR_APPWORKSPACE));
//		EndPaint(hWnd, &ps);
//	}
//	return 0;
//	}
//	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
//}
//
//LRESULT CWorkWnd::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch (uMsg)
//	{
//	case WM_PAINT:
//	{
//		PAINTSTRUCT ps;
//		BeginPaint(hWnd, &ps);
//		FillRect(ps.hdc, &ps.rcPaint, m_pCtx->pWorkWnd->m_hbrWorkWindow);
//		EndPaint(hWnd, &ps);
//	}
//	return 0;
//
//	case WM_MOVE:
//		for (auto x : m_pCtx->pMultiSelMarker)
//			x->MoveToTargetWindow();
//		return 0;
//
//	case WM_SETCURSOR:
//	{
//		if (m_pCtx->pThis->m_bPlacingCtrl)
//		{
//			SetCursor(LoadCursorW(nullptr, IDC_CROSS));
//			return 0;
//		}
//	}
//	break;
//
//	case WM_LBUTTONDOWN:
//		m_pCtx->pThis->m_CtrlPlacing.OnLButtonDown(m_pCtx, hWnd, lParam);
//		return 0;
//
//	case WM_LBUTTONUP:
//		m_pCtx->pThis->OnWWLButtonUp(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam, m_pCtx);
//		return 0;
//
//	case WM_MOUSEMOVE:
//		m_pCtx->pThis->m_CtrlPlacing.OnMouseMove(m_pCtx, hWnd, lParam);
//		return 0;
//
//	case WM_CLOSE:
//		return 0;
//
//	case WM_RBUTTONDOWN:
//	{
//		SetCapture(hWnd);
//		m_pCtx->pWorkWnd->m_bRBtnDown = TRUE;
//	}
//	return 0;
//
//	case WM_RBUTTONUP:
//		m_pCtx->pThis->OnWWRButtonUp(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), m_pCtx);
//		return 0;
//	}
//	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
//}
