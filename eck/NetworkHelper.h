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


inline std::wstring_view HeaderGetParam(PCWSTR pszHeader, PCWSTR pszName, int cchName = -1)
{
	const auto pos = FindStrI(pszHeader, pszName);
	if (pos == StrNPos)
		return {};
	const auto posEnd = FindStrI(pszHeader, L"\r\n", pos);
	if (posEnd == StrNPos)
		return {};
	if (cchName < 0)
		cchName = (int)wcslen(pszName);
	return { pszHeader + pos + cchName + 2, size_t(posEnd - pos - cchName - 2) };
}

template<class FPostConnect>
inline BOOL RequestUrl(FPostConnect&& fn, BOOL bRealRequest, PCWSTR pszUrl, PCWSTR pszMethod = L"GET",
	void* pData = nullptr, SIZE_T cbData = 0u,
	PCWSTR pszHeader = nullptr, PCWSTR pszCookies = nullptr, BOOL bAutoHeader = TRUE,
	PCWSTR pszProxy = nullptr, PCWSTR pszUserAgent = nullptr)
{
	URL_COMPONENTSW urlc{ sizeof(urlc) };
	urlc.dwSchemeLength = urlc.dwHostNameLength =
		urlc.dwUrlPathLength = urlc.dwExtraInfoLength = 1;
	// 分解URL
	if (!WinHttpCrackUrl(pszUrl, -1, 0, &urlc))
		return FALSE;
	const UniquePtrWinHttpHandle hSession(WinHttpOpen(pszUserAgent,
		pszProxy ? WINHTTP_ACCESS_TYPE_NAMED_PROXY : WINHTTP_ACCESS_TYPE_NO_PROXY,
		pszProxy, WINHTTP_NO_PROXY_BYPASS, 0));
	if (!hSession)
		return FALSE;
	const CRefStrW rsHost(urlc.lpszHostName, urlc.dwHostNameLength);
	const UniquePtrWinHttpHandle hConnect(WinHttpConnect(hSession.get(), rsHost.Data(), urlc.nPort, 0));
	if (!hConnect)
		return FALSE;
	CRefStrW rsPath{};
	rsPath.Reserve(urlc.dwUrlPathLength + urlc.dwExtraInfoLength + 1);
	if (urlc.dwUrlPathLength)
		rsPath.PushBack(urlc.lpszUrlPath, urlc.dwUrlPathLength);
	else
		rsPath.PushBackChar(L'/');
	rsPath.PushBack(urlc.lpszExtraInfo, urlc.dwExtraInfoLength);
	const UniquePtrWinHttpHandle hRequest(WinHttpOpenRequest(hConnect.get(), pszMethod, rsPath.Data(),
		nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
		(urlc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0)));
	if (!hRequest)
		return FALSE;
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
			rsHeader.ReplaceSubStr(L"Connection: keep-alive\r\n", -1, nullptr, 0, pos, 1);
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
		cchHeaderFinal = (DWORD)wcslen(pszHeader);
	}

	if (pszHeaderFinal)
		if (!WinHttpAddRequestHeaders(hRequest.get(), pszHeaderFinal, cchHeaderFinal, WINHTTP_ADDREQ_FLAG_ADD))
		{
			EckDbgPrintFormatMessage(GetLastError());
			return FALSE;
		}
	if(bRealRequest)
	{
		if (!WinHttpSendRequest(hRequest.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			pData, (DWORD)cbData, (DWORD)cbData, 0))
			return FALSE;
		if (!WinHttpReceiveResponse(hRequest.get(), nullptr))
			return FALSE;
	}
	
	return fn(hSession.get(), hConnect.get(), hRequest.get());
}


/// <summary>
/// 访问URL
/// </summary>
/// <param name="pszUrl">URL</param>
/// <param name="pszMethod">请求方式，默认为GET</param>
/// <param name="pData">请求数据</param>
/// <param name="cbData">请求数据长度</param>
/// <param name="pszHeader">请求头</param>
/// <param name="pszCookies">Cookies</param>
/// <param name="bAutoHeader">自动补全请求头</param>
/// <param name="prsResponseHeaders">返回响应头</param>
/// <param name="pszProxy">代理</param>
/// <param name="pszUserAgent">UA</param>
/// <returns>成功返回请求到的数据，失败返回空字节集</returns>
inline CRefBin RequestUrl(PCWSTR pszUrl, PCWSTR pszMethod = L"GET",
	void* pData = nullptr, SIZE_T cbData = 0u,
	PCWSTR pszHeader = nullptr, PCWSTR pszCookies = nullptr, BOOL bAutoHeader = TRUE, 
	CRefStrW* prsResponseHeaders = nullptr,
	PCWSTR pszProxy = nullptr, PCWSTR pszUserAgent = nullptr)
{
	CRefBin rb{};
	if (!RequestUrl([&](HINTERNET hSession, HINTERNET hConnect, HINTERNET hRequest) -> BOOL
		{
			eck::CRefStrW rsResponseHeaders{};
			DWORD cbHeaders = 0;
			WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
				WINHTTP_HEADER_NAME_BY_INDEX, nullptr, &cbHeaders, WINHTTP_NO_HEADER_INDEX);
			if (cbHeaders)
			{
				rsResponseHeaders.ReSize(cbHeaders / sizeof(WCHAR) - 1);
				WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
					WINHTTP_HEADER_NAME_BY_INDEX, rsResponseHeaders.Data(), &cbHeaders, WINHTTP_NO_HEADER_INDEX);
			}
			const auto svContentLength = HeaderGetParam(rsResponseHeaders.Data(), L"Content-Length");
			if (!svContentLength.empty())
			{
				const auto cbContent = _wtoll(svContentLength.data());// 将在第一个非数字字符处停止
				if (cbContent > 1'073'741'824i64)// 大于1G，不读
				{
					if (prsResponseHeaders)
						*prsResponseHeaders = std::move(rsResponseHeaders);
					return FALSE;
				}
				rb.Reserve((size_t)cbContent);
			}
			if (prsResponseHeaders)
				*prsResponseHeaders = std::move(rsResponseHeaders);
			DWORD cbAvailable, cbRead;
			while (WinHttpQueryDataAvailable(hRequest, &cbAvailable))
			{
				if (!cbAvailable)
					break;
				if (!WinHttpReadData(hRequest, rb.PushBack(cbAvailable), cbAvailable, &cbRead))
					return FALSE;
				rb.PopBack(cbAvailable - cbRead);
			}
			return TRUE;
		}, TRUE, pszUrl, pszMethod, pData, cbData,
		pszHeader, pszCookies, bAutoHeader, pszProxy, pszUserAgent))
		return {};
	return rb;
}
ECK_NAMESPACE_END