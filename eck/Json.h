#pragma once
#include "CRefStr.h"
#include "CRefBin.h"

#pragma push_macro("free")
#pragma push_macro("malloc")
#pragma push_macro("realloc")
#undef free
#undef malloc
#undef realloc
#include "YyJson/yyjson.h"

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

enum class JInitValType :UINT
{
	Invalid,
	Null,
	Bool,
	Int,
	Int64,
	UInt64,
	Real,
	String,
	StringW,
	Object,
	ArrayPh,
};


EckInline BOOL YyLocateStringPos(PCSTR pszText, size_t cchText, size_t ocbPos,
	size_t& nLine, size_t& nCol, size_t& nChar)
{
	return yyjson_locate_pos(pszText, cchText, ocbPos, &nLine, &nCol, &nChar);
}

namespace Priv
{
	template<class TThis>
	EckInline auto JsonValAt(TThis& This, PCSTR pszKey, size_t cchKey = SizeTMax)
	{
		return This.ValAt(pszKey, cchKey);
	}

	template<class TThis, class T>
	EckInline auto JsonValAtVarType(TThis& This, const T& x)
	{
		if constexpr (std::is_integral_v<T>)
			return This.ArrAt(x);
		else if constexpr (std::is_same_v<std::remove_cvref_t<T>, PSTR> ||
			std::is_same_v<std::remove_cvref_t<T>, PBYTE> ||
			std::is_same_v<std::remove_cvref_t<T>, char8_t*> ||
			std::is_same_v<std::remove_cvref_t<T>, PCSTR> ||
			std::is_same_v<std::remove_cvref_t<T>, PCBYTE> ||
			std::is_same_v<std::remove_cvref_t<T>, const char8_t*> ||
			std::is_convertible_v<std::remove_cvref_t<T>, PCSTR> ||
			std::is_convertible_v<std::remove_cvref_t<T>, PCBYTE> ||
			std::is_convertible_v<std::remove_cvref_t<T>, const char8_t*>)
		{
			return Priv::JsonValAt(This, (PCSTR)x);
		}
		else if constexpr (std::is_same_v<std::remove_cvref_t<T>, CRefStrA>)
			return Priv::JsonValAt(This, x.Data(), x.Size());
		else if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string> ||
			std::is_same_v<std::remove_cvref_t<T>, std::u8string>)
			return Priv::JsonValAt(This, x.c_str(), x.size());
		else if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string_view> ||
			std::is_same_v<std::remove_cvref_t<T>, std::u8string_view>)
			return Priv::JsonValAt(This, x.data(), x.size());
		else
			static_assert(false, "Unsupported type.");
	}

	template<class T>
	EckInline bool YyEqualIter(const T& x, const T& y)
	{
		if (x.m_iter.cur)
			if (y.m_iter.cur)
				return x.m_iter.cur == y.m_iter.cur;
			else
				return !x.HasNext();
		else
			if (y.m_iter.cur)
				return !y.HasNext();
			else
				return true;
	}
}

struct CJsonArrProxy;
struct CJsonObjProxy;
struct CJsonMutArrProxy;
struct CJsonMutObjProxy;

class CJsonVal
{
private:
	YyVal* m_pVal;
public:
	constexpr CJsonVal(YyVal* pVal) : m_pVal{ pVal } {}

	EckInlineNd constexpr auto GetValPtr() const { return m_pVal; }

	EckInlineNd constexpr BOOL IsValid() const { return !!m_pVal; }

	EckInlineNd YyType GetType() const { return yyjson_get_type(m_pVal); }

	EckInlineNd YySubType GetSubType() const { return yyjson_get_subtype(m_pVal); }

	EckInlineNd uint8_t GetTag() const { return yyjson_get_tag(m_pVal); }

	EckInlineNd PCSTR GetTypeDesc() const { return yyjson_get_type_desc(m_pVal); }

	EckInlineNd BOOL IsNull() const { return yyjson_is_null(m_pVal); }

	EckInlineNd BOOL IsTrue() const { return yyjson_is_true(m_pVal); }

	EckInlineNd BOOL IsFalse() const { return yyjson_is_false(m_pVal); }

	EckInlineNd BOOL IsBool() const { return yyjson_is_bool(m_pVal); }

	EckInlineNd BOOL IsUInt64() const { return yyjson_is_uint(m_pVal); }

	EckInlineNd BOOL IsInt64() const { return yyjson_is_sint(m_pVal); }

	EckInlineNd BOOL IsInt() const { return yyjson_is_int(m_pVal); }

	EckInlineNd BOOL IsReal() const { return yyjson_is_real(m_pVal); }

	EckInlineNd BOOL IsNum() const { return yyjson_is_num(m_pVal); }

	EckInlineNd BOOL IsStr() const { return yyjson_is_str(m_pVal); }

	EckInlineNd BOOL IsArr() const { return yyjson_is_arr(m_pVal); }

	EckInlineNd BOOL IsObj() const { return yyjson_is_obj(m_pVal); }

	EckInlineNd BOOL IsContainer() const { return yyjson_is_ctn(m_pVal); }

	EckInlineNd BOOL IsRaw() const { return yyjson_is_raw(m_pVal); }

	EckInlineNd PCSTR GetRaw() const { return yyjson_get_raw(m_pVal); }

	EckInlineNd bool GetBool() const { return yyjson_get_bool(m_pVal); }

	EckInlineNd uint64_t GetUInt64() const { return yyjson_get_uint(m_pVal); }

