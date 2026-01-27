#pragma once
#include "ComPtr.h"
#include "CUnknown.h"
#include "CSrwLock.h"
#include "CEvent.h"

#include <mfapi.h>
#include <mfidl.h>
#include <evr.h>

ECK_NAMESPACE_BEGIN
// 提供调用Media Foundation执行简单媒体播放的能力
// 不得跨线程调用同一实例上的方法
class CMediaPlayer final : public CUnknown<CMediaPlayer, IMFAsyncCallback>
{
public:
    struct EVENT_PARAM
    {
        CMediaPlayer* pPlayer;
        MediaEventType eType;
        IMFAsyncResult* pAsyncResult;
        IMFMediaEvent* pEvent;
        void* pUserData;
    };

    using FEventCallback = HRESULT(*)(const EVENT_PARAM&);

    struct RATE_SUPPORT
    {
        float fMin;
        float fMax;

        float fMinReverse;
        float fMaxReverse;

        float fMinThin;
        float fMaxThin;

        float fMinReverseThin;
        float fMaxReverseThin;
    };

    enum class State : BYTE
    {
        Empty,  // 无效

        Opened, // 已打开媒体
        Playing,
        Paused,
        Stopped,

        BeginPaly,
        BeginPause,
        BeginStop,
    };
    enum class OpenResult : BYTE
    {
        Ok,
        MediaSession,
        SourceResolver,
        Source,
        SourceQi,
        Presentation,
        Topology,
        EnumStream,
        NoStream,
        MediaTypeHandler,
        MediaType,
        Activate,
        SourceNode,
        OutputNode,
        Graph,
        Clock,
    };
private:
    ComPtr<IMFMediaSession> m_pSession{};
    ComPtr<IMFMediaSource> m_pSource{};
    ComPtr<IMFVideoDisplayControl> m_pVideoDisplayControl{};
    ComPtr<IMFPresentationClock> m_pClock{};
    ComPtr<IMFRateControl> m_pRateControl{};
    ComPtr<IMFSimpleAudioVolume> m_pAudioVolume{};

    State m_eState{ State::Empty };
    BOOLEAN m_bHasAudio{};
    BOOLEAN m_bHasVideo{};
    ULONGLONG m_msDuration{};

    FEventCallback m_pfnEventCallback{};
    void* m_pEventUserData{};

    CSrwLock m_Lock{};
    CEvent m_Evt{};

