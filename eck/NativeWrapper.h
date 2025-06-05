#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
EckInline void* VAlloc(SIZE_T cb, ULONG ulProtect)
{
	void* p{};
	(void)NtAllocateVirtualMemory(NtCurrentProcess(), &p, 0, &cb, MEM_COMMIT, ulProtect);
	return p;
}

EckInline void* VAlloc(SIZE_T cb)
{
	void* p{};
	(void)NtAllocateVirtualMemory(NtCurrentProcess(), &p, 0, &cb, MEM_COMMIT, PAGE_READWRITE);
	return p;
}

EckInline NTSTATUS VFree(void* p)
{
	SIZE_T cb{};
	return NtFreeVirtualMemory(NtCurrentProcess(), &p, &cb, MEM_RELEASE);
}

template<class T_ = void>
struct DelVA
{
	using T = T_;
	void operator()(T* p) { VFree(p); }
};

inline HANDLE NaOpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, UINT uProcessId)
{
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, nullptr, bInheritHandle ? OBJ_INHERIT : 0, nullptr, nullptr);
	CLIENT_ID cid{ ULongToHandle(uProcessId) };
	HANDLE hProcess;
	if (NT_SUCCESS(NtOpenProcess(&hProcess, dwDesiredAccess, &oa, &cid)))
		return hProcess;
	return nullptr;
}

/// <summary>
/// 打开文件
/// </summary>
/// <param name="pusFile">文件的NT路径</param>
/// <param name="dwAccess">CreateFileW --> dwAccess，注意CreateFileW总是追加SYNCHRONIZE | FILE_READ_ATTRIBUTES，考虑使用FILE_GENERIC_*</param>
/// <param name="dwShareMode">CreateFileW --> dwShareMode</param>
/// <param name="dwOptions">NtOpenFile --> OpenOptions</param>
/// <param name="pnts">错误码</param>
/// <param name="piost">IO状态</param>
/// <param name="bInheritHandle">句柄是否可继承</param>
/// <param name="hRootDirectory">根目录句柄</param>
/// <returns>成功返回文件句柄，失败返回INVALID_HANDLE_VALUE</returns>
inline HANDLE NaOpenFile(UNICODE_STRING* pusFile, DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions = 0u,
	NTSTATUS* pnts = nullptr, IO_STATUS_BLOCK* piost = nullptr, BOOL bInheritHandle = FALSE, HANDLE hRootDirectory = nullptr)
{
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, pusFile, OBJ_CASE_INSENSITIVE, hRootDirectory, nullptr);
	HANDLE hFile;
	NTSTATUS nts;
	IO_STATUS_BLOCK iost;
	nts = NtOpenFile(&hFile, dwAccess, &oa, piost ? piost : &iost, dwShareMode, dwOptions);
	if (pnts)
		*pnts = nts;
	if (NT_SUCCESS(nts))
		return hFile;
	return INVALID_HANDLE_VALUE;
}

/// <summary>
/// 打开文件。
/// 封装与CreateFileW相近的Native API调用，不支持打开控制台
/// </summary>
/// <param name="pszFile">文件路径</param>
/// <param name="dwAccess">CreateFileW --> dwAccess，注意CreateFileW总是追加SYNCHRONIZE | FILE_READ_ATTRIBUTES，考虑使用FILE_GENERIC_*</param>
/// <param name="dwShareMode">CreateFileW --> dwShareMode</param>
/// <param name="dwOptions">NtOpenFile --> OpenOptions</param>
/// <param name="pnts">错误码</param>
/// <param name="piost">IO状态</param>
/// <param name="bInheritHandle">句柄是否可继承</param>
/// <returns>成功返回文件句柄，失败返回INVALID_HANDLE_VALUE</returns>
inline HANDLE NaOpenFile(PCWSTR pszFile, DWORD dwAccess,
	DWORD dwShareMode, DWORD dwOptions = 0u,
	NTSTATUS* pnts = nullptr, IO_STATUS_BLOCK* piost = nullptr,
	BOOL bInheritHandle = FALSE, HANDLE hRootDirectory = nullptr)
{
	UNICODE_STRING usFile;
	if (hRootDirectory)
		RtlInitUnicodeString(&usFile, pszFile);
	else
		if (!RtlDosPathNameToNtPathName_U(pszFile, &usFile, nullptr, nullptr))
		{
			if (pnts)
				*pnts = STATUS_OBJECT_PATH_NOT_FOUND;
			return INVALID_HANDLE_VALUE;
		}
	const auto hFile = NaOpenFile(&usFile, dwAccess, dwShareMode, dwOptions,
		pnts, piost, bInheritHandle, hRootDirectory);
	if (!hRootDirectory)
		RtlFreeHeap(RtlProcessHeap(), 0, usFile.Buffer);
	return hFile;
}

