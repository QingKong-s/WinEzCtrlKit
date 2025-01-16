#pragma once
#include "MediaTag.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
// 采样率 单位Hz
constexpr inline USHORT MpegSampleRateTable[][3]
{
	{ 44100,48000,32000 },	// MPEG-1
	{ 22050,24000,16000 },	// MPEG-2
	{ 11025,12000, 8000 }	// MPEG-2.5
};

constexpr inline USHORT MpegBitrateFree = std::numeric_limits<USHORT>::max();

// 比特率 单位kbps
constexpr inline USHORT MpegBitrateTable[][16]
{
	{ MpegBitrateFree,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0 },	// MPEG-1 Layer 1
	{ MpegBitrateFree,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,0 },	// MPEG-1 Layer 2
	{ MpegBitrateFree,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,0 },	// MPEG-1 Layer 3
	{ MpegBitrateFree,32,48,56, 64, 80, 96,112,128,144,160,176,192,224,256,0 },	// MPEG-2/2.5 Layer 1
	{ MpegBitrateFree, 8,16,24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0 },	// MPEG-2/2.5 Layer 2
	{ MpegBitrateFree, 8,16,24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0 }	// MPEG-2/2.5 Layer 3
};

// 采样数
constexpr inline USHORT MpegSampleCountTable[][3]
{
	{ 384,1152,1152 },	// MPEG-1
	{ 384,1152, 576 },	// MPEG-2
	{ 384,1152, 576 }	// MPEG-2.5
};

class CMpegInfo
{
public:
	enum class Version :BYTE
	{
		Mpeg2_5,
		Reserved,
		Mpeg2,
		Mpeg1,
	};

	enum class Layer :BYTE
	{
		Reserved,
		Layer3,
		Layer2,
		Layer1,
	};

	enum class Channel :BYTE
	{
		Stereo,			// 立体声
		JointStereo,	// 联合立体声
		DualChannel,	// 双声道
		Mono,			// 单声道
	};

	enum class Emphasis :BYTE
	{
		None,
		Ms50_15,
		Reserved,
		CCIT_J17,
	};

	struct MPEG_INFO
	{
		Layer eLayer;
		Version eVersion;
		BYTE BitrateIndex;
		BYTE SampleRateIndex;
		Channel eChannel;
		Emphasis eEmphasis;
		BYTE eExtMode;

		BITBOOL bCrc : 1;
		BITBOOL bPadding : 1;
		BITBOOL bPrivate : 1;
		BITBOOL bCopyright : 1;
		BITBOOL bOriginal : 1;
	};

	/*
	struct MPEG_HEADER
	{
		UINT Emphasis : 2;
		UINT bOriginal : 1;
		UINT bCopyright : 1;
		UINT ExtMode : 2;
		UINT Channel : 2;

		UINT bPrivate : 1;
		UINT bPadding : 1;
		UINT SampleRateIndex : 2;
		UINT BitrateIndex : 4;

		UINT bCrc : 1;
		UINT Layer : 2;
		UINT Version : 2;
		UINT Sync : 11;
	};
	*/

	CMediaFile& m_File;
	CStreamWalker m_Stream{};

	SIZE_T m_posBegin{};
	MPEG_INFO m_Info{};
public:
	CMpegInfo(CMediaFile& File) :m_File{ File }, m_Stream(File.GetStream())
	{
		m_Stream.GetStream()->AddRef();
	}

	~CMpegInfo()
	{
		m_Stream.GetStream()->Release();
	}

