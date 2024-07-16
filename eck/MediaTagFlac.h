/*
* WinEzCtrlKit Library
*
* MediaTagFlac.h ： Flac注释读写
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "MediaTag.h"

ECK_NAMESPACE_BEGIN
ECK_MEDIATAG_NAMESPACE_BEGIN
class CFlac
{
public:
	struct ITEM
	{
		CRefStrA rsKey;
		CRefStrW rsValue;
	};

	enum class BlockType : BYTE
	{
		StreamInfo = 0,
		Padding = 1,
		Application = 2,
		SeekTable = 3,
		VorbisComment = 4,
		CueSheet = 5,
		Picture = 6,
		Invalid = 127,
	};

	struct BLOCK
	{
		BlockType eType;
		CRefBin rbData;
	};

	struct STREAMINFO
	{
		USHORT cBlockSampleMin;
		USHORT cBlockSampleMax;
		DWORD cbMinFrame;
		DWORD cbMaxFrame;
		DWORD cSampleRate;
		BYTE cChannels;
		BYTE cBitsPerSample;
		ULONGLONG ullTotalSamples;
		BYTE Md5[16];
	};
private:
	CMediaFile& m_File;
	CStreamWalker m_Stream{};

	std::vector<ITEM> m_vItem{};	// 所有Vorbis注释
	std::vector<MUSICPIC> m_vPic{};	// 所有图片
	std::vector<BLOCK> m_vBlock{};	// 其他块，STREAMINFO、Vorbis注释、图片和填充除外
	STREAMINFO m_si{};				// Flac流信息

	CRefStrW m_rsVendor{};

	static void ParseImageBlock(CRefBin& rb, MUSICPIC& Pic)
	{
		CMemWalker r(rb.Data(), rb.Size());
		DWORD t;
		DWORD dwType;
		r >> dwType;// 图片类型
		if (dwType < (BYTE)PicType::Begin___ || dwType >= (BYTE)PicType::End___)
			Pic.eType = PicType::Invalid;
		else
			Pic.eType = (PicType)dwType;

		r >> t;// 长度
		t = ReverseInteger(t);
		CRefStrA rsMime(t);
		r.Read(rsMime.Data(), t);// MIME类型字符串
		Pic.rsMime = StrX2W(rsMime.Data(), rsMime.Size());

		r >> t;// 描述字符串长度
		t = ReverseInteger(t);
		CRefStrA u8Desc(t);
		r.Read(u8Desc.Data(), t);// MIME类型字符串
		Pic.rsDesc = StrX2W(u8Desc.Data(), u8Desc.Size(), CP_UTF8);

		r += 16;// 跳过宽度、高度、色深、索引图颜色数

		r >> t;// 图片数据长度
		t = ReverseInteger(t);// 图片数据长度

		Pic.bLink = (Pic.rsMime == L"-->");

		if (Pic.bLink)
		{
			CRefStrA u8(t);
			r.Read(u8.Data(), t);
			Pic.varPic = StrX2W(u8.Data(), u8.Size(), CP_UTF8);
		}
		else
		{
			rb.Erase(0, r.Data() - rb.Data());
			Pic.varPic = std::move(rb);
		}
	}
public:
	ECK_DISABLE_COPY_MOVE(CFlac)
public:
	CFlac(CMediaFile& File) :m_File{ File }, m_Stream(File.GetStream())
	{
		m_Stream.GetStream()->AddRef();
	}

	~CFlac() { m_Stream.GetStream()->Release(); }

	Result SimpleExtract(MUSICINFO& mi)
	{
		for (const auto& e : m_vItem)
		{
			if ((mi.uFlag & MIM_TITLE) && e.rsKey == "TITLE")
			{
				mi.rsTitle = e.rsValue;
				mi.uMaskRead |= MIM_TITLE;
			}
			else if ((mi.uFlag & MIM_ARTIST) && e.rsKey == "ARTIST")
			{
				mi.AppendArtist(e.rsValue);
				mi.uMaskRead |= MIM_ARTIST;
			}
			else if ((mi.uFlag & MIM_ALBUM) && e.rsKey == "ALBUM")
			{
				mi.rsAlbum = e.rsValue;
				mi.uMaskRead |= MIM_ALBUM;
			}
			else if ((mi.uFlag & MIM_LRC) && e.rsKey == "LYRICS")
			{
				mi.rsLrc = e.rsValue;
				mi.uMaskRead |= MIM_LRC;
			}
			else if ((mi.uFlag & MIM_COMMENT) && e.rsKey == "DESCRIPTION")
			{
				mi.AppendComment(e.rsValue);
				mi.uMaskRead |= MIM_COMMENT;
			}
			else if ((mi.uFlag & MIM_GENRE) && e.rsKey == "GENRE")
			{
				mi.rsGenre = e.rsValue;
				mi.uMaskRead |= MIM_GENRE;
			}
			else if ((mi.uFlag & MIM_DATE) && e.rsKey == "DATE")
			{
				WORD y, m{}, d{};
				if (swscanf(e.rsValue.Data(), L"%hd-%hd-%hd", &y, &m, &d) >= 1)
				{
					mi.Date = SYSTEMTIME{ .wYear = y,.wMonth = m,.wDay = d };
					mi.uMaskRead |= MIM_DATE;
				}
			}
		}
		if ((mi.uFlag & MIM_COVER) && !m_vPic.empty())
		{
			for (const auto& e : m_vPic)
				mi.vImage.emplace_back(e);
			mi.uMaskRead |= MIM_COVER;
		}
	}

	Result ReadTag(UINT uFlags)
	{
		m_Stream.MoveToBegin() += 4;
		FLAC_BlockHeader Header;
		DWORD cbBlock;
		UINT t;
		do
		{
			m_Stream >> Header;
			cbBlock = Header.bySize[2] | Header.bySize[1] << 8 | Header.bySize[0] << 16;
			if (cbBlock <= 0)
				return Result::LenErr;
			switch (Header.by & 0x7F)
			{
			case 0:// 流信息
			{
				m_Stream >> m_si.cBlockSampleMin >> m_si.cBlockSampleMax;
				m_Stream.ReadRev(&m_si.cbMinFrame, 3).Read(&m_si.cbMaxFrame, 3);
				ULONGLONG ull;
				m_Stream >> ull;
				m_si.cSampleRate = (UINT)GetLowNBits(ull, 20);
				m_si.cChannels = (BYTE)GetLowNBits(ull >> 20, 3) + 1;
				m_si.cBitsPerSample = (BYTE)GetLowNBits(ull >> 23, 5) + 1;
				m_si.ullTotalSamples = GetHighNBits(ull, 36);
				m_Stream.Read(m_si.Md5, 16);
			}
			break;
			case 4:// 标签信息，注意：这一部分是小端序
			{
				m_Stream >> t;// 编码器信息大小
				CRefStrA u8(t);
				m_Stream.Read(u8.Data(), t);

				UINT cItem;
				m_Stream >> cItem;// 标签数量

				EckCounterNV(cItem)
				{
					m_Stream >> t;// 标签大小
					u8.ReSize(t);
					m_Stream.Read(u8.Data(), t);// 读标签
					int iPos = u8.Find("=");// 找等号
					if (iPos == StrNPos || iPos == 0 || iPos == u8.Size() - 1)
						continue;
					++iPos;
					const int cchActual = u8.Size() - iPos;
					if (u8.IsStartOf("METADATA_BLOCK_PICTURE"))
					{
						auto rb = Base64Decode(u8.Data() + iPos, cchActual);
						MUSICPIC Pic{};
						ParseImageBlock(rb, Pic);
						m_vPic.emplace_back(std::move(Pic));
					}
					else ECKLIKELY
					{
						m_vItem.emplace_back(
							u8.SubStr(0, iPos - 1),
							StrX2W(u8.Data() + iPos, cchActual, CP_UTF8));
					}
				}
			}
			break;
			case 6:// 图片（大端序）
			{
				CRefBin rb(cbBlock);
				m_Stream.Read(rb.Data(), cbBlock);
				MUSICPIC Pic{};
				ParseImageBlock(rb, Pic);
				m_vPic.emplace_back(std::move(Pic));
			}
			break;
			default:
			{
				CRefBin rb(cbBlock);
				m_Stream.Read(rb.Data(), cbBlock);
				m_vBlock.emplace_back((BlockType)(Header.by & 0x7F), std::move(rb));
			}
			break;
			}

		} while (!(Header.by & 0x80));// 检查最高位，判断是不是最后一个块
		return Result::Ok;
	}

	Result WriteTag(UINT uFlags)
	{

	}
};
ECK_MEDIATAG_NAMESPACE_END
ECK_NAMESPACE_END