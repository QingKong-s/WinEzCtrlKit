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
    std::wstring_view svHeader,
    std::wstring_view svName) noexcept
{
    const auto pos = FindStringLengthI(
        svHeader.data(), (int)svHeader.size(),
        svName.data(), (int)svName.size());
    if (pos < 0)
        return {};
    const auto posEnd = FindStringLength(
        svHeader.data(), (int)svHeader.size(),
        EckStrAndLen(L"\r\n"));
    if (posEnd < 0)
        return {};
    const auto pszValue = TrimStringLeft(svHeader.data() + pos + svName.size() + 1/*冒号*/);
    return { pszValue, size_t(posEnd - (pszValue - svHeader.data())) };
}

inline BOOL HeaderComplete(
    CStringW& rsHeader,
    std::wstring_view svHeader,
    std::wstring_view svMethod,
    std::wstring_view svCookies,
    std::wstring_view svUserAgent) noexcept
{
    rsHeader.PushBack(svHeader);

#undef ECK_TEMP_HIT
#define ECK_TEMP_HIT(s) \
    (FindStringLengthI(svHeader.data(), (int)svHeader.size(), EckStrAndLen(s)) < 0)

    if (ECK_TEMP_HIT(L"User-Agent:"))
        rsHeader.PushBack(L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n");
    if (ECK_TEMP_HIT(L"Accept:"))
        rsHeader.PushBack(L"Accept: text/html, application/xhtml+xml, */*\r\n");
    if (ECK_TEMP_HIT(L"Accept-Language:"))
        rsHeader.PushBack(L"Accept-Language: *\r\n");
    if (ECK_TEMP_HIT(L"Accept-Encoding:"))
        rsHeader.PushBack(L"Accept-Encoding: gzip, deflate\r\n");
    if (ECK_TEMP_HIT(L"Cache-Control:"))
        rsHeader.PushBack(L"Cache-Control: no-cache\r\n");
    if (svMethod == L"GET"sv &&
        ECK_TEMP_HIT(L"Content-Type:"))
        rsHeader.PushBack(L"Content-Type: application/x-www-form-urlencoded\r\n");
    rsHeader.ReplaceSubString(EckStrAndLen(L"Connection: keep-alive\r\n"), nullptr, 0, 0, 1);
    if (!svCookies.empty())
    {
        auto pos0 = rsHeader.FindI(L"Cookie:"sv);
        if (pos0 >= 0)
        {
            pos0 += 7;
            const auto ch = rsHeader[pos0];
            if (ch == L'\0')
                return FALSE;
            if (ch != L'\r')
                rsHeader[pos0++] = L' ';
            const auto pos1 = rsHeader.FindI(L"\r\n"sv, pos0);
            if (pos1 < 0)
                return FALSE;
            rsHeader.Replace(pos0, pos1 - pos0, svCookies);
        }
        else
        {
            rsHeader.PushBack(L"Cookie: ");
            rsHeader.PushBack(svCookies);
            rsHeader.PushBack(L"\r\n");
        }
    }
    return TRUE;
#undef ECK_TEMP_HIT
}

EckInline HWhSession WhOpenSession(
    BOOL bAsync = TRUE,
    _In_opt_z_ PCWSTR pszUserAgent = nullptr,
    _In_opt_z_ PCWSTR pszProxy = nullptr) noexcept
{
    return WinHttpOpen(pszUserAgent,
        pszProxy ? WINHTTP_ACCESS_TYPE_NAMED_PROXY : WINHTTP_ACCESS_TYPE_NO_PROXY,
        pszProxy, WINHTTP_NO_PROXY_BYPASS, bAsync ? WINHTTP_FLAG_ASYNC : 0);
}

inline HWhConnect WhPrepareConnect(
    _Out_ URL_COMPONENTSW& urlc,
    _Inout_ CStringW& rsWork,
    _In_ HWhSession hSession,
    _In_reads_(cchUrl) PCWSTR pszUrl,
    int cchUrl) noexcept
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

EckInline HWhRequest WhOpenRequest(
    const URL_COMPONENTSW& urlc,
    HWhConnect hConnect,
    _In_z_ PCWSTR pszMethod,
    _In_opt_z_ PCWSTR pszObjectName) noexcept
{
    return WinHttpOpenRequest(hConnect, pszMethod, pszObjectName,
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        (urlc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0));
}

struct CHttpRequestAsync
{
    using TProgress = std::pair<UINT, UINT>;
    using Task = CoroTask<HRESULT, TProgress>;

    std::wstring_view Header{};
    std::span<const BYTE> Data{};
    std::wstring_view Cookies{};
    BITBOOL AutoAddHeader : 1{ TRUE };
    BITBOOL AutoDecompress : 1{ TRUE };

    int ResponseCode{};
    CStringW ResponseHeader{};
    std::variant<CByteBuffer, CStringW, ComPtr<IStream>> Response{};

    // 异步发送请求
    // 进度结果：(已接收字节数，Content-Length)，其中Content-Length可能为0
    Task DoRequest(
        PCWSTR pszMethod,
        PCWSTR pszUrl,
        int cchUrl = -1,
        PCWSTR pszUserAgent = nullptr,
        PCWSTR pszProxy = nullptr) noexcept
    {
        co_await CoroResumeBackground();
        auto Token{ co_await CoroGetPromiseToken() };

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
            UINT cbTotal;				// Content-Length
            UINT cbRead;				// 已读取的字节数，用于报告进度
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
                            HeaderGetParam(rs.ToStringView(), L"Content-Length"sv);
                        if (!svContentLength.empty())
                        {
                            const auto cbContent = (UINT)_wcstoui64(
                                svContentLength.data(), nullptr, 10);
                            pCtx->cbTotal = cbContent;
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
            HeaderComplete(rsWork, Header, pszMethod, Cookies, pszUserAgent);
            if (!WinHttpAddRequestHeaders(hRequest.get(),
                rsWork.Data(), (DWORD)rsWork.Size(), WINHTTP_ADDREQ_FLAG_ADD))
                co_return HRESULT_FROM_WIN32(NaGetLastError());
        }
        else if (!Header.empty())
        {
            if (!WinHttpAddRequestHeaders(hRequest.get(),
                Header.data(), (DWORD)Header.size(), WINHTTP_ADDREQ_FLAG_ADD))
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
            (void*)Data.data(), (DWORD)Data.size(), (DWORD)Data.size(), (DWORD_PTR)&Ctx))
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

    EckInlineNdCe auto& GetByteBuffer() const noexcept { return std::get<CByteBuffer>(Response); }
    EckInlineNdCe auto& GetByteBuffer() noexcept { return std::get<CByteBuffer>(Response); }
    EckInlineNdCe auto& GetFilePath() const noexcept { return std::get<CStringW>(Response); }
    EckInlineNdCe auto& GetFilePath() noexcept { return std::get<CStringW>(Response); }
    EckInlineNdCe auto& GetStream() const noexcept { return std::get<ComPtr<IStream>>(Response); }
    EckInlineNdCe auto& GetStream() noexcept { return std::get<ComPtr<IStream>>(Response); }

    void WantByteBuffer() noexcept
    {
        if (Response.index() == 0)
            std::get<CByteBuffer>(Response).Clear();
        else
            Response.emplace<CByteBuffer>();
    }

    template<class T>
    void WantFilePath(T&& rsFilePath) noexcept
    {
        Response.emplace<CStringW>(std::forward<T>(rsFilePath));
    }

    void WantStream(IStream* pStream) noexcept
    {
        Response.emplace<ComPtr<IStream>>(pStream);
    }

    void ClearResponse() noexcept
    {
        ResponseHeader.Clear();
        switch (Response.index())
        {
        case 0: std::get<CByteBuffer>(Response).Clear();     break;
        case 1: std::get<CStringW>(Response).Clear();        break;
        case 2: std::get<ComPtr<IStream>>(Response).Clear(); break;
        }
    }

    void Reset() noexcept
    {
        AutoAddHeader = TRUE;
        AutoDecompress = TRUE;
        Header = {};
        Data = {};
        Cookies = {};
        ResponseHeader.Clear();
        WantByteBuffer();
    }
};
ECK_NAMESPACE_END