	EckInlineNd int64_t GetInt64() const { return yyjson_get_sint(m_pVal); }

	EckInlineNd int GetInt() const { return yyjson_get_int(m_pVal); }

	EckInlineNd double GetReal() const { return yyjson_get_real(m_pVal); }

	EckInlineNd double GetNum() const { return yyjson_get_num(m_pVal); }

	EckInlineNd PCSTR GetStr() const { return yyjson_get_str(m_pVal); }

	EckInlineNd size_t GetLen() const { return yyjson_get_len(m_pVal); }

	EckInlineNd CRefStrW GetStrW() const
	{
		return StrX2W(GetStr(), (int)GetLen(), CP_UTF8);
	}

	BOOL ToString(CRefStrA& rs) const
	{
		if (IsStr())
			rs.PushBack(GetStr(), (int)GetLen());
		else if (IsInt64())
			rs.AppendFormat("%I64d", GetInt64());
		else if (IsUInt64())
			rs.AppendFormat("%I64u", GetUInt64());
		else if (IsReal())
			rs.AppendFormat("%g", GetReal());
		else if (IsBool())
			if (IsTrue())
				rs.PushBack(EckStrAndLen("true"));
			else
				rs.PushBack(EckStrAndLen("false"));
		else if (IsNull())
			rs.PushBack(EckStrAndLen("null"));
		else
			return FALSE;
		return TRUE;
	}

	BOOL ToString(CRefStrW& rs) const
	{
		if (IsStr())
			rs.PushBack(StrX2W(GetStr(), (int)GetLen()));
		else if (IsInt64())
			rs.AppendFormat(L"%I64d", GetInt64());
		else if (IsUInt64())
			rs.AppendFormat(L"%I64u", GetUInt64());
		else if (IsReal())
			rs.AppendFormat(L"%g", GetReal());
		else if (IsBool())
			if (IsTrue())
				rs.PushBack(EckStrAndLen(L"true"));
			else
				rs.PushBack(EckStrAndLen(L"false"));
		else if (IsNull())
			rs.PushBack(EckStrAndLen(L"null"));
		else
			return FALSE;
		return TRUE;
	}

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

	EckInlineNd size_t ArrSize() const { return yyjson_arr_size(m_pVal); }

	EckInlineNd CJsonVal ArrAt(size_t idx) const { return CJsonVal(yyjson_arr_get(m_pVal, idx)); }

	EckInlineNd CJsonVal ArrFront() const { return CJsonVal(yyjson_arr_get_first(m_pVal)); }

	EckInlineNd CJsonVal ArrBack() const { return CJsonVal(yyjson_arr_get_last(m_pVal)); }

	EckInlineNd size_t ObjSize() const { return yyjson_obj_size(m_pVal); }

	EckInlineNd CJsonVal ObjAt(PCSTR pszKey) const { return CJsonVal(yyjson_obj_get(m_pVal, pszKey)); }

	EckInlineNd CJsonVal ObjAt(PCSTR pszKey, size_t cchKey) const { return CJsonVal(yyjson_obj_getn(m_pVal, pszKey, cchKey)); }

	EckInlineNd CJsonVal ObjGetVal(CJsonVal Key) const { return CJsonVal(yyjson_obj_iter_get_val(Key.GetValPtr())); }

	EckInlineNd PSTR Write(size_t* pcchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
	{
		return yyjson_val_write_opts(m_pVal, uFlags, pAlc, pcchOut, pErr);
	}

	EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
	{
		return yyjson_val_write_file(pszFile, m_pVal, uFlags, pAlc, pErr);
	}

	EckInlineNd CJsonVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax, YyPtrErr* pErr = nullptr) const
	{
		return CJsonVal(yyjson_ptr_getx(m_pVal, pszPtr,
			cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pErr));
	}

	template<class T>
	[[nodiscard]] CJsonVal operator[](const T& x) const
	{
		return Priv::JsonValAtVarType(*this, x);
	}

	EckInlineNd CJsonArrProxy AsArr() const;

	EckInlineNd CJsonObjProxy AsObj() const;
};

class CMutJson;

