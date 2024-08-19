/*
* WinEzCtrlKit Library
*
* MediaTagID3v2.h ： ID3v2读写
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "MediaTag.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
class CID3v2 :public CTag
{
private:
	ID3v2_Header m_Header{};

	SIZE_T m_cbTag{};				// 标签长度
	SIZE_T m_SeekVal{ SIZETMax };	// 根据SEEK帧查找到的追加标签位置
	SIZE_T m_cbPrependTag{};		// 预置标签长度

	/// <summary>
	/// 按指定编码处理文本
	/// </summary>
	/// <param name="pStream">字节流指针；未指定iTextEncoding时指向整个文本帧，指定iTextEncoding时指向字符串</param>
	/// <param name="iLength">长度；未指定iTextEncoding时表示整个文本帧长度（包括1B的编码标记，不含结尾NULL），指定iTextEncoding时表示字符串长度（不含结尾NULL）</param>
	/// <param name="iTextEncoding">自定义文本编码；-1（缺省）指示处理的是文本帧</param>
	/// <returns>返回处理完毕的文本</returns>
	static CRefStrW GetID3v2_ProcString(CMemWalker& w, int cb, int iTextEncoding = -1)
	{
		const BOOL bHasTNull = (cb < 0);
		SIZE_T cbTNull{};
		int iType = iTextEncoding, cchBuf;
		if (iTextEncoding == -1)
		{
			iType = *w.Data();
			w += 1;// 跳过文本编码标志
			--cb;
		}

		CRefStrW rsResult{};

		switch (iType)
		{
		case 0:// ISO-8859-1
			if (bHasTNull)
			{
				cb = (int)strlen((PCSTR)w.Data());
				cbTNull = 1u;
			}

			if (cchBuf = MultiByteToWideChar(CP_ACP/*keep ANSI encoding tag happy*/, 0,
				(PCCH)w.Data(), cb, NULL, 0))
			{
				rsResult.ReSize(cchBuf);
				MultiByteToWideChar(CP_ACP, 0, (PCCH)w.Data(), cb, rsResult.Data(), cchBuf);
			}
			break;

		case 1:// UTF-16LE
			if (bHasTNull)
			{
				cb = (int)wcslen((PCWSTR)w.Data()) * sizeof(WCHAR);
				cbTNull = 2u;
			}

			if (*(PWSTR)w.Data() == L'\xFEFF')// 跳BOM
			{
				w += sizeof(WCHAR);
				cb -= sizeof(WCHAR);
			}
			else if (*(PWSTR)w.Data() == L'\xFFFE')// for ID3v2.3
			{
				w += sizeof(WCHAR);
				cb -= sizeof(WCHAR);
				goto Utf16BE;
			}

			if (cb)
			{
				cchBuf = cb / sizeof(WCHAR);
				rsResult.ReSize(cchBuf);
				wmemcpy(rsResult.Data(), (PWSTR)w.Data(), cchBuf);
			}
			break;

		case 2:// UTF-16BE
			if (bHasTNull)
			{
				cb = (int)wcslen((PCWSTR)w.Data()) * sizeof(WCHAR);
				cbTNull = 2u;
			}

			if (*(PWSTR)w.Data() == L'\xFFFE')// 跳BOM
			{
				w += sizeof(WCHAR);
				cb -= sizeof(WCHAR);
			}
		Utf16BE:
			if (cb)
			{
				cchBuf = cb / sizeof(WCHAR);
				rsResult.ReSize(cchBuf);
				LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
					(PCWSTR)w.Data(), cchBuf, rsResult.Data(), cchBuf, NULL, NULL, 0);
			}
			break;

		case 3:// UTF-8
			if (bHasTNull)
			{
				cb = (int)strlen((PCSTR)w.Data());
				cbTNull = 1u;
			}

			if (cchBuf = MultiByteToWideChar(CP_UTF8, 0, (PCCH)w.Data(), cb, NULL, 0))
			{
				rsResult.ReSize(cchBuf);
				MultiByteToWideChar(CP_UTF8, 0, (PCCH)w.Data(), cb, rsResult.Data(), cchBuf);
			}
			break;
		default:
			EckDbgBreak();
			break;
		}

		w += cb;
		if (bHasTNull)
			w += cbTNull;
		return rsResult;
	}
public:
	enum class TagSizeRestriction :BYTE
	{
		Max128Frames_1MB,
		Max64Frames_128KB,
		Max32Frames_40KB,
		Max32Frames_4KB,
	};

	enum class TextEncodingRestriction :BYTE
	{
		No,
		OnlyLatin1OrU8
	};

	enum class TextFieldSizeRestriction :BYTE
	{
		No,
		Max1024Char,
		Max128Char,
		Max30Char,
	};

	enum class ImageFormatRestriction :BYTE
	{
		No,
		OnlyPngOrJpeg
	};

	enum class ImageSizeRestriction :BYTE
	{
		No,
		Max256x256,
		Max64x64,
		Only64x64
	};

	struct EXTHDR_INFO
	{
		BYTE bTagAlter : 1;		// ID3v2.4 Only
		BYTE bCrc : 1;
		BYTE bRestrictions : 1;	// ID3v2.4 Only
		BYTE bPaddingSize : 1;	// ID3v2.3 Only
		TagSizeRestriction eTagSize;			// ID3v2.4 Only
		TextEncodingRestriction eTextEncoding;	// ID3v2.4 Only
		TextFieldSizeRestriction eTextFieldSize;// ID3v2.4 Only
		ImageFormatRestriction eImageFormat;	// ID3v2.4 Only
		ImageSizeRestriction eImageSize;		// ID3v2.4 Only
		UINT uCrc;
		UINT cbPadding;			// ID3v2.3 Only
	};

	enum class EventType :BYTE
	{
		Padding,				// 填充(无意义)
		EndOfInitialSilence,	// 初始静音结束
		IntroStart,				// 前奏开始
		MainPartStart,			// 主体部分开始
		OutroStart,				// 序曲开始
		OutroEnd,				// 片尾曲结束
		VerseStart,				// 诗句开始
		RefrainStart,			// 重唱开始
		InterludeStart,			// 插曲开始
		ThemeStart,				// 主题开始
		VariationStart,			// 变奏开始
		KeyChange,				// 调性变化
		TimeChange,				// 时间变化
		MomentaryUnwantedNoise,	// 瞬时噪音("啪"、"噼啪"和"噗噗"声)
		SustainedNoise,			// 持续噪音
		SustainedNoiseEnd,		// 持续噪音结束
		IntroEnd,				// 前奏结束
		MainPartEnd,			// 主体部分结束
		VerseEnd,				// 诗句结束
		RefrainEnd,				// 重唱结束
		ThemeEnd,				// 主题结束
		Profanity,				// 脏话 **(仅ID3v2.4)**
		ProfanityEnd,			// 脏话结束 **(仅ID3v2.4)**

		InvalidBeginV2_4,		// ！无效区开始 **(仅ID3v2.4)**
		InvalidEnd = 0xFC,		// ！无效区结束

		AudioEnd = 0xFD,		// 音频结束(静音开始)
		AudioFileEnds,			// 音频文件结束

		InvalidBeginV2_3 = Profanity,// 无效区开始 **(仅ID3v2.3)**
	};

	enum class TimestampFmt :BYTE
	{
		MpegFrame,
		Milliseconds,
		Max
	};

	enum class TextEncoding :BYTE
	{
		Latin1,
		UTF16V2_3,
		UTF16LE = UTF16V2_3,
		MaxV2_3,
		UTF16BE = MaxV2_3,
		UTF8,
		Max
	};

	enum class LrcContentType :BYTE
	{
		Other,				// 其他
		Lyrics,				// 歌词
		TextTranscription,	// 文字转录
		Movement,			// 乐章/乐段名称(如"Adagio")。
		Events,				// 事件(如"堂吉诃德登场")
		Chord,				// 和弦(如"Bb F Fsus")
		Trivia,				// 小节/弹出式信息
		WebpageUrl,			// 网页URL
		ImageUrl,			// 图片URL
		Max
	};

	enum class ChannelType :BYTE
	{
		Other,			// 其他
		MasterVolume,	// 主音量
		FrontRight,		// 前右
		FrontLeft,		// 前左
		BackRight,		// 后右
		BackLeft,		// 后左
		FrontCentre,	// 前中
		BackCentre,		// 后中
		Subwoofer,		// 低音炮
		Max
	};

	enum class Interpolation :BYTE
	{
		Band,	// 不进行插值。从一个调整级别到另一个调整级别的跳变发生在两个调整点之间的中间位置。
		Linear,	// 调整点之间的插值是线性的。
		Max
	};

	enum class ReceivedWay :BYTE
	{
		Other,
		StandardCdAlbumWithOtherSongs,
		CompressedAudioOnCd,
		FileOverTheInternet,
		StreamOverTheInternet,
		AsNoteSheets,
		AsNoteSheetsInABookWithOtherSheets,
		MusicOnOtherMedia,
		NonMusicalMerchandise,
		Max
	};

#define ECK_DECL_ID3FRAME_METHOD_CLONE(x)					\
	FRAME* Clone() const override { return new x{ *this }; }\
	x(x&&) = default;										\
	x(const x&) = default;

#define ECK_DECL_ID3FRAME_METHOD_CLONE_DEF_CONS(x)			\
	FRAME* Clone() const override { return new x{ *this }; }\
	x(x&&) = default;										\
	x(const x&) = default;									\
	x() = default;