    HRESULT TopoCreateSourceNode(
        ComPtr<IMFTopologyNode>& pNode,
        IMFPresentationDescriptor* pPresentation,
        IMFStreamDescriptor* pStream) noexcept
    {
        HRESULT hr;
        hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, pNode.AddrOfClear());
        if (FAILED(hr))
            return hr;
        pNode->SetUnknown(MF_TOPONODE_SOURCE, m_pSource.Get());
        pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPresentation);
        pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pStream);
        return S_OK;
    }
    HRESULT TopoCreateOutputNode(
        ComPtr<IMFTopologyNode>& pNode,
        IMFActivate* pActivate) noexcept
    {
        HRESULT hr;
        hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, pNode.AddrOfClear());
        if (FAILED(hr))
            return hr;
        hr = pNode->SetObject(pActivate);
        pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
        return hr;
    }

    // IMFAsyncCallback

    HRESULT STDMETHODCALLTYPE GetParameters(
        __RPC__out DWORD* pdwFlags, __RPC__out DWORD* pdwQueue) override
    {
        *pdwFlags = 0;
        *pdwQueue = 0;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Invoke(__RPC__in_opt IMFAsyncResult* pAsyncResult) override
    {
        HRESULT hr;
        ComPtr<IMFMediaEvent> pEvent;
        hr = m_pSession->EndGetEvent(pAsyncResult, &pEvent);
        if (FAILED(hr))
            return hr;
        MediaEventType eType;
        hr = pEvent->GetType(&eType);
        if (FAILED(hr))
            return hr;
        switch (eType)
        {
        case MESessionClosed:
        {
            m_Evt.Signal();
            CSrwWriteGuard _{ m_Lock };
            m_eState = State::Empty;
        }
        break;
        case MESessionStarted:
        {
            CSrwWriteGuard _{ m_Lock };
            m_eState = State::Playing;
        }
        break;
        case MESessionPaused:
        {
            CSrwWriteGuard _{ m_Lock };
            m_eState = State::Paused;
        }
        break;
        case MESessionStopped:
        {
            CSrwWriteGuard _{ m_Lock };
            m_eState = State::Stopped;
        }
        break;
        case MESessionTopologyStatus:
        {
            UINT32 eStatus;
            hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &eStatus);
            if (FAILED(hr))
                return hr;
            if (eStatus == MF_TOPOSTATUS_READY)
            {
                ComPtr<IMFVideoDisplayControl> pVideo;
                ComPtr<IMFRateControl> pRateControl;
                ComPtr<IMFSimpleAudioVolume> pAudioVolume;
                MFGetService(m_pSession.Get(), MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&pVideo));
                MFGetService(m_pSession.Get(), MF_RATE_CONTROL_SERVICE, IID_PPV_ARGS(&pRateControl));
                MFGetService(m_pSession.Get(), MR_POLICY_VOLUME_SERVICE, IID_PPV_ARGS(&pAudioVolume));
                {
                    CSrwWriteGuard _{ m_Lock };
                    pVideo.Swap(m_pVideoDisplayControl);
                    pRateControl.Swap(m_pRateControl);
                    pAudioVolume.Swap(m_pAudioVolume);
                    m_eState = State::Opened;
                }
                m_Evt.Signal();
            }
        }
        break;
        }
        FEventCallback pfnCallback;
        void* pUserData;
        {
            CSrwReadGuard _{ m_Lock };
            pfnCallback = m_pfnEventCallback;
            pUserData = m_pEventUserData;
        }
        if (pfnCallback)
        {
            EVENT_PARAM Param;
            Param.pPlayer = this;
            Param.eType = eType;
            Param.pAsyncResult = pAsyncResult;
            Param.pEvent = pEvent.Get();
            Param.pUserData = pUserData;
            hr = pfnCallback(Param);
            if (FAILED(hr))
                return hr;
        }
        return m_pSession->BeginGetEvent(this, nullptr);
    }
