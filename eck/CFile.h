#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CFile
{
public:
	enum Mode
	{
		ModeCover = CREATE_ALWAYS,				// 若文件已存在则清除其数据，若不存在则创建
		ModeNoCover = OPEN_ALWAYS,				// 若文件已存在则打开，若不存在则创建
		ModeNew = CREATE_NEW,					// 若文件已存在则失败，若不存在则创建
		ModeExisting = OPEN_EXISTING,			// 若文件已存在则打开，若不存在则失败
		ModeCoverExisting = TRUNCATE_EXISTING,	// 若文件已存在则清除其数据，若不存在则失败
	};
private:
	HANDLE m_hFile = INVALID_HANDLE_VALUE;
public:
	~CFile()
	{
		Close();
	}

	EckInline void Close()
	{
		if (m_hFile != INVALID_HANDLE_VALUE && m_hFile)
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
	}

	EckInline HANDLE Open(PCWSTR pszFile, DWORD dwMode = OPEN_EXISTING, DWORD dwAccess = GENERIC_READ,
		DWORD dwShareMode = 0, DWORD dwAttr = FILE_ATTRIBUTE_NORMAL)
	{
		Close();
		return (m_hFile = CreateFileW(pszFile, dwAccess, dwShareMode, NULL, dwMode, dwAttr, NULL));
	}

	EckInline HANDLE GetHandle()
	{
		return m_hFile;
	}

	EckInline LONGLONG GetSize()
	{
		LARGE_INTEGER lint;
		GetFileSizeEx(m_hFile, &lint);
		return lint.QuadPart;
	}

	EckInline DWORD GetSize32()
	{
		return GetFileSize(m_hFile, NULL);
	}

	EckInline BOOL Read(void* pBuf, DWORD dwSize, DWORD* pdwRead = NULL)
	{
		DWORD dw;
		if (!pdwRead)
			pdwRead = &dw;
		return ReadFile(m_hFile, pBuf, dwSize, pdwRead, NULL);
	}

	EckInline BOOL Write(PCVOID pBuf, DWORD dwSize, DWORD* pdwWritten = NULL)
	{
		DWORD dw;
		if (!pdwWritten)
			pdwWritten = &dw;
		return WriteFile(m_hFile, pBuf, dwSize, pdwWritten, NULL);
	}

	EckInline BOOL End()
	{
		return SetEndOfFile(m_hFile);
	}

	template<class T>
	EckInline CFile& operator>>(T& Buf)
	{
		Read(&Buf, sizeof(T));
		return *this;
	}

	template<class T>
	EckInline CFile& operator<<(T& Buf)
	{
		Write(&Buf, sizeof(T));
		return *this;
	}

	EckInline CFile& operator+=(LONG l)
	{
		SetFilePointer(m_hFile, l, NULL, FILE_CURRENT);
		return *this;
	}

	EckInline CFile& operator-=(LONG l)
	{
		SetFilePointer(m_hFile, -l, NULL, FILE_CURRENT);
		return *this;
	}

	EckInline CFile& MoveToBegin()
	{
		SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);
		return *this;
	}

	EckInline CFile& MoveToEnd()
	{
		SetFilePointer(m_hFile, 0, NULL, FILE_END);
		return *this;
	}

	EckInline CFile& MoveTo(LONG l)
	{
		SetFilePointer(m_hFile, l, NULL, FILE_BEGIN);
		return *this;
	}

	EckInline DWORD GetCurrPos()
	{
		return SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
	}

	EckInline void Flush()
	{
		FlushFileBuffers(m_hFile);
	}
};

class CMappingFile
{
private:
	CFile m_File{};
	HANDLE m_hMapping = NULL;
	void* m_pFile = NULL;
public:
	~CMappingFile()
	{
		Close();
	}

	void Close();

	void* Open(PCWSTR pszFile, DWORD dwMap = FILE_MAP_READ, DWORD dwProtect = PAGE_READONLY, DWORD dwMode = OPEN_EXISTING,
		DWORD dwAccess = GENERIC_READ, DWORD dwShareMode = 0, DWORD dwAttr = FILE_ATTRIBUTE_NORMAL);

	EckInline CFile& GetFile()
	{
		return m_File;
	}

	EckInline HANDLE GetHMapping()
	{
		return m_hMapping;
	}

	EckInline void* GetPFile()
	{
		return m_pFile;
	}
};

class CMappingFile2
{
private:
	CFile& m_File;
	HANDLE m_hMapping = NULL;
	void* m_pFile = NULL;
public:
	CMappingFile2() = delete;
	CMappingFile2(CFile& File) : m_File(File)
	{

	}

	~CMappingFile2()
	{
		Close();
	}

	void Close();

	void* Create(DWORD dwMap = FILE_MAP_READ, DWORD dwProtect = PAGE_READONLY);

	EckInline CFile& GetFile()
	{
		return m_File;
	}

	EckInline HANDLE GetHMapping()
	{
		return m_hMapping;
	}

	EckInline void* GetPFile()
	{
		return m_pFile;
	}
};
ECK_NAMESPACE_END