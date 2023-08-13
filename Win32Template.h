#pragma once
#include <Windows.h>

//constexpr PCWSTR WCN_MAIN = L"QK.WndClass.Template";
#define WCN_MAIN	L"QK.WndClass.Template"

#ifdef ___HEADER_PRIVATE_MAIN___
LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_About(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif