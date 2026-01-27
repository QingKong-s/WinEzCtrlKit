#pragma once
#include "Utility2.h"
#include "Crc.h"
#include "NativeWrapper.h"

ECK_NAMESPACE_BEGIN
using GETVERSIONOUTPARAMS = GETVERSIONINPARAMS;

#define IDE_ATAPI_IDENTIFY				0xA1
#define IDE_ATA_IDENTIFY				0xEC

#define FILE_DEVICE_SCSI				0x0000001b
#define IOCTL_SCSI_MINIPORT_IDENTIFY	((FILE_DEVICE_SCSI << 16) + 0x0501)
#define IOCTL_SCSI_MINIPORT				0x0004D008

#pragma pack(push, 1)
struct IDENTIFY_DATA
{
    USHORT GeneralConfiguration;
    USHORT NumberOfCylinders;
    USHORT Reserved1;
    USHORT NumberOfHeads;
    USHORT UnformattedBytesPerTrack;
    USHORT UnformattedBytesPerSector;
    USHORT SectorsPerTrack;
    USHORT VendorUnique1[3];
    USHORT SerialNumber[10];
    USHORT BufferType;
    USHORT BufferSectorSize;
    USHORT NumberOfEccBytes;
    USHORT FirmwareRevision[4];
    USHORT ModelNumber[20];
    UCHAR  MaximumBlockTransfer;
    UCHAR  VendorUnique2;
    USHORT DoubleWordIo;
    USHORT Capabilities;
    USHORT Reserved2;
    UCHAR  VendorUnique3;
    UCHAR  PioCycleTimingMode;
    UCHAR  VendorUnique4;
    UCHAR  DmaCycleTimingMode;
    USHORT TranslationFieldsValid : 1;
    USHORT Reserved3 : 15;
    USHORT NumberOfCurrentCylinders;
    USHORT NumberOfCurrentHeads;
    USHORT CurrentSectorsPerTrack;
    ULONG  CurrentSectorCapacity;
    USHORT CurrentMultiSectorSetting;
    ULONG  UserAddressableSectors;
    USHORT SingleWordDMASupport : 8;
    USHORT SingleWordDMAActive : 8;
    USHORT MultiWordDMASupport : 8;
    USHORT MultiWordDMAActive : 8;
    USHORT AdvancedPIOModes : 8;
    USHORT Reserved4 : 8;
    USHORT MinimumMWXferCycleTime;
    USHORT RecommendedMWXferCycleTime;
    USHORT MinimumPIOCycleTime;
    USHORT MinimumPIOCycleTimeIORDY;
    USHORT Reserved5[2];
    USHORT ReleaseTimeOverlapped;
    USHORT ReleaseTimeServiceCommand;
    USHORT MajorRevision;
    USHORT MinorRevision;
    USHORT Reserved6[50];
    USHORT SpecialFunctionsEnabled;
    USHORT Reserved7[128];
};
#pragma pack(pop)

struct SRB_IO_CONTROL
{
    ULONG HeaderLength;
    UCHAR Signature[8];
    ULONG Timeout;
    ULONG ControlCode;
    ULONG ReturnCode;
    ULONG Length;
};