/// <summary>
/// 打开文件
/// </summary>
/// <param name="pusFile">文件的NT路径</param>
/// <param name="dwAccess">CreateFileW --> dwAccess</param>
/// <param name="dwShareMode">CreateFileW --> dwShareMode</param>
/// <param name="dwOptions">NtCreateFile --> CreateOptions</param>
/// <param name="dwCreationDisposition">NtCreateFile --> CreateDisposition</param>
/// <param name="pnts">错误码</param>
/// <param name="piost">IO状态</param>
/// <param name="dwAttributes">若创建文件，则此参数指定文件属性</param>
/// <param name="cbInit">若创建文件，则此参数指定初始大小</param>
/// <param name="bInheritHandle">句柄是否可继承</param>
/// <param name="hRootDirectory">根目录句柄</param>
/// <returns>成功返回文件句柄，失败返回INVALID_HANDLE_VALUE</returns>
inline HANDLE NaCreateFile(UNICODE_STRING* pusFile, DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions,
	DWORD dwCreationDisposition, NTSTATUS* pnts = nullptr, IO_STATUS_BLOCK* piost = nullptr,
	DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL, ULONGLONG cbInit = 0ll,
	BOOL bInheritHandle = FALSE, HANDLE hRootDirectory = nullptr)
{
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, pusFile, OBJ_CASE_INSENSITIVE, hRootDirectory, nullptr);
	HANDLE hFile;
	NTSTATUS nts;
	IO_STATUS_BLOCK iost;
	LARGE_INTEGER li{ .QuadPart = (LONGLONG)cbInit };
	nts = NtCreateFile(&hFile, dwAccess, &oa, 
		piost ? piost : &iost, 
		cbInit ? &li : nullptr,
		dwAttributes, dwShareMode, dwCreationDisposition, dwOptions, nullptr, 0);
	if (pnts)
		*pnts = nts;
	if (NT_SUCCESS(nts))
		return hFile;
	return INVALID_HANDLE_VALUE;
}

/// <summary>
/// 打开文件。
/// 封装与CreateFileW相近的Native API调用，不支持打开控制台
/// </summary>
/// <param name="pszFile">文件路径</param>
/// <param name="dwAccess">CreateFileW --> dwAccess</param>
/// <param name="dwShareMode">CreateFileW --> dwShareMode</param>
/// <param name="dwOptions">NtCreateFile --> CreateOptions</param>
/// <param name="dwCreationDisposition">NtCreateFile --> CreateDisposition</param>
/// <param name="pnts">错误码</param>
/// <param name="piost">IO状态</param>
/// <param name="dwAttributes">若创建文件，则此参数指定文件属性</param>
/// <param name="cbInit">若创建文件，则此参数指定初始大小</param>
/// <param name="bInheritHandle">句柄是否可继承</param>
/// <returns>成功返回文件句柄，失败返回INVALID_HANDLE_VALUE</returns>
inline HANDLE NaCreateFile(PCWSTR pszFile, DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions,
	DWORD dwCreationDisposition, NTSTATUS* pnts = nullptr, IO_STATUS_BLOCK* piost = nullptr,
	DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL, ULONGLONG cbInit = 0ll,
	BOOL bInheritHandle = FALSE)
{
	UNICODE_STRING usFile;
	if (!RtlDosPathNameToNtPathName_U(pszFile, &usFile, nullptr, nullptr))
	{
		if (pnts)
			*pnts = STATUS_OBJECT_PATH_NOT_FOUND;
		return INVALID_HANDLE_VALUE;
	}
	const auto hFile = NaCreateFile(&usFile, dwAccess, dwShareMode, dwOptions,
		dwCreationDisposition, pnts, piost, dwAttributes, cbInit, bInheritHandle);
	RtlFreeHeap(RtlProcessHeap(), 0, usFile.Buffer);
	return hFile;
}

inline NTSTATUS NaDeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, PVOID pInBuf, DWORD cbInBuf,
	PVOID pOutBuf, DWORD cbOutBuf, DWORD* pcbReturned = nullptr)
{
	NTSTATUS nts;
	IO_STATUS_BLOCK iosb;
	if ((dwIoControlCode >> 16) == FILE_DEVICE_FILE_SYSTEM)
	{
		nts = NtFsControlFile(hDevice, nullptr, nullptr, nullptr, &iosb,
			dwIoControlCode, pInBuf, cbInBuf, pOutBuf, cbOutBuf);
	}
	else
	{
		nts = NtDeviceIoControlFile(hDevice, nullptr, nullptr, nullptr, &iosb,
			dwIoControlCode, pInBuf, cbInBuf, pOutBuf, cbOutBuf);
	}

	if (nts == STATUS_PENDING)
	{
		NtWaitForSingleObject(hDevice, FALSE, nullptr);
		nts = iosb.Status;
	}
	if (pcbReturned)
		*pcbReturned = (DWORD)iosb.Information;
	return nts;
}
ECK_NAMESPACE_END