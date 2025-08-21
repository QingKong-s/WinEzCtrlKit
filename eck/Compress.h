#pragma once
#include "CRefStr.h"
#include "CRefBin.h"
#include "AutoPtrDef.h"

#include "ZLib/zlib.h"

ECK_NAMESPACE_BEGIN
EckInlineNdCe BOOL ZLibSuccess(int iRet)
{
	return iRet == Z_OK || iRet == Z_STREAM_END;
}

inline int ZLibDecompress(_In_reads_bytes_(cbOrg) PCVOID pOrg,
	SIZE_T cbOrg, CRefBin& rbResult)
{
	int iRet;
	z_stream zs{};
	if ((iRet = inflateInit(&zs)) != Z_OK)
		return iRet;
	zs.next_in = (Bytef*)pOrg;
	zs.avail_in = (uInt)cbOrg;

	rbResult.ExtendToCapacity();
	if (rbResult.IsEmpty())
		rbResult.ReSize(cbOrg);
	zs.next_out = rbResult.Data();
	zs.avail_out = (uInt)rbResult.Size();
	EckLoop()
	{
		iRet = inflate(&zs, Z_NO_FLUSH);
		if (iRet == Z_STREAM_END)// 解压完成
		{
			rbResult.PopBack(zs.avail_out);
			break;
		}
		else if (iRet == Z_BUF_ERROR)// 缓冲区不足
		{
			rbResult.PopBack(zs.avail_out);
			zs.next_out = rbResult.PushBackNoExtra(cbOrg / 2);
			zs.avail_out = (uInt)(cbOrg / 2);
		}
		else if (iRet != Z_OK)// 出错
		{
			rbResult.Clear();
			break;
		}
	}
	inflateEnd(&zs);
	return iRet;
}

inline int ZLibCompress(_In_reads_bytes_(cbOrg) PCVOID pOrg,
	SIZE_T cbOrg, CRefBin& rbResult, int iLevel = Z_DEFAULT_COMPRESSION)
{
	int iRet;
	z_stream zs{};
	if ((iRet = deflateInit(&zs, iLevel)) != Z_OK)
		return iRet;
	zs.next_in = (Bytef*)pOrg;
	zs.avail_in = (uInt)cbOrg;

	rbResult.ExtendToCapacity();
	if (rbResult.IsEmpty())
		rbResult.ReSize(cbOrg * 2 / 3);
	zs.next_out = rbResult.Data();
	zs.avail_out = (uInt)rbResult.Size();
	EckLoop()
	{
		iRet = deflate(&zs, Z_FINISH);
		if (iRet == Z_STREAM_END)// 压缩完成
		{
			rbResult.PopBack(zs.avail_out);
			break;
		}
		else if (iRet == Z_BUF_ERROR)// 缓冲区不足
		{
			rbResult.PopBack(zs.avail_out);
			zs.next_out = rbResult.PushBackNoExtra(cbOrg / 3);
			zs.avail_out = (uInt)(cbOrg / 3);
		}
		else if (iRet != Z_OK)// 出错
		{
			rbResult.Clear();
			break;
		}
	}
	deflateEnd(&zs);
	return iRet;
}

inline int GZipDecompress(_In_reads_bytes_(cbOrg) PCVOID pOrg,
	SIZE_T cbOrg, CRefBin& rbResult)
{
	int iRet;
	z_stream zs{};
	if ((iRet = inflateInit2(&zs, 16 + MAX_WBITS)) != Z_OK)
		return iRet;
	zs.next_in = (Bytef*)pOrg;
	zs.avail_in = (uInt)cbOrg;

	rbResult.ExtendToCapacity();
	if (rbResult.IsEmpty())
		rbResult.ReSize(cbOrg);
	zs.next_out = rbResult.Data();
	zs.avail_out = (uInt)rbResult.Size();
	EckLoop()
	{
		iRet = inflate(&zs, Z_NO_FLUSH);
		if (iRet == Z_STREAM_END)// 解压完成
		{
			rbResult.PopBack(zs.avail_out);
			break;
		}
		else if (iRet == Z_BUF_ERROR)// 缓冲区不足
		{
			rbResult.PopBack(zs.avail_out);
			zs.next_out = rbResult.PushBackNoExtra(cbOrg / 2);
			zs.avail_out = (uInt)(cbOrg / 2);
		}
		else if (iRet != Z_OK)// 出错
		{
			rbResult.Clear();
			break;
		}
	}
	inflateEnd(&zs);
	return iRet;
}