#define ECK_DECL_ID3FRAME_METHOD(x)			\
	ECK_DECL_ID3FRAME_METHOD_CLONE(x)		\
	x() { memcpy(Id, #x, 4); }

#define ECK_DECL_ID3FRAME_METHOD_ID(x)		\
	ECK_DECL_ID3FRAME_METHOD_CLONE(x)		\
	x() = default;							\
	x(PCSTR psz) { memcpy(Id, psz, 4); }


	struct FRAME
	{
		CHAR Id[4]{};		// 帧标识
		BYTE uFlags[2]{};	// 标志，[0] = 状态，[1] = 格式
		MIIWFLAG byAddtFlags{};

		virtual ~FRAME() {}

		virtual Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) = 0;

		virtual FRAME* Clone() const = 0;
	protected:
		EckInline constexpr void AddFileAlterDiscardFlag(ID3v2_Header* phdr)
		{
			if (phdr->Ver == 3)
				uFlags[0] |= ID3V23FF_FILE_ALTER_DISCARD;
			else
				uFlags[0] |= ID3V24FF_FILE_ALTER_DISCARD;
		}

		CMemWalker PreSerialize(CRefBin& rb, ID3v2_Header* phdr, size_t cbFrame)
		{
			const auto cbTotal = sizeof(ID3v2_FrameHeader) + cbFrame;
			CMemWalker w(rb.PushBack(cbTotal), cbTotal);
			ID3v2_FrameHeader* pfhdr;
			w.SkipPointer(pfhdr);
			memcpy(pfhdr->ID, Id, 4);
			memcpy(pfhdr->Flags, &uFlags, 2);
			if (phdr->Ver == 4)
				DwordToSynchSafeInt(pfhdr->Size, (DWORD)cbFrame);
			else
				*(DWORD*)pfhdr->Size = ReverseInteger((DWORD)cbFrame);
			return w;
		}

		void PostSerialize(CRefBin& rb, ID3v2_Header* phdr, size_t cbFrame)
		{
			EckAssert(rb.Size() >= sizeof(ID3v2_Header) + cbFrame);
			if (phdr->Ver == 4)
			{
				if (uFlags[1] & ID3V24FF_COMPRESSION)
				{
					// TODO:压缩
				}

				if (phdr->Flags & ID3V2HF_UNSYNCHRONIZATION || uFlags[1] & ID3V24FF_UNSYNCHRONIZATION)
				{
					for (size_t i = rb.Size() - cbFrame; i < rb.Size() - 1; ++i)
					{
						if (rb[i] == 0xFF &&
							(rb[i + 1] == 0 || IsBitSet(rb[i + 1], 0b1110'0000)))
							rb.Insert(i + 1, 0);
					}
				}
			}
			else
			{
				if (uFlags[1] & ID3V24FF_COMPRESSION)
				{
					// TODO:压缩
				}

				if (phdr->Flags & ID3V2HF_UNSYNCHRONIZATION)
				{
					for (size_t i = rb.Size() - cbFrame; i < rb.Size() - 1; ++i)
					{
						if (rb[i] == 0xFF &&
							(rb[i + 1] == 0 || IsBitSet(rb[i + 1], 0b1110'0000)))
							rb.Insert(i + 1, 0);
					}
				}
			}
		}

		static CRefBin CovertTextEncoding(const CRefStrW& rsStr,
			TextEncoding eEncoding, BOOL bAddTerNull = FALSE)
		{
			CRefBin rb{};
			if (rsStr.IsEmpty())
			{
				if (bAddTerNull)
					switch (eEncoding)
					{
					case TextEncoding::Latin1:
					case TextEncoding::UTF8:
						rb.ReSize(1);
						rb.Front() = 0;
						break;
					case TextEncoding::UTF16LE:
					case TextEncoding::UTF16BE:
						rb.ReSize(2);
						rb[0] = 0;
						rb[1] = 0;
						break;
					}
				return rb;
			}

			switch (eEncoding)
			{
			case TextEncoding::Latin1:
				if (int cchBuf; cchBuf = WideCharToMultiByte(CP_ACP, 0,
					rsStr.Data(), rsStr.Size(), NULL, 0, NULL, NULL))
				{
					rb.ReSize(bAddTerNull ? cchBuf + 1 : cchBuf);
					WideCharToMultiByte(CP_ACP, 0,
						rsStr.Data(), rsStr.Size(), (PSTR)rb.Data(), cchBuf, NULL, NULL);
					if (bAddTerNull)
						rb.Back() = 0;
				}
				break;

			case TextEncoding::UTF16LE:
				rb.ReSize(rsStr.ByteSize() + 2 - (bAddTerNull ? 0 : sizeof(WCHAR)));
				memcpy(rb.Data(), BOM_UTF16LE, 2);
				memcpy(rb.Data() + 2, rsStr.Data(), rsStr.ByteSize() - (bAddTerNull ? 0 : sizeof(WCHAR)));
				break;

			case TextEncoding::UTF16BE:
				rb.ReSize(rsStr.ByteSize() - (bAddTerNull ? 0 : sizeof(WCHAR)));
				LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
					rsStr.Data(), rsStr.Size() + (bAddTerNull ? 1 : 0),
					(PWSTR)rb.Data(), rsStr.Size() + (bAddTerNull ? 1 : 0), NULL, NULL, 0);
				break;

			case TextEncoding::UTF8:
				if (int cchBuf; cchBuf = WideCharToMultiByte(CP_UTF8, 0,
					rsStr.Data(), rsStr.Size(), NULL, 0, NULL, NULL))
				{
					rb.ReSize(bAddTerNull ? cchBuf + 1 : cchBuf);
					WideCharToMultiByte(CP_UTF8, 0,
						rsStr.Data(), rsStr.Size(), (PSTR)rb.Data(), cchBuf, NULL, NULL);
					if (bAddTerNull)
						rb.Back() = 0;
				}
				break;
			default:
				EckDbgBreak();
				break;
			}
			return rb;
		}

		static CRefBin CovertTextEncoding(const std::vector<CRefStrW>& v,
			TextEncoding eEncoding, BOOL bAddTerNull = FALSE)
		{
			if (v.size() == 1u)
				return CovertTextEncoding(v.front(), eEncoding, bAddTerNull);
			CRefStrW rs{};
			int cch{};
			for (const auto& e : v)
				cch += (e.Size() + 1);
			rs.ReSize(bAddTerNull ? cch : cch - 1);
			for (PWSTR psz = rs.Data(); const auto & e : v)
			{
				wmemcpy(psz, e.Data(), e.Size() + 1);
				psz += (e.Size() + 1);
			}
			return CovertTextEncoding(rs, eEncoding, bAddTerNull);
		}
	};

	struct UFID :public FRAME
	{
		CRefStrA rsEmail{};
		CRefBin rbOwnerData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = rsEmail.Size() + 1 + rbOwnerData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << rbOwnerData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(UFID)
	};

	struct TEXTFRAME :public FRAME
	{
		TextEncoding eEncoding{};
		std::vector<CRefStrW> vText{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbText = CovertTextEncoding(vText, eEncoding);
			size_t cbFrame = 1 + rbText.Size();
			if (memcmp(Id, "TENC", 4) == 0 || memcmp(Id, "TLEN", 4) == 0)
				AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rbText;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD_ID(TEXTFRAME)
	};

	struct TXXX :public FRAME
	{
		TextEncoding eEncoding{};
		CRefStrW rsDesc{};
		CRefStrW rsText{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			CRefBin rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			CRefBin rbText = CovertTextEncoding(rsText, eEncoding);
			const size_t cbFrame = 1 + rbDesc.Size() + rbText.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rbDesc << rbText;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(TXXX)
	};

	struct LINKFRAME :public FRAME
	{
		CRefStrA rsUrl{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			auto w = PreSerialize(rb, phdr, rsUrl.Size());
			w.Write(rsUrl.Data(), rsUrl.Size());
			PostSerialize(rb, phdr, rsUrl.Size());
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD_ID(LINKFRAME)
	};

	struct WXXX :public FRAME
	{
		TextEncoding eEncoding{};
		CRefStrW rsDesc{};
		CRefStrA rsUrl{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			const size_t cbFrame = 1 + rbDesc.Size() + rsUrl.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rbDesc;
			w.Write(rsUrl.Data(), rsUrl.Size());
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(WXXX)
	};

	struct MCID :public FRAME
	{
		CRefBin rbToc{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			auto w = PreSerialize(rb, phdr, rbToc.Size());
			w << rbToc;
			PostSerialize(rb, phdr, rbToc.Size());
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(MCID)
	};

	struct ETCO :public FRAME
	{
		struct EVENT
		{
			EventType eType;
			UINT uTimestamp;
		};

		TimestampFmt eTimestampFmt{};
		std::vector<EVENT> vEvent{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 1 + 5 * vEvent.size();
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eTimestampFmt;
			for (const auto& e : vEvent)
				(w << e.eType).WriteRev(e.uTimestamp);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(ETCO)
	};

	struct MLLT :public FRAME
	{
		struct REF
		{
			ULONGLONG ByteOffset;
			ULONGLONG MillisecondsOffset;
		};
		UINT cByte{};
		UINT cMilliseconds{};
		USHORT cMpegFrame{};
		BYTE cByteOffsetValBit{};
		BYTE cMillisecondsOffsetValBit{};
		std::vector<REF> vRef{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			// 只支持整数个字节写入
			if (cByteOffsetValBit % 8 || cMillisecondsOffsetValBit % 8 ||
				cByteOffsetValBit > 64 || cMillisecondsOffsetValBit > 64)
				return Result::InvalidVal;
			const size_t cbFrame = 0;
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteRev(cMpegFrame).WriteRev(&cByte, 3).WriteRev(&cMilliseconds, 3);
			w << cByteOffsetValBit << cMillisecondsOffsetValBit;
			for (const auto& e : vRef)
			{
				w.WriteRev(&e.ByteOffset, cByteOffsetValBit / 8);
				w.WriteRev(&e.MillisecondsOffset, cMillisecondsOffsetValBit / 8);
			}
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(MLLT)
	};

	struct SYTC :public FRAME
	{
		struct TEMPO
		{
			USHORT bpm{};
			UINT uTimestamp{};
		};
		TimestampFmt eTimestampFmt{};
		std::vector<TEMPO> vTempo{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			size_t cbFrame = 1 + vTempo.size() * 5;
			for (const auto& e : vTempo)
				if (e.bpm >= 0xFF)
					++cbFrame;
				else if (e.bpm > 510)
					return Result::InvalidVal;

			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eTimestampFmt;
			for (const auto& e : vTempo)
			{
				if (e.bpm >= 0xFF)
					w << (BYTE)0xFF << (BYTE)(e.bpm - 0xFF);
				else
					w << (BYTE)e.bpm;
				w.WriteRev(e.uTimestamp);
			}
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(SYTC)
	};

	struct USLT :public FRAME
	{
		TextEncoding eEncoding{};
		CHAR byLang[3]{};
		CRefStrW rsDesc{};
		CRefStrW rsLrc{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			const auto rbLrc = CovertTextEncoding(rsLrc, eEncoding);
			const size_t cbFrame = 4 + rbDesc.Size() + rbLrc.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << byLang << rbDesc << rbLrc;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(USLT)
	};

	struct SYLT :public FRAME
	{
		struct SYNC
		{
			CRefStrW rsText{};
			UINT uTimestamp{};
		};

		TextEncoding eEncoding{};
		CHAR byLang[3]{};
		TimestampFmt eTimestampFmt{};
		LrcContentType eContent{};
		CRefStrW rsDesc{};
		std::vector<SYNC> vSync{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			size_t cbFrame = 6 + rbDesc.Size() + vSync.size() * 4;

			std::vector<CRefBin> vLrc(vSync.size());
			EckCounter(vSync.size(), i)
			{
				vLrc[i] = CovertTextEncoding(vSync[i].rsText, eEncoding, TRUE);
				cbFrame += vLrc[i].Size();
			}

			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << byLang << eTimestampFmt << rbDesc;
			EckCounter(vSync.size(), i)
				(w << vLrc[i]).WriteRev(vSync[i].uTimestamp);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(SYLT)
	};

	struct COMM :public FRAME
	{
		TextEncoding eEncoding{};
		CHAR byLang[3]{};
		CRefStrW rsDesc{};
		CRefStrW rsText{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			const auto rbText = CovertTextEncoding(rsText, eEncoding);
			const size_t cbFrame = 4 + rbDesc.Size() + rbText.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << byLang << rbDesc << rbText;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(COMM)
	};

	struct RVA2 :public FRAME
	{
		struct CHANNEL
		{
			ChannelType eChannel{};
			BYTE cPeekVolBit{};
			short shVol{};
			CBitSet<256> PeekVol{};
		};
		CRefStrA rsId{};
		std::vector<CHANNEL> vChannel{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 0;
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsId;
			for (const auto& e : vChannel)
			{
				w << e.eChannel;
				w.WriteRev(e.shVol);
				w << e.cPeekVolBit;
				w.WriteRev(e.PeekVol.Data(), (e.cPeekVolBit - 1) / 8 + 1);
			}
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(RVA2)
	};

	struct EQU2 :public FRAME
	{
		struct POINT
		{
			USHORT uFreq{};	// 单位1/2Hz
			short shVol{};
		};
		Interpolation eInterpolation{};
		CRefStrA rsId{};
		std::vector<POINT> vPoint{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 2 + rsId.Size() + vPoint.size() * 4;
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eInterpolation << rsId;
			for (auto e : vPoint)
				w.WriteRev(e.uFreq).WriteRev(e.shVol);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(EQU2)
	};

	struct RVRB :public FRAME
	{
		USHORT Left{};
		USHORT Right{};
		BYTE BouncesLeft{};
		BYTE BouncesRight{};
		BYTE FeedbackLeftToLeft{};
		BYTE FeedbackLeftToRight{};
		BYTE FeedbackRightToRight{};
		BYTE FeedbackRightToLeft{};
		BYTE PremixLeftToRight{};
		BYTE PremixRightToLeft{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			constexpr size_t cbFrame = 12;
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteRev(Left).WriteRev(Right).Write(&BouncesLeft, 8);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(RVRB)
	};

	struct APIC :public FRAME
	{
		TextEncoding eEncoding{};
		PicType eType{};
		CRefStrA rsMime{};
		CRefStrW rsDesc{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			if (rsMime.IsEmpty())
				rsMime = "";
			const auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			const size_t cbFrame = 3 + rsMime.Size() + rbDesc.Size() + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rsMime << eType << rbDesc << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(APIC)
	};

	struct GEOB :public FRAME
	{
		TextEncoding eEncoding{};
		CRefStrA rsMime{};
		CRefStrW rsFile{};
		CRefStrW rsDesc{};
		CRefBin rbObj{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbFile = CovertTextEncoding(rsFile, eEncoding, TRUE);
			const auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);
			const size_t cbFrame = 2 + rsMime.Size() + rbFile.Size() + rbDesc.Size() + rbObj.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rsMime << rbFile << rbDesc << rbObj;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(GEOB)
	};

	struct PCNT :public FRAME
	{
		ULONGLONG cPlay{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			size_t cbFrame = 4;
			for (int i = 7; i >= 4; --i)
			{
				if (GetIntegerByte(cPlay, i))
				{
					cbFrame = i + 1;
					break;
				}
			}
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteRev(&cPlay, cbFrame);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(PCNT)
	};

	struct POPM :public FRAME
	{
		CRefStrA rsEmail{};
		BYTE byRating{};
		ULONGLONG cPlay{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			size_t cbPlayCount = 4;
			for (int i = 7; i >= 4; --i)
			{
				if (GetIntegerByte(cPlay, i))
				{
					cbPlayCount = i + 1;
					break;
				}
			}
			const size_t cbFrame = 2 + rsEmail.Size() + cbPlayCount;
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << byRating;
			w.WriteRev(&cPlay, cbPlayCount);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(POPM)
	};

	struct RBUF :public FRAME
	{
		UINT cbBuf{};
		BYTE b{};
		UINT ocbNextTag{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			constexpr size_t cbFrame = 8;
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteRev(&cbBuf, 3) << b;
			w.WriteRev(ocbNextTag);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(RBUF)
	};

	struct AENC :public FRAME
	{
		CRefStrA rsOwnerId{};
		USHORT usPreviewBegin{};
		USHORT usPreviewLength{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 5 + rsOwnerId.Size() + rbData.Size();
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsOwnerId;
			w.WriteRev(usPreviewBegin).WriteRev(usPreviewLength);
			w << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(AENC)
	};

	struct LINK :public FRAME
	{
		CHAR IdTarget[4]{};
		CRefStrA rsUrl{};
		CRefStrA rsAdditional{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 5 + rsUrl.Size() + rsUrl.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << IdTarget << rsUrl << rsAdditional;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(LINK)
	};

	struct POSS :public FRAME
	{
		TimestampFmt eTimestamp{};
		UINT uTime{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 5;
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			(w << eTimestamp).WriteRev(uTime);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(POSS)
	};

	struct USER :public FRAME
	{
		TextEncoding eEncoding{};
		CHAR byLang[3]{};
		CRefStrW rsText{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbText = CovertTextEncoding(rsText, eEncoding);
			const size_t cbFrame = 4 + rbText.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << byLang << rbText;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(USER)
	};

	struct OWNE :public FRAME
	{
		TextEncoding eEncoding{};
		CRefStrA rsPrice{};
		CHAR szDate[8]{};
		CRefStrW rsSeller{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbSeller = CovertTextEncoding(rsSeller, eEncoding);
			const size_t cbFrame = 10 + rsPrice.Size() + rbSeller.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rsPrice << szDate << rbSeller;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(OWNE)
	};

	struct COMR :public FRAME
	{
		TextEncoding eEncoding{};
		ReceivedWay eReceivedWay{};
		CRefStrA rsPrice{};
		CHAR szDate[8]{};
		CRefStrA rsUrl{};
		CRefStrW rsSeller{};
		CRefStrW rsDesc{};
		CRefStrA rsMime{};
		CRefBin rbLogo{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const auto rbSeller = CovertTextEncoding(rsSeller, eEncoding, TRUE);
			const auto rbDesc = CovertTextEncoding(rsDesc, eEncoding, TRUE);

			const size_t cbFrame = 13 + rsPrice.Size() + rsUrl.Size() +
				rbSeller.Size() + rbDesc.Size() + rsMime.Size() + rbLogo.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << eReceivedWay << rsPrice << szDate << rsUrl <<
				rbSeller << rbDesc << rsMime << rbLogo;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(COMR)
	};

	struct ENCR :public FRAME
	{
		CRefStrA rsEmail{};
		BYTE byMethod{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 2 + rsEmail.Size() + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << byMethod << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(ENCR)
	};

	struct GRID :public FRAME
	{
		CRefStrA rsEmail{};
		BYTE byId{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 2 + rsEmail.Size() + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << byId << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(GRID)
	};

	struct PRIV :public FRAME
	{
		CRefStrA rsEmail{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 1 + rsEmail.Size() + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(PRIV)
	};

	struct SIGN :public FRAME
	{
		BYTE byGroupId{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 1 + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << byGroupId << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(SIGN)
	};

	struct SEEK :public FRAME
	{
		UINT ocbNextTag{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			constexpr size_t cbFrame = 4;
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteRev(ocbNextTag);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(SEEK)
	};

	struct ASPI :public FRAME
	{
		UINT S{};
		UINT L{};
		USHORT N{};
		BYTE b{};
		std::vector<USHORT> vIndex{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 10 + vIndex.size() * 2;
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteRev(S).WriteRev(L).WriteRev(N) << b;
			for (const auto e : vIndex)
				w.WriteRev(&e, b / 8);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(ASPI)
	};

	struct OTHERFRAME :public FRAME
	{
		CRefBin rbData;

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD_ID(OTHERFRAME)
	};

	//---------v2.3---------
	struct IPLS :public FRAME
	{
		struct MAP
		{
			CRefStrW rsPosition{};
			CRefStrW rsName{};
		};
		TextEncoding eEncoding{};
		std::vector<MAP> vMap{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			int cch{};
			for (const auto& e : vMap)
				cch += (e.rsPosition.Size() + e.rsName.Size() + 2);
			if (!cch)
				return Result::EmptyData;
			CRefStrW rs(cch);
			for (PWSTR psz = rs.Data(); const auto & e : vMap)
			{
				wmemcpy(psz, e.rsPosition.Data(), e.rsPosition.Size() + 1);
				psz += (e.rsPosition.Size() + 1);
				wmemcpy(psz, e.rsName.Data(), e.rsName.Size() + 1);
				psz += (e.rsName.Size() + 1);
			}

			const auto rbText = CovertTextEncoding(rs, eEncoding, TRUE);
			const size_t cbFrame = 1 + rbText.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << eEncoding << rbText;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(IPLS)
	};

	struct RVAD :public FRAME
	{
		BYTE byCtrl{};
		BYTE cBits{};
		ULONG rVol{};	// 右调整
		ULONG lVol{};	// 左调整
		ULONG rPeak{};	// 右峰值
		ULONG lPeak{};	// 左峰值
		ULONG rbVol{};	// 右后调整
		ULONG lbVol{};	// 左后调整
		ULONG rbPeak{};	// 右后峰值
		ULONG lbPeak{};	// 左后峰值
		ULONG mVol{};	// 中央调整
		ULONG mPeak{};	// 中央峰值
		ULONG bsVol{};	// 低音调整
		ULONG bsPeak{};	// 低音峰值

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			if (byCtrl & 0b1100'0000)
				return Result::ReservedDataErr;
			const size_t cbField = (cBits + 1) / 8 - 1;
			size_t cbFrame = cbField * 4 + 2;
			if (byCtrl & 0b0000'1100)// 左右后
				cbFrame += (cbField * 4);
			if (byCtrl & 0b0001'0000)// 中央
				cbFrame += (cbField * 2);
			if (byCtrl & 0b0010'0000)// 低音
				cbFrame += (cbField * 2);
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << byCtrl << cBits;
			w.WriteRev(&rVol, cbField)
				.WriteRev(&lVol, cbField)
				.WriteRev(&rPeak, cbField)
				.WriteRev(&lPeak, cbField);
			if (byCtrl & 0b0000'1100)
			{
				w.WriteRev(&rbVol, cbField)
					.WriteRev(&lbVol, cbField)
					.WriteRev(&rbPeak, cbField)
					.WriteRev(&lbPeak, cbField);
			}
			if (byCtrl & 0b0001'0000)
			{
				w.WriteRev(&mVol, cbField)
					.WriteRev(&mPeak, cbField);
			}
			if (byCtrl & 0b0010'0000)
			{
				w.WriteRev(&bsVol, cbField)
					.WriteRev(&bsPeak, cbField);
			}
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(RVAD)
	};

	struct EQUA :public FRAME
	{
		struct ADJ
		{
			short shFreq{};
			ULONG ulAdjust{};
		};
		BYTE cBits{};
		std::vector<ADJ> vAdjust{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbField = (cBits + 1) / 8 - 1;
			const size_t cbFrame = 1 + vAdjust.size() * (2 + cbField);
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << cBits;
			for (const auto& e : vAdjust)
			{
				w.WriteRev(&e.shFreq, 2)
					.WriteRev(&e.ulAdjust, cbField);
			}
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(EQUA)
	};

	//---------v2.2---------
	struct CRM :public FRAME
	{
		CRefStrA rsEmail{};
		CRefStrA rsDesc{};
		CRefBin rbData{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 2 + rsEmail.Size() + rsDesc.Size() + rbData.Size();
			auto w = PreSerialize(rb, phdr, cbFrame);
			w << rsEmail << rsDesc << rbData;
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(CRM)
	};

	//---------非标---------
	struct iTunesNORM :public COMM
	{
		ULONG rAdjust{};
		ULONG lAdjust{};
		ULONG rAdjust2{};
		ULONG lAdjust2{};
		ULONG Dummy4{};
		ULONG Dummy5{};
		ULONG rPeak{};
		ULONG lPeak{};
		ULONG Dummy8{};
		ULONG Dummy9{};

		// 保存时将忽略COMM的所有字段
		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			constexpr size_t cbFrame = 1 + 3 + 8 + (8 + 1) * 10;
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			constexpr CHAR szLang[3]{ 'X','X','X' };
			w << TextEncoding::Latin1 << szLang << "iTunesNORM";
			sprintf_s((CHAR*)w.Data(), w.GetLeaveSize(),
				" %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X",
				rAdjust, lAdjust, rAdjust2, lAdjust2, Dummy4, Dummy5, rPeak, lPeak, Dummy8, Dummy9);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD_CLONE_DEF_CONS(iTunesNORM);

		iTunesNORM(const COMM& comm) :COMM(comm) {}

		iTunesNORM(COMM&& comm) noexcept :COMM(comm) {}
	};

	enum class ReplayGainType :BYTE
	{
		TrackGain,
		AlbumGain,
		TrackPeak,
		AlbumPeak,
	};

	struct TXXX_ReplayGain :public TXXX
	{
		ReplayGainType eType{};
		float fVal{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			if (phdr->Ver == 4)
				uFlags[0] |= ID3V24FF_FILE_ALTER_DISCARD;
			else
				uFlags[0] |= ID3V23FF_FILE_ALTER_DISCARD;

			const auto rsVal = Format(
				(eType == ReplayGainType::TrackGain || eType == ReplayGainType::AlbumGain) ?
				"%.2f dB" : "%.6f", (double)fVal);
			// 验证'.'前面是否只有一个数字
			if (eType == ReplayGainType::TrackGain || eType == ReplayGainType::AlbumGain)
			{
				if (rsVal.Size() < 6)// 0.0 dB
					return Result::InvalidVal;
				if (rsVal.Front() == '-')
				{
					if (rsVal.FindChar('.') != 2)
						return Result::InvalidVal;
				}
				else
				{
					if (rsVal.FindChar('.') != 1)
						return Result::InvalidVal;
				}
			}
			else
			{
				if (rsVal.Size() < 3)// 0.0
					return Result::InvalidVal;
				if (rsVal.FindChar('.') != 1)
					return Result::InvalidVal;
			}
			const size_t cbFrame = 1 + 22 + rsVal.Size();
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			constexpr CHAR c_szKey[4][22]
			{
				"REPLAYGAIN_TRACK_GAIN",
				"REPLAYGAIN_ALBUM_GAIN",
				"REPLAYGAIN_TRACK_PEAK",
				"REPLAYGAIN_ALBUM_PEAK",
			};

			w << TextEncoding::Latin1 << c_szKey[(int)eType];
			w.Write(rsVal.Data(), rsVal.Size());
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD_CLONE_DEF_CONS(TXXX_ReplayGain);
	};

	struct RGAD :public FRAME
	{
		ULONG ulPeak{};
		USHORT usTrackGain{};
		USHORT usAlbumGain{};

		Result SerializeData(CRefBin& rb, ID3v2_Header* phdr) override
		{
			const size_t cbFrame = 4 + 2 + 2;
			AddFileAlterDiscardFlag(phdr);
			auto w = PreSerialize(rb, phdr, cbFrame);
			w.WriteRev(ulPeak).WriteRev(usTrackGain).WriteRev(usAlbumGain);
			PostSerialize(rb, phdr, cbFrame);
			return Result::Ok;
		}

		ECK_DECL_ID3FRAME_METHOD(RGAD)
	};

	struct XRVA :public RVA2
	{
		ECK_DECL_ID3FRAME_METHOD(XRVA);
	};


	static BOOL IsLegalFrameId(const CHAR* ch)
	{
		return memcmp(ch, "ID3", 3) != 0 && memcmp(ch, "3DI", 3) != 0 &&
			isalnum(ch[0]) && isalnum(ch[1]) && isalnum(ch[2]) && isalnum(ch[3]);
	}
private:
	std::vector<FRAME*> m_vFrame{};

	EXTHDR_INFO m_ExtHdrInfo{};

	struct CFrameProcesser
	{
		CStreamWalker& m_w;
		const ID3v2_FrameHeader& m_Header{};
		CRefBin m_rbFrame{};
		DWORD m_cbFrame{};
		BOOL m_bUnsync = FALSE;

		CFrameProcesser(CID3v2& id3, const ID3v2_FrameHeader& Header, DWORD cbFrame, FRAME* pFrame) :
			m_w(id3.m_Stream), m_Header(Header), m_cbFrame(cbFrame),
			m_bUnsync(id3.m_Header.Flags& ID3V2HF_UNSYNCHRONIZATION),
			m_rbFrame(cbFrame)
		{
			memcpy(pFrame->Id, Header.ID, 4);
			memcpy(&pFrame->uFlags, Header.Flags, 2);
			if (!m_bUnsync && id3.m_Header.Ver == 4)
				m_bUnsync = !!(m_Header.Flags[1] & ID3V24FF_UNSYNCHRONIZATION);
		}

		std::pair<CMemWalker, DWORD> Prepare()
		{
			m_w.Read(m_rbFrame.Data(), m_cbFrame);
			if (m_bUnsync)
				m_rbFrame.ReplaceSubBin({ 0xFF, 0x00 }, { 0xFF });// 恢复不同步处理
			return
			{
				CMemWalker(m_rbFrame.Data(), m_rbFrame.Size()),
				(DWORD)m_rbFrame.Size()
			};
		}
	};

	EckInline static CRefStrW GetID3v2_ProcString(CMemWalker& w, int cb, TextEncoding iTextEncoding)
	{
		return GetID3v2_ProcString(w, cb, (int)iTextEncoding);
	}

	DWORD PreJudgeFrame(ID3v2_FrameHeader& FrameHdr)
	{
		m_Stream >> FrameHdr;
		DWORD cbExtra{}, cbUnit;
		if (m_Header.Ver == 3)
		{
			cbUnit = ReverseInteger(*(DWORD*)FrameHdr.Size);// v2.3：32位数据，不包括帧头（偏4字节）
			if (FrameHdr.Flags[1] & ID3V23FF_HAS_GROUP_IDENTITY)
				cbExtra += 1;// 跳过组标识符
			if (FrameHdr.Flags[1] & ID3V23FF_ENCRYPTION)
				cbExtra += 1;// 跳过加密类型标识符
			if (FrameHdr.Flags[1] & ID3V23FF_COMPRESSION)
				cbExtra += 4;// 跳过数据长度指示器
		}
		else/* if (m_Header.Ver == 4)*/
		{
			if (FrameHdr.Flags[1] & ID3V24FF_HAS_GROUP_IDENTITY)
				cbExtra += 1;// 跳过组标识符
			if (FrameHdr.Flags[1] & ID3V24FF_ENCRYPTION)
				cbExtra += 1;// 跳过加密类型标识符
			if (FrameHdr.Flags[1] & ID3V24FF_HAS_DATA_LENGTH_INDICATOR)
				cbExtra += 4;// 跳过数据长度指示器
			cbUnit = SynchSafeIntToDWORD(FrameHdr.Size);// v2.4：28位数据（同步安全整数）
		}
		cbUnit -= cbExtra;
		m_Stream += cbExtra;
		// TODO:处理压缩帧
		return cbUnit;
	}

#define ECK_HIT_ID3FRAME(x) (memcmp(FrameHdr.ID, #x, 4) == 0)

	Result ParseFrameBody(SIZE_T posEnd, SIZE_T* pposActualEnd = NULL)
	{
		posEnd = std::min(posEnd, (SIZE_T)m_Stream.GetSizeUli().QuadPart);
		ID3v2_FrameHeader FrameHdr;
		while (m_Stream.GetPos() < posEnd)
		{
			const auto cbUnit = PreJudgeFrame(FrameHdr);

			if (ECK_HIT_ID3FRAME(UFID))
			{
				const auto p = new UFID{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();

				w >> p->rsEmail;
				if (w.GetLeaveSize() > 64u)
				{
					delete p;
					return Result::TooLargeData;
				}
				else
				{
					p->rbOwnerData.DupStream(w.Data(), w.GetLeaveSize());
					m_vFrame.push_back(p);
				}
			}
			else if (ECK_HIT_ID3FRAME(TXXX))
			{
				const auto p = new TXXX{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();

				w >> p->eEncoding;
				if (p->eEncoding >= TextEncoding::Max)
				{
					delete p;
					return Result::IllegalEnum_TextEncoding;
				}
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsText = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), p->eEncoding);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(MCID))
			{
				const auto p = new MCID{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				p->rbToc = std::move(f.m_rbFrame);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(WXXX))
			{
				const auto p = new WXXX{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();

				w >> p->eEncoding;
				if (p->eEncoding >= TextEncoding::Max)
				{
					delete p;
					return Result::IllegalEnum_TextEncoding;
				}
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsUrl.DupString((PCSTR)w.Data(), (int)w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(ETCO))
			{
				const auto p = new ETCO{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();

				w >> p->eTimestampFmt;
				if (p->eTimestampFmt >= TimestampFmt::Max)
				{
					delete p;
					return Result::InvalidEnumVal;
				}
				if ((cb - 1) % 5)
				{
					delete p;
					return Result::LenErr;
				}
				EckCounterNV((cb - 1) / 5)
				{
					auto& e = p->vEvent.emplace_back();
					(w >> e.eType).ReadRev(e.uTimestamp);
				}

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(MLLT))
			{
				const auto p = new MLLT{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();

				w.ReadRev(p->cMpegFrame)
					.ReadRev(&p->cByte, 3)
					.ReadRev(&p->cMilliseconds, 3)
					>> p->cByteOffsetValBit
					>> p->cMillisecondsOffsetValBit;
				if (p->cByteOffsetValBit % 8 || p->cMillisecondsOffsetValBit % 8 ||
					p->cByteOffsetValBit > 64 || p->cMillisecondsOffsetValBit > 64)
				{
					p->rbData.ReSize(w.GetLeaveSize());
					w.Read(p->rbData.Data(), w.GetLeaveSize());
				}
				else
				{
					const size_t cb1 = p->cByteOffsetValBit / 8;
					const size_t cb2 = p->cMillisecondsOffsetValBit / 8;
					while (!w.IsEnd())
					{
						auto& e = p->vRef.emplace_back();
						w.ReadRev(&e.ByteOffset, cb1);
						w.ReadRev(&e.MillisecondsOffset, cb2);
					}
				}
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(SYTC))
			{
				const auto p = new SYTC{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				w >> p->eTimestampFmt;
				BYTE by;
				Result result{ Result::Ok };
				while (!w.IsEnd())
				{
					auto& e = p->vTempo.emplace_back();
					w >> by;
					if (by == 0xFF)
					{
						if (w.IsEnd())
						{
							result = Result::LenErr;
							goto Failed;
						}
						w >> by;
						e.bpm = 255 + by;
					}
					else
						e.bpm = by;
					if (w.IsEnd())
					{
						result = Result::LenErr;
						goto Failed;
					}
					w.ReadRev(e.uTimestamp);
				}
				m_vFrame.push_back(p);
				goto Ok;
			Failed:
				delete p;
				return result;
			Ok:;
			}
			else if (ECK_HIT_ID3FRAME(USLT))
			{
				const auto p = new USLT{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 4)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->eEncoding;
				if (p->eEncoding >= TextEncoding::Max)
				{
					delete p;
					return Result::IllegalEnum_TextEncoding;
				}
				w >> p->byLang;
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsLrc = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), p->eEncoding);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(SYLT))
			{
				const auto p = new SYLT{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 7)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->eEncoding;
				if (p->eEncoding >= TextEncoding::Max)
				{
					delete p;
					return Result::IllegalEnum_TextEncoding;
				}
				w >> p->byLang >> p->eTimestampFmt >> p->eContent;
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				while (!w.IsEnd())
				{
					auto& e = p->vSync.emplace_back();
					e.rsText = GetID3v2_ProcString(w, -1, p->eEncoding);
					if (w.IsEnd())
					{
						delete p;
						return Result::LenErr;
					}
					w.ReadRev(e.uTimestamp);
				}
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(COMM))
			{
				const auto p = new COMM{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 5)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->eEncoding;
				if (p->eEncoding >= TextEncoding::Max)
				{
					delete p;
					return Result::IllegalEnum_TextEncoding;
				}
				w >> p->byLang;
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsText = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), p->eEncoding);
				if (p->rsDesc == L"iTunesNORM" &&
					p->rsText.Size() >= (1 + 8) * 10 - 1)
				{
					const auto p2 = new iTunesNORM{ std::move(*p) };
					auto pszBegin =
						(p->rsText.Front() == L' ' ? p->rsText.Data() + 1 : p->rsText.Data());

					p2->rAdjust = wcstoul(pszBegin, NULL, 16);
					pszBegin += 9;
					p2->lAdjust = wcstoul(pszBegin, NULL, 16);
					pszBegin += 9;
					p2->rAdjust2 = wcstoul(pszBegin, NULL, 16);
					pszBegin += 9;
					p2->lAdjust2 = wcstoul(pszBegin, NULL, 16);
					pszBegin += 9;
					p2->Dummy4 = wcstoul(pszBegin, NULL, 16);
					pszBegin += 9;
					p2->Dummy5 = wcstoul(pszBegin, NULL, 16);
					pszBegin += 9;
					p2->rPeak = wcstoul(pszBegin, NULL, 16);
					pszBegin += 9;
					p2->lPeak = wcstoul(pszBegin, NULL, 16);
					pszBegin += 9;
					p2->Dummy8 = wcstoul(pszBegin, NULL, 16);
					pszBegin += 9;
					p2->Dummy9 = wcstoul(pszBegin, NULL, 16);
					m_vFrame.push_back(p);
				}
				else
					m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(RVA2))
			{
				const auto p = new RVA2{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 5)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->rsId;
				while (!w.IsEnd())
				{
					auto& e = p->vChannel.emplace_back();
					w >> e.eChannel;
					w.ReadRev(e.shVol) >> e.cPeekVolBit;
					if (e.cPeekVolBit)
						w.ReadRev(e.PeekVol.Data(), DivUpper(e.cPeekVolBit, 8));
				}

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(EQU2))
			{
				const auto p = new EQU2{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 6)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->eInterpolation >> p->rsId;
				if (w.GetLeaveSize() % 4)
				{
					delete p;
					return Result::LenErr;
				}
				const auto cPoint = w.GetLeaveSize() / 4;
				EckCounterNV(cPoint)
				{
					auto& e = p->vPoint.emplace_back();
					w.ReadRev(e.uFreq).ReadRev(e.shVol);
				}
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(RVRB))
			{
				const auto p = new RVRB{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb != 12)
				{
					delete p;
					return Result::LenErr;
				}
				w.ReadRev(p->Left).ReadRev(p->Right).Read(&p->BouncesLeft, 8);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(APIC))
			{
				const auto p = new APIC{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 5)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->eEncoding >> p->rsMime >> p->eType;
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rbData.DupStream(w.Data(), w.GetLeaveSize());

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(GEOB))
			{
				const auto p = new GEOB{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 4)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->eEncoding >> p->rsMime;
				p->rsFile = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rbObj.DupStream(w.Data(), w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(PCNT))
			{
				const auto p = new PCNT{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 4)
				{
					delete p;
					return Result::LenErr;
				}
				w.ReadRev(&p->cPlay, std::min(cb, 8ul));// 截断到8字节

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(POPM))
			{
				const auto p = new POPM{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 6)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->rsEmail >> p->byRating;
				w.ReadRev(&p->cPlay, std::min(w.GetLeaveSize(), (SIZE_T)8u));// 截断到8字节
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(RBUF))
			{
				const auto p = new RBUF{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb != 8)
				{
					delete p;
					return Result::LenErr;
				}
				(w.ReadRev(&p->cbBuf, 3) >> p->b).ReadRev(p->ocbNextTag);

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(AENC))
			{
				const auto p = new AENC{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 5)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->rsOwnerId;
				w.ReadRev(p->usPreviewBegin).ReadRev(p->usPreviewLength);
				p->rbData.DupStream(w.Data(), w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(LINK))
			{
				const auto p = new LINK{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 5)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->IdTarget >> p->rsUrl;
				p->rsAdditional.DupString((PCSTR)w.Data(), (int)w.GetLeaveSize());

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(POSS))
			{
				const auto p = new POSS{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb != 5)
				{
					delete p;
					return Result::LenErr;
				}
				(w >> p->eTimestamp).ReadRev(p->uTime);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(USER))
			{
				const auto p = new USER{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 4)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->eEncoding >> p->byLang;
				p->rsText = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), p->eEncoding);

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(OWNE))
			{
				const auto p = new OWNE{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 10)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->eEncoding >> p->rsPrice >> p->szDate;
				p->rsSeller = GetID3v2_ProcString(w, (int)w.GetLeaveSize(), p->eEncoding);
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(COMR))
			{
				const auto p = new COMR{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 15)
				{
					delete p;
					return Result::LenErr;
				}

				w >> p->eEncoding >> p->rsPrice >> p->szDate >> p->rsUrl >> p->eReceivedWay;
				p->rsSeller = GetID3v2_ProcString(w, -1, p->eEncoding);
				p->rsDesc = GetID3v2_ProcString(w, -1, p->eEncoding);
				w >> p->rsMime;
				p->rbLogo.DupStream(w.Data(), w.GetLeaveSize());

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(ENCR))
			{
				const auto p = new ENCR{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 2)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->rsEmail >> p->byMethod;
				p->rbData.DupStream(w.Data(), w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(GRID))
			{
				const auto p = new GRID{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 2)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->rsEmail >> p->byId;
				p->rbData.DupStream(w.Data(), w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(PRIV))
			{
				const auto p = new PRIV{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 1)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->rsEmail;

				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(SIGN))
			{
				const auto p = new SIGN{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 1)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->byGroupId;
				p->rbData.DupStream(w.Data(), w.GetLeaveSize());
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(SEEK))
			{
				const auto p = new SEEK{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb != 4)
				{
					delete p;
					return Result::LenErr;
				}
				w.ReadRev(p->ocbNextTag);
				if (m_SeekVal != SIZETMax)
				{
					delete p;
					return Result::IllegalRepeat;
				}
				m_SeekVal = p->ocbNextTag;
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(ASPI))
			{
				const auto p = new ASPI{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 12)
				{
					delete p;
					return Result::LenErr;
				}
				w.ReadRev(p->S).ReadRev(p->L).ReadRev(p->N) >> p->b;
				if (p->b != 8 && p->b != 16)
				{
					delete p;
					return Result::InvalidVal;
				}
				const auto cbField = p->b / 8;
				while (!w.IsEnd())
				{
					auto& e = p->vIndex.emplace_back();
					if (cbField == 1)
						w >> e;
					else
						w.ReadRev(e);
				}
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(IPLS))
			{
				const auto p = new IPLS{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 1)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->eEncoding;
				auto rs = GetID3v2_ProcString(w, cb - 1, p->eEncoding);
				std::vector<CRefStrW> v{};
				if (!rs.IsEmpty())
				{
					const auto itBegin = rs.begin();
					const auto itEnd = rs.end();
					for (auto it = itBegin; it <= itEnd; )
					{
						const int cch = (int)wcslen(it);
						v.emplace_back(it, cch);
						it += (cch + 1);
					}
				}
				p->vMap.resize(v.size() / 2);
				for (size_t i{}; i < v.size(); i += 2)
				{
					p->vMap[i / 2].rsPosition = std::move(v[i]);
					p->vMap[i / 2].rsName = std::move(v[i + 1]);
				}
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(RVAD))
			{
				const auto p = new RVAD{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 2)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->byCtrl >> p->cBits;
				if (p->byCtrl & 0b1100'0000)
				{
					delete p;
					return Result::InvalidVal;
				}

				const size_t cbField = DivUpper(p->cBits, 8);
				w.ReadRev(&p->rVol, cbField)
					.ReadRev(&p->lVol, cbField)
					.ReadRev(&p->rPeak, cbField)
					.ReadRev(&p->lPeak, cbField);
				if (p->byCtrl & 0b0000'1100)
				{
					w.ReadRev(&p->rbVol, cbField)
						.ReadRev(&p->lbVol, cbField)
						.ReadRev(&p->rbPeak, cbField)
						.ReadRev(&p->lbPeak, cbField);
				}
				if (p->byCtrl & 0b0001'0000)
				{
					w.ReadRev(&p->mVol, cbField)
						.ReadRev(&p->mPeak, cbField);
				}
				if (p->byCtrl & 0b0010'0000)
				{
					w.ReadRev(&p->bsVol, cbField)
						.ReadRev(&p->bsPeak, cbField);
				}
				m_vFrame.push_back(p);
			}
			else if (ECK_HIT_ID3FRAME(EQUA))
			{
				const auto p = new EQUA{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				if (cb < 1)
				{
					delete p;
					return Result::LenErr;
				}
				w >> p->cBits;
				const size_t cbField = (size_t)roundf(p->cBits / 8.f);
				while (!w.IsEnd())
				{
					auto& e = p->vAdjust.emplace_back();
					w.ReadRev(e.shFreq);
					w.ReadRev(&e.ulAdjust, cbField);
				}
				m_vFrame.push_back(p);
			}
			else if (FrameHdr.ID[0] == 'T')
			{
				const auto p = new TEXTFRAME{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				w >> p->eEncoding;
				auto rs = GetID3v2_ProcString(w, cb - 1, p->eEncoding);
				if (!rs.IsEmpty())
				{
					const auto itBegin = rs.begin();
					const auto itEnd = rs.end();
					for (auto it = itBegin; it <= itEnd; )
					{
						const int cch = (int)wcslen(it);
						p->vText.emplace_back(it, cch);
						it += (cch + 1);
					}
				}
				m_vFrame.push_back(p);
			}
			else if (FrameHdr.ID[0] == 'W')
			{
				const auto p = new LINKFRAME{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				auto [w, cb] = f.Prepare();
				p->rsUrl.DupString((PCSTR)w.Data(), (int)cb);
				m_vFrame.push_back(p);
			}
			else if (IsLegalFrameId(FrameHdr.ID))
			{
				const auto p = new OTHERFRAME{};
				CFrameProcesser f(*this, FrameHdr, cbUnit, p);
				f.Prepare();
				p->rbData = std::move(f.m_rbFrame);
				m_vFrame.push_back(p);
			}
			else
			{
				if (pposActualEnd)
					*pposActualEnd = m_Stream.GetPos() - 10;
				return Result::Ok;
			}
		}
		if (pposActualEnd)
			*pposActualEnd = posEnd;
		return Result::Ok;
	}

	// 初始化m_cbTag、m_cbPrependTag、m_Header、m_ExtHdrInfo
	Result PreReadWrite()
	{
		if (m_File.m_Loc.posV2 != SIZETMax)
			m_Stream.MoveTo(m_File.m_Loc.posV2);
		else if (m_File.m_Loc.posV2Footer != SIZETMax)
			m_Stream.MoveTo(m_File.m_Loc.posV2FooterHdr);
		else
		{
			return Result::NoTag;
			m_cbPrependTag = m_cbTag = 0u;
		}
		m_Stream >> m_Header;
		m_cbPrependTag = m_cbTag = SynchSafeIntToDWORD(m_Header.Size);
		m_ExtHdrInfo = {};
		if (m_Header.Ver == 3)// 2.3
		{
			if (m_Header.Flags & 0x20)
			{
				m_ExtHdrInfo.bPaddingSize = TRUE;
				UINT cb;
				BYTE byFlags[2];
				UINT cbPadding;
				m_Stream >> cb >> byFlags >> cbPadding;
				cb = ReverseInteger(cb);
				if (cb != 6 && cb != 10)
					return Result::TagErr;
				if (byFlags[0] & 0b1000'0000)
				{
					UINT uCrc;
					m_Stream >> uCrc;
					m_ExtHdrInfo.bCrc = TRUE;
					m_ExtHdrInfo.uCrc = ReverseInteger(uCrc);
				}
			}
			return Result::Ok;
		}
		else if (m_Header.Ver == 4)// 2.4
		{
			if (m_Header.Flags & 0x20)
			{
				UINT cb;
				BYTE byFlags;
				m_Stream >> cb;
				m_Stream += 1;
				m_Stream >> byFlags;

				m_ExtHdrInfo.bTagAlter = !!(byFlags & 0b0100'0000);
				if (byFlags & 0b0010'0000)
				{
					m_ExtHdrInfo.bCrc = TRUE;
					BYTE t[5];
					m_Stream >> t;
					m_ExtHdrInfo.uCrc = ((t[0] & 0x7F) << 28) | ((t[1] & 0x7F) << 21) |
						((t[2] & 0x7F) << 14) | ((t[3] & 0x7F) << 7) | (t[4] & 0x7F);
				}
				if (byFlags & 0b0001'0000)
				{
					m_ExtHdrInfo.bRestrictions = TRUE;
					BYTE t;
					m_Stream >> t;
					m_ExtHdrInfo.eTagSize = TagSizeRestriction((t >> 6) & 0b11);
					m_ExtHdrInfo.eTextEncoding = TextEncodingRestriction((t >> 5) & 1);
					m_ExtHdrInfo.eTextFieldSize = TextFieldSizeRestriction((t >> 3) & 0b11);
					m_ExtHdrInfo.eImageFormat = ImageFormatRestriction((t >> 2) & 1);
					m_ExtHdrInfo.eImageSize = ImageSizeRestriction(t & 0b11);
				}
			}
			return Result::Ok;
		}
		else
		{
			m_cbTag = 0u;
			EckDbgPrintWithPos(L"未识别的ID3版本");
			return Result::TagErr;
		}
	}
public:
	ECK_DISABLE_COPY_MOVE(CID3v2)
public:
	CID3v2(CMediaFile& File) :CTag(File) {}

	~CID3v2()
	{
		for (const auto e : m_vFrame)
			delete e;
	}

	Result SimpleExtract(MUSICINFO& mi) override
	{
		mi.Clear();
		if (m_vFrame.empty())
			return Result::NoTag;
		for (const auto e : m_vFrame)
		{
			if ((mi.uMask & MIM_TITLE) && memcmp(e->Id, "TIT2", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				if (!p->vText.empty())
				{
					mi.rsTitle = p->vText[0];
					mi.uMaskRead |= MIM_TITLE;
				}
			}
			else if ((mi.uMask & MIM_ARTIST) && memcmp(e->Id, "TPE1", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				for (const auto& e : p->vText)
				{
					mi.AppendArtist(e);
					mi.uMaskRead |= MIM_ARTIST;
				}
			}
			else if ((mi.uMask & MIM_ALBUM) && memcmp(e->Id, "TALB", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				if (!p->vText.empty())
				{
					mi.rsAlbum = p->vText[0];
					mi.uMaskRead |= MIM_ALBUM;
				}
			}
			else if ((mi.uMask & MIM_LRC) && memcmp(e->Id, "USLT", 4) == 0)
			{
				const auto p = DynCast<USLT*>(e);
				mi.rsLrc = p->rsLrc;
				mi.uMaskRead |= MIM_LRC;
			}
			else if ((mi.uMask & MIM_COMMENT) && memcmp(e->Id, "COMM", 4) == 0)
			{
				const auto p = DynCast<COMM*>(e);
				mi.AppendComment(p->rsText);
				mi.uMaskRead |= MIM_COMMENT;
			}
			else if ((mi.uMask & MIM_COVER) && memcmp(e->Id, "APIC", 4) == 0)
			{
				const auto p = DynCast<APIC*>(e);
				MUSICPIC Pic{};
				Pic.eType = p->eType;
				Pic.rsDesc = p->rsDesc;
				Pic.rsMime = p->rsMime;
				Pic.bLink = (p->rsMime == "-->");
				if (Pic.bLink)
					Pic.varPic = StrX2W((PCSTR)p->rbData.Data(), (int)p->rbData.Size());
				else
					Pic.varPic = p->rbData;
				mi.vImage.emplace_back(std::move(Pic));
				mi.uMaskRead |= MIM_COVER;
			}
			else if ((mi.uMask & MIM_GENRE) && memcmp(e->Id, "TCON", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				if (!p->vText.empty())
				{
					mi.rsGenre = p->vText[0];
					mi.uMaskRead |= MIM_GENRE;
				}
			}
			else if ((mi.uMask & MIM_DATE) && memcmp(e->Id, "TYER", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				if (!p->vText.empty())
				{
					if (mi.uFlag & MIF_DATE_STRING)
						mi.Date = p->vText[0];
					else
						mi.Date = SYSTEMTIME{ .wYear = (WORD)_wtoi(p->vText[0].Data()) };
					mi.uMaskRead |= MIM_DATE;
				}
			}
			else if ((mi.uMask & MIM_DATE) && memcmp(e->Id, "TDRC", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				if (!p->vText.empty())
				{
					if (mi.uFlag & MIF_DATE_STRING)
					{
						mi.Date = p->vText[0];
						mi.uMaskRead |= MIM_DATE;
					}
					else
					{
						SYSTEMTIME st{};
						if (swscanf(p->vText[0].Data(), L"%hd-%hd-%hdT%hd:%hd:%hd",
							&st.wYear, &st.wMonth, &st.wDay,
							&st.wHour, &st.wMinute, &st.wSecond) > 0)
						{
							mi.Date = st;
							mi.uMaskRead |= MIM_DATE;
						}
					}
					mi.uMaskRead |= MIM_DATE;
				}
			}
		}
		return Result::Ok;
	}

	Result SimpleExtractMove(MUSICINFO& mi) override
	{
		mi.Clear();
		if (m_vFrame.empty())
			return Result::NoTag;
		for (const auto e : m_vFrame)
		{
			if ((mi.uMask & MIM_TITLE) && memcmp(e->Id, "TIT2", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				if (!p->vText.empty())
				{
					mi.rsTitle = std::move(p->vText[0]);
					mi.uMaskRead |= MIM_TITLE;
				}
			}
			else if ((mi.uMask & MIM_ARTIST) && memcmp(e->Id, "TPE1", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				for (auto& e : p->vText)
				{
					mi.AppendArtist(std::move(e));
					mi.uMaskRead |= MIM_ARTIST;
				}
			}
			else if ((mi.uMask & MIM_ALBUM) && memcmp(e->Id, "TALB", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				if (!p->vText.empty())
				{
					mi.rsAlbum = std::move(p->vText[0]);
					mi.uMaskRead |= MIM_ALBUM;
				}
			}
			else if ((mi.uMask & MIM_LRC) && memcmp(e->Id, "USLT", 4) == 0)
			{
				const auto p = DynCast<USLT*>(e);
				mi.rsLrc = std::move(p->rsLrc);
				mi.uMaskRead |= MIM_LRC;
			}
			else if ((mi.uMask & MIM_COMMENT) && memcmp(e->Id, "COMM", 4) == 0)
			{
				const auto p = DynCast<COMM*>(e);
				mi.AppendComment(std::move(p->rsText));
				mi.uMaskRead |= MIM_COMMENT;
			}
			else if ((mi.uMask & MIM_COVER) && memcmp(e->Id, "APIC", 4) == 0)
			{
				const auto p = DynCast<APIC*>(e);
				MUSICPIC Pic{};
				Pic.eType = p->eType;
				Pic.rsDesc = std::move(p->rsDesc);
				Pic.rsMime = std::move(p->rsMime);
				Pic.bLink = (Pic.rsMime == "-->");
				if (Pic.bLink)
					Pic.varPic = StrX2W((PCSTR)p->rbData.Data(), (int)p->rbData.Size());
				else
					Pic.varPic = std::move(p->rbData);
				mi.vImage.emplace_back(std::move(Pic));
				mi.uMaskRead |= MIM_COVER;
			}
			else if ((mi.uMask & MIM_GENRE) && memcmp(e->Id, "TCON", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				if (!p->vText.empty())
				{
					mi.rsGenre = std::move(p->vText[0]);
					mi.uMaskRead |= MIM_GENRE;
				}
			}
			else if ((mi.uMask & MIM_DATE) && memcmp(e->Id, "TYER", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				if (!p->vText.empty())
				{
					if (mi.uFlag & MIF_DATE_STRING)
						mi.Date = std::move(p->vText[0]);
					else
						mi.Date = SYSTEMTIME{ .wYear = (WORD)_wtoi(p->vText[0].Data()) };
					mi.uMaskRead |= MIM_DATE;
				}
			}
			else if ((mi.uMask & MIM_DATE) && memcmp(e->Id, "TDRC", 4) == 0)
			{
				const auto p = DynCast<TEXTFRAME*>(e);
				if (!p->vText.empty())
				{
					if (mi.uFlag & MIF_DATE_STRING)
					{
						mi.Date = std::move(p->vText[0]);
						mi.uMaskRead |= MIM_DATE;
					}
					else
					{
						SYSTEMTIME st{};
						if (swscanf(p->vText[0].Data(), L"%hd-%hd-%hdT%hd:%hd:%hd",
							&st.wYear, &st.wMonth, &st.wDay,
							&st.wHour, &st.wMinute, &st.wSecond) > 0)
						{
							mi.Date = st;
							mi.uMaskRead |= MIM_DATE;
						}
					}
					mi.uMaskRead |= MIM_DATE;
				}
			}
		}
		return Result::Ok;
	}

	Result ReadTag(UINT uFlags) override
	{
		Result r;
		if (!m_cbTag)
			if ((r = PreReadWrite()) != Result::Ok)
				return r;
		for (auto e : m_vFrame)
			delete e;
		m_vFrame.clear();
		m_SeekVal = SIZETMax;
		SIZE_T posActualEnd;
		if (m_File.m_Loc.posV2 != SIZETMax)
		{
			m_Stream.MoveTo(m_File.m_Loc.posV2 + 10u);
			r = ParseFrameBody(m_File.m_Loc.posV2 + m_cbTag, &posActualEnd);
			if (r != Result::Ok)
				return r;
			if (posActualEnd < m_File.m_Loc.posV2 + m_cbTag)// 可能有填充或追加标签
				m_cbPrependTag = (DWORD)(posActualEnd - m_File.m_Loc.posV2);
			if (m_cbPrependTag > m_cbTag)
			{
				EckDbgPrintWithPos(L"查找到的预置标签末尾超出标签头指示的长度");
				return Result::TagErr;
			}
			if (m_SeekVal != SIZETMax)// 若找到了SEEK帧，则移至其指示的位置继续解析，此时不可能含有空白填充
			{
				if (m_File.m_Loc.posV2 == SIZETMax || m_cbPrependTag == 0)
					return Result::TagErr;
				m_SeekVal += (m_cbPrependTag + m_File.m_Loc.posV2);
				if (m_SeekVal >= m_Stream.GetSizeUli() - 10)
				{
					EckDbgPrintWithPos(L"追加标签末尾超出文件长度");
					return Result::TagErr;
				}
				m_Stream.MoveTo(m_SeekVal);
				r = ParseFrameBody(m_SeekVal + m_cbTag);
			}
		}
		else if (m_File.m_Loc.posV2Footer != SIZETMax)
		{
			m_cbPrependTag = 0u;
			m_Stream.MoveTo(m_File.m_Loc.posV2Footer);
			r = ParseFrameBody(m_File.m_Loc.posV2Footer + m_cbTag);
		}
		else
			return Result::TagErr;
		return Result::Ok;
	}

	Result WriteTag(UINT uFlags) override
	{
		if (!m_cbTag)
			if (Result r; (r = PreReadWrite()) != Result::Ok)
				return r;
		const BOOL bOnlyAppend = m_File.m_Loc.posV2Footer != SIZETMax &&
			m_File.m_Loc.posV2 == SIZETMax;
		const BOOL bShouldAppend = (uFlags & MIF_APPEND_TAG);
		ID3v2_Header Hdr{ m_Header };
		if (uFlags & MIF_CREATE_ID3V2_3)
			Hdr.Ver = 3;
		else if (uFlags & MIF_CREATE_ID3V2_4)
			Hdr.Ver = 4;
		else if (Hdr.Ver != 3 && Hdr.Ver != 4)
			Hdr.Ver = 3;
		CRefBin rbPrepend{}, rbAppend{};
		BOOL bSeekFrameFound = FALSE;
		for (const auto e : m_vFrame)
		{
			if (!bSeekFrameFound && memcmp(e->Id, "SEEK", 4) == 0)
				bSeekFrameFound = TRUE;
			if (bShouldAppend || (e->byAddtFlags & MIIWF_APPEND_ID3V2_4))
			{
				e->SerializeData(rbAppend, &Hdr);
				Hdr.Ver = 4;
			}
			else
				e->SerializeData(rbPrepend, &Hdr);
		}

		const BOOL bAllowPadding = (uFlags & MIF_ALLOW_PADDING) &&
			!(!rbPrepend.IsEmpty() && !rbAppend.IsEmpty());

		const auto cbFrames = (DWORD)(rbPrepend.Size() + rbAppend.Size());
		DwordToSynchSafeInt(Hdr.Size, cbFrames);
		SIZE_T cbBody{ SIZETMax };
		SIZE_T dHdrFooterToEnd{ SIZETMax };
		//-----------------写入追加标签-----------------
		if (!rbAppend.IsEmpty())
		{
			if (m_File.m_Loc.posV2Footer != SIZETMax)
			{
				if (m_cbTag < rbAppend.Size())
				{
					m_Stream.Insert(ToUli(m_File.m_Loc.posV2Footer + m_cbTag),
						ToUli(rbAppend.Size() - m_cbTag));
				}
				else
				{
					const auto cbPadding = m_cbTag - rbAppend.Size();
					if (cbPadding)// 无论如何都不对追加标签使用填充
					{
						m_Stream.Erase(ToUli(m_File.m_Loc.posV2Footer + rbAppend.Size()),
							ToUli(cbPadding));
					}
				}
				m_Stream.MoveTo(m_File.m_Loc.posV2Footer);
			}
			else if (m_SeekVal != SIZETMax)
			{
				EckAssert(m_cbTag != SIZETMax);
				const auto cbOldAppend = m_cbTag - m_cbPrependTag;
				if (cbOldAppend < rbAppend.Size())
				{
					m_Stream.Insert(ToUli(m_SeekVal + cbOldAppend),
						ToUli(rbAppend.Size() - cbOldAppend));
				}
				else
				{
					const auto cbPadding = cbOldAppend - rbAppend.Size();
					if (cbPadding)// 无论如何都不对追加标签使用填充
					{
						m_Stream.Erase(ToUli(m_SeekVal + rbAppend.Size()),
							ToUli(cbPadding));
					}
				}
				m_Stream.MoveTo(m_SeekVal);
			}
			else
			{
				SIZE_T posInsert;
				if (m_File.m_Loc.posV1Ext != SIZETMax)
					posInsert = m_File.m_Loc.posV1Ext;
				else if (m_File.m_Loc.posV1 != SIZETMax)
					posInsert = m_File.m_Loc.posV1;
				else
					posInsert = m_Stream.GetSize();
				m_Stream.Insert(ToUli(posInsert), ToUli(rbAppend.Size() + 10));
				m_Stream.MoveTo(posInsert);
			}
			if (m_File.m_Loc.posV2 == SIZETMax)
				cbBody = m_Stream.GetPos();
			else
				cbBody = m_Stream.GetPos() - m_cbPrependTag - m_File.m_Loc.posV2 - 10;
			m_Stream << rbAppend;
			if (rbPrepend.IsEmpty())
			{
				DwordToSynchSafeInt(Hdr.Size, (DWORD)rbPrepend.Size());
				memcpy(Hdr.Header, "3DI", 3);
				m_Stream << Hdr;
			}
			else
				dHdrFooterToEnd = m_Stream.GetSize() - m_Stream.GetPos();
		}
		else if (m_File.m_Loc.posV2Footer != SIZETMax)// 删除先前的追加标签
		{
			m_Stream.Erase(ToUli(m_File.m_Loc.posV2Footer), ToUli(m_cbTag - m_cbPrependTag + 10));
		}

		//-----------------写入预置标签-----------------

		// 预置部分的长度，包含标签头，不含填充
		SIZE_T cbPrependTotal
		{
			rbPrepend.Size() +
			((rbPrepend.IsEmpty() || (uFlags & MIF_CREATE_ID3V2_EXT_HEADER)) ? 10 : 0)
		};

		if (cbPrependTotal)
		{
			if (!bSeekFrameFound && cbBody != SIZETMax)// 补下SEEK帧
			{
				SEEK seek{};
				seek.uFlags[0] |= (Hdr.Ver == 3 ?
					ID3V23FF_FILE_ALTER_DISCARD : ID3V24FF_FILE_ALTER_DISCARD);
				seek.ocbNextTag = (UINT)cbBody;
				seek.SerializeData(rbPrepend, &Hdr);
				cbPrependTotal += 14;
			}

			auto cbTotal = rbPrepend.Size() + rbAppend.Size();// 填充大小悬而未决
			BYTE byExtHdr[24];
			SIZE_T cbExtHdr{};
			UINT* pcbPaddingExtHdrV23{};
			if (uFlags & MIF_CREATE_ID3V2_EXT_HEADER)// XXX:暂不支持计算CRC
				if (Hdr.Ver == 3)
				{
					EckAssert(rbAppend.IsEmpty());
					CMemWalker w(byExtHdr, sizeof(byExtHdr));
					w << ReverseInteger(6u) << 0_us;
					w.SkipPointer(pcbPaddingExtHdrV23);// 填充大小悬而未决
					cbTotal += 6;
					cbExtHdr = 6;
				}
				else
				{
					CMemWalker w(byExtHdr, sizeof(byExtHdr));
					BYTE byFlags{};
					BYTE cbExt{};
					BYTE byRestrictions{};
					if (m_ExtHdrInfo.bTagAlter)
						byFlags |= ID3V24EH_UPDATE;
					if (m_ExtHdrInfo.bRestrictions)
					{
						byFlags |= ID3V24EH_RESTRICTIONS;
						++cbExt;
						byRestrictions |= ((BYTE)m_ExtHdrInfo.eTagSize << 6);
						byRestrictions |= ((BYTE)m_ExtHdrInfo.eTextEncoding << 5);
						byRestrictions |= ((BYTE)m_ExtHdrInfo.eTextFieldSize << 3);
						byRestrictions |= ((BYTE)m_ExtHdrInfo.eImageFormat << 2);
						byRestrictions |= ((BYTE)m_ExtHdrInfo.eImageSize);
					}
					cbExtHdr = cbExt + 6;
					DwordToSynchSafeInt(byExtHdr, (DWORD)cbExtHdr);
					w += 4;
					w << cbExt << byFlags;
					if (m_ExtHdrInfo.bRestrictions)
						w << byRestrictions;
					cbTotal += cbExtHdr;
				}

			SIZE_T cbPadding{};

			if (m_File.m_Loc.posV2 != SIZETMax)
			{
				const auto cbPrependOld = (m_SeekVal == SIZETMax ? m_cbTag : m_cbPrependTag);
				if (cbPrependOld < cbPrependTotal)
				{
					m_Stream.Insert(ToUli(m_File.m_Loc.posV2 + 10 + m_cbPrependTag),
						ToUli(rbPrepend.Size() - m_cbPrependTag));
				}
				else
				{
					if (cbPadding = cbPrependOld - cbPrependTotal)
					{
						if (bAllowPadding && cbPadding < 1024 && rbAppend.IsEmpty())
						{
							m_Stream.MoveTo(m_File.m_Loc.posV2 + 10 + rbPrepend.Size());
							void* p = VAlloc(cbPadding);
							EckCheckMem(p);
							m_Stream.Write(p, (ULONG)cbPadding);
							VFree(p);
						}
						else
						{
							m_Stream.Erase(ToUli(m_File.m_Loc.posV2 + 10 + rbPrepend.Size()),
								ToUli(cbPadding));
							cbPadding = 0u;
						}
					}
				}
				m_Stream.MoveTo(m_File.m_Loc.posV2);
			}
			else
			{
				m_Stream.Insert(0u, rbPrepend.Size() + 10u);
				m_Stream.MoveToBegin();
			}
			cbTotal += cbPadding;// cbTotal Completed
			if (pcbPaddingExtHdrV23)
				*pcbPaddingExtHdrV23 = ReverseInteger((UINT)cbPadding);// ExtHdr Padding Completed
			// 准备头
			memcpy(Hdr.Header, "ID3", 3);
			if (!rbAppend.IsEmpty())
				Hdr.Flags |= ID3V2HF_FOOTER;// 含标签尾
			DwordToSynchSafeInt(Hdr.Size, (DWORD)cbTotal);
			// 写入
			m_Stream << Hdr;
			if (cbExtHdr)
				m_Stream.Write(byExtHdr, cbExtHdr);
			m_Stream << rbPrepend;
			// 若标签尾写入挂起，完成之
			if (dHdrFooterToEnd != SIZETMax)
			{
				m_Stream->Seek(ToLi(dHdrFooterToEnd), STREAM_SEEK_END, NULL);
				memcpy(Hdr.Header, "3DI", 3);
				m_Stream << Hdr;
			}
		}
		else if (m_File.m_Loc.posV2 != SIZETMax)// 删除先前的预置标签
		{
			m_Stream.Erase(m_File.m_Loc.posV2, m_cbPrependTag + 10u);
		}
		m_Stream->Commit(STGC_DEFAULT);
		return Result::Ok;
	}

	void Reset() override
	{
		for (const auto e : m_vFrame)
			delete e;
		m_vFrame.clear();
		m_Header = {};
		m_ExtHdrInfo = {};
		m_cbTag = 0u;
		m_cbPrependTag = 0u;
		m_SeekVal = SIZETMax;
	}

	EckInline auto& GetFrameList() { return m_vFrame; }

	EckInline const auto& GetFrameList() const { return m_vFrame; }

	EckInline const auto& GetHeader() const { return m_Header; }

	FRAME* GetFrame(PCSTR Id) const
	{
		for (const auto e : m_vFrame)
		{
			if (memcmp(e->Id, Id, 4) == 0)
				return e;
		}
		return NULL;
	}

	std::vector<FRAME*> GetFrameList(PCSTR Id) const
	{
		std::vector<FRAME*> v{};
		for (const auto e : m_vFrame)
		{
			if (memcmp(e->Id, Id, 4) == 0)
				v.push_back(e);
		}
		return v;
	}

	FRAME* CreateFrame(PCSTR Id)
	{
		FRAME* p;
		if (memcmp(Id, "UFID", 4) == 0)
			p = new UFID{};
		else if (memcmp(Id, "TXXX", 4) == 0)
			p = new TXXX{};
		else if (memcmp(Id, "WXXX", 4) == 0)
			p = new WXXX{};
		else if (memcmp(Id, "MCID", 4) == 0)
			p = new MCID{};
		else if (memcmp(Id, "ETCO", 4) == 0)
			p = new ETCO{};
		else if (memcmp(Id, "MLLT", 4) == 0)
			p = new MLLT{};
		else if (memcmp(Id, "SYTC", 4) == 0)
			p = new SYTC{};
		else if (memcmp(Id, "USLT", 4) == 0)
			p = new USLT{};
		else if (memcmp(Id, "SYLT", 4) == 0)
			p = new SYLT{};
		else if (memcmp(Id, "COMM", 4) == 0)
			p = new COMM{};
		else if (memcmp(Id, "RVA2", 4) == 0)
			p = new RVA2{};
		else if (memcmp(Id, "EQU2", 4) == 0)
			p = new EQU2{};
		else if (memcmp(Id, "RVRB", 4) == 0)
			p = new RVRB{};
		else if (memcmp(Id, "APIC", 4) == 0)
			p = new APIC{};
		else if (memcmp(Id, "GEOB", 4) == 0)
			p = new GEOB{};
		else if (memcmp(Id, "PCNT", 4) == 0)
			p = new PCNT{};
		else if (memcmp(Id, "POPM", 4) == 0)
			p = new POPM{};
		else if (memcmp(Id, "RBUF", 4) == 0)
			p = new RBUF{};
		else if (memcmp(Id, "AENC", 4) == 0)
			p = new AENC{};
		else if (memcmp(Id, "LINK", 4) == 0)
			p = new LINK{};
		else if (memcmp(Id, "POSS", 4) == 0)
			p = new POSS{};
		else if (memcmp(Id, "USER", 4) == 0)
			p = new USER{};
		else if (memcmp(Id, "OWNE", 4) == 0)
			p = new OWNE{};
		else if (memcmp(Id, "COMR", 4) == 0)
			p = new COMR{};
		else if (memcmp(Id, "ENCR", 4) == 0)
			p = new ENCR{};
		else if (memcmp(Id, "GRID", 4) == 0)
			p = new GRID{};
		else if (memcmp(Id, "PRIV", 4) == 0)
			p = new PRIV{};
		else if (memcmp(Id, "SIGN", 4) == 0)
			p = new SIGN{};
		else if (memcmp(Id, "SEEK", 4) == 0)
			p = new SEEK{};
		else if (memcmp(Id, "ASPI", 4) == 0)
			p = new ASPI{};
		else if (memcmp(Id, "IPLS", 4) == 0)
			p = new IPLS{};
		else if (memcmp(Id, "RVAD", 4) == 0)
			p = new RVAD{};
		else if (memcmp(Id, "EQUA", 4) == 0)
			p = new EQUA{};
		else if (memcmp(Id, "RGAD", 4) == 0)
			p = new RGAD{};
		else if (memcmp(Id, "XRVA", 4) == 0)
			p = new XRVA{};
		else if (Id[0] == 'T')
			p = new TEXTFRAME(Id);
		else if (Id[0] == 'W')
			p = new LINKFRAME(Id);
		else
			p = new OTHERFRAME(Id);
		m_vFrame.push_back(p);
		return p;
	}

	EckInline FRAME* GetOrCreateFrame(PCSTR Id)
	{
		auto p = GetFrame(Id);
		if (p)
			return p;
		else
			return CreateFrame(Id);
	}

	EckInline auto& GetExtHdrInfo() { return m_ExtHdrInfo; }

	EckInline const auto& GetExtHdrInfo() const { return m_ExtHdrInfo; }
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END