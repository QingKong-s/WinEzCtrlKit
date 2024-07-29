/*
* WinEzCtrlKit Library
*
* NetworkHelper.h ： 网络相关辅助
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "Utility.h"
#include "Utility2.h"

#include <winhttp.h>

ECK_NAMESPACE_BEGIN
struct WinHttpHandleDeleter
{
	EckInline void operator()(HINTERNET h)
	{
		WinHttpCloseHandle(h);
	}
};

using UniquePtrWinHttpHandle = std::unique_ptr<std::remove_pointer_t<HINTERNET>, WinHttpHandleDeleter>;

inline CRefBin RequestUrl(PCWSTR pszUrl, PCWSTR pszMethod,
	void* pData = NULL, SIZE_T cbData = 0u,
	PCWSTR pszHeader = NULL, PCWSTR pszCookies = NULL, BOOL bAutoHeader = TRUE, CRefStrW* prsResponseHeaders = NULL,
	PCWSTR pszProxy = NULL, PCWSTR pszUserAgent = NULL)
{
	URL_COMPONENTSW urlc{ sizeof(urlc) };
	urlc.dwSchemeLength = urlc.dwHostNameLength =
		urlc.dwUrlPathLength = urlc.dwExtraInfoLength = 1;
	// 分解URL
	if (!WinHttpCrackUrl(pszUrl, -1, 0, &urlc))
		return {};
	const UniquePtrWinHttpHandle hSession(WinHttpOpen(pszUserAgent,
		pszProxy ? WINHTTP_ACCESS_TYPE_NAMED_PROXY : WINHTTP_ACCESS_TYPE_NO_PROXY,
		pszProxy, WINHTTP_NO_PROXY_BYPASS, 0));
	if (!hSession)
		return {};
	const CRefStrW rsHost(urlc.lpszHostName, urlc.dwHostNameLength);
	const UniquePtrWinHttpHandle hConnect(WinHttpConnect(hSession.get(), rsHost.Data(), urlc.nPort, 0));
	if (!hConnect)
		return {};
	CRefStrW rsPath{};
	rsPath.Reserve(urlc.dwUrlPathLength + urlc.dwExtraInfoLength + 1);
	if (urlc.dwUrlPathLength)
		rsPath.PushBack(urlc.lpszUrlPath, urlc.dwUrlPathLength);
	else
		rsPath.PushBackChar(L'/');
	rsPath.PushBack(urlc.lpszExtraInfo, urlc.dwExtraInfoLength);
	const UniquePtrWinHttpHandle hRequest(WinHttpOpenRequest(hConnect.get(), pszMethod, rsPath.Data(),
		NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
		(urlc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0)));
	if (!hRequest)
		return {};
	// 忽略证书错误
	ULONG uSecurityFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
		SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
		SECURITY_FLAG_IGNORE_UNKNOWN_CA |
		SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
	WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_SECURITY_FLAGS, &uSecurityFlags, sizeof(uSecurityFlags));
	// 处理请求头
	CRefStrW rsHeader{};
	PCWSTR pszHeaderFinal;
	DWORD cchHeaderFinal;
	if (bAutoHeader)
	{
		if (pszHeader)
		{
			rsHeader.PushBack(pszHeader);
			rsHeader.PushBack(L"\r\n");
		}
		if (!pszHeader || FindStrI(pszHeader, L"User-Agent:") == StrNPos)
			rsHeader.PushBack(L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n");
		if (!pszHeader || FindStrI(pszHeader, L"Accept:") == StrNPos)
			rsHeader.PushBack(L"Accept: text/html, application/xhtml+xml, */*\r\n");
		if (!pszHeader || FindStrI(pszHeader, L"Accept-Language:") == StrNPos)
			rsHeader.PushBack(L"Accept-Language: *\r\n");
		if (!pszHeader || FindStrI(pszHeader, L"Accept-Encoding:") == StrNPos)
			rsHeader.PushBack(L"Accept-Encoding: gzip, deflate\r\n");
		if (!pszHeader || FindStrI(pszHeader, L"Cache-Control:") == StrNPos)
			rsHeader.PushBack(L"Cache-Control: no-cache\r\n");
		if (wcscmp(pszMethod, L"GET") == 0 && (!pszHeader || FindStrI(pszHeader, L"Content-Type:") == StrNPos))
			rsHeader.PushBack(L"Content-Type: application/x-www-form-urlencoded\r\n");
		const auto pos = FindStrI(pszHeader, L"Connection: keep-alive");
		if (pos != StrNPos)
			rsHeader.ReplaceSubStr(L"Connection: keep-alive\r\n", -1, NULL, 0, pos, 1);
		if (pszCookies && (!pszHeader || FindStrI(pszHeader, L"Cookie:") == StrNPos))
		{
			rsHeader.PushBack(L"Cookie: ");
			rsHeader.PushBack(pszCookies);
			rsHeader.PushBack(L"\r\n");
		}
		pszHeaderFinal = rsHeader.Data();
		cchHeaderFinal = rsHeader.Size();
	}
	else
	{
		pszHeaderFinal = pszHeader;
		cchHeaderFinal = -1;
	}
	if (pszHeaderFinal)
		WinHttpAddRequestHeaders(hRequest.get(), pszHeaderFinal, cchHeaderFinal, WINHTTP_ADDREQ_FLAG_ADD);
	if (!WinHttpSendRequest(hRequest.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		pData, (DWORD)cbData, (DWORD)cbData, 0))
		return {};
	if (!WinHttpReceiveResponse(hRequest.get(), NULL))
		return {};
	DWORD cbAvailable, cbRead;
	CRefBin rb{};
	while (WinHttpQueryDataAvailable(hRequest.get(), &cbAvailable))
	{
		if (!cbAvailable)
			break;
		if (!WinHttpReadData(hRequest.get(), rb.PushBack(cbAvailable), cbAvailable, &cbRead))
			return {};
		rb.PopBack(cbAvailable - cbRead);
	}
	if (prsResponseHeaders)
	{
		DWORD cbHeaders = 0;
		WinHttpQueryHeaders(hRequest.get(), WINHTTP_QUERY_RAW_HEADERS_CRLF,
			WINHTTP_HEADER_NAME_BY_INDEX, NULL, &cbHeaders, WINHTTP_NO_HEADER_INDEX);
		if (cbHeaders)
		{
			prsResponseHeaders->ReSize(cbHeaders);
			WinHttpQueryHeaders(hRequest.get(), WINHTTP_QUERY_RAW_HEADERS_CRLF,
				WINHTTP_HEADER_NAME_BY_INDEX, prsResponseHeaders->Data(), &cbHeaders, WINHTTP_NO_HEADER_INDEX);
		}
	}
	return rb;
}
ECK_NAMESPACE_END