inline int GZipCompress(_In_reads_bytes_(cbOrg) PCVOID pOrg,
	SIZE_T cbOrg, CRefBin& rbResult, int iLevel = Z_DEFAULT_COMPRESSION)
{
	int iRet;
	z_stream zs{};
	if ((iRet = deflateInit2(&zs, iLevel, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY)) != Z_OK)
		return iRet;
	zs.next_in = (Bytef*)pOrg;
	zs.avail_in = (uInt)cbOrg;

	rbResult.ExtendToCapacity();
	if (rbResult.IsEmpty())
		rbResult.ReSize(cbOrg * 2 / 3);
	zs.next_out = rbResult.Data();
	zs.avail_out = (uInt)rbResult.Size();
	EckLoop()
	{
		iRet = deflate(&zs, Z_FINISH);
		if (iRet == Z_STREAM_END)// 压缩完成
		{
			rbResult.PopBack(zs.avail_out);
			break;
		}
		else if (iRet == Z_BUF_ERROR)// 缓冲区不足
		{
			rbResult.PopBack(zs.avail_out);

			zs.next_out = rbResult.PushBackNoExtra(cbOrg / 3);
			zs.avail_out = (uInt)(cbOrg / 3);
		}
		else if (iRet != Z_OK)// 出错
		{
			rbResult.Clear();
			break;
		}
	}
	deflateEnd(&zs);
	return iRet;
}

struct NTCOM_HDR
{
	BYTE byMagic[4];
	UINT cbOrg;
	USHORT usEngine;
	USHORT usReserved;
};

/// <summary>
/// NT压缩。
/// 函数将在压缩结果前附加NTCOM_HDR头
/// </summary>
/// <param name="pOrg">原始数据</param>
/// <param name="cbOrg">原始数据长度</param>
/// <param name="rbResult">压缩结果</param>
/// <param name="uEngine">选项</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS Compress(_In_reads_bytes_(cbOrg) PCVOID pOrg, SIZE_T cbOrg,
	CRefBin& rbResult, USHORT uEngine = COMPRESSION_FORMAT_LZNT1)
{
	NTSTATUS nts;
	ULONG cbRequired, Dummy;
	if (!NT_SUCCESS(nts = RtlGetCompressionWorkSpaceSize(uEngine, &cbRequired, &Dummy)))
		return nts;
	UnqPtrMA<void> pWorkSpace{ malloc(cbRequired) };
	rbResult.ExtendToCapacity();
	if (rbResult.Size() < sizeof(NTCOM_HDR))
		rbResult.ReSize(sizeof(NTCOM_HDR) + cbOrg * 2 / 3);

	NTCOM_HDR Hdr;
	memcpy(Hdr.byMagic, "EKNC", 4);
	Hdr.cbOrg = (UINT)cbOrg;
	Hdr.usEngine = uEngine;
	Hdr.usReserved = 0;

	EckLoop()
	{
		nts = RtlCompressBuffer(uEngine, (UCHAR*)pOrg, (ULONG)cbOrg,
			rbResult.Data() + sizeof(NTCOM_HDR), (ULONG)rbResult.Size() - sizeof(NTCOM_HDR),
			4096, &cbRequired, pWorkSpace.get());
		switch (nts)
		{
		case STATUS_SUCCESS:
			rbResult.ReSize(cbRequired + sizeof(NTCOM_HDR));
			memcpy(rbResult.Data(), &Hdr, sizeof(NTCOM_HDR));
			return STATUS_SUCCESS;
		case STATUS_BUFFER_TOO_SMALL:
			rbResult.PushBackNoExtra(cbOrg / 3);
			break;
		default:
			rbResult.Clear();
			return nts;
		}
	}
	ECK_UNREACHABLE;
}

/// <summary>
/// NT解压。
/// 函数能且只能解压由Compress函数压缩的结果
/// </summary>
/// <param name="pOrg">已压缩数据</param>
/// <param name="cbOrg">已压缩数据长度</param>
/// <param name="rbResult">解压结果</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS Decompress(_In_reads_bytes_(cbOrg) PCVOID pOrg,
	SIZE_T cbOrg, CRefBin& rbResult)
{
	if (cbOrg < sizeof(NTCOM_HDR))
		return STATUS_UNSUPPORTED_COMPRESSION;
	const auto* const pHdr = (NTCOM_HDR*)pOrg;
	if (memcmp(pHdr->byMagic, "EKNC", 4) != 0)
		return STATUS_UNSUPPORTED_COMPRESSION;
	ULONG Dummy;
	rbResult.ReSize(pHdr->cbOrg);
	return RtlDecompressBuffer(pHdr->usEngine,
		rbResult.PushBackNoExtra(pHdr->cbOrg),
		(ULONG)pHdr->cbOrg,
		(UCHAR*)pOrg + sizeof(NTCOM_HDR),
		(ULONG)cbOrg - sizeof(NTCOM_HDR), &Dummy);
}
ECK_NAMESPACE_END