public:
    ECK_DISABLE_COPY_MOVE_DEF_CONS(CMediaPlayer);

    ~CMediaPlayer() { Close(); }

    // 同步。函数成功返回后，调用方需要显式调用Play
    HRESULT OpenFile(
        _In_z_ PCWSTR pszFilePath,
        _In_opt_ HWND hWndVideo = nullptr,
        _Out_opt_ OpenResult* pResult = nullptr) noexcept
    {
        Close();

        OpenResult r;
        if (!pResult)
            pResult = &r;
        HRESULT hr;
        // 创建媒体会话
        hr = MFCreateMediaSession(nullptr, &m_pSession);
        if (FAILED(hr))
        {
            *pResult = OpenResult::MediaSession;
            return hr;
        }
        hr = m_pSession->BeginGetEvent(this, nullptr);
        if (FAILED(hr))
        {
            *pResult = OpenResult::MediaSession;
            return hr;
        }
        // 创建媒体源
        ComPtr<IMFSourceResolver> pResolver;
        hr = MFCreateSourceResolver(&pResolver);
        if (FAILED(hr))
        {
            *pResult = OpenResult::SourceResolver;
            return hr;
        }

        MF_OBJECT_TYPE eType;
        ComPtr<IUnknown> pUnkSource;
        hr = pResolver->CreateObjectFromURL(
            pszFilePath,
            MF_RESOLUTION_MEDIASOURCE,
            nullptr,
            &eType,
            &pUnkSource);
        if (FAILED(hr))
        {
            *pResult = OpenResult::Source;
            return hr;
        }

        hr = pUnkSource.As(m_pSource);
        if (FAILED(hr))
        {
            *pResult = OpenResult::SourceQi;
            return hr;
        }
        // 创建媒体源描述符
        ComPtr<IMFPresentationDescriptor> pPresentation;
        hr = m_pSource->CreatePresentationDescriptor(&pPresentation);
        if (FAILED(hr))
        {
            *pResult = OpenResult::Presentation;
            return hr;
        }
        pPresentation->GetUINT64(MF_PD_DURATION, &m_msDuration);
        // 创建拓扑
        ComPtr<IMFTopology> pTopology;
        hr = MFCreateTopology(&pTopology);
        if (FAILED(hr))
        {
            *pResult = OpenResult::Topology;
            return hr;
        }
        // 枚举流
        DWORD cStream;
        hr = pPresentation->GetStreamDescriptorCount(&cStream);
        if (FAILED(hr))
        {
            *pResult = OpenResult::EnumStream;
            return hr;
        }

        ComPtr<IMFStreamDescriptor> pStream;
        ComPtr<IMFMediaTypeHandler> pHandler;
        ComPtr<IMFActivate> pActivate;
        ComPtr<IMFTopologyNode> pSourceNode, pOutputNode;
        BOOL bSelected;
        GUID guidMajorType;
        UINT cValidStream{};
        EckCounter(cStream, i)
        {
            hr = pPresentation->GetStreamDescriptorByIndex(i, &bSelected, pStream.AddrOfClear());
            if (FAILED(hr))
            {
                *pResult = OpenResult::EnumStream;
                return hr;
            }
            if (!bSelected)
                continue;
            // 取类型
            hr = pStream->GetMediaTypeHandler(pHandler.AddrOfClear());
            if (FAILED(hr))
            {
                *pResult = OpenResult::MediaTypeHandler;
                return hr;
            }
            hr = pHandler->GetMajorType(&guidMajorType);
            if (FAILED(hr))
            {
                *pResult = OpenResult::MediaType;
                return hr;
            }
            // 创建渲染器激活
            if (guidMajorType == MFMediaType_Audio)
            {
                hr = MFCreateAudioRendererActivate(pActivate.AddrOfClear());
                m_bHasAudio = TRUE;
            }
            else if (hWndVideo)
            {
                hr = MFCreateVideoRendererActivate(hWndVideo, pActivate.AddrOfClear());
                m_bHasVideo = TRUE;
            }
            else
                continue;
            if (FAILED(hr))
            {
                *pResult = OpenResult::Activate;
                return hr;
            }

            hr = TopoCreateSourceNode(pSourceNode, pPresentation.Get(), pStream.Get());
            if (FAILED(hr))
            {
                *pResult = OpenResult::SourceNode;
                return hr;
            }
            hr = TopoCreateOutputNode(pOutputNode, pActivate.Get());
            if (FAILED(hr))
            {
                *pResult = OpenResult::OutputNode;
                return hr;
            }

            hr = pTopology->AddNode(pSourceNode.Get());
            if (FAILED(hr))
            {
                *pResult = OpenResult::Graph;
                return hr;
            }
            hr = pTopology->AddNode(pOutputNode.Get());
            if (FAILED(hr))
            {
                *pResult = OpenResult::Graph;
                return hr;
            }
            hr = pSourceNode->ConnectOutput(0, pOutputNode.Get(), 0);
            if (FAILED(hr))
            {
                *pResult = OpenResult::Graph;
                return hr;
            }
            ++cValidStream;
        }

        if (!cValidStream)
        {
            *pResult = OpenResult::NoStream;
            return S_FALSE;
        }

        hr = m_pSession->SetTopology(0, pTopology.Get());
        if (FAILED(hr))
        {
            *pResult = OpenResult::Graph;
            return hr;
        }

        ComPtr<IMFClock> pClock;
        hr = m_pSession->GetClock(&pClock);
        if (FAILED(hr))
        {
            *pResult = OpenResult::Clock;
            return hr;
        }
        pClock.As(m_pClock);

        *pResult = OpenResult::Ok;
        WaitObject(m_Evt);
        return S_OK;
    }

    // 同步
    void Close() noexcept
    {
        HRESULT hr;
        m_Evt.NoSignal();
        if (m_pSession.Get())
        {
            hr = m_pSession->Close();
            if (SUCCEEDED(hr))
                WaitObject(m_Evt);
        }
        if (m_pSource.Get())
            m_pSource->Shutdown();
        if (m_pSession.Get())
            m_pSession->Shutdown();

        m_Evt.NoSignal();
        m_pSession.Clear();
        m_pSource.Clear();
        m_pVideoDisplayControl.Clear();
        m_pClock.Clear();
        m_pRateControl.Clear();
        m_pAudioVolume.Clear();
        m_eState = State::Empty;
        m_bHasAudio = FALSE;
        m_bHasVideo = FALSE;
        m_msDuration = 0ull;
    }

    // 异步
    HRESULT Play(LONGLONG msBeginPos = 0ll) noexcept
    {
        PROPVARIANT Var;
        PropVariantInit(&Var);
        Var.vt = VT_I8;
        Var.hVal.QuadPart = msBeginPos * 10000ll;// 100纳秒
        const auto hr = m_pSession->Start(&GUID_NULL, &Var);
        if (SUCCEEDED(hr))
        {
            CSrwWriteGuard _{ m_Lock };
            m_eState = State::BeginPaly;
        }
        return hr;
    }

    // 异步
    HRESULT Pause() noexcept
    {
        const auto hr = m_pSession->Pause();
        if (SUCCEEDED(hr))
        {
            CSrwWriteGuard _{ m_Lock };
            m_eState = State::BeginPause;
        }
        return hr;
    }

    // 异步
    HRESULT Resume() noexcept
    {
        PROPVARIANT Var;
        PropVariantInit(&Var);
        const auto hr = m_pSession->Start(&GUID_NULL, &Var);
        if (SUCCEEDED(hr))
        {
            CSrwWriteGuard _{ m_Lock };
            m_eState = State::BeginPaly;
        }
        return hr;
    }

    // 异步
    HRESULT Stop() noexcept
    {
        const auto hr = m_pSession->Stop();
        if (SUCCEEDED(hr))
        {
            CSrwWriteGuard _{ m_Lock };
            m_eState = State::BeginStop;
        }
        return hr;
    }

    HRESULT GetPosition(_Out_ LONGLONG& msPosition) noexcept
    {
        if (!m_pClock.Get())
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        MFTIME hnsTime;
        const auto hr = m_pClock->GetTime(&hnsTime);
        msPosition = hnsTime / 10000ll;
        return hr;
    }

    // 异步，若先前未播放，操作完成后将播放媒体
    HRESULT SetPosition(LONGLONG msPosition) noexcept
    {
        return Play(msPosition);
    }

    State GetRawState() noexcept
    {
        CSrwReadGuard _{ m_Lock };
        return m_eState;
    }
    State GetState() noexcept
    {
        const auto eState = GetRawState();
        switch (eState)
        {
        case State::BeginPaly:
            return State::Playing;
        case State::BeginPause:
            return State::Paused;
        case State::BeginStop:
            return State::Stopped;
        default:
            return eState;
        }
    }

    HRESULT GetDuration(_Out_ ULONGLONG& ms) noexcept
    {
        ms = m_msDuration;
        return S_OK;
    }

    // 查询速率范围和抽帧播放支持
    // 通常在打开媒体后调用函数一次，如有必要，调用方缓存结果，函数将不支持的区间对设置为1.0f
    // 速率定义为播放速度相对于正常速度的倍数
    HRESULT QueryRateSupport(_Out_ RATE_SUPPORT& Support) noexcept
    {
        HRESULT hr;
        ComPtr<IMFRateSupport> pRateSupport;
        if (!m_pRateControl.Get())
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            goto Exit;
        }
        hr = MFGetService(m_pSession.Get(), MF_RATE_CONTROL_SERVICE, IID_PPV_ARGS(&pRateSupport));
        if (FAILED(hr))
        {
        Exit:
            Support.fMin = Support.fMax =
                Support.fMinReverse = Support.fMaxReverse =
                Support.fMinThin = Support.fMaxThin =
                Support.fMinReverseThin = Support.fMaxReverseThin = 1.0f;
            return hr;
        }
        hr = pRateSupport->GetSlowestRate(MFRATE_FORWARD, FALSE, &Support.fMin);
        if (FAILED(hr))
            Support.fMin = 1.0f;
        hr = pRateSupport->GetFastestRate(MFRATE_FORWARD, FALSE, &Support.fMax);
        if (FAILED(hr))
            Support.fMax = 1.0f;

        hr = pRateSupport->GetSlowestRate(MFRATE_REVERSE, FALSE, &Support.fMinReverse);
        if (FAILED(hr))
            Support.fMinReverse = 1.0f;
        hr = pRateSupport->GetFastestRate(MFRATE_REVERSE, FALSE, &Support.fMaxReverse);
        if (FAILED(hr))
            Support.fMaxReverse = 1.0f;

        hr = pRateSupport->GetSlowestRate(MFRATE_FORWARD, TRUE, &Support.fMinThin);
        if (FAILED(hr))
            Support.fMinThin = 1.0f;
        hr = pRateSupport->GetFastestRate(MFRATE_FORWARD, TRUE, &Support.fMaxThin);
        if (FAILED(hr))
            Support.fMaxThin = 1.0f;

        hr = pRateSupport->GetSlowestRate(MFRATE_REVERSE, TRUE, &Support.fMinReverseThin);
        if (FAILED(hr))
            Support.fMinReverseThin = 1.0f;
        hr = pRateSupport->GetFastestRate(MFRATE_REVERSE, TRUE, &Support.fMaxReverseThin);
        if (FAILED(hr))
            Support.fMaxReverseThin = 1.0f;
        return S_OK;
    }

    HRESULT GetRate(_Out_ float& fRate, _Out_opt_ BOOL* pbThin = nullptr) noexcept
    {
        if (!m_pRateControl.Get())
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        fRate = 1.0f;
        if (pbThin)
            *pbThin = FALSE;
        return m_pRateControl->GetRate(pbThin, &fRate);
    }

    HRESULT SetRate(float fRate, BOOL bThin = FALSE) noexcept
    {
        if (!m_pRateControl.Get())
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        return m_pRateControl->SetRate(bThin, fRate);
    }

    EckInlineNdCe BOOL HasAudio() const noexcept { return m_bHasAudio; }
    EckInlineNdCe BOOL HasVideo() const noexcept { return m_bHasVideo; }

    /// <summary>
    /// 设置事件回调
    /// </summary>
    /// <param name="pfnCallback">接收Media Foundation事件的函数，
    /// 播放器在内部处理必要事件后回调此函数，调用方不得在回调内执行耗时操作
    /// </param>
    /// <param name="pUserData">自定义数据</param>
    void SetEventCallback(FEventCallback pfnCallback, void* pUserData) noexcept
    {
        CSrwWriteGuard _{ m_Lock };
        m_pfnEventCallback = pfnCallback;
        m_pEventUserData = pUserData;
    }

    HRESULT VideoGetNativeSize(_Out_ SIZE& size,
        _Out_opt_ SIZE* psizeAspectRatio = nullptr) noexcept
    {
        if (!m_pVideoDisplayControl.Get())
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        size = {};
        if (psizeAspectRatio)
            *psizeAspectRatio = {};
        return m_pVideoDisplayControl->GetNativeVideoSize(&size, psizeAspectRatio);
    }
    HRESULT VideoSetDisplayRect(const RECT& rcDst,
        const MFVideoNormalizedRect* pRectSrc = nullptr) noexcept
    {
        if (!m_pVideoDisplayControl.Get())
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        return m_pVideoDisplayControl->SetVideoPosition(pRectSrc, (RECT*)&rcDst);
    }
    HRESULT VideoPaint() noexcept
    {
        if (!m_pVideoDisplayControl.Get())
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        return m_pVideoDisplayControl->RepaintVideo();
    }

    HRESULT GetVolume(_Out_ float& fVolume) noexcept
    {
        if (!m_pAudioVolume.Get())
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        return m_pAudioVolume->GetMasterVolume(&fVolume);
    }
    HRESULT SetVolume(float fVolume) noexcept
    {
        if (!m_pAudioVolume.Get())
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        return m_pAudioVolume->SetMasterVolume(fVolume);
    }
    HRESULT GetMute(_Out_ BOOL& bMute) noexcept
    {
        if (!m_pAudioVolume.Get())
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        return m_pAudioVolume->GetMute(&bMute);
    }
    HRESULT SetMute(BOOL bMute) noexcept
    {
        if (!m_pAudioVolume.Get())
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        return m_pAudioVolume->SetMute(bMute);
    }

    // Low-Level control Interface

    void LciGetVideoDisplayControl(ComPtr<IMFVideoDisplayControl>& pControl) noexcept
    {
        pControl = m_pVideoDisplayControl;
    }
};
ECK_NAMESPACE_END