	Result Read()
	{
		if (m_File.m_Loc.posV2 != SIZETMax)
		{
			m_Stream.MoveTo(m_File.m_Loc.posV2);
			ID3v2_Header Hdr;
			m_Stream >> Hdr;
			m_posBegin = SynchSafeIntToDWORD(Hdr.Size) + 10 + m_File.m_Loc.posV2;// 跳过ID3v2以避免错误同步
		}
		else
			m_posBegin = 0;
		m_Stream.MoveTo(m_posBegin);
		// 同步到MPEG头
		BYTE byHdr[4]{};
		m_Stream >> byHdr;
		if (byHdr[0] != 0xFF && (byHdr[1] & 0b1110'0000_by) != 0b1110'0000_by)
		{
			// 没有同步字节，重新同步
			m_Stream.MoveTo(m_posBegin);
			BYTE* pBuf = (BYTE*)VAlloc(4096);
			UniquePtr<DelVA<BYTE>> _(pBuf);
			EckCheckMem(pBuf);
			constexpr size_t cbSegment = 1024;
			BYTE bySync[2]{};
			EckLoop()
			{
				m_Stream.Read(pBuf, cbSegment);
				if (size_t cbLastRead; (cbLastRead = (size_t)m_Stream.GetLastRWSize()) < cbSegment)
				{
					if (!cbLastRead || FAILED(m_Stream.GetLastErr()))
						break;
					for (auto p = pBuf; p < pBuf + cbLastRead; ++p)
					{
						bySync[0] = bySync[1];
						bySync[1] = *p;
						if (bySync[0] == 0xFF && (bySync[1] & 0b1110'0000_by) == 0b1110'0000)
						{
							m_posBegin += (p - pBuf - 1);
							m_Stream.MoveTo(m_posBegin)>>byHdr;
							goto SyncSucceed;
						}
					}
					m_posBegin += cbLastRead;
				}
				else
				{
					for (auto p = pBuf; p < pBuf + cbSegment; ++p)
					{
						bySync[0] = bySync[1];
						bySync[1] = *p;
						if (bySync[0] == 0xFF && (bySync[1] & 0b1110'0000_by) == 0b1110'0000)
						{
							m_posBegin += (p - pBuf - 1);
							m_Stream.MoveTo(m_posBegin) >> byHdr;
							goto SyncSucceed;
						}
					}
					m_posBegin += cbSegment;
				}
			}
			return Result::MpegSyncFailed;
		}
	SyncSucceed:
		m_Info.eVersion = Version((byHdr[1] >> 3) & 0b11_by);
		m_Info.eLayer = Layer((byHdr[1] >> 1) & 0b11_by);
		m_Info.bCrc = byHdr[1] & 1_by;

		m_Info.BitrateIndex = (byHdr[2] >> 4) & 0b1111_by;
		m_Info.SampleRateIndex = (byHdr[2] >> 2) & 0b11_by;
		m_Info.bPadding = (byHdr[2] >> 1) & 1_by;
		m_Info.bPrivate = byHdr[2] & 1_by;

		m_Info.eChannel = Channel((byHdr[3] >> 6) & 0b11_by);
		m_Info.eExtMode = (byHdr[3] >> 4) & 0b11_by;
		m_Info.bCopyright = (byHdr[3] >> 3) & 1_by;
		m_Info.bOriginal = (byHdr[3] >> 2) & 1_by;
		m_Info.eEmphasis = Emphasis(byHdr[3] & 0b11_by);
		return Result::Ok;
	}

	EckInline constexpr const auto& GetInfo() const{ return m_Info; }

	constexpr USHORT GetBitrate() const
	{
		const size_t idxVer = ((m_Info.eVersion == Version::Mpeg1) ? 0u : 1u);
		size_t idxLayer;
		switch (m_Info.eLayer)
		{
			case Layer::Layer1: idxLayer = 0; break;
			case Layer::Layer2: idxLayer = 1; break;
			case Layer::Layer3: idxLayer = 2; break;
			default: idxLayer = 0; return 0;
		}
		
		return MpegBitrateTable[idxVer * 3 + idxLayer][m_Info.BitrateIndex];
	}

	constexpr USHORT GetSampleRate() const
	{
		size_t idxVer;
		switch (m_Info.eVersion)
		{
			case Version::Mpeg1: idxVer = 0; break;
			case Version::Mpeg2: idxVer = 1; break;
			case Version::Mpeg2_5: idxVer = 2; break;
			default: idxVer = 0; return 0;
		}

		return MpegSampleRateTable[idxVer][m_Info.SampleRateIndex];
	}

	/// <summary>
	/// 计算每帧持续时间。
	/// 因ID3v2中某些字段可以用MPEG帧作时长单位，故提供此方法
	/// </summary>
	/// <returns>以毫秒计的帧持续时间</returns>
	constexpr double CalcDurationPerFrame() const
	{
		size_t idxVer;
		switch (m_Info.eVersion)
		{
		case Version::Mpeg1: idxVer = 0; break;
		case Version::Mpeg2: idxVer = 1; break;
		case Version::Mpeg2_5: idxVer = 2; break;
		default: return 0.;
		}

		size_t idxLayer;
		switch (m_Info.eLayer)
		{
		case Layer::Layer1: idxLayer = 0; break;
		case Layer::Layer2: idxLayer = 1; break;
		case Layer::Layer3: idxLayer = 2; break;
		default: return 0.;
		}

		return (double)MpegSampleCountTable[idxVer][idxLayer] / (double)GetSampleRate() * 1000.;
	}
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END