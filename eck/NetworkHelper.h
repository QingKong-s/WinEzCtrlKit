#pragma once
#include "AutoPtrDef.h"
#include "CoroutineHelper.h"
#include "ComPtr.h"
#include "CString.h"
#include "CByteBuffer.h"
#include "NativeWrapper.h"

#include <winhttp.h>

ECK_NAMESPACE_BEGIN
ECK_DECL_HANDLE_DELETER(HWinHttp, HINTERNET, WinHttpCloseHandle);

using HWhSession = HINTERNET;
using HWhConnect = HINTERNET;
using HWhRequest = HINTERNET;

inline std::wstring_view HeaderGetParam(
    _In_z_ PCWSTR pszHeader,
    _In_z_ PCWSTR pszName,
    int cchName = -1) noexcept
{
    const auto pos = FindStrI(pszHeader, pszName);
    if (pos == StrNPos)
        return {};
    const auto posEnd = FindStrI(pszHeader, L"\r\n", pos);
    if (posEnd == StrNPos)
        return {};
    if (cchName < 0)
        cchName = (int)TcsLen(pszName);
    const auto pszValue = LTrimStr(pszHeader + pos + cchName + 2);
    return { pszValue, size_t(posEnd - (pszValue - pszHeader)) };
}

template<class FPostConnect>
inline BOOL RequestUrl(FPostConnect&& fn,
    BOOL bRealRequest, PCWSTR pszUrl, PCWSTR pszMethod = L"GET",
    void* pData = nullptr, SIZE_T cbData = 0u,
    PCWSTR pszHeader = nullptr, PCWSTR pszCookies = nullptr, BOOL bAutoHeader = TRUE,
    PCWSTR pszProxy = nullptr, PCWSTR pszUserAgent = nullptr) noexcept
{
    URL_COMPONENTSW urlc{ sizeof(urlc) };
    urlc.dwSchemeLength = urlc.dwHostNameLength =
        urlc.dwUrlPathLength = urlc.dwExtraInfoLength = 1;
    // 分解URL
    if (!WinHttpCrackUrl(pszUrl, -1, 0, &urlc))
        return FALSE;
    const UniquePtr<DelHWinHttp> hSession(WinHttpOpen(pszUserAgent,
        pszProxy ? WINHTTP_ACCESS_TYPE_NAMED_PROXY : WINHTTP_ACCESS_TYPE_NO_PROXY,
        pszProxy, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!hSession)
        return FALSE;
    const CStringW rsHost(urlc.lpszHostName, urlc.dwHostNameLength);
    const UniquePtr<DelHWinHttp>
        hConnect(WinHttpConnect(hSession.get(), rsHost.Data(), urlc.nPort, 0));
    if (!hConnect)
        return FALSE;
    CStringW rsPath{};
    rsPath.Reserve(urlc.dwUrlPathLength + urlc.dwExtraInfoLength + 1);
    if (urlc.dwUrlPathLength)
        rsPath.PushBack(urlc.lpszUrlPath, urlc.dwUrlPathLength);
    else
        rsPath.PushBackChar(L'/');
    rsPath.PushBack(urlc.lpszExtraInfo, urlc.dwExtraInfoLength);
    const UniquePtr<DelHWinHttp>
        hRequest(WinHttpOpenRequest(hConnect.get(), pszMethod, rsPath.Data(),
            nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
            (urlc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0)));
    if (!hRequest)
        return FALSE;
    // 忽略证书错误
    ULONG uSecurityFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
        SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
        SECURITY_FLAG_IGNORE_UNKNOWN_CA |
        SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
    WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_SECURITY_FLAGS,
        &uSecurityFlags, sizeof(uSecurityFlags));
    // 处理请求头
    CStringW rsHeader{};
    PCWSTR pszHeaderFinal;
    DWORD cchHeaderFinal;
    if (bAutoHeader)
    {
        if (pszHeader)
        {
            rsHeader.PushBack(pszHeader);
            const auto pos = FindStrI(pszHeader, L"Connection: keep-alive");
            if (pos != StrNPos)
                rsHeader.ReplaceSubString(L"Connection: keep-alive\r\n", -1, nullptr, 0, pos, 1);
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
        cchHeaderFinal = (DWORD)TcsLen(pszHeader);
    }

    if (pszHeaderFinal)
        if (!WinHttpAddRequestHeaders(hRequest.get(), pszHeaderFinal, cchHeaderFinal, WINHTTP_ADDREQ_FLAG_ADD))
        {
            EckDbgPrintFormatMessage(GetLastError());
            return FALSE;
        }
    if (bRealRequest)
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
inline CByteBuffer RequestUrl(PCWSTR pszUrl, PCWSTR pszMethod = L"GET",
    void* pData = nullptr, SIZE_T cbData = 0u,
    PCWSTR pszHeader = nullptr, PCWSTR pszCookies = nullptr, BOOL bAutoHeader = TRUE,
    CStringW* prsResponseHeaders = nullptr,
    PCWSTR pszProxy = nullptr, PCWSTR pszUserAgent = nullptr) noexcept
{
    CByteBuffer rb{};
    if (!RequestUrl([&](HINTERNET hSession, HINTERNET hConnect, HINTERNET hRequest) -> BOOL
        {
            CStringW rsResponseHeaders{};
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

struct CHttpRequest
{
    BITBOOL AutoAddHeader : 1{ TRUE };
    BITBOOL AutoDecompress : 1{ TRUE };
    int ResponseCode{};
    PCWSTR Header{};
    void* Data{};
    SIZE_T DataSize{};
    PCWSTR Cookies{};
    PCWSTR UserAgent{};
    PCWSTR Proxy{};

    CStringW ResponseHeader{};
    CByteBuffer Response{};

    void DoRequest(PCWSTR pszUrl, PCWSTR pszMethod = L"GET") noexcept
    {
        RequestUrl([&](HINTERNET hSession, HINTERNET hConnect, HINTERNET hRequest) -> BOOL
            {
                if (AutoDecompress)
                {
                    DWORD dwFlags{ WINHTTP_DECOMPRESSION_FLAG_ALL };
                    if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_DECOMPRESSION,
                        &dwFlags, sizeof(dwFlags)))
                        return FALSE;
                }
                if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                    Data, (DWORD)DataSize, (DWORD)DataSize, 0))
                    return FALSE;
                if (!WinHttpReceiveResponse(hRequest, nullptr))
                    return FALSE;

                DWORD cbHeaders = 0;
                WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                    WINHTTP_HEADER_NAME_BY_INDEX, nullptr, &cbHeaders, WINHTTP_NO_HEADER_INDEX);
                if (cbHeaders)
                {
                    ResponseHeader.ReSize(cbHeaders / sizeof(WCHAR) - 1);
                    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                        WINHTTP_HEADER_NAME_BY_INDEX, ResponseHeader.Data(), &cbHeaders, WINHTTP_NO_HEADER_INDEX);
                }
                const auto svContentLength = HeaderGetParam(ResponseHeader.Data(), L"Content-Length");
                if (!svContentLength.empty())
                {
                    const auto cbContent = _wtoll(svContentLength.data());// 将在第一个非数字字符处停止
                    if (cbContent > 1'073'741'824i64)// 大于1G，不读
                        return FALSE;
                    Response.Reserve((size_t)cbContent);
                }
                DWORD cbAvailable, cbRead;
                while (WinHttpQueryDataAvailable(hRequest, &cbAvailable))
                {
                    if (!cbAvailable)
                        break;
                    if (!WinHttpReadData(hRequest,
                        Response.PushBack(cbAvailable), cbAvailable, &cbRead))
                        return FALSE;
                    Response.PopBack(cbAvailable - cbRead);
                }
                return TRUE;
            }, FALSE, pszUrl, pszMethod, Data, DataSize,
            Header, Cookies, AutoAddHeader, Proxy, UserAgent);
    }

    void DoRequest(const CStringW& rsUrl, PCWSTR pszMethod = L"GET") noexcept
    {
        DoRequest(rsUrl.Data(), pszMethod);
    }

    constexpr void Reset() noexcept
    {
        AutoAddHeader = TRUE;
        AutoDecompress = TRUE;
        Header = nullptr;
        Data = nullptr;
        DataSize = 0;
        Cookies = nullptr;
        UserAgent = nullptr;
        Proxy = nullptr;
        ResponseHeader.Clear();
        Response.Clear();
    }
};



EckInline HWhSession WhOpenSession(BOOL bAsync = TRUE,
    _In_opt_z_ PCWSTR pszUserAgent = nullptr, _In_opt_z_ PCWSTR pszProxy = nullptr) noexcept
{
    return WinHttpOpen(pszUserAgent,
        pszProxy ? WINHTTP_ACCESS_TYPE_NAMED_PROXY : WINHTTP_ACCESS_TYPE_NO_PROXY,
        pszProxy, WINHTTP_NO_PROXY_BYPASS, bAsync ? WINHTTP_FLAG_ASYNC : 0);
}

inline HWhConnect WhPrepareConnect(_Out_ URL_COMPONENTSW& urlc, _Inout_ CStringW& rsWork,
    _In_ HWhSession hSession, _In_reads_(cchUrl) PCWSTR pszUrl, int cchUrl) noexcept
{
    // 分解URL
    urlc = { sizeof(urlc) };
    urlc.dwSchemeLength = urlc.dwHostNameLength =
        urlc.dwUrlPathLength = urlc.dwExtraInfoLength = -1;
    if (!WinHttpCrackUrl(pszUrl, (DWORD)cchUrl, 0, &urlc))
        return nullptr;
    // 制主机名
    rsWork.Reserve(std::max(urlc.dwUrlPathLength + urlc.dwExtraInfoLength + 1, urlc.dwHostNameLength));
    rsWork.Assign(urlc.lpszHostName, urlc.dwHostNameLength);
    // 连接
    const auto hConnect{ WinHttpConnect(hSession, rsWork.Data(), urlc.nPort, 0) };
    if (!hConnect)
        return nullptr;
    // 制对象名
    if (urlc.dwUrlPathLength)
        rsWork.Assign(urlc.lpszUrlPath, urlc.dwUrlPathLength);
    else
        rsWork.Assign(L"/", 1);
    rsWork.PushBack(urlc.lpszExtraInfo, urlc.dwExtraInfoLength);
    return hConnect;
}

EckInline HWhRequest WhOpenRequest(const URL_COMPONENTSW& urlc, HWhConnect hConnect,
    _In_z_ PCWSTR pszMethod, _In_opt_z_ PCWSTR pszObjectName) noexcept
{
    return WinHttpOpenRequest(hConnect, pszMethod, pszObjectName,
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        (urlc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0));
}

inline void WhCompleteHeader(_In_opt_z_ PCWSTR pszHeader, _Inout_ CStringW& rsHeader,
    _In_z_ PCWSTR pszMethod, _In_opt_z_ PCWSTR pszCookies, _In_opt_z_ PCWSTR pszUserAgent) noexcept
{
    if (pszHeader)
        rsHeader.PushBack(pszHeader);
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
    rsHeader.ReplaceSubString(L"Connection: keep-alive\r\n", -1, nullptr, 0, 0, 1);
    if (pszCookies && (!pszHeader || FindStrI(pszHeader, L"Cookie:") == StrNPos))
    {
        rsHeader.PushBack(L"Cookie: ");
        rsHeader.PushBack(pszCookies);
        rsHeader.PushBack(L"\r\n");
    }
}

struct CHttpRequestAsync
{
    using TProgress = std::pair<SIZE_T, SIZE_T>;
    using Task = eck::CoroTask<HRESULT, TProgress>;

    PCWSTR Header{};
    void* Data{};
    SIZE_T DataSize{};
    PCWSTR Cookies{};
    BITBOOL AutoAddHeader : 1{ TRUE };
    BITBOOL AutoDecompress : 1{ TRUE };

    int ResponseCode{};
    CStringW ResponseHeader{};
    std::variant<CByteBuffer, CStringW, ComPtr<IStream>> Response{};

    // 异步发送请求
    // 进度结果：(已接收字节数，Content-Length)，其中Content-Length可能为0
    Task DoRequest(PCWSTR pszMethod, PCWSTR pszUrl,
        int cchUrl = -1, PCWSTR pszUserAgent = nullptr, PCWSTR pszProxy = nullptr) noexcept
    {
        co_await eck::CoroResumeBackground();
        auto Token{ co_await eck::CoroGetPromiseToken() };

        static UniquePtr<DelHWinHttp> s_hSession{ WhOpenSession() };

        URL_COMPONENTSW urlc;
        CStringW rsWork;
        UniquePtr<DelHWinHttp>
            hConnect{ WhPrepareConnect(urlc, rsWork, s_hSession.get(), pszUrl, cchUrl) };
        if (!hConnect)
            co_return HRESULT_FROM_WIN32(NaGetLastError());

        struct CTX
        {
            // 下列字段仅供回调访问

            CHttpRequestAsync* pThis;	// 指向CHttpRequestAsync实例
            decltype(Token)& Token;		// 协程控制
            HWhRequest hRequest;		// 请求句柄
            union
            {							// 当接收为流时，此联合无效
                DWORD cbData;			// 当接收为字节集时，存储WinHttpQueryDataAvailable的结果
                HANDLE hFile;			// 当接收为文件时，存储文件句柄
            };
            UniquePtr<DelVA<void>> pBuf;// 8K缓冲区
            SIZE_T cbTotal;				// Content-Length
            SIZE_T cbRead;				// 已读取的字节数，用于报告进度
            BITBOOL bRequestClosed : 1;	// 请求句柄是否已关闭

            // 下列字段由回调和调用方共享

            HRESULT hr;					// 因失败而取消
            CEvent EvtSafeExit{ {},FALSE,FALSE };	// 安全退出事件

            void Cancel(HRESULT hr)
            {
                if (hRequest)
                {
                    this->hr = hr;
                    WinHttpCloseHandle(hRequest);
                }
            }
        }
        Ctx{ this, Token };

        constexpr SIZE_T BufSize{ 8192 };// 8K为建议大小

        const auto Ret = WinHttpSetStatusCallback(hConnect.get(),
            [](HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus,
                LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
            {
                if (!dwContext)
                    return;
                const auto pCtx{ (CTX*)dwContext };
                switch (dwInternetStatus)
                {
                case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
                    // Expect WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE
                    if (!WinHttpReceiveResponse(hInternet, nullptr))
                        pCtx->Cancel(HRESULT_FROM_WIN32(NaGetLastError()));
                    break;

                case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
                {
                    DWORD cbHeaders{};
                    WinHttpQueryHeaders(pCtx->hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                        WINHTTP_HEADER_NAME_BY_INDEX, nullptr, &cbHeaders, WINHTTP_NO_HEADER_INDEX);
                    if (cbHeaders)
                    {
                        auto& rs = pCtx->pThis->ResponseHeader;
                        rs.ReSize(cbHeaders / sizeof(WCHAR) - 1);
                        WinHttpQueryHeaders(pCtx->hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                            WINHTTP_HEADER_NAME_BY_INDEX,
                            rs.Data(), &cbHeaders, WINHTTP_NO_HEADER_INDEX);
                        const auto svContentLength =
                            HeaderGetParam(rs.Data(), EckStrAndLen(L"Content-Length"));
                        if (!svContentLength.empty())
                        {
                            const auto cbContent = _wcstoui64(
                                svContentLength.data(), nullptr, 10);
                            pCtx->cbTotal = (SIZE_T)cbContent;
                            if (pCtx->pThis->Response.index() == 0)
                                std::get<CByteBuffer>(pCtx->pThis->Response)
                                .Reserve((size_t)cbContent);
                        }
                    }

                    switch (pCtx->pThis->Response.index())
                    {
                    case 0:
                    {
                        // Expect WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE
                        if (!WinHttpQueryDataAvailable(pCtx->hRequest, nullptr))
                            pCtx->Cancel(HRESULT_FROM_WIN32(NaGetLastError()));
                    }
                    break;
                    case 1:
                        pCtx->hFile = CreateFileW(std::get<CStringW>(pCtx->pThis->Response).Data(),
                            GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
                        if (pCtx->hFile == INVALID_HANDLE_VALUE)
                        {
                            pCtx->Cancel(HRESULT_FROM_WIN32(NaGetLastError()));
                            break;
                        }
                        [[fallthrough]];
                    case 2:
                    {
                        pCtx->pBuf.reset(VAlloc(BufSize));
                        if (!pCtx->pBuf)
                        {
                            pCtx->Cancel(E_OUTOFMEMORY);
                            break;
                        }
                        // Expect WINHTTP_CALLBACK_STATUS_READ_COMPLETE
                        if (!WinHttpReadData(pCtx->hRequest, pCtx->pBuf.get(), BufSize, nullptr))
                            pCtx->Cancel(HRESULT_FROM_WIN32(NaGetLastError()));
                    }
                    break;
                    }
                }
                break;

                case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
                {
                    const auto cbData = *(DWORD*)lpvStatusInformation;
                    pCtx->cbData = cbData;
                    if (cbData)
                    {
                        // Expect WINHTTP_CALLBACK_STATUS_READ_COMPLETE
                        if (!WinHttpReadData(
                            pCtx->hRequest, std::get<CByteBuffer>(pCtx->pThis->Response).PushBack(cbData),
                            cbData, nullptr))
                            pCtx->Cancel(FALSE);
                    }
                    else// 读取完毕
                        pCtx->Cancel(FALSE);
                }
                break;

                case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
                {
                    pCtx->cbRead += dwStatusInformationLength;
                    pCtx->Token.GetPromise().OnProgress(
                        std::make_pair(pCtx->cbRead, pCtx->cbTotal));
                    switch (pCtx->pThis->Response.index())
                    {
                    case 0:
                    {
                        std::get<CByteBuffer>(pCtx->pThis->Response).
                            PopBack(pCtx->cbData - dwStatusInformationLength);
                        // Expect WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE
                        if (!WinHttpQueryDataAvailable(pCtx->hRequest, nullptr))
                            pCtx->Cancel(HRESULT_FROM_WIN32(NaGetLastError()));
                    }
                    break;
                    case 1:
                    {
                        if (!dwStatusInformationLength)
                        {
                            pCtx->Cancel(S_OK);
                            break;
                        }
                        DWORD Dummy{};
                        WriteFile(pCtx->hFile, pCtx->pBuf.get(),
                            dwStatusInformationLength, &Dummy, nullptr);
                        // Expect WINHTTP_CALLBACK_STATUS_READ_COMPLETE
                        if (!WinHttpReadData(pCtx->hRequest, pCtx->pBuf.get(), BufSize, nullptr))
                            pCtx->Cancel(S_OK);
                    }
                    break;
                    case 2:
                    {
                        if (!dwStatusInformationLength)
                        {
                            pCtx->Cancel(S_OK);
                            break;
                        }
                        std::get<ComPtr<IStream>>(pCtx->pThis->Response)->Write(
                            pCtx->pBuf.get(), dwStatusInformationLength, nullptr);
                        // Expect WINHTTP_CALLBACK_STATUS_READ_COMPLETE
                        if (!WinHttpReadData(pCtx->hRequest, pCtx->pBuf.get(), BufSize, nullptr))
                            pCtx->Cancel(S_OK);
                    }
                    break;
                    }
                }
                break;

                case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
                {
                    const auto hClosed{ *(HINTERNET*)lpvStatusInformation };
                    if (hClosed == pCtx->hRequest)
                    {
                        pCtx->bRequestClosed = TRUE;
                        pCtx->hRequest = nullptr;
                        pCtx->EvtSafeExit.Signal();
                    }
                }
                break;

                case WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION:
                case WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED:
                case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
                case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
                    pCtx->Cancel(E_FAIL);
                    break;
                }
            }, WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_FLAG_HANDLES, 0);
        if (Ret == WINHTTP_INVALID_STATUS_CALLBACK)
            co_return HRESULT_FROM_WIN32(NaGetLastError());

        UniquePtr<DelHWinHttp>
            hRequest{ WhOpenRequest(urlc, hConnect.get(), pszMethod, rsWork.Data()) };
        if (!hRequest)
            co_return HRESULT_FROM_WIN32(NaGetLastError());

        if (AutoAddHeader)
        {
            rsWork.Clear();
            WhCompleteHeader(Header, rsWork, pszMethod, Cookies, pszUserAgent);
            if (!WinHttpAddRequestHeaders(hRequest.get(),
                rsWork.Data(), (DWORD)rsWork.Size(), WINHTTP_ADDREQ_FLAG_ADD))
                co_return HRESULT_FROM_WIN32(NaGetLastError());
        }
        else if (Header)
        {
            if (!WinHttpAddRequestHeaders(hRequest.get(),
                Header, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD))
                co_return HRESULT_FROM_WIN32(NaGetLastError());
        }

        if (AutoDecompress)
        {
            DWORD dwFlags{ WINHTTP_DECOMPRESSION_FLAG_ALL };
            if (!WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_DECOMPRESSION,
                &dwFlags, sizeof(dwFlags)))
                co_return HRESULT_FROM_WIN32(NaGetLastError());
        }
        WinHttpSetTimeouts(hRequest.get(), 5000, 5000, 5000, 5000);

        Ctx.hRequest = hRequest.release();

        Token.GetPromise().SetCanceller([](void* pCtx)
            {
                ((CTX*)pCtx)->Cancel(E_ABORT);
            }, &Ctx);

        // Expect WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE
        if (!WinHttpSendRequest(Ctx.hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            Data, (DWORD)DataSize, (DWORD)DataSize, (DWORD_PTR)&Ctx))
        {
            Ctx.Cancel(HRESULT_FROM_WIN32(NaGetLastError()));
            Ctx.EvtSafeExit.Signal();
        }

        WaitObject(Ctx.EvtSafeExit);
        WinHttpSetStatusCallback(hConnect.get(), nullptr,
            WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
        if (Response.index() == 1)
            NtClose(Ctx.hFile);
        co_return Ctx.hr;
    }

    EckInlineNdCe auto& GetBin() const noexcept { return std::get<CByteBuffer>(Response); }
    EckInlineNdCe auto& GetBin() noexcept { return std::get<CByteBuffer>(Response); }
    EckInlineNdCe auto& GetFilePath() const noexcept { return std::get<CStringW>(Response); }
    EckInlineNdCe auto& GetFilePath() noexcept { return std::get<CStringW>(Response); }
    EckInlineNdCe auto& GetStream() const noexcept { return std::get<ComPtr<IStream>>(Response); }
    EckInlineNdCe auto& GetStream() noexcept { return std::get<ComPtr<IStream>>(Response); }

    constexpr void WantBin() noexcept
    {
        if (Response.index() == 0)
            std::get<CByteBuffer>(Response).Clear();
        else
            Response.emplace<CByteBuffer>();
    }

    template<class T>
    constexpr void WantFilePath(T&& rsFilePath) noexcept
    {
        Response.emplace<CStringW>(std::forward<T>(rsFilePath));
    }

    constexpr void WantStream(IStream* pStream) noexcept
    {
        Response.emplace<ComPtr<IStream>>(pStream);
    }

    constexpr void ClearResponse() noexcept
    {
        ResponseHeader.Clear();
        switch (Response.index())
        {
        case 0: std::get<CByteBuffer>(Response).Clear(); break;
        case 1: std::get<CStringW>(Response).Clear(); break;
        case 2: std::get<ComPtr<IStream>>(Response).Clear(); break;
        }
    }

    constexpr void Reset() noexcept
    {
        AutoAddHeader = TRUE;
        AutoDecompress = TRUE;
        Header = nullptr;
        Data = nullptr;
        DataSize = 0;
        Cookies = nullptr;
        ResponseHeader.Clear();
        WantBin();
    }
};
ECK_NAMESPACE_END