namespace Priv
{
    inline constexpr UINT CalculateDriveIdentifier(const IDENTIFY_DATA* pidd) noexcept
    {
        UINT s{};
        for (const auto e : pidd->ModelNumber)
            s += e;
        for (const auto e : pidd->FirmwareRevision)
            s += e;
        for (const auto e : pidd->SerialNumber)
            s += e;
        const UINT t = pidd->BufferSectorSize + pidd->SectorsPerTrack +
            pidd->NumberOfHeads + pidd->NumberOfCylinders;
        const ULONGLONG r = t * 65536ull + s;
        if (r <= 0xFFFF'FFFFull)
            return (UINT)r;
        else
            return ((t - 1) % 65535 + 1) * 65536 + s % 65535;
    }
}

using FGetDiskId = NTSTATUS(*)(PCVOID pData, size_t cbData, BOOL bIdentifyData);

/// <summary>
/// 取硬盘特征字
/// </summary>
/// <param name="fnProcessData">处理数据回调</param>
/// <param name="idxDrive">物理硬盘索引</param>
/// <returns>NTSTATUS</returns>
template<class F>
inline NTSTATUS IntGetPhysicalDriveIdentifier(F fnProcessData, int idxDrive) noexcept
{
    NTSTATUS nts;
    WCHAR szDevice[48];
    swprintf(szDevice, LR"(\\.\PhysicalDrive%d)", idxDrive);
    HANDLE hDevice = NaOpenFile(szDevice,
        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (hDevice != INVALID_HANDLE_VALUE)
    {
        GETVERSIONOUTPARAMS gvop{};
        if (NT_SUCCESS(nts = NaDeviceIoControl(hDevice, SMART_GET_VERSION,
            nullptr, 0, &gvop, sizeof(gvop))))
        {
            SENDCMDINPARAMS scip
            {
                .cBufferSize = 512,
                .irDriveRegs =
                {
                    .bSectorCountReg = 1,
                    .bSectorNumberReg = 1,
                    .bDriveHeadReg = BYTE(0xA0 | ((idxDrive & 1) << 4)),
                    .bCommandReg = BYTE((gvop.bIDEDeviceMap >> idxDrive & 0x10) ?
                            IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY)
                },
                .bDriveNumber = (BYTE)idxDrive
            };

            BYTE byDummy2[sizeof(SENDCMDOUTPARAMS) - 1/*bBuffer*/ + 512]{};
            const auto pscop = (SENDCMDOUTPARAMS*)byDummy2;

            if (NT_SUCCESS(nts = NaDeviceIoControl(hDevice, SMART_RCV_DRIVE_DATA,
                &scip, sizeof(scip) - 1/*bBuffer*/, pscop, sizeof(SENDCMDOUTPARAMS))))
            {
                NtClose(hDevice);
                return fnProcessData(pscop->bBuffer, sizeof(IDENTIFY_DATA), TRUE);
            }
        }
        NtClose(hDevice);
    }

    swprintf(szDevice, LR"(\\.\SCSI%d:)", idxDrive);
    hDevice = NaOpenFile(szDevice,
        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (hDevice != INVALID_HANDLE_VALUE)
    {
        constexpr size_t cbIn = sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1/*bBuffer*/;
        BYTE byDummy1[cbIn]{};
        const auto psrbic = (SRB_IO_CONTROL*)byDummy1;
        const auto pscip = (SENDCMDINPARAMS*)(psrbic + 1);
        ZeroMemory(psrbic, cbIn);

        psrbic->HeaderLength = sizeof(SRB_IO_CONTROL);
        memcpy(psrbic->Signature, "SCSIDISK", 8);
        psrbic->Timeout = 200;
        psrbic->Length = sizeof(SENDCMDOUTPARAMS) - 1/*bBuffer*/ + 512;
        psrbic->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;

        pscip->cBufferSize = 512;
        pscip->irDriveRegs.bSectorCountReg = 1;
        pscip->irDriveRegs.bSectorNumberReg = 1;
        pscip->irDriveRegs.bDriveHeadReg = BYTE(0xA0 | ((idxDrive & 1) << 4)),
            pscip->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
        pscip->bDriveNumber = idxDrive;

        constexpr size_t cbOut = sizeof(SRB_IO_CONTROL) +
            sizeof(SENDCMDOUTPARAMS) - 1/*bBuffer*/ + 512;
        BYTE byDummy2[cbOut]{};

        const auto pscop = ((SENDCMDOUTPARAMS*)(byDummy2 + sizeof(SRB_IO_CONTROL)));

        if (NT_SUCCESS(nts = NaDeviceIoControl(hDevice, IOCTL_SCSI_MINIPORT,
            psrbic, cbIn, byDummy2, cbOut)))
        {
            NtClose(hDevice);
            return fnProcessData(pscop->bBuffer, sizeof(IDENTIFY_DATA), FALSE);
        }
        NtClose(hDevice);
    }

    swprintf(szDevice, LR"(\\.\PhysicalDrive%d)", idxDrive);
    hDevice = NaOpenFile(szDevice,
        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (hDevice != INVALID_HANDLE_VALUE)
    {
        STORAGE_PROPERTY_QUERY spq
        {
            .PropertyId = StorageDeviceProperty,
            .QueryType = PropertyStandardQuery
        };

        constexpr DWORD cbBuf = 4096;
        UniquePtr<DelVA<void>> pBuf(VAlloc(cbBuf));
        DWORD cbRet{};
        if (NT_SUCCESS(nts = NaDeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
            &spq, sizeof(spq), pBuf.get(), cbBuf, &cbRet)))
        {
            if (cbRet)
            {
                NtClose(hDevice);
                return fnProcessData(pBuf.get(), cbRet, FALSE);
            }
        }
        NtClose(hDevice);
    }
    return STATUS_ACCESS_DENIED;
}

/// <summary>
/// 取硬盘特征字
/// </summary>
/// <param name="idxDrive">物理硬盘索引</param>
/// <param name="uCrc32">返回CRC32</param>
/// <returns>NTSTATUS</returns>
EckInline NTSTATUS GetPhysicalDriveIdentifierCrc32(int idxDrive, UINT& uCrc32)
{
    return IntGetPhysicalDriveIdentifier([&uCrc32](PCVOID pData, size_t cbData, BOOL)->NTSTATUS
        {
            uCrc32 = CalculateCrc32(pData, cbData);
            return STATUS_SUCCESS;
        }, idxDrive);
}

/// <summary>
/// 取硬盘特征字
/// </summary>
/// <param name="idxDrive">物理硬盘索引</param>
/// <param name="pMd5">返回MD5，至少16字节</param>
/// <returns>NTSTATUS</returns>
EckInline NTSTATUS GetPhysicalDriveIdentifierMd5(int idxDrive, _Out_writes_bytes_(16) void* pMd5)
{
    return IntGetPhysicalDriveIdentifier([pMd5](PCVOID pData, size_t cbData, BOOL)->NTSTATUS
        {
            return CalculateMd5(pData, cbData, pMd5);
        }, idxDrive);
}
ECK_NAMESPACE_END