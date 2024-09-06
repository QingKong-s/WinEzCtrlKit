/*
* WinEzCtrlKit Library
*
* Json.h ： yyjson封装
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CRefStr.h"

#pragma push_macro("free")
#pragma push_macro("malloc")
#pragma push_macro("realloc")
#undef free
#undef malloc
#undef realloc
#include "YyJson/yyjson.h"
#pragma pop_macro("free")
#pragma pop_macro("malloc")
#pragma pop_macro("realloc")

ECK_NAMESPACE_BEGIN
using YyReadFlag = yyjson_read_flag;
using YyReadErr = yyjson_read_err;
using YyAlc = yyjson_alc;
using YyDoc = yyjson_doc;
using YyMutDoc = yyjson_mut_doc;
using YyVal = yyjson_val;
using YyMutVal = yyjson_mut_val;
using YyType = yyjson_type;
using YySubType = yyjson_subtype;
using YyWriteFlag = yyjson_write_flag;
using YyWriteErr = yyjson_write_err;
using YyArrIter = yyjson_arr_iter;
using YyMutArrIter = yyjson_mut_arr_iter;
using YyObjIter = yyjson_obj_iter;
using YyMutObjIter = yyjson_mut_obj_iter;
using YyPtrErr = yyjson_ptr_err;
using YyPtrCtx = yyjson_ptr_ctx;

EckInline BOOL YyLocateStringPos(PCSTR pszText, size_t cchText, size_t ocbPos,
	size_t& nLine, size_t& nCol, size_t& nChar)
{
	return yyjson_locate_pos(pszText, cchText, ocbPos, &nLine, &nCol, &nChar);
}

class CJsonVal
{
private:
	YyVal* m_pVal;
public:
	CJsonVal(YyVal* pVal) : m_pVal{ pVal } {}

	EckInline [[nodiscard]] constexpr auto GetValPtr() const { return m_pVal; }

	EckInline [[nodiscard]] constexpr BOOL IsValid() const { return !!m_pVal; }

	EckInline [[nodiscard]] YyType GetType() const { return yyjson_get_type(m_pVal); }

	EckInline [[nodiscard]] YySubType GetSubType() const { return yyjson_get_subtype(m_pVal); }

	EckInline [[nodiscard]] uint8_t GetTag() const { return yyjson_get_tag(m_pVal); }

	EckInline [[nodiscard]] PCSTR GetTypeDesc() const { return yyjson_get_type_desc(m_pVal); }

	EckInline [[nodiscard]] BOOL IsNull() const { return yyjson_is_null(m_pVal); }

	EckInline [[nodiscard]] BOOL IsTrue() const { return yyjson_is_true(m_pVal); }

	EckInline [[nodiscard]] BOOL IsFalse() const { return yyjson_is_false(m_pVal); }

	EckInline [[nodiscard]] BOOL IsBool() const { return yyjson_is_bool(m_pVal); }

	EckInline [[nodiscard]] BOOL IsUInt64() const { return yyjson_is_uint(m_pVal); }

	EckInline [[nodiscard]] BOOL IsInt64() const { return yyjson_is_sint(m_pVal); }

	EckInline [[nodiscard]] BOOL IsInt() const { return yyjson_is_int(m_pVal); }

	EckInline [[nodiscard]] BOOL IsReal() const { return yyjson_is_real(m_pVal); }

	EckInline [[nodiscard]] BOOL IsNum() const { return yyjson_is_num(m_pVal); }

	EckInline [[nodiscard]] BOOL IsStr() const { return yyjson_is_str(m_pVal); }

	EckInline [[nodiscard]] BOOL IsArr() const { return yyjson_is_arr(m_pVal); }

	EckInline [[nodiscard]] BOOL IsObj() const { return yyjson_is_obj(m_pVal); }

	EckInline [[nodiscard]] BOOL IsContainer() const { return yyjson_is_ctn(m_pVal); }

	EckInline [[nodiscard]] BOOL IsRaw() const { return yyjson_is_raw(m_pVal); }

	EckInline [[nodiscard]] PCSTR GetRaw() const { return yyjson_get_raw(m_pVal); }

	EckInline [[nodiscard]] bool GetBool() const { return yyjson_get_bool(m_pVal); }

	EckInline [[nodiscard]] uint64_t GetUInt64() const { return yyjson_get_uint(m_pVal); }

	EckInline [[nodiscard]] int64_t GetInt64() const { return yyjson_get_sint(m_pVal); }

	EckInline [[nodiscard]] int GetInt() const { return yyjson_get_int(m_pVal); }

	EckInline [[nodiscard]] double GetReal() const { return yyjson_get_real(m_pVal); }

	EckInline [[nodiscard]] double GetNum() const { return yyjson_get_num(m_pVal); }

	EckInline [[nodiscard]] PCSTR GetStr() const { return yyjson_get_str(m_pVal); }

	EckInline [[nodiscard]] size_t GetLen() const { return yyjson_get_len(m_pVal); }

	EckInline bool EqualStr(PCSTR pszStr) const { return yyjson_equals_str(m_pVal, pszStr); }

	EckInline bool EqualStr(PCSTR pszStr, size_t cchStr) const { return yyjson_equals_strn(m_pVal, pszStr, cchStr); }

	EckInline bool SetRaw(PCSTR pszRaw, size_t cchRaw) const { return yyjson_set_raw(m_pVal, pszRaw, cchRaw); }

	EckInline bool SetNull() const { return yyjson_set_null(m_pVal); }

	EckInline bool SetBool(bool bVal) const { return yyjson_set_bool(m_pVal, bVal); }

	EckInline bool SetUInt64(uint64_t uVal) const { return yyjson_set_uint(m_pVal, uVal); }

	EckInline bool SetInt64(int64_t iVal) const { return yyjson_set_sint(m_pVal, iVal); }

	EckInline bool SetInt(int iVal) const { return yyjson_set_int(m_pVal, iVal); }

	EckInline bool SetReal(double dVal) const { return yyjson_set_real(m_pVal, dVal); }

	EckInline bool SetStr(PCSTR pszVal) const { return yyjson_set_str(m_pVal, pszVal); }

	EckInline bool SetStr(PCSTR pszVal, size_t cchVal) const { return yyjson_set_strn(m_pVal, pszVal, cchVal); }

	EckInline [[nodiscard]] size_t ArrSize() const { return yyjson_arr_size(m_pVal); }

	EckInline [[nodiscard]] CJsonVal ArrAt(size_t idx) const { return CJsonVal(yyjson_arr_get(m_pVal, idx)); }

	EckInline [[nodiscard]] CJsonVal ArrFront() const { return CJsonVal(yyjson_arr_get_first(m_pVal)); }

	EckInline [[nodiscard]] CJsonVal ArrBack() const { return CJsonVal(yyjson_arr_get_last(m_pVal)); }

	EckInline [[nodiscard]] size_t ObjSize() const { return yyjson_obj_size(m_pVal); }

	EckInline [[nodiscard]] CJsonVal ObjAt(PCSTR pszKey) const { return CJsonVal(yyjson_obj_get(m_pVal, pszKey)); }

	EckInline [[nodiscard]] CJsonVal ObjAt(PCSTR pszKey, size_t cchKey) const { return CJsonVal(yyjson_obj_getn(m_pVal, pszKey, cchKey)); }

	EckInline [[nodiscard]] CJsonVal ObjGetVal(CJsonVal Key) const { return CJsonVal(yyjson_obj_iter_get_val(Key.GetValPtr())); }

	EckInline [[nodiscard]] PSTR Write(size_t* pcchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = NULL, YyWriteErr* pErr = NULL)
	{
		return yyjson_val_write_opts(m_pVal, uFlags, pAlc, pcchOut, pErr);
	}

	EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = NULL, YyWriteErr* pErr = NULL)
	{
		return yyjson_val_write_file(pszFile, m_pVal, uFlags, pAlc, pErr);
	}

	EckInline [[nodiscard]] CJsonVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax, YyPtrErr* pErr = NULL) const
	{
		return CJsonVal(yyjson_ptr_getx(m_pVal, pszPtr,
			cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pErr));
	}

	[[nodiscard]] CJsonVal operator[](PCSTR pszKey) const
	{
		if (*pszKey != '/')
		{
			const size_t cchKey = strlen(pszKey) + 1;
			const auto pszKey1 = (PSTR)_malloca(cchKey + 1);
			EckCheckMem(pszKey1);
			*pszKey1 = '/';
			memcpy(pszKey1 + 1, pszKey, cchKey);// 包含结尾NULL
			const auto r = ValAt(pszKey1, cchKey);
			_freea(pszKey1);
			return r;
		}
		else
			return ValAt(pszKey);
	}

	EckInline [[nodiscard]] CJsonVal operator[](size_t idx) const
	{
		return ArrAt(idx);
	}
};

class CMutJson;

class CJson
{
private:
	YyDoc* m_pDoc{};
public:
	ECK_DISABLE_COPY_DEF_CONS(CJson);
	CJson(PCSTR pszJson, size_t cchJson = SizeTMax, YyReadFlag uFlags = 0,
		const YyAlc* pAlc = NULL, YyReadErr* pErr = NULL)
	{
		m_pDoc = yyjson_read_opts((PSTR)pszJson, cchJson == SizeTMax ? strlen(pszJson) : cchJson,
			uFlags, pAlc, pErr);
	}

	CJson(YyDoc* pDoc) : m_pDoc{ pDoc } {}

	CJson(CJson&& x) noexcept : m_pDoc{ x.Detach() } {}

	CJson& operator=(CJson&& x) noexcept
	{
		std::swap(m_pDoc, x.m_pDoc);
		return *this;
	}

	~CJson() { Free(); }

	EckInline [[nodiscard]] constexpr BOOL IsValid() const { return !!m_pDoc; }

	EckInline [[nodiscard]] constexpr YyDoc* GetDocPtr() const { return m_pDoc; }

	void Free()
	{
		if (m_pDoc)
		{
			yyjson_doc_free(m_pDoc);
			m_pDoc = NULL;
		}
	}

	EckInline [[nodiscard]] constexpr YyDoc* Detach()
	{
		YyDoc* pDoc = m_pDoc;
		m_pDoc = NULL;
		return pDoc;
	}

	EckInline constexpr YyDoc* Attach(YyDoc* pDoc)
	{
		YyDoc* pOldDoc = m_pDoc;
		m_pDoc = pDoc;
		return pOldDoc;
	}

	EckInline [[nodiscard]] CJsonVal GetRoot() const { return CJsonVal(yyjson_doc_get_root(m_pDoc)); }

	EckInline [[nodiscard]] size_t GetReadSize() const { return yyjson_doc_get_read_size(m_pDoc); }

	EckInline [[nodiscard]] size_t GetValCount() const { return yyjson_doc_get_val_count(m_pDoc); }

	EckInline [[nodiscard]] CJsonVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax, YyPtrErr* pErr = NULL) const
	{
		return CJsonVal(yyjson_doc_ptr_getx(m_pDoc, pszPtr, cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pErr));
	}

	EckInline [[nodiscard]] PSTR Write(size_t* pcchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = NULL, YyWriteErr* pErr = NULL)
	{
		return yyjson_write_opts(m_pDoc, uFlags, pAlc, pcchOut, pErr);
	}

	EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = NULL, YyWriteErr* pErr = NULL)
	{
		return yyjson_write_file(pszFile, m_pDoc, uFlags, pAlc, pErr);
	}

	EckInline [[nodiscard]] CMutJson Clone(const YyAlc* pAlc = NULL) const;

	[[nodiscard]] CJsonVal operator[](PCSTR pszKey) const
	{
		if (*pszKey != '/')
		{
			const size_t cchKey = strlen(pszKey) + 1;
			const auto pszKey1 = (PSTR)_malloca(cchKey + 1);
			EckCheckMem(pszKey1);
			*pszKey1 = '/';
			memcpy(pszKey1 + 1, pszKey, cchKey);// 包含结尾NULL
			const auto r = ValAt(pszKey1, cchKey);
			_freea(pszKey1);
			return r;
		}
		else
			return ValAt(pszKey);
	}
};

class CJsonArrIter
{
private:
	YyArrIter m_iter;
public:
	CJsonArrIter() :m_iter{} {}

	CJsonArrIter(YyArrIter iter) :m_iter{ iter } {}

	CJsonArrIter(CJsonVal val) :m_iter{ yyjson_arr_iter_with(val.GetValPtr()) } {}

	EckInline void FromVal(CJsonVal val) { m_iter = yyjson_arr_iter_with(val.GetValPtr()); }

	EckInline [[nodiscard]] BOOL HasNext() { return yyjson_arr_iter_has_next(&m_iter); }

	EckInline [[nodiscard]] CJsonVal Next() { return CJsonVal(yyjson_arr_iter_next(&m_iter)); }
};

class CJsonObjIter
{
private:
	YyObjIter m_iter;
public:
	CJsonObjIter() :m_iter{} {}

	CJsonObjIter(YyObjIter iter) :m_iter{ iter } {}

	CJsonObjIter(CJsonVal val) :m_iter{ yyjson_obj_iter_with(val.GetValPtr()) } {}

	EckInline void FromVal(CJsonVal val) { m_iter = yyjson_obj_iter_with(val.GetValPtr()); }

	EckInline [[nodiscard]] BOOL HasNext() { return yyjson_obj_iter_has_next(&m_iter); }

	EckInline [[nodiscard]] CJsonVal Next() { return CJsonVal(yyjson_obj_iter_next(&m_iter)); }

	EckInline [[nodiscard]] CJsonVal Get(PCSTR pszKey) { return yyjson_obj_iter_get(&m_iter, pszKey); }

	EckInline [[nodiscard]] CJsonVal Get(PCSTR pszKey, size_t cchKey) { return yyjson_obj_iter_getn(&m_iter, pszKey, cchKey); }
};







class CJsonMutVal
{
private:
	YyMutVal* m_pVal;
public:
	CJsonMutVal(YyMutVal* pVal) : m_pVal{ pVal } {}

	EckInline [[nodiscard]] constexpr auto GetValPtr() const { return m_pVal; }

	EckInline [[nodiscard]] constexpr BOOL IsValid() const { return !!m_pVal; }

	EckInline [[nodiscard]] YyType GetType() const { return yyjson_mut_get_type(m_pVal); }

	EckInline [[nodiscard]] YySubType GetSubType() const { return yyjson_mut_get_subtype(m_pVal); }

	EckInline [[nodiscard]] uint8_t GetTag() const { return yyjson_mut_get_tag(m_pVal); }

	EckInline [[nodiscard]] PCSTR GetTypeDesc() const { return yyjson_mut_get_type_desc(m_pVal); }

	EckInline [[nodiscard]] BOOL IsNull() const { return yyjson_mut_is_null(m_pVal); }

	EckInline [[nodiscard]] BOOL IsTrue() const { return yyjson_mut_is_true(m_pVal); }

	EckInline [[nodiscard]] BOOL IsFalse() const { return yyjson_mut_is_false(m_pVal); }

	EckInline [[nodiscard]] BOOL IsBool() const { return yyjson_mut_is_bool(m_pVal); }

	EckInline [[nodiscard]] BOOL IsUInt64() const { return yyjson_mut_is_uint(m_pVal); }

	EckInline [[nodiscard]] BOOL IsInt64() const { return yyjson_mut_is_sint(m_pVal); }

	EckInline [[nodiscard]] BOOL IsInt() const { return yyjson_mut_is_int(m_pVal); }

	EckInline [[nodiscard]] BOOL IsReal() const { return yyjson_mut_is_real(m_pVal); }

	EckInline [[nodiscard]] BOOL IsNum() const { return yyjson_mut_is_num(m_pVal); }

	EckInline [[nodiscard]] BOOL IsStr() const { return yyjson_mut_is_str(m_pVal); }

	EckInline [[nodiscard]] BOOL IsArr() const { return yyjson_mut_is_arr(m_pVal); }

	EckInline [[nodiscard]] BOOL IsObj() const { return yyjson_mut_is_obj(m_pVal); }

	EckInline [[nodiscard]] BOOL IsContainer() const { return yyjson_mut_is_ctn(m_pVal); }

	EckInline [[nodiscard]] BOOL IsRaw() const { return yyjson_mut_is_raw(m_pVal); }

	EckInline [[nodiscard]] PCSTR GetRaw() const { return yyjson_mut_get_raw(m_pVal); }

	EckInline [[nodiscard]] bool GetBool() const { return yyjson_mut_get_bool(m_pVal); }

	EckInline [[nodiscard]] uint64_t GetUInt64() const { return yyjson_mut_get_uint(m_pVal); }

	EckInline [[nodiscard]] int64_t GetInt64() const { return yyjson_mut_get_sint(m_pVal); }

	EckInline [[nodiscard]] int GetInt() const { return yyjson_mut_get_int(m_pVal); }

	EckInline [[nodiscard]] double GetReal() const { return yyjson_mut_get_real(m_pVal); }

	EckInline [[nodiscard]] double GetNum() const { return yyjson_mut_get_num(m_pVal); }

	EckInline [[nodiscard]] PCSTR GetStr() const { return yyjson_mut_get_str(m_pVal); }

	EckInline [[nodiscard]] size_t GetLen() const { return yyjson_mut_get_len(m_pVal); }

	EckInline [[nodiscard]] bool EqualStr(PCSTR pszStr) const { return yyjson_mut_equals_str(m_pVal, pszStr); }

	EckInline [[nodiscard]] bool EqualStr(PCSTR pszStr, size_t cchStr) const { return yyjson_mut_equals_strn(m_pVal, pszStr, cchStr); }

	EckInline bool SetRaw(PCSTR pszRaw, size_t cchRaw) const { return yyjson_mut_set_raw(m_pVal, pszRaw, cchRaw); }

	EckInline bool SetNull() const { return yyjson_mut_set_null(m_pVal); }

	EckInline bool SetBool(bool bVal) const { return yyjson_mut_set_bool(m_pVal, bVal); }

	EckInline bool SetUInt64(uint64_t uVal) const { return yyjson_mut_set_uint(m_pVal, uVal); }

	EckInline bool SetInt64(int64_t iVal) const { return yyjson_mut_set_sint(m_pVal, iVal); }

	EckInline bool SetInt(int iVal) const { return yyjson_mut_set_int(m_pVal, iVal); }

	EckInline bool SetReal(double dVal) const { return yyjson_mut_set_real(m_pVal, dVal); }

	EckInline bool SetStr(PCSTR pszVal) const { return yyjson_mut_set_str(m_pVal, pszVal); }

	EckInline bool SetStr(PCSTR pszVal, size_t cchVal) const { return yyjson_mut_set_strn(m_pVal, pszVal, cchVal); }

	EckInline [[nodiscard]] size_t ArrSize() const { return yyjson_mut_arr_size(m_pVal); }

	EckInline [[nodiscard]] CJsonMutVal ArrAt(size_t idx) const { return CJsonMutVal(yyjson_mut_arr_get(m_pVal, idx)); }

	EckInline [[nodiscard]] CJsonMutVal ArrFront() const { return CJsonMutVal(yyjson_mut_arr_get_first(m_pVal)); }

	EckInline [[nodiscard]] CJsonMutVal ArrBack() const { return CJsonMutVal(yyjson_mut_arr_get_last(m_pVal)); }

	EckInline BOOL ArrInsert(size_t idx, CJsonMutVal Val) const
	{
		return yyjson_mut_arr_insert(m_pVal, Val.GetValPtr(), idx);
	}

	EckInline BOOL ArrPushBack(CJsonMutVal Val) const { return yyjson_mut_arr_append(m_pVal, Val.GetValPtr()); }

	EckInline BOOL ArrPushFront(CJsonMutVal Val) const { return yyjson_mut_arr_prepend(m_pVal, Val.GetValPtr()); }

	EckInline CJsonMutVal ArrReplace(size_t idx, CJsonMutVal Val) const
	{
		return CJsonMutVal(yyjson_mut_arr_replace(m_pVal, idx, Val.GetValPtr()));
	}

	EckInline CJsonMutVal ArrRemove(size_t idx) const { return CJsonMutVal(yyjson_mut_arr_remove(m_pVal, idx)); }

	EckInline BOOL ArrRemove(size_t idx, size_t c) const
	{
		return yyjson_mut_arr_remove_range(m_pVal, idx, c);
	}

	EckInline CJsonMutVal ArrPopBack() const { return CJsonMutVal(yyjson_mut_arr_remove_last(m_pVal)); }

	EckInline CJsonMutVal ArrPopFront() const { return CJsonMutVal(yyjson_mut_arr_remove_first(m_pVal)); }

	EckInline BOOL ArrClear() const { return yyjson_mut_arr_clear(m_pVal); }

	EckInline BOOL ArrRotate(size_t idx) const { return yyjson_mut_arr_rotate(m_pVal, idx); }

	EckInline [[nodiscard]] size_t ObjSize() const { return yyjson_mut_obj_size(m_pVal); }

	EckInline [[nodiscard]] CJsonMutVal ObjAt(PCSTR pszKey) const { return CJsonMutVal(yyjson_mut_obj_get(m_pVal, pszKey)); }

	EckInline [[nodiscard]] CJsonMutVal ObjAt(PCSTR pszKey, size_t cchKey) const
	{
		return CJsonMutVal(yyjson_mut_obj_getn(m_pVal, pszKey, cchKey));
	}

	EckInline BOOL ObjInsert(size_t idx, CJsonMutVal Key, CJsonMutVal Val) const
	{
		return yyjson_mut_obj_insert(m_pVal, Key.GetValPtr(), Val.GetValPtr(), idx);
	}

	EckInline CJsonMutVal ObjRemove(CJsonMutVal Key) const
	{
		return CJsonMutVal(yyjson_mut_obj_remove(m_pVal, Key.GetValPtr()));
	}

	EckInline CJsonMutVal ObjRemove(PCSTR pszKey) const
	{
		return CJsonMutVal(yyjson_mut_obj_remove_key(m_pVal, pszKey));
	}

	EckInline BOOL ObjClear() const { return yyjson_mut_obj_clear(m_pVal); }

	EckInline BOOL ObjReplace(CJsonMutVal Key, CJsonMutVal Val) const
	{
		return yyjson_mut_obj_replace(m_pVal, Key.GetValPtr(), Val.GetValPtr());
	}

	EckInline BOOL ObjRotate(size_t idx) const { return yyjson_mut_obj_rotate(m_pVal, idx); }

	EckInline [[nodiscard]] PSTR Write(size_t* pcchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = NULL, YyWriteErr* pErr = NULL)
	{
		return yyjson_mut_val_write_opts(m_pVal, uFlags, pAlc, pcchOut, pErr);
	}

	EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = NULL, YyWriteErr* pErr = NULL)
	{
		return yyjson_mut_val_write_file(pszFile, m_pVal, uFlags, pAlc, pErr);
	}

	EckInline [[nodiscard]] CJsonMutVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax,
		YyPtrCtx* pCtx = NULL, YyPtrErr* pErr = NULL) const
	{
		return CJsonMutVal(yyjson_mut_ptr_getx(m_pVal, pszPtr,
			cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pCtx, pErr));
	}

	[[nodiscard]] CJsonMutVal operator[](PCSTR pszKey) const
	{
		if (*pszKey != '/')
		{
			const size_t cchKey = strlen(pszKey) + 1;
			const auto pszKey1 = (PSTR)_malloca(cchKey + 1);
			EckCheckMem(pszKey1);
			*pszKey1 = '/';
			memcpy(pszKey1 + 1, pszKey, cchKey);// 包含结尾NULL
			const auto r = ValAt(pszKey1, cchKey);
			_freea(pszKey1);
			return r;
		}
		else
			return ValAt(pszKey);
	}

	EckInline [[nodiscard]] CJsonMutVal operator[](size_t idx) const
	{
		return ArrAt(idx);
	}
};

class CMutJson
{
private:
	YyMutDoc* m_pDoc{};
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CMutJson);

	CMutJson(const YyAlc* pAlc) : m_pDoc{ yyjson_mut_doc_new(pAlc) } {}

	CMutJson(const CJson& Doc, const YyAlc* pAlc = NULL) : m_pDoc{ yyjson_doc_mut_copy(Doc.GetDocPtr(), pAlc) } {}

	CMutJson(const CMutJson& Doc, const YyAlc* pAlc = NULL) : m_pDoc{ yyjson_mut_doc_mut_copy(Doc.GetDocPtr(), pAlc) } {}

	CMutJson(YyMutDoc* pDoc) : m_pDoc{ pDoc } {}

	EckInline [[nodiscard]] constexpr BOOL IsValid() const { return !!m_pDoc; }

	EckInline [[nodiscard]] constexpr YyMutDoc* GetDocPtr() const { return m_pDoc; }

	void Free()
	{
		if (m_pDoc)
		{
			yyjson_mut_doc_free(m_pDoc);
			m_pDoc = NULL;
		}
	}

	EckInline [[nodiscard]] CJsonMutVal GetRoot() const { return CJsonMutVal(yyjson_mut_doc_get_root(m_pDoc)); }

	EckInline void SetRoot(CJsonMutVal Val) const { yyjson_mut_doc_set_root(m_pDoc, Val.GetValPtr()); }

	EckInline BOOL SetStringPoolSize(size_t cb) const { return yyjson_mut_doc_set_str_pool_size(m_pDoc, cb); }

	EckInline BOOL SetValuePoolSize(size_t cb) const { return yyjson_mut_doc_set_val_pool_size(m_pDoc, cb); }

	EckInline [[nodiscard]] CJsonMutVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax,
		YyPtrCtx* pCtx = NULL, YyPtrErr* pErr = NULL) const
	{
		return CJsonMutVal(yyjson_mut_doc_ptr_getx(m_pDoc, pszPtr,
			cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pCtx, pErr));
	}

	EckInline [[nodiscard]] PSTR Write(size_t* pcchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = NULL, YyWriteErr* pErr = NULL)
	{
		return yyjson_mut_write_opts(m_pDoc, uFlags, pAlc, pcchOut, pErr);
	}

	EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = NULL, YyWriteErr* pErr = NULL)
	{
		return yyjson_mut_write_file(pszFile, m_pDoc, uFlags, pAlc, pErr);
	}

	EckInline [[nodiscard]] CMutJson Clone() const { return CMutJson(yyjson_mut_doc_mut_copy(m_pDoc, NULL)); }

	EckInline [[nodiscard]] CJson CloneImut() const { return CJson(yyjson_mut_doc_imut_copy(m_pDoc, NULL)); }

	EckInline [[nodiscard]] CJsonMutVal NewRaw(PCSTR pszRaw, size_t cchRaw = SizeTMax) const
	{
		return CJsonMutVal(yyjson_mut_rawn(m_pDoc, pszRaw, cchRaw == SizeTMax ? strlen(pszRaw) : cchRaw));
	}

	EckInline [[nodiscard]] CJsonMutVal NewRawCpy(PCSTR pszRaw, size_t cchRaw = SizeTMax) const
	{
		return CJsonMutVal(yyjson_mut_rawncpy(m_pDoc, pszRaw, cchRaw == SizeTMax ? strlen(pszRaw) : cchRaw));
	}

	EckInline [[nodiscard]] CJsonMutVal NewNull() const { return CJsonMutVal(yyjson_mut_null(m_pDoc)); }

	EckInline [[nodiscard]] CJsonMutVal NewTrue() const { return CJsonMutVal(yyjson_mut_true(m_pDoc)); }

	EckInline [[nodiscard]] CJsonMutVal NewFalse() const { return CJsonMutVal(yyjson_mut_false(m_pDoc)); }

	EckInline [[nodiscard]] CJsonMutVal NewBool(bool bVal) const { return CJsonMutVal(yyjson_mut_bool(m_pDoc, bVal)); }

	EckInline [[nodiscard]] CJsonMutVal NewUInt64(uint64_t uVal) const { return CJsonMutVal(yyjson_mut_uint(m_pDoc, uVal)); }

	EckInline [[nodiscard]] CJsonMutVal NewInt64(int64_t iVal) const { return CJsonMutVal(yyjson_mut_sint(m_pDoc, iVal)); }

	EckInline [[nodiscard]] CJsonMutVal NewInt(int iVal) const { return CJsonMutVal(yyjson_mut_int(m_pDoc, iVal)); }

	EckInline [[nodiscard]] CJsonMutVal NewReal(double dVal) const { return CJsonMutVal(yyjson_mut_real(m_pDoc, dVal)); }

	EckInline [[nodiscard]] CJsonMutVal NewStr(PCSTR pszVal, size_t cchVal = SizeTMax) const
	{
		return CJsonMutVal(yyjson_mut_strn(m_pDoc, pszVal, cchVal == SizeTMax ? strlen(pszVal) : cchVal));
	}

	EckInline [[nodiscard]] CJsonMutVal NewStrCpy(PCSTR pszVal, size_t cchVal = SizeTMax) const
	{
		return CJsonMutVal(yyjson_mut_strncpy(m_pDoc, pszVal, cchVal == SizeTMax ? strlen(pszVal) : cchVal));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr() const { return CJsonMutVal(yyjson_mut_arr(m_pDoc)); }

	EckInline [[nodiscard]] CJsonMutVal NewArr(const bool* pbVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_bool(m_pDoc, pbVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const int8_t* piVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_sint8(m_pDoc, piVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const int16_t* piVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_sint16(m_pDoc, piVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const int32_t* piVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_sint32(m_pDoc, piVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const int64_t* piVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_sint64(m_pDoc, piVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const uint8_t* puVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_uint8(m_pDoc, puVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const uint16_t* puVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_uint16(m_pDoc, puVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const uint32_t* puVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_uint32(m_pDoc, puVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const uint64_t* puVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_uint64(m_pDoc, puVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const float* pVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_float(m_pDoc, pVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const double* pdVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_real(m_pDoc, pdVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const char** ppszVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_str(m_pDoc, ppszVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArr(const char** ppszVals, const size_t* pcch, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_strn(m_pDoc, ppszVals, pcch, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArrCpy(const char** ppszVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_strcpy(m_pDoc, ppszVals, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewArrCpy(const char** ppszVals, const size_t* pcch, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_strncpy(m_pDoc, ppszVals, pcch, cVals));
	}

	EckInline [[nodiscard]] CJsonMutVal NewObj() const { return CJsonMutVal(yyjson_mut_obj(m_pDoc)); }

	EckInline [[nodiscard]] CJsonMutVal NewObj(const char** ppszKeys, const char** pVals, size_t cPairs) const
	{
		return CJsonMutVal(yyjson_mut_obj_with_str(m_pDoc, ppszKeys, pVals, cPairs));
	}

	EckInline [[nodiscard]] CJsonMutVal NewObj(const char** ppszKV, size_t cPairs) const
	{
		return CJsonMutVal(yyjson_mut_obj_with_kv(m_pDoc, ppszKV, cPairs));
	}

	[[nodiscard]] CJsonMutVal operator[](PCSTR pszKey) const
	{
		if (*pszKey != '/')
		{
			const size_t cchKey = strlen(pszKey) + 1;
			const auto pszKey1 = (PSTR)_malloca(cchKey + 1);
			EckCheckMem(pszKey1);
			*pszKey1 = '/';
			memcpy(pszKey1 + 1, pszKey, cchKey);// 包含结尾NULL
			const auto r = ValAt(pszKey1, cchKey);
			_freea(pszKey1);
			return r;
		}
		else
			return ValAt(pszKey);
	}
};

class CJsonMutArrIter
{
private:
	YyMutArrIter m_iter;
public:
	CJsonMutArrIter() :m_iter{} {}

	CJsonMutArrIter(const YyMutArrIter& iter) :m_iter{ iter } {}

	CJsonMutArrIter(CJsonMutVal val) :m_iter{ yyjson_mut_arr_iter_with(val.GetValPtr()) } {}

	EckInline void FromVal(CJsonMutVal val) { m_iter = yyjson_mut_arr_iter_with(val.GetValPtr()); }

	EckInline [[nodiscard]] BOOL HasNext() { return yyjson_mut_arr_iter_has_next(&m_iter); }

	EckInline [[nodiscard]] CJsonMutVal Next() { return CJsonMutVal(yyjson_mut_arr_iter_next(&m_iter)); }

	EckInline CJsonMutVal Remove() { return yyjson_mut_arr_iter_remove(&m_iter); }
};

class CJsonMutObjIter
{
private:
	YyMutObjIter m_iter;
public:
	CJsonMutObjIter() :m_iter{} {}

	CJsonMutObjIter(const YyMutObjIter& iter) :m_iter{ iter } {}

	CJsonMutObjIter(CJsonMutVal val) :m_iter{ yyjson_mut_obj_iter_with(val.GetValPtr()) } {}

	EckInline void FromVal(CJsonMutVal val) { m_iter = yyjson_mut_obj_iter_with(val.GetValPtr()); }

	EckInline [[nodiscard]] BOOL HasNext() { return yyjson_mut_obj_iter_has_next(&m_iter); }

	EckInline [[nodiscard]] CJsonMutVal Next() { return CJsonMutVal(yyjson_mut_obj_iter_next(&m_iter)); }

	EckInline [[nodiscard]] CJsonMutVal Get(PCSTR pszKey) { return yyjson_mut_obj_iter_get(&m_iter, pszKey); }

	EckInline [[nodiscard]] CJsonMutVal Get(PCSTR pszKey, size_t cchKey) { return yyjson_mut_obj_iter_getn(&m_iter, pszKey, cchKey); }

	EckInline CJsonMutVal Remove() { return yyjson_mut_obj_iter_remove(&m_iter); }
};



EckInline CMutJson CJson::Clone(const YyAlc* pAlc) const
{
	return CMutJson(yyjson_doc_mut_copy(m_pDoc, pAlc));
}
ECK_NAMESPACE_END