class CJson
{
private:
	YyDoc* m_pDoc{};
public:
	ECK_DISABLE_COPY_DEF_CONS(CJson);
	CJson(PCSTR pszJson, size_t cchJson = SizeTMax, YyReadFlag uFlags = 0,
		const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
	{
		m_pDoc = yyjson_read_opts((PSTR)pszJson, cchJson == SizeTMax ? strlen(pszJson) : cchJson,
			uFlags, pAlc, pErr);
	}

	template<class TAlloc>
	CJson(const CRefBinT<TAlloc>& rb, YyReadFlag uFlags = 0,
		const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
		:CJson((PCSTR)rb.Data(), rb.Size(), uFlags, pAlc, pErr) {
	}

	template<class TTraits, class TAlloc>
	CJson(const CRefStrT<CHAR, TTraits, TAlloc>& rs, YyReadFlag uFlags = 0,
		const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
		: CJson(rs.Data(), rs.Size(), uFlags, pAlc, pErr) {
	}

	template<class TTraits, class TAlloc>
	CJson(const std::basic_string<CHAR, TTraits, TAlloc>& s, YyReadFlag uFlags = 0,
		const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
		: CJson(s.c_str(), s.size(), uFlags, pAlc, pErr) {
	}

	template<class TTraits>
	CJson(const std::basic_string_view<CHAR, TTraits>& sv, YyReadFlag uFlags = 0,
		const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
		: CJson(sv.data(), sv.size(), uFlags, pAlc, pErr) {
	}

	CJson(const char8_t* pszJson, size_t cchJson = SizeTMax, YyReadFlag uFlags = 0,
		const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
		:CJson((PCSTR)pszJson, cchJson, uFlags, pAlc, pErr) {
	}

	constexpr CJson(YyDoc* pDoc) : m_pDoc{ pDoc } {}

	constexpr CJson(CJson&& x) noexcept : m_pDoc{ x.Detach() } {}

	constexpr CJson& operator=(CJson&& x) noexcept
	{
		std::swap(m_pDoc, x.m_pDoc);
		return *this;
	}

	~CJson() { Free(); }

	EckInlineNd constexpr BOOL IsValid() const { return !!m_pDoc; }

	EckInlineNd constexpr YyDoc* GetDocPtr() const { return m_pDoc; }

	void Free()
	{
		if (m_pDoc)
		{
			yyjson_doc_free(m_pDoc);
			m_pDoc = nullptr;
		}
	}

	EckInlineNd constexpr YyDoc* Detach()
	{
		YyDoc* pDoc = m_pDoc;
		m_pDoc = nullptr;
		return pDoc;
	}

	EckInline constexpr YyDoc* Attach(YyDoc* pDoc)
	{
		YyDoc* pOldDoc = m_pDoc;
		m_pDoc = pDoc;
		return pOldDoc;
	}

	EckInlineNd CJsonVal GetRoot() const { return CJsonVal(yyjson_doc_get_root(m_pDoc)); }

	EckInlineNd size_t GetReadSize() const { return yyjson_doc_get_read_size(m_pDoc); }

	EckInlineNd size_t GetValCount() const { return yyjson_doc_get_val_count(m_pDoc); }

	EckInlineNd CJsonVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax, YyPtrErr* pErr = nullptr) const
	{
		return CJsonVal(yyjson_doc_ptr_getx(m_pDoc, pszPtr, cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pErr));
	}

	EckInlineNd PSTR Write(size_t* pcchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
	{
		return yyjson_write_opts(m_pDoc, uFlags, pAlc, pcchOut, pErr);
	}

	EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
	{
		return yyjson_write_file(pszFile, m_pDoc, uFlags, pAlc, pErr);
	}

	EckInlineNd CMutJson Clone(const YyAlc* pAlc = nullptr) const;

	EckInlineNd CJsonVal operator[](PCSTR pszKey) const
	{
		return Priv::JsonValAt(*this, pszKey);
	}
};

struct CJsonArrIter
{
	YyArrIter m_iter;

	CJsonArrIter() :m_iter{} {}

	CJsonArrIter(YyArrIter iter) :m_iter{ iter } {}

	CJsonArrIter(CJsonVal val) :m_iter{ yyjson_arr_iter_with(val.GetValPtr()) } {}

	EckInline void FromVal(CJsonVal val) { m_iter = yyjson_arr_iter_with(val.GetValPtr()); }

	EckInlineNd BOOL HasNext() const { return yyjson_arr_iter_has_next((YyArrIter*)&m_iter); }

	EckInlineNd CJsonVal Next() { return CJsonVal(yyjson_arr_iter_next(&m_iter)); }

	EckInline constexpr CJsonVal GetCurr() const { return CJsonVal(m_iter.cur); }

	EckInline CJsonArrIter& operator++() { Next(); return *this; }

	EckInline constexpr CJsonVal operator*() const { return GetCurr(); }
};

EckInline bool operator==(const CJsonArrIter& x, const CJsonArrIter& y)
{
	return Priv::YyEqualIter<CJsonArrIter>(x, y);
}

struct CJsonObjIter
{
	YyObjIter m_iter;

	constexpr CJsonObjIter() :m_iter{} {}

	constexpr CJsonObjIter(YyObjIter iter) : m_iter{ iter } {}

	CJsonObjIter(CJsonVal val) :m_iter{ yyjson_obj_iter_with(val.GetValPtr()) } {}

	EckInline void FromVal(CJsonVal val) { m_iter = yyjson_obj_iter_with(val.GetValPtr()); }

	EckInlineNd BOOL HasNext() const { return yyjson_obj_iter_has_next((YyObjIter*)&m_iter); }

	EckInlineNd CJsonVal Next() { return CJsonVal(yyjson_obj_iter_next(&m_iter)); }

	EckInlineNd CJsonVal Get(PCSTR pszKey) { return yyjson_obj_iter_get(&m_iter, pszKey); }

	EckInlineNd CJsonVal Get(PCSTR pszKey, size_t cchKey) { return yyjson_obj_iter_getn(&m_iter, pszKey, cchKey); }

	EckInline constexpr CJsonVal GetCurr() const { return CJsonVal(m_iter.cur); }

	EckInline CJsonObjIter& operator++() { Next(); return *this; }

	EckInline constexpr CJsonVal operator*() const { return GetCurr(); }
};

EckInline bool operator==(const CJsonObjIter& x, const CJsonObjIter& y)
{
	return Priv::YyEqualIter<CJsonObjIter>(x, y);
}

struct CJsonArrProxy
{
	CJsonVal Val;

	CJsonArrProxy(CJsonVal val) : Val{ val } {}

	EckInline CJsonArrIter begin() const { return CJsonArrIter{ Val }; }

	EckInline CJsonArrIter end() const { return CJsonArrIter{}; }
};

struct CJsonObjProxy
{
	CJsonVal Val;

	CJsonObjProxy(CJsonVal val) : Val{ val } {}

	EckInline CJsonObjIter begin() const { return CJsonObjIter{ Val }; }

	EckInline CJsonObjIter end() const { return CJsonObjIter{}; }
};



class CJsonMutVal;
class CMutJson;

struct JArr_T {};

struct CJsonInitProxy
{
	JInitValType eType{};
	UINT cch{ UINT_MAX };
	union
	{
		bool b;
		uint64_t u64;
		int64_t i64;
		int i;
		double d;
		PCSTR s;
		PCWSTR ws;
		const std::initializer_list<CJsonInitProxy>* pObj;
	} Val;

	CJsonInitProxy(std::nullptr_t) :eType{ JInitValType::Null } { Val.pObj = nullptr; }

	CJsonInitProxy(bool b) :eType{ JInitValType::Bool } { Val.b = b; }

	CJsonInitProxy(uint64_t u64) :eType{ JInitValType::UInt64 } { Val.u64 = u64; }

	CJsonInitProxy(int64_t i64) :eType{ JInitValType::Int64 } { Val.i64 = i64; }

	CJsonInitProxy(int i) :eType{ JInitValType::Int } { Val.i = i; }

	CJsonInitProxy(double d) :eType{ JInitValType::Real } { Val.d = d; }

	CJsonInitProxy(const char* s) :eType{ JInitValType::String } { Val.s = s; }

	CJsonInitProxy(const char8_t* s) :eType{ JInitValType::String } { Val.s = (PCSTR)s; }

	CJsonInitProxy(const wchar_t* ws) :eType{ JInitValType::StringW } { Val.ws = ws; }

	CJsonInitProxy(JArr_T) :eType{ JInitValType::ArrayPh } { Val.pObj = nullptr; }

	CJsonInitProxy(const std::initializer_list<CJsonInitProxy>& il) :eType{ JInitValType::Object } { Val.pObj = &il; }

	template<class TTraits, class TAlloc>
	CJsonInitProxy(const std::basic_string<char, TTraits, TAlloc>& s)
		: eType{ JInitValType::String } {
		Val.s = s.data(); cch = (UINT)s.size();
	}

	template<class TTraits, class TAlloc>
	CJsonInitProxy(const std::basic_string<char8_t, TTraits, TAlloc>& s)
		: eType{ JInitValType::String } {
		Val.s = (PCSTR)s.data(); cch = (UINT)s.size();
	}

	template<class TTraits, class TAlloc>
	CJsonInitProxy(const std::basic_string<WCHAR, TTraits, TAlloc>& s)
		: eType{ JInitValType::StringW } {
		Val.ws = s.data(); cch = (UINT)s.size();
	}

	template<class TTraits>
	CJsonInitProxy(const std::basic_string_view<char, TTraits>& s)
		: eType{ JInitValType::String } {
		Val.s = s.data(); cch = (UINT)s.size();
	}

	template<class TTraits>
	CJsonInitProxy(const std::basic_string_view<char8_t, TTraits>& s)
		: eType{ JInitValType::String } {
		Val.s = (PCSTR)s.data(); cch = (UINT)s.size();
	}

	template<class TTraits>
	CJsonInitProxy(const std::basic_string_view<WCHAR, TTraits>& s)
		: eType{ JInitValType::StringW } {
		Val.ws = s.data(); cch = (UINT)s.size();
	}

	template<class TTraits, class TAlloc>
	CJsonInitProxy(const CRefStrT<char, TTraits, TAlloc>& rs)
		: eType{ JInitValType::String } {
		Val.s = rs.Data(); cch = (UINT)rs.Size();
	}

	template<class TTraits, class TAlloc>
	CJsonInitProxy(const CRefStrT<char8_t, TTraits, TAlloc>& rs)
		: eType{ JInitValType::String } {
		Val.s = (PCSTR)rs.Data(); cch = (UINT)rs.Size();
	}

	template<class TTraits, class TAlloc>
	CJsonInitProxy(const CRefStrT<WCHAR, TTraits, TAlloc>& rs)
		: eType{ JInitValType::StringW } {
		Val.ws = rs.Data(); cch = (UINT)rs.Size();
	}

	template<class TAlloc>
	CJsonInitProxy(const CRefBinT<TAlloc>& rb)
		: eType{ JInitValType::String } {
		Val.s = (PCSTR)rb.Data(); cch = (UINT)rb.Size();
	}

	inline CJsonMutVal ToMutVal(const CMutJson& Doc) const;
};

class CJsonMutVal
{
private:
	YyMutVal* m_pVal;
public:
	constexpr CJsonMutVal(YyMutVal* pVal) : m_pVal{ pVal } {}

	EckInlineNd constexpr auto GetValPtr() const { return m_pVal; }

	EckInlineNd constexpr BOOL IsValid() const { return !!m_pVal; }

	EckInlineNd YyType GetType() const { return yyjson_mut_get_type(m_pVal); }

	EckInlineNd YySubType GetSubType() const { return yyjson_mut_get_subtype(m_pVal); }

	EckInlineNd uint8_t GetTag() const { return yyjson_mut_get_tag(m_pVal); }

	EckInlineNd PCSTR GetTypeDesc() const { return yyjson_mut_get_type_desc(m_pVal); }

	EckInlineNd BOOL IsNull() const { return yyjson_mut_is_null(m_pVal); }

	EckInlineNd BOOL IsTrue() const { return yyjson_mut_is_true(m_pVal); }

	EckInlineNd BOOL IsFalse() const { return yyjson_mut_is_false(m_pVal); }

	EckInlineNd BOOL IsBool() const { return yyjson_mut_is_bool(m_pVal); }

	EckInlineNd BOOL IsUInt64() const { return yyjson_mut_is_uint(m_pVal); }

	EckInlineNd BOOL IsInt64() const { return yyjson_mut_is_sint(m_pVal); }

	EckInlineNd BOOL IsInt() const { return yyjson_mut_is_int(m_pVal); }

	EckInlineNd BOOL IsReal() const { return yyjson_mut_is_real(m_pVal); }

	EckInlineNd BOOL IsNum() const { return yyjson_mut_is_num(m_pVal); }

	EckInlineNd BOOL IsStr() const { return yyjson_mut_is_str(m_pVal); }

	EckInlineNd BOOL IsArr() const { return yyjson_mut_is_arr(m_pVal); }

	EckInlineNd BOOL IsObj() const { return yyjson_mut_is_obj(m_pVal); }

	EckInlineNd BOOL IsContainer() const { return yyjson_mut_is_ctn(m_pVal); }

	EckInlineNd BOOL IsRaw() const { return yyjson_mut_is_raw(m_pVal); }

	EckInlineNd PCSTR GetRaw() const { return yyjson_mut_get_raw(m_pVal); }

	EckInlineNd bool GetBool() const { return yyjson_mut_get_bool(m_pVal); }

	EckInlineNd uint64_t GetUInt64() const { return yyjson_mut_get_uint(m_pVal); }

	EckInlineNd int64_t GetInt64() const { return yyjson_mut_get_sint(m_pVal); }

	EckInlineNd int GetInt() const { return yyjson_mut_get_int(m_pVal); }

	EckInlineNd double GetReal() const { return yyjson_mut_get_real(m_pVal); }

	EckInlineNd double GetNum() const { return yyjson_mut_get_num(m_pVal); }

	EckInlineNd PCSTR GetStr() const { return yyjson_mut_get_str(m_pVal); }

	EckInlineNd size_t GetLen() const { return yyjson_mut_get_len(m_pVal); }

	EckInlineNd CRefStrW GetStrW() const
	{
		return StrX2W(GetStr(), (int)GetLen(), CP_UTF8);
	}

	EckInlineNd bool EqualStr(PCSTR pszStr) const { return yyjson_mut_equals_str(m_pVal, pszStr); }

	EckInlineNd bool EqualStr(PCSTR pszStr, size_t cchStr) const { return yyjson_mut_equals_strn(m_pVal, pszStr, cchStr); }

	EckInline bool SetRaw(PCSTR pszRaw, size_t cchRaw) const { return yyjson_mut_set_raw(m_pVal, pszRaw, cchRaw); }

	EckInline bool SetNull() const { return yyjson_mut_set_null(m_pVal); }

	EckInline bool SetBool(bool bVal) const { return yyjson_mut_set_bool(m_pVal, bVal); }

	EckInline bool SetUInt64(uint64_t uVal) const { return yyjson_mut_set_uint(m_pVal, uVal); }

	EckInline bool SetInt64(int64_t iVal) const { return yyjson_mut_set_sint(m_pVal, iVal); }

	EckInline bool SetInt(int iVal) const { return yyjson_mut_set_int(m_pVal, iVal); }

	EckInline bool SetReal(double dVal) const { return yyjson_mut_set_real(m_pVal, dVal); }

	EckInline bool SetStr(PCSTR pszVal) const { return yyjson_mut_set_str(m_pVal, pszVal); }

	EckInline bool SetStr(PCSTR pszVal, size_t cchVal) const { return yyjson_mut_set_strn(m_pVal, pszVal, cchVal); }

	EckInlineNd size_t ArrSize() const { return yyjson_mut_arr_size(m_pVal); }

	EckInlineNd CJsonMutVal ArrAt(size_t idx) const { return CJsonMutVal(yyjson_mut_arr_get(m_pVal, idx)); }

	EckInlineNd CJsonMutVal ArrFront() const { return CJsonMutVal(yyjson_mut_arr_get_first(m_pVal)); }

	EckInlineNd CJsonMutVal ArrBack() const { return CJsonMutVal(yyjson_mut_arr_get_last(m_pVal)); }

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

	EckInlineNd size_t ObjSize() const { return yyjson_mut_obj_size(m_pVal); }

	EckInlineNd CJsonMutVal ObjAt(PCSTR pszKey) const { return CJsonMutVal(yyjson_mut_obj_get(m_pVal, pszKey)); }

	EckInlineNd CJsonMutVal ObjAt(PCSTR pszKey, size_t cchKey) const
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

	EckInlineNd PSTR Write(size_t& cchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
	{
		return yyjson_mut_val_write_opts(m_pVal, uFlags, pAlc, &cchOut, pErr);
	}

	EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
	{
		return yyjson_mut_val_write_file(pszFile, m_pVal, uFlags, pAlc, pErr);
	}

	EckInlineNd CRefStrW WriteW(YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
	{
		size_t cchOut;
		const auto pszU8 = Write(cchOut, uFlags, pAlc, pErr);
		if (!pszU8)
			return {};
		const auto rs = StrX2W(pszU8, (int)cchOut, CP_UTF8);
		if (pAlc)
			pAlc->free(pAlc->ctx, pszU8);
		else
			free(pszU8);
		return rs;
	}

	EckInlineNd CJsonMutVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax,
		YyPtrCtx* pCtx = nullptr, YyPtrErr* pErr = nullptr) const
	{
		return CJsonMutVal(yyjson_mut_ptr_getx(m_pVal, pszPtr,
			cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pCtx, pErr));
	}

	template<class T>
	EckInlineNd CJsonMutVal operator[](const T& x) const
	{
		return Priv::JsonValAtVarType(*this, x);
	}

	EckInlineNd CJsonMutArrProxy AsArr() const;

	EckInlineNd CJsonMutObjProxy AsObj() const;
};

class CMutJson
{
private:
	YyMutDoc* m_pDoc{};
public:
	ECK_DISABLE_COPY(CMutJson);

	CMutJson() : m_pDoc{ yyjson_mut_doc_new(nullptr) } {}

	CMutJson(const CJson& Doc, const YyAlc* pAlc = nullptr) : m_pDoc{ yyjson_doc_mut_copy(Doc.GetDocPtr(), pAlc) } {}

	CMutJson(const CMutJson& Doc, const YyAlc* pAlc = nullptr) : m_pDoc{ yyjson_mut_doc_mut_copy(Doc.GetDocPtr(), pAlc) } {}

	constexpr CMutJson(YyMutDoc* pDoc) : m_pDoc{ pDoc } {}

	constexpr CMutJson(CMutJson&& x) noexcept : m_pDoc{ x.Detach() } {}

	CMutJson& operator=(CMutJson&& x) noexcept
	{
		std::swap(m_pDoc, x.m_pDoc);
		return *this;
	}

	~CMutJson() { Free(); }

	EckInline void Create(const YyAlc* pAlc = nullptr) { m_pDoc = yyjson_mut_doc_new(pAlc); }

	EckInline void Create(const CJson& Doc, const YyAlc* pAlc = nullptr)
	{
		m_pDoc = yyjson_doc_mut_copy(Doc.GetDocPtr(), pAlc);
	}

	EckInline void Create(const CMutJson& Doc, const YyAlc* pAlc = nullptr)
	{
		m_pDoc = yyjson_mut_doc_mut_copy(Doc.GetDocPtr(), pAlc);
	}

	EckInlineNd constexpr BOOL IsValid() const { return !!m_pDoc; }

	EckInlineNd constexpr YyMutDoc* GetDocPtr() const { return m_pDoc; }

	void Free()
	{
		if (m_pDoc)
		{
			yyjson_mut_doc_free(m_pDoc);
			m_pDoc = nullptr;
		}
	}

	EckInlineNd constexpr YyMutDoc* Detach()
	{
		YyMutDoc* pDoc = m_pDoc;
		m_pDoc = nullptr;
		return pDoc;
	}

	EckInline constexpr YyMutDoc* Attach(YyMutDoc* pDoc)
	{
		YyMutDoc* pOldDoc = m_pDoc;
		m_pDoc = pDoc;
		return pOldDoc;
	}

	EckInlineNd CJsonMutVal GetRoot() const { return CJsonMutVal(yyjson_mut_doc_get_root(m_pDoc)); }

	EckInline void SetRoot(CJsonMutVal Val) const { yyjson_mut_doc_set_root(m_pDoc, Val.GetValPtr()); }

	EckInline BOOL SetStringPoolSize(size_t cb) const { return yyjson_mut_doc_set_str_pool_size(m_pDoc, cb); }

	EckInline BOOL SetValuePoolSize(size_t cb) const { return yyjson_mut_doc_set_val_pool_size(m_pDoc, cb); }

	EckInlineNd CJsonMutVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax,
		YyPtrCtx* pCtx = nullptr, YyPtrErr* pErr = nullptr) const
	{
		return CJsonMutVal(yyjson_mut_doc_ptr_getx(m_pDoc, pszPtr,
			cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pCtx, pErr));
	}

	EckInlineNd PSTR Write(size_t& cchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
	{
		return yyjson_mut_write_opts(m_pDoc, uFlags, pAlc, &cchOut, pErr);
	}

	EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
	{
		return yyjson_mut_write_file(pszFile, m_pDoc, uFlags, pAlc, pErr);
	}

	EckInlineNd CRefStrW WriteW(YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
	{
		size_t cchOut;
		const auto pszU8 = Write(cchOut, uFlags, pAlc, pErr);
		if (!pszU8)
			return {};
		const auto rs = StrX2W(pszU8, (int)cchOut, CP_UTF8);
		if (pAlc)
			pAlc->free(pAlc->ctx, pszU8);
		else
			free(pszU8);
		return rs;
	}

	EckInlineNd CMutJson Clone() const { return CMutJson(yyjson_mut_doc_mut_copy(m_pDoc, nullptr)); }

	EckInlineNd CJson CloneImut() const { return CJson(yyjson_mut_doc_imut_copy(m_pDoc, nullptr)); }

	EckInlineNd CJsonMutVal NewRaw(PCSTR pszRaw, size_t cchRaw = SizeTMax) const
	{
		return CJsonMutVal(yyjson_mut_rawn(m_pDoc, pszRaw, cchRaw == SizeTMax ? strlen(pszRaw) : cchRaw));
	}

	EckInlineNd CJsonMutVal NewRawCpy(PCSTR pszRaw, size_t cchRaw = SizeTMax) const
	{
		return CJsonMutVal(yyjson_mut_rawncpy(m_pDoc, pszRaw, cchRaw == SizeTMax ? strlen(pszRaw) : cchRaw));
	}

	EckInlineNd CJsonMutVal NewNull() const { return CJsonMutVal(yyjson_mut_null(m_pDoc)); }

	EckInlineNd CJsonMutVal NewTrue() const { return CJsonMutVal(yyjson_mut_true(m_pDoc)); }

	EckInlineNd CJsonMutVal NewFalse() const { return CJsonMutVal(yyjson_mut_false(m_pDoc)); }

	EckInlineNd CJsonMutVal NewBool(bool bVal) const { return CJsonMutVal(yyjson_mut_bool(m_pDoc, bVal)); }

	EckInlineNd CJsonMutVal NewUInt64(uint64_t uVal) const { return CJsonMutVal(yyjson_mut_uint(m_pDoc, uVal)); }

	EckInlineNd CJsonMutVal NewInt64(int64_t iVal) const { return CJsonMutVal(yyjson_mut_sint(m_pDoc, iVal)); }

	EckInlineNd CJsonMutVal NewInt(int iVal) const { return CJsonMutVal(yyjson_mut_int(m_pDoc, iVal)); }

	EckInlineNd CJsonMutVal NewReal(double dVal) const { return CJsonMutVal(yyjson_mut_real(m_pDoc, dVal)); }

	EckInlineNd CJsonMutVal NewStr(PCSTR pszVal, size_t cchVal = SizeTMax) const
	{
		return CJsonMutVal(yyjson_mut_strn(m_pDoc, pszVal, cchVal == SizeTMax ? strlen(pszVal) : cchVal));
	}

	EckInlineNd CJsonMutVal NewStrCpy(PCSTR pszVal, size_t cchVal = SizeTMax) const
	{
		return CJsonMutVal(yyjson_mut_strncpy(m_pDoc, pszVal, cchVal == SizeTMax ? strlen(pszVal) : cchVal));
	}

	EckInlineNd CJsonMutVal NewStrCpy(PCWSTR pszVal, size_t cchVal = SizeTMax) const
	{
		if (cchVal == SizeTMax)
			cchVal = wcslen(pszVal);
		const auto u8 = StrW2X(pszVal, (int)cchVal, CP_UTF8);
		return NewStrCpy(u8.Data(), u8.Size());
	}

	EckInlineNd CJsonMutVal NewArr() const { return CJsonMutVal(yyjson_mut_arr(m_pDoc)); }

	EckInlineNd CJsonMutVal NewArr(const bool* pbVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_bool(m_pDoc, pbVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const int8_t* piVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_sint8(m_pDoc, piVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const int16_t* piVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_sint16(m_pDoc, piVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const int32_t* piVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_sint32(m_pDoc, piVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const int64_t* piVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_sint64(m_pDoc, piVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const uint8_t* puVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_uint8(m_pDoc, puVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const uint16_t* puVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_uint16(m_pDoc, puVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const uint32_t* puVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_uint32(m_pDoc, puVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const uint64_t* puVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_uint64(m_pDoc, puVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const float* pVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_float(m_pDoc, pVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const double* pdVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_real(m_pDoc, pdVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const char** ppszVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_str(m_pDoc, ppszVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArr(const char** ppszVals, const size_t* pcch, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_strn(m_pDoc, ppszVals, pcch, cVals));
	}

	EckInlineNd CJsonMutVal NewArrCpy(const char** ppszVals, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_strcpy(m_pDoc, ppszVals, cVals));
	}

	EckInlineNd CJsonMutVal NewArrCpy(const char** ppszVals, const size_t* pcch, size_t cVals) const
	{
		return CJsonMutVal(yyjson_mut_arr_with_strncpy(m_pDoc, ppszVals, pcch, cVals));
	}

	EckInlineNd CJsonMutVal NewObj() const { return CJsonMutVal(yyjson_mut_obj(m_pDoc)); }

	EckInlineNd CJsonMutVal NewObj(const char** ppszKeys, const char** pVals, size_t cPairs) const
	{
		return CJsonMutVal(yyjson_mut_obj_with_str(m_pDoc, ppszKeys, pVals, cPairs));
	}

	EckInlineNd CJsonMutVal NewObj(const char** ppszKV, size_t cPairs) const
	{
		return CJsonMutVal(yyjson_mut_obj_with_kv(m_pDoc, ppszKV, cPairs));
	}

	[[nodiscard]] CJsonMutVal operator[](PCSTR pszKey) const
	{
		return Priv::JsonValAt(*this, pszKey);
	}

	EckInline CMutJson& operator=(CJsonInitProxy x)
	{
		SetRoot(x.ToMutVal(*this));
		return *this;
	}
};

inline CJsonMutVal CJsonInitProxy::ToMutVal(const CMutJson& Doc) const
{
	switch (eType)
	{
	case JInitValType::Null:
		return Doc.NewNull();
	case JInitValType::Bool:
		return Doc.NewBool(Val.b);
	case JInitValType::Int:
		return Doc.NewInt(Val.i);
	case JInitValType::Int64:
		return Doc.NewInt64(Val.i64);
	case JInitValType::UInt64:
		return Doc.NewUInt64(Val.u64);
	case JInitValType::Real:
		return Doc.NewReal(Val.d);
	case JInitValType::String:
		return Doc.NewStrCpy(Val.s, cch == UINT_MAX ? strlen(Val.s) : cch);
	case JInitValType::StringW:
	{
		const auto u8 = StrW2X(Val.ws, (int)cch, CP_UTF8);
		return Doc.NewStrCpy(u8.Data(), u8.Size());
	}
	ECK_UNREACHABLE;
	case JInitValType::Object:
	{
		if (Val.pObj->begin()->eType == JInitValType::ArrayPh)
		{
			CJsonMutVal Ret{ Doc.NewArr() };
			for (auto it = Val.pObj->begin() + 1; it != Val.pObj->end(); ++it)
				Ret.ArrPushBack(it->ToMutVal(Doc));
			return Ret;
		}
		else
		{
			EckAssert(Val.pObj->size() % 2 == 0 && L"对象键值总数必须为偶数");
			CJsonMutVal Ret{ Doc.NewObj() };
			CJsonMutVal Key{ nullptr };
			for (size_t i{}; const auto& e : *Val.pObj)
			{
				if (i % 2 == 0)
					Key = e.ToMutVal(Doc);
				else
					Ret.ObjInsert(i / 2, Key, e.ToMutVal(Doc));
				++i;
			}
			return Ret;
		}
	}
	ECK_UNREACHABLE;
	}
	EckDbgBreak();
	return { nullptr };
}

struct CJsonMutArrIter
{
	YyMutArrIter m_iter;

	constexpr CJsonMutArrIter() :m_iter{} {}

	constexpr CJsonMutArrIter(const YyMutArrIter& iter) : m_iter{ iter } {}

	CJsonMutArrIter(CJsonMutVal val) :m_iter{ yyjson_mut_arr_iter_with(val.GetValPtr()) } {}

	EckInline void FromVal(CJsonMutVal val) { m_iter = yyjson_mut_arr_iter_with(val.GetValPtr()); }

	EckInlineNd BOOL HasNext() const { return yyjson_mut_arr_iter_has_next((YyMutArrIter*)&m_iter); }

	EckInlineNd CJsonMutVal Next() { return CJsonMutVal(yyjson_mut_arr_iter_next(&m_iter)); }

	EckInline CJsonMutVal Remove() { return yyjson_mut_arr_iter_remove(&m_iter); }

	EckInline constexpr CJsonMutVal GetCurr() const { return CJsonMutVal(m_iter.cur); }

	EckInline CJsonMutArrIter& operator++() { Next(); return *this; }

	EckInline constexpr CJsonMutVal operator*() const { return GetCurr(); }
};

EckInline bool operator==(const CJsonMutArrIter& x, const CJsonMutArrIter& y)
{
	return Priv::YyEqualIter<CJsonMutArrIter>(x, y);
}

struct CJsonMutObjIter
{
	YyMutObjIter m_iter;

	constexpr CJsonMutObjIter() :m_iter{} {}

	constexpr CJsonMutObjIter(const YyMutObjIter& iter) : m_iter{ iter } {}

	CJsonMutObjIter(CJsonMutVal val) :m_iter{ yyjson_mut_obj_iter_with(val.GetValPtr()) } {}

	EckInline void FromVal(CJsonMutVal val) { m_iter = yyjson_mut_obj_iter_with(val.GetValPtr()); }

	EckInlineNd BOOL HasNext() const { return yyjson_mut_obj_iter_has_next((YyMutObjIter*)&m_iter); }

	EckInlineNd CJsonMutVal Next() { return CJsonMutVal(yyjson_mut_obj_iter_next(&m_iter)); }

	EckInlineNd CJsonMutVal Get(PCSTR pszKey) { return yyjson_mut_obj_iter_get(&m_iter, pszKey); }

	EckInlineNd CJsonMutVal Get(PCSTR pszKey, size_t cchKey) { return yyjson_mut_obj_iter_getn(&m_iter, pszKey, cchKey); }

	EckInline CJsonMutVal Remove() { return yyjson_mut_obj_iter_remove(&m_iter); }

	EckInline constexpr CJsonMutVal GetCurr() const { return CJsonMutVal(m_iter.cur); }

	EckInline CJsonMutObjIter& operator++() { Next(); return *this; }

	EckInline constexpr CJsonMutVal operator*() const { return GetCurr(); }
};

EckInline bool operator==(const CJsonMutObjIter& x, const CJsonMutObjIter& y)
{
	return Priv::YyEqualIter<CJsonMutObjIter>(x, y);
}

struct CJsonMutArrProxy
{
	CJsonMutVal Val;

	EckInline CJsonMutArrProxy(CJsonMutVal val) : Val{ val } {}

	EckInline CJsonMutArrIter begin() const { return CJsonMutArrIter{ Val }; }

	EckInline CJsonMutArrIter end() const { return CJsonMutArrIter{}; }
};

struct CJsonMutObjProxy
{
	CJsonMutVal Val;

	EckInline CJsonMutObjProxy(CJsonMutVal val) : Val{ val } {}

	EckInline CJsonMutObjIter begin() const { return CJsonMutObjIter{ Val }; }

	EckInline CJsonMutObjIter end() const { return CJsonMutObjIter{}; }
};



EckInlineNd CMutJson CJson::Clone(const YyAlc* pAlc) const
{
	return CMutJson(yyjson_doc_mut_copy(m_pDoc, pAlc));
}

EckInlineNd CJsonArrProxy CJsonVal::AsArr() const
{
	return CJsonArrProxy(*this);
}

EckInlineNd CJsonObjProxy CJsonVal::AsObj() const
{
	return CJsonObjProxy(*this);
}

EckInlineNd CJsonMutArrProxy CJsonMutVal::AsArr() const
{
	return CJsonMutArrProxy(*this);
}

EckInlineNd CJsonMutObjProxy CJsonMutVal::AsObj() const
{
	return CJsonMutObjProxy(*this);
}
ECK_NAMESPACE_END

#pragma pop_macro("free")
#pragma pop_macro("malloc")
#pragma pop_macro("realloc")