#pragma once
#include "CRefStr.h"
#include "CRefBin.h"

#pragma push_macro("free")
#pragma push_macro("malloc")
#pragma push_macro("realloc")
#undef free
#undef malloc
#undef realloc
#include "../ThirdPartyLib/YyJson/yyjson.h"

#define ECK_JSON_NAMESPACE_BEGIN	namespace Json {
#define ECK_JSON_NAMESPACE_END		}

ECK_NAMESPACE_BEGIN
ECK_JSON_NAMESPACE_BEGIN
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

class CVal;
class CDoc;
class CMutVal;
class CMutDoc;
struct CArrProxy;
struct CObjProxy;
struct CMutArrProxy;
struct CMutObjProxy;
namespace Priv { struct CInitProxy; }

struct JArr_T {};

EckInline BOOL YyLocateStringPos(PCSTR pszText, size_t cchText, size_t ocbPos,
    size_t& nLine, size_t& nCol, size_t& nChar)
{
    return yyjson_locate_pos(pszText, cchText, ocbPos, &nLine, &nCol, &nChar);
}

namespace Priv
{
    EckInline auto JsonValAt(auto& This, PCSTR pszKey, size_t cchKey = SizeTMax)
    {
        return This.ValAt(pszKey, cchKey);
    }

    template<class TThis, class T>
    EckInline auto JsonValAtVarType(TThis& This, const T& x)
    {
        using T1 = std::remove_cvref_t<T>;
        if constexpr (std::is_integral_v<T1>)
            return This.ArrAt(x);
        else if constexpr (std::is_convertible_v<T1, PCCH> ||
            std::is_convertible_v<T1, PCBYTE> ||
            std::is_convertible_v<T1, const char8_t*>)
        {
            return JsonValAt(This, (PCSTR)x);
        }
        else if constexpr (IsSameTemplate<CRefStrT, T1>::V)
            return JsonValAt(This, x.Data(), x.Size());
        else if constexpr (IsSameTemplate<std::basic_string, T1>::V &&
            sizeof(typename T1::value_type) == 1)
            return JsonValAt(This, x.c_str(), x.size());
        else if constexpr (IsSameTemplate<std::basic_string_view, T1>::V &&
            sizeof(typename T1::value_type) == 1)
            return JsonValAt(This, x.data(), x.size());
        else
            static_assert(false, "Unsupported type.");
    }

    template<class T>
    EckInline bool EqualIterator(const T& x, const T& y)
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

    inline CRefStrW WriteW(PSTR pszU8, size_t cchU8, _In_opt_ YyAlc* pAlc)
    {
        if (!pszU8)
            return {};
        CRefStrW rs{ StrX2W(pszU8, (int)cchU8, CP_UTF8) };
        if (pAlc && pAlc->free)
            pAlc->free(pAlc->ctx, pszU8);
        else
            free(pszU8);
        return rs;
    }

    class CValBase
    {
    protected:
        void* m_pVal{};
    public:
        constexpr CValBase(void* p) :m_pVal{ p } {}
        EckInlineNd YyType GetType() const { return unsafe_yyjson_get_type(m_pVal); }
        EckInlineNd YySubType GetSubType() const { return unsafe_yyjson_get_subtype(m_pVal); }
        EckInlineNd uint8_t GetTag() const { return unsafe_yyjson_get_tag(m_pVal); }
        EckInlineNd PCSTR GetTypeDesc() const { return yyjson_get_type_desc((yyjson_val*)m_pVal); }

        EckInline bool EqualStr(PCSTR pszStr) const { return yyjson_equals_str((yyjson_val*)m_pVal, pszStr); }
        EckInline bool EqualStr(PCSTR pszStr, size_t cchStr) const { return yyjson_equals_strn((yyjson_val*)m_pVal, pszStr, cchStr); }

        EckInlineNd bool IsNull() const { return unsafe_yyjson_is_null(m_pVal); }
        EckInlineNd bool IsTrue() const { return unsafe_yyjson_is_true(m_pVal); }
        EckInlineNd bool IsFalse() const { return unsafe_yyjson_is_false(m_pVal); }
        EckInlineNd bool IsBool() const { return unsafe_yyjson_is_bool(m_pVal); }
        EckInlineNd bool IsUInt64() const { return unsafe_yyjson_is_uint(m_pVal); }
        EckInlineNd bool IsInt64() const { return unsafe_yyjson_is_sint(m_pVal); }
        EckInlineNd bool IsInt() const { return unsafe_yyjson_is_int(m_pVal); }
        EckInlineNd bool IsReal() const { return unsafe_yyjson_is_real(m_pVal); }
        EckInlineNd bool IsNum() const { return unsafe_yyjson_is_num(m_pVal); }
        EckInlineNd bool IsStr() const { return unsafe_yyjson_is_str(m_pVal); }
        EckInlineNd bool IsArr() const { return unsafe_yyjson_is_arr(m_pVal); }
        EckInlineNd bool IsObj() const { return unsafe_yyjson_is_obj(m_pVal); }
        EckInlineNd bool IsContainer() const { return unsafe_yyjson_is_ctn(m_pVal); }
        EckInlineNd bool IsRaw() const { return unsafe_yyjson_is_raw(m_pVal); }

        EckInlineNd PCSTR GetRaw() const { return yyjson_get_raw((yyjson_val*)m_pVal); }
        EckInlineNd bool GetBool() const { return yyjson_get_bool((yyjson_val*)m_pVal); }
        EckInlineNd uint64_t GetUInt64() const { return yyjson_get_uint((yyjson_val*)m_pVal); }
        EckInlineNd int64_t GetInt64() const { return yyjson_get_sint((yyjson_val*)m_pVal); }
        EckInlineNd int GetInt() const { return yyjson_get_int((yyjson_val*)m_pVal); }
        EckInlineNd double GetReal() const { return yyjson_get_real((yyjson_val*)m_pVal); }
        EckInlineNd double GetNum() const { return yyjson_get_num((yyjson_val*)m_pVal); }
        EckInlineNd PCSTR GetStr() const { return yyjson_get_str((yyjson_val*)m_pVal); }
        EckInlineNd size_t GetLen() const { return yyjson_get_len((yyjson_val*)m_pVal); }
        EckInlineNd CRefStrW GetStrW() const { return StrX2W(GetStr(), (int)GetLen(), CP_UTF8); }
    };
}

class CVal : public Priv::CValBase
{
public:
    constexpr CVal(YyVal* pVal) : CValBase{ (void*)pVal } {}
    EckInlineNd constexpr auto Ptr() const { return (YyVal*)m_pVal; }
    EckInlineNd constexpr BOOL IsValid() const { return !!m_pVal; }

    EckInline bool SetRaw(PCSTR pszRaw, size_t cchRaw) const { return yyjson_set_raw(Ptr(), pszRaw, cchRaw); }
    EckInline bool SetNull() const { return yyjson_set_null(Ptr()); }
    EckInline bool SetBool(bool bVal) const { return yyjson_set_bool(Ptr(), bVal); }
    EckInline bool SetUInt64(uint64_t uVal) const { return yyjson_set_uint(Ptr(), uVal); }
    EckInline bool SetInt64(int64_t iVal) const { return yyjson_set_sint(Ptr(), iVal); }
    EckInline bool SetInt(int iVal) const { return yyjson_set_int(Ptr(), iVal); }
    EckInline bool SetReal(double dVal) const { return yyjson_set_real(Ptr(), dVal); }
    EckInline bool SetStr(PCSTR pszVal) const { return yyjson_set_str(Ptr(), pszVal); }
    EckInline bool SetStr(PCSTR pszVal, size_t cchVal) const { return yyjson_set_strn(Ptr(), pszVal, cchVal); }

    EckInlineNd size_t ArrSize() const { return yyjson_arr_size(Ptr()); }
    EckInlineNd CVal ArrAt(size_t idx) const { return CVal(yyjson_arr_get(Ptr(), idx)); }
    EckInlineNd CVal ArrFront() const { return CVal(yyjson_arr_get_first(Ptr())); }
    EckInlineNd CVal ArrBack() const { return CVal(yyjson_arr_get_last(Ptr())); }
    EckInlineNd size_t ObjSize() const { return yyjson_obj_size(Ptr()); }
    EckInlineNd CVal ObjAt(PCSTR pszKey) const { return CVal(yyjson_obj_get(Ptr(), pszKey)); }
    EckInlineNd CVal ObjAt(PCSTR pszKey, size_t cchKey) const { return CVal(yyjson_obj_getn(Ptr(), pszKey, cchKey)); }
    EckInlineNd CVal ObjGetVal(CVal Key) const { return CVal(yyjson_obj_iter_get_val(Key.Ptr())); }

    EckInlineNd PSTR Write(size_t* pcchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
    {
        return yyjson_val_write_opts(Ptr(), uFlags, pAlc, pcchOut, pErr);
    }
    EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
    {
        return yyjson_val_write_file(pszFile, Ptr(), uFlags, pAlc, pErr);
    }
    CRefStrW WriteW(YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
    {
        size_t cchU8{};
        PSTR pszU8 = Write(&cchU8, uFlags, pAlc, pErr);
        return Priv::WriteW(pszU8, cchU8, pAlc);
    }

    EckInlineNd CVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax, YyPtrErr* pErr = nullptr) const
    {
        return CVal(yyjson_ptr_getx(Ptr(), pszPtr,
            cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pErr));
    }
    [[nodiscard]] CVal operator[](const auto& x) const { return Priv::JsonValAtVarType(*this, x); }

    EckInlineNd CArrProxy AsArr() const;
    EckInlineNd CObjProxy AsObj() const;
};

class CDoc
{
private:
    YyDoc* m_pDoc{};
public:
    ECK_DISABLE_COPY_DEF_CONS(CDoc);
    CDoc(PCSTR pszJson, size_t cchJson = SizeTMax, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
    {
        m_pDoc = yyjson_read_opts((PSTR)pszJson, cchJson == SizeTMax ? strlen(pszJson) : cchJson,
            uFlags, pAlc, pErr);
    }

    template<class TAlloc>
    CDoc(const CRefBinT<TAlloc>& rb, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
        :CDoc((PCSTR)rb.Data(), rb.Size(), uFlags, pAlc, pErr) {
    }

    template<class TTraits, class TAlloc>
    CDoc(const CRefStrT<CHAR, TTraits, TAlloc>& rs, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
        : CDoc(rs.Data(), rs.Size(), uFlags, pAlc, pErr) {
    }

    template<class TTraits, class TAlloc>
    CDoc(const std::basic_string<CHAR, TTraits, TAlloc>& s, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
        : CDoc(s.c_str(), s.size(), uFlags, pAlc, pErr) {
    }

    template<class TTraits>
    CDoc(const std::basic_string_view<CHAR, TTraits>& sv, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
        : CDoc(sv.data(), sv.size(), uFlags, pAlc, pErr) {
    }

    CDoc(const char8_t* pszJson, size_t cchJson = SizeTMax, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr)
        :CDoc((PCSTR)pszJson, cchJson, uFlags, pAlc, pErr) {
    }

    constexpr CDoc(YyDoc* pDoc) : m_pDoc{ pDoc } {}
    constexpr CDoc(CDoc&& x) noexcept : m_pDoc{ x.Detach() } {}
    constexpr CDoc& operator=(CDoc&& x) noexcept
    {
        std::swap(m_pDoc, x.m_pDoc);
        return *this;
    }
    ~CDoc() { Free(); }

    EckInlineNd constexpr BOOL IsValid() const { return !!m_pDoc; }
    EckInlineNd constexpr YyDoc* GetDocPtr() const { return m_pDoc; }
    EckInline void Free()
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

    EckInlineNd CVal GetRoot() const { return CVal(yyjson_doc_get_root(m_pDoc)); }
    EckInlineNd size_t GetReadSize() const { return yyjson_doc_get_read_size(m_pDoc); }
    EckInlineNd size_t GetValCount() const { return yyjson_doc_get_val_count(m_pDoc); }
    EckInlineNd CVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax, YyPtrErr* pErr = nullptr) const
    {
        return CVal(yyjson_doc_ptr_getx(m_pDoc, pszPtr, cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pErr));
    }
    EckInlineNd PSTR Write(size_t* pcchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
    {
        return yyjson_write_opts(m_pDoc, uFlags, pAlc, pcchOut, pErr);
    }
    EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
    {
        return yyjson_write_file(pszFile, m_pDoc, uFlags, pAlc, pErr);
    }
    CRefStrW WriteW(YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
    {
        size_t cchU8{};
        PSTR pszU8 = Write(&cchU8, uFlags, pAlc, pErr);
        return Priv::WriteW(pszU8, cchU8, pAlc);
    }
    EckInlineNd CMutDoc Clone(const YyAlc* pAlc = nullptr) const;
    EckInlineNd CVal operator[](PCSTR pszKey) const
    {
        return Priv::JsonValAt(*this, pszKey);
    }
};

struct CArrIter
{
    YyArrIter m_iter{};

    CArrIter() = default;
    constexpr CArrIter(YyArrIter iter) : m_iter{ iter } {}
    CArrIter(CVal val) : m_iter{ yyjson_arr_iter_with(val.Ptr()) } {}

    EckInline void FromVal(CVal val) { m_iter = yyjson_arr_iter_with(val.Ptr()); }
    EckInlineNd BOOL HasNext() const { return yyjson_arr_iter_has_next((YyArrIter*)&m_iter); }
    EckInlineNd CVal Next() { return CVal(yyjson_arr_iter_next(&m_iter)); }
    EckInline constexpr CVal GetCurr() const { return CVal(m_iter.cur); }
    EckInline CArrIter& operator++() { Next(); return *this; }
    EckInline constexpr CVal operator*() const { return GetCurr(); }
};
EckInline bool operator==(const CArrIter& x, const CArrIter& y) { return Priv::EqualIterator<CArrIter>(x, y); }

struct CObjIter
{
    YyObjIter m_iter{};

    CObjIter() = default;
    constexpr CObjIter(YyObjIter iter) : m_iter{ iter } {}
    CObjIter(CVal val) :m_iter{ yyjson_obj_iter_with(val.Ptr()) } {}

    EckInline void FromVal(CVal val) { m_iter = yyjson_obj_iter_with(val.Ptr()); }
    EckInlineNd BOOL HasNext() const { return yyjson_obj_iter_has_next((YyObjIter*)&m_iter); }
    EckInlineNd CVal Next() { return CVal(yyjson_obj_iter_next(&m_iter)); }
    EckInlineNd CVal Get(PCSTR pszKey) { return yyjson_obj_iter_get(&m_iter, pszKey); }
    EckInlineNd CVal Get(PCSTR pszKey, size_t cchKey) { return yyjson_obj_iter_getn(&m_iter, pszKey, cchKey); }
    EckInline constexpr CVal GetCurr() const { return CVal(m_iter.cur); }
    EckInline CObjIter& operator++() { Next(); return *this; }
    EckInline constexpr CVal operator*() const { return GetCurr(); }
};
EckInline bool operator==(const CObjIter& x, const CObjIter& y) { return Priv::EqualIterator<CObjIter>(x, y); }

struct CArrProxy
{
    CVal Val;
    EckInline CArrIter begin() const { return CArrIter{ Val }; }
    EckInline CArrIter end() const { return CArrIter{}; }
};
struct CObjProxy
{
    CVal Val;
    EckInline CObjIter begin() const { return CObjIter{ Val }; }
    EckInline CObjIter end() const { return CObjIter{}; }
};

class CMutVal : public Priv::CValBase
{
private:
    const CMutDoc* m_pDoc{};
public:
    constexpr CMutVal(YyMutVal* pVal, const CMutDoc* pDoc = nullptr) : CValBase{ pVal }, m_pDoc{ pDoc } {}
    EckInlineNd constexpr auto Ptr() const { return (YyMutVal*)m_pVal; }
    EckInlineNd constexpr BOOL IsValid() const { return !!m_pVal; }

    EckInline void SetRaw(PCSTR pszRaw, size_t cchRaw) const { return unsafe_yyjson_set_raw(Ptr(), pszRaw, cchRaw); }
    EckInline void SetNull() const { return unsafe_yyjson_set_null(Ptr()); }
    EckInline void SetBool(bool bVal) const { return unsafe_yyjson_set_bool(Ptr(), bVal); }
    EckInline void SetUInt64(uint64_t uVal) const { return unsafe_yyjson_set_uint(Ptr(), uVal); }
    EckInline void SetInt64(int64_t iVal) const { return unsafe_yyjson_set_sint(Ptr(), iVal); }
    EckInline void SetInt(int iVal) const { return unsafe_yyjson_set_sint(Ptr(), iVal); }
    EckInline void SetReal(double dVal) const { return unsafe_yyjson_set_real(Ptr(), dVal); }
    EckInline void SetStr(PCSTR pszVal) const { return unsafe_yyjson_set_str(Ptr(), pszVal); }
    EckInline void SetStr(PCSTR pszVal, size_t cchVal) const { return unsafe_yyjson_set_strn(Ptr(), pszVal, cchVal); }
    EckInline void SetArr(size_t c = 0) const { return unsafe_yyjson_set_arr(Ptr(), c); }
    EckInline void SetObj(size_t c = 0) const { return unsafe_yyjson_set_obj(Ptr(), c); }

    EckInlineNd size_t ArrSize() const { return yyjson_mut_arr_size(Ptr()); }
    EckInlineNd CMutVal ArrAt(size_t idx) const { return CMutVal(yyjson_mut_arr_get(Ptr(), idx), m_pDoc); }
    EckInlineNd CMutVal ArrFront() const { return CMutVal(yyjson_mut_arr_get_first(Ptr()), m_pDoc); }
    EckInlineNd CMutVal ArrBack() const { return CMutVal(yyjson_mut_arr_get_last(Ptr()), m_pDoc); }
    EckInline BOOL ArrInsert(size_t idx, CMutVal Val) const
    {
        return yyjson_mut_arr_insert(Ptr(), Val.Ptr(), idx);
    }
    EckInline BOOL ArrPushBack(CMutVal Val) const { return yyjson_mut_arr_append(Ptr(), Val.Ptr()); }
    EckInline BOOL ArrPushFront(CMutVal Val) const { return yyjson_mut_arr_prepend(Ptr(), Val.Ptr()); }
    EckInline CMutVal ArrReplace(size_t idx, CMutVal Val) const
    {
        return CMutVal(yyjson_mut_arr_replace(Ptr(), idx, Val.Ptr()), m_pDoc);
    }
    EckInline CMutVal ArrRemove(size_t idx) const { return CMutVal(yyjson_mut_arr_remove(Ptr(), idx), m_pDoc); }
    EckInline BOOL ArrRemove(size_t idx, size_t c) const
    {
        return yyjson_mut_arr_remove_range(Ptr(), idx, c);
    }
    EckInline CMutVal ArrPopBack() const { return CMutVal(yyjson_mut_arr_remove_last(Ptr()), m_pDoc); }
    EckInline CMutVal ArrPopFront() const { return CMutVal(yyjson_mut_arr_remove_first(Ptr()), m_pDoc); }
    EckInline BOOL ArrClear() const { return yyjson_mut_arr_clear(Ptr()); }
    EckInline BOOL ArrRotate(size_t idx) const { return yyjson_mut_arr_rotate(Ptr(), idx); }

    EckInlineNd size_t ObjSize() const { return yyjson_mut_obj_size(Ptr()); }
    EckInlineNd CMutVal ObjAt(PCSTR pszKey) const { return CMutVal(yyjson_mut_obj_get(Ptr(), pszKey), m_pDoc); }
    EckInlineNd CMutVal ObjAt(PCSTR pszKey, size_t cchKey) const
    {
        return CMutVal(yyjson_mut_obj_getn(Ptr(), pszKey, cchKey), m_pDoc);
    }
    EckInline BOOL ObjInsert(size_t idx, CMutVal Key, CMutVal Val) const
    {
        return yyjson_mut_obj_insert(Ptr(), Key.Ptr(), Val.Ptr(), idx);
    }
    EckInline CMutVal ObjRemove(CMutVal Key) const
    {
        return CMutVal(yyjson_mut_obj_remove(Ptr(), Key.Ptr()), m_pDoc);
    }
    EckInline CMutVal ObjRemove(PCSTR pszKey) const
    {
        return CMutVal(yyjson_mut_obj_remove_key(Ptr(), pszKey), m_pDoc);
    }
    EckInline BOOL ObjClear() const { return yyjson_mut_obj_clear(Ptr()); }
    EckInline BOOL ObjReplace(CMutVal Key, CMutVal Val) const
    {
        return yyjson_mut_obj_replace(Ptr(), Key.Ptr(), Val.Ptr());
    }
    EckInline BOOL ObjRotate(size_t idx) const { return yyjson_mut_obj_rotate(Ptr(), idx); }

    EckInlineNd PSTR Write(size_t& cchOut, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
    {
        return yyjson_mut_val_write_opts(Ptr(), uFlags, pAlc, &cchOut, pErr);
    }
    EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
    {
        return yyjson_mut_val_write_file(pszFile, Ptr(), uFlags, pAlc, pErr);
    }
    EckInlineNd CRefStrW WriteW(YyWriteFlag uFlags = 0, YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr)
    {
        size_t cchOut;
        const auto pszU8 = Write(cchOut, uFlags, pAlc, pErr);
        return Priv::WriteW(pszU8, cchOut, pAlc);
    }

    EckInlineNd CMutVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax,
        YyPtrCtx* pCtx = nullptr, YyPtrErr* pErr = nullptr) const
    {
        return CMutVal(yyjson_mut_ptr_getx(Ptr(), pszPtr,
            cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pCtx, pErr));
    }

    EckInlineNd CMutVal operator[](const auto& x) const { return Priv::JsonValAtVarType(*this, x); }
    EckInline const CMutVal& operator=(Priv::CInitProxy x) const;
    EckInlineCe void SetParentDoc(const CMutDoc* pDoc) { m_pDoc = pDoc; }

    EckInlineNd CMutArrProxy AsArr() const;
    EckInlineNd CMutObjProxy AsObj() const;
};

class CMutDoc
{
private:
    YyMutDoc* m_pDoc{};
public:
    ECK_DISABLE_COPY(CMutDoc);

    CMutDoc() : m_pDoc{ yyjson_mut_doc_new(nullptr) } {}
    explicit CMutDoc(const CDoc& Doc, const YyAlc* pAlc = nullptr) : m_pDoc{ yyjson_doc_mut_copy(Doc.GetDocPtr(), pAlc) } {}
    explicit CMutDoc(const CMutDoc& Doc, const YyAlc* pAlc = nullptr) : m_pDoc{ yyjson_mut_doc_mut_copy(Doc.GetDocPtr(), pAlc) } {}
    explicit constexpr CMutDoc(YyMutDoc* pDoc) : m_pDoc{ pDoc } {}
    constexpr CMutDoc(CMutDoc&& x) noexcept : m_pDoc{ x.Detach() } {}
    CMutDoc& operator=(CMutDoc&& x) noexcept
    {
        std::swap(m_pDoc, x.m_pDoc);
        return *this;
    }
    ~CMutDoc() { Free(); }

    EckInline void Create(const YyAlc* pAlc = nullptr)
    {
        Free();
        m_pDoc = yyjson_mut_doc_new(pAlc);
    }
    EckInline void Create(const CDoc& Doc, const YyAlc* pAlc = nullptr)
    {
        Free();
        m_pDoc = yyjson_doc_mut_copy(Doc.GetDocPtr(), pAlc);
    }
    EckInline void Create(const CMutDoc& Doc, const YyAlc* pAlc = nullptr)
    {
        Free();
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
        const auto pDoc = m_pDoc;
        m_pDoc = nullptr;
        return pDoc;
    }
    EckInline constexpr YyMutDoc* Attach(YyMutDoc* pDoc)
    {
        const auto pOldDoc = m_pDoc;
        m_pDoc = pDoc;
        return pOldDoc;
    }

    EckInlineNd CMutVal GetRoot() const { return CMutVal(yyjson_mut_doc_get_root(m_pDoc)); }
    EckInline void SetRoot(CMutVal Val) const { yyjson_mut_doc_set_root(m_pDoc, Val.Ptr()); }
    EckInline BOOL SetStringPoolSize(size_t cb) const { return yyjson_mut_doc_set_str_pool_size(m_pDoc, cb); }
    EckInline BOOL SetValuePoolSize(size_t cb) const { return yyjson_mut_doc_set_val_pool_size(m_pDoc, cb); }
    EckInlineNd CMutVal ValAt(PCSTR pszPtr, size_t cchPtr = SizeTMax,
        YyPtrCtx* pCtx = nullptr, YyPtrErr* pErr = nullptr) const
    {
        return CMutVal(yyjson_mut_doc_ptr_getx(m_pDoc, pszPtr,
            cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pCtx, pErr), this);
    }
    EckInlineNd PSTR Write(size_t& cchOut, YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const
    {
        return yyjson_mut_write_opts(m_pDoc, uFlags, pAlc, &cchOut, pErr);
    }
    EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const
    {
        return yyjson_mut_write_file(pszFile, m_pDoc, uFlags, pAlc, pErr);
    }
    EckInlineNd CRefStrW WriteW(YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const
    {
        size_t cchOut;
        const auto pszU8 = Write(cchOut, uFlags, pAlc, pErr);
        return Priv::WriteW(pszU8, cchOut, pAlc);
    }

    EckInlineNd CMutDoc Clone() const { return CMutDoc(yyjson_mut_doc_mut_copy(m_pDoc, nullptr)); }
    EckInlineNd CDoc CloneImut() const { return CDoc(yyjson_mut_doc_imut_copy(m_pDoc, nullptr)); }

    EckInlineNd CMutVal NewRaw(PCSTR pszRaw, size_t cchRaw = SizeTMax) const
    {
        return CMutVal(yyjson_mut_rawn(m_pDoc, pszRaw, cchRaw == SizeTMax ? strlen(pszRaw) : cchRaw), this);
    }
    EckInlineNd CMutVal NewRawCpy(PCSTR pszRaw, size_t cchRaw = SizeTMax) const
    {
        return CMutVal(yyjson_mut_rawncpy(m_pDoc, pszRaw, cchRaw == SizeTMax ? strlen(pszRaw) : cchRaw), this);
    }
    EckInlineNd CMutVal NewNull() const { return CMutVal(yyjson_mut_null(m_pDoc), this); }
    EckInlineNd CMutVal NewTrue() const { return CMutVal(yyjson_mut_true(m_pDoc), this); }
    EckInlineNd CMutVal NewFalse() const { return CMutVal(yyjson_mut_false(m_pDoc), this); }
    EckInlineNd CMutVal NewBool(bool bVal) const { return CMutVal(yyjson_mut_bool(m_pDoc, bVal), this); }
    EckInlineNd CMutVal NewUInt64(uint64_t uVal) const { return CMutVal(yyjson_mut_uint(m_pDoc, uVal), this); }
    EckInlineNd CMutVal NewInt64(int64_t iVal) const { return CMutVal(yyjson_mut_sint(m_pDoc, iVal), this); }
    EckInlineNd CMutVal NewInt(int iVal) const { return CMutVal(yyjson_mut_int(m_pDoc, iVal), this); }
    EckInlineNd CMutVal NewReal(double dVal) const { return CMutVal(yyjson_mut_real(m_pDoc, dVal), this); }
    EckInlineNd CMutVal NewStr(PCSTR pszVal, size_t cchVal = SizeTMax) const
    {
        return CMutVal(yyjson_mut_strn(m_pDoc, pszVal, cchVal == SizeTMax ? strlen(pszVal) : cchVal), this);
    }
    EckInlineNd CMutVal NewStrCpy(PCSTR pszVal, size_t cchVal = SizeTMax) const
    {
        return CMutVal(yyjson_mut_strncpy(m_pDoc, pszVal, cchVal == SizeTMax ? strlen(pszVal) : cchVal), this);
    }
    EckInlineNd CMutVal NewStrCpy(PCWSTR pszVal, size_t cchVal = SizeTMax) const
    {
        if (cchVal == SizeTMax)
            cchVal = wcslen(pszVal);
        const auto u8 = StrW2X(pszVal, (int)cchVal, CP_UTF8);
        return NewStrCpy(u8.Data(), u8.Size());
    }
    EckInlineNd CMutVal NewArr() const { return CMutVal(yyjson_mut_arr(m_pDoc), this); }
    EckInlineNd CMutVal NewArr(const bool* pbVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_bool(m_pDoc, pbVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const int8_t* piVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_sint8(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const int16_t* piVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_sint16(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const int32_t* piVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_sint32(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const int64_t* piVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_sint64(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const uint8_t* puVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_uint8(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const uint16_t* puVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_uint16(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const uint32_t* puVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_uint32(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const uint64_t* puVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_uint64(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const float* pVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_float(m_pDoc, pVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const double* pdVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_real(m_pDoc, pdVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const char** ppszVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_str(m_pDoc, ppszVals, cVals), this);
    }
    EckInlineNd CMutVal NewArr(const char** ppszVals, const size_t* pcch, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_strn(m_pDoc, ppszVals, pcch, cVals), this);
    }
    EckInlineNd CMutVal NewArrCpy(const char** ppszVals, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_strcpy(m_pDoc, ppszVals, cVals), this);
    }
    EckInlineNd CMutVal NewArrCpy(const char** ppszVals, const size_t* pcch, size_t cVals) const
    {
        return CMutVal(yyjson_mut_arr_with_strncpy(m_pDoc, ppszVals, pcch, cVals), this);
    }

    EckInlineNd CMutVal NewObj() const { return CMutVal(yyjson_mut_obj(m_pDoc), this); }
    EckInlineNd CMutVal NewObj(const char** ppszKeys, const char** pVals, size_t cPairs) const
    {
        return CMutVal(yyjson_mut_obj_with_str(m_pDoc, ppszKeys, pVals, cPairs), this);
    }
    EckInlineNd CMutVal NewObj(const char** ppszKV, size_t cPairs) const
    {
        return CMutVal(yyjson_mut_obj_with_kv(m_pDoc, ppszKV, cPairs), this);
    }

    [[nodiscard]] CMutVal operator[](PCSTR pszKey) const
    {
        return Priv::JsonValAt(*this, pszKey);
    }
    EckInline const CMutDoc& operator=(Priv::CInitProxy x) const;
};

namespace Priv
{
    struct CInitProxy
    {
        enum class Type : UINT
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
            ArrayMark,
            JVal,
        };

        Type eType{};
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
            const std::initializer_list<CInitProxy>* pObj;
            YyMutVal* jVal;
        } v;

        CInitProxy(std::nullptr_t) :eType{ Type::Null } { v.pObj = nullptr; }
        CInitProxy(bool b) :eType{ Type::Bool } { v.b = b; }
        CInitProxy(uint64_t u64) :eType{ Type::UInt64 } { v.u64 = u64; }
        CInitProxy(int64_t i64) :eType{ Type::Int64 } { v.i64 = i64; }
        CInitProxy(int i) :eType{ Type::Int } { v.i = i; }
        CInitProxy(UINT i) : CInitProxy(uint64_t(i)) {}
        CInitProxy(double d) :eType{ Type::Real } { v.d = d; }
        CInitProxy(const char* s) :eType{ Type::String } { v.s = s; }
        CInitProxy(const char8_t* s) :eType{ Type::String } { v.s = (PCSTR)s; }
        CInitProxy(const wchar_t* ws) :eType{ Type::StringW } { v.ws = ws; }
        CInitProxy(JArr_T) :eType{ Type::ArrayMark } { v.pObj = nullptr; }
        CInitProxy(const CMutVal& Val) :eType{ Type::JVal } { v.jVal = Val.Ptr(); }
        CInitProxy(const std::initializer_list<CInitProxy>& il) :eType{ Type::Object } { v.pObj = &il; }
        
        template<CcpEnum T>
        CInitProxy(T e) : CInitProxy(std::underlying_type_t<T>(e)) {}

        template<class TTraits, class TAlloc>
        CInitProxy(const std::basic_string<char, TTraits, TAlloc>& s) : eType{ Type::String }
        {
            v.s = s.data(); cch = (UINT)s.size();
        }
        template<class TTraits, class TAlloc>
        CInitProxy(const std::basic_string<char8_t, TTraits, TAlloc>& s) : eType{ Type::String }
        {
            v.s = (PCSTR)s.data(); cch = (UINT)s.size();
        }
        template<class TTraits, class TAlloc>
        CInitProxy(const std::basic_string<WCHAR, TTraits, TAlloc>& s) : eType{ Type::StringW }
        {
            v.ws = s.data(); cch = (UINT)s.size();
        }
        template<class TTraits>
        CInitProxy(const std::basic_string_view<char, TTraits>& s) : eType{ Type::String }
        {
            v.s = s.data(); cch = (UINT)s.size();
        }
        template<class TTraits>
        CInitProxy(const std::basic_string_view<char8_t, TTraits>& s) : eType{ Type::String }
        {
            v.s = (PCSTR)s.data(); cch = (UINT)s.size();
        }
        template<class TTraits>
        CInitProxy(const std::basic_string_view<WCHAR, TTraits>& s) : eType{ Type::StringW }
        {
            v.ws = s.data(); cch = (UINT)s.size();
        }
        template<class TTraits, class TAlloc>
        CInitProxy(const CRefStrT<char, TTraits, TAlloc>& rs) : eType{ Type::String }
        {
            v.s = rs.Data(); cch = (UINT)rs.Size();
        }
        template<class TTraits, class TAlloc>
        CInitProxy(const CRefStrT<char8_t, TTraits, TAlloc>& rs) : eType{ Type::String }
        {
            v.s = (PCSTR)rs.Data(); cch = (UINT)rs.Size();
        }
        template<class TTraits, class TAlloc>
        CInitProxy(const CRefStrT<WCHAR, TTraits, TAlloc>& rs) : eType{ Type::StringW }
        {
            v.ws = rs.Data(); cch = (UINT)rs.Size();
        }
        template<class TAlloc>
        CInitProxy(const CRefBinT<TAlloc>& rb) : eType{ Type::String }
        {
            v.s = (PCSTR)rb.Data(); cch = (UINT)rb.Size();
        }

        inline CMutVal ToMutVal(const CMutDoc& Doc) const
        {
            switch (eType)
            {
            case Type::Null:    return Doc.NewNull();
            case Type::Bool:    return Doc.NewBool(v.b);
            case Type::Int:     return Doc.NewInt(v.i);
            case Type::Int64:   return Doc.NewInt64(v.i64);
            case Type::UInt64:  return Doc.NewUInt64(v.u64);
            case Type::Real:    return Doc.NewReal(v.d);
            case Type::String:
                if (!v.s || !cch)
                    return Doc.NewStr("", 0);
                else
                    return Doc.NewStrCpy(v.s, cch == UINT_MAX ? strlen(v.s) : cch);
            case Type::StringW:
            {
                const auto u8 = StrW2X(v.ws, (int)cch, CP_UTF8);
                return Doc.NewStrCpy(u8.Data(), u8.Size());
            }
            ECK_UNREACHABLE;
            case Type::JVal:    return CMutVal{ v.jVal };
            case Type::Object:
            {
                if (v.pObj->begin()->eType == Type::ArrayMark)
                {
                    CMutVal Ret{ Doc.NewArr() };
                    for (auto it = v.pObj->begin() + 1; it != v.pObj->end(); ++it)
                        Ret.ArrPushBack(it->ToMutVal(Doc));
                    return Ret;
                }
                else
                {
                    EckAssert(v.pObj->size() % 2 == 0);// 必须为偶数
                    CMutVal Ret{ Doc.NewObj() };
                    CMutVal Key{ nullptr };
                    for (size_t i{}; const auto& e : *v.pObj)
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
            ECK_UNREACHABLE;
            return { nullptr };
        }
        inline void ReplaceMutValue(const CMutDoc& Doc, const CMutVal& Val) const
        {
            switch (eType)
            {
            case Type::Null:    Val.SetNull();         break;
            case Type::Bool:    Val.SetBool(v.b);      break;
            case Type::Int:     Val.SetInt(v.i);       break;
            case Type::Int64:   Val.SetInt64(v.i64);   break;
            case Type::UInt64:  Val.SetUInt64(v.u64);  break;
            case Type::Real:    Val.SetReal(v.d);      break;
            case Type::String:
            {
                if (!v.s || !cch)
                    Val.SetStr("", 0);
                else
                    Val.SetStr(v.s, cch == UINT_MAX ? strlen(v.s) : cch);
            }
            break;
            case Type::StringW:
            {
                const auto u8 = StrW2X(v.ws, (int)cch, CP_UTF8);
                Val.SetStr(u8.Data(), u8.Size());
            }
            break;
            case Type::Object:
            {
                if (v.pObj->begin()->eType == Type::ArrayMark)
                {
                    Val.SetArr();
                    for (auto it = v.pObj->begin() + 1; it != v.pObj->end(); ++it)
                        Val.ArrPushBack(it->ToMutVal(Doc));
                }
                else
                {
                    EckAssert(v.pObj->size() % 2 == 0);// 必须为偶数
                    Val.SetObj();
                    CMutVal Key{ nullptr };
                    for (size_t i{}; const auto& e : *v.pObj)
                    {
                        if (i % 2 == 0)
                            Key = e.ToMutVal(Doc);
                        else
                            Val.ObjInsert(i / 2, Key, e.ToMutVal(Doc));
                        ++i;
                    }
                }
            }
            break;
            default:
                ECK_UNREACHABLE;
            }
        }
    };
}

struct CMutArrIter
{
    YyMutArrIter m_iter{};

    CMutArrIter() = default;
    constexpr CMutArrIter(const YyMutArrIter& iter) : m_iter{ iter } {}
    CMutArrIter(CMutVal val) :m_iter{ yyjson_mut_arr_iter_with(val.Ptr()) } {}

    EckInline void FromVal(CMutVal val) { m_iter = yyjson_mut_arr_iter_with(val.Ptr()); }
    EckInlineNd BOOL HasNext() const { return yyjson_mut_arr_iter_has_next((YyMutArrIter*)&m_iter); }
    EckInlineNd CMutVal Next() { return CMutVal(yyjson_mut_arr_iter_next(&m_iter)); }
    EckInline CMutVal Remove() { return yyjson_mut_arr_iter_remove(&m_iter); }
    EckInline constexpr CMutVal GetCurr() const { return CMutVal(m_iter.cur); }
    EckInline CMutArrIter& operator++() { Next(); return *this; }
    EckInline constexpr CMutVal operator*() const { return GetCurr(); }
};
EckInline bool operator==(const CMutArrIter& x, const CMutArrIter& y) { return Priv::EqualIterator<CMutArrIter>(x, y); }

struct CMutObjIter
{
    YyMutObjIter m_iter;

    CMutObjIter() = default;
    constexpr CMutObjIter(const YyMutObjIter& iter) : m_iter{ iter } {}
    CMutObjIter(CMutVal val) :m_iter{ yyjson_mut_obj_iter_with(val.Ptr()) } {}

    EckInline void FromVal(CMutVal val) { m_iter = yyjson_mut_obj_iter_with(val.Ptr()); }
    EckInlineNd BOOL HasNext() const { return yyjson_mut_obj_iter_has_next((YyMutObjIter*)&m_iter); }
    EckInlineNd CMutVal Next() { return CMutVal(yyjson_mut_obj_iter_next(&m_iter)); }
    EckInlineNd CMutVal Get(PCSTR pszKey) { return yyjson_mut_obj_iter_get(&m_iter, pszKey); }
    EckInlineNd CMutVal Get(PCSTR pszKey, size_t cchKey) { return yyjson_mut_obj_iter_getn(&m_iter, pszKey, cchKey); }
    EckInline CMutVal Remove() { return yyjson_mut_obj_iter_remove(&m_iter); }
    EckInline constexpr CMutVal GetCurr() const { return CMutVal(m_iter.cur); }
    EckInline CMutObjIter& operator++() { Next(); return *this; }
    EckInline constexpr CMutVal operator*() const { return GetCurr(); }
};
EckInline bool operator==(const CMutObjIter& x, const CMutObjIter& y) { return Priv::EqualIterator<CMutObjIter>(x, y); }

struct CMutArrProxy
{
    CMutVal Val;
    EckInline CMutArrIter begin() const { return CMutArrIter{ Val }; }
    EckInline CMutArrIter end() const { return CMutArrIter{}; }
};
struct CMutObjProxy
{
    CMutVal Val;
    EckInline CMutObjIter begin() const { return CMutObjIter{ Val }; }
    EckInline CMutObjIter end() const { return CMutObjIter{}; }
};

EckInline const CMutVal& CMutVal::operator=(Priv::CInitProxy x) const
{
    x.ReplaceMutValue(*m_pDoc, *this);
    return *this;
}
EckInline const CMutDoc& CMutDoc::operator=(Priv::CInitProxy x) const
{
    SetRoot(x.ToMutVal(*this));
    return *this;
}
EckInlineNd CMutDoc CDoc::Clone(const YyAlc* pAlc) const { return CMutDoc(yyjson_doc_mut_copy(m_pDoc, pAlc)); }
EckInlineNd CArrProxy CVal::AsArr() const { return CArrProxy(*this); }
EckInlineNd CObjProxy CVal::AsObj() const { return CObjProxy(*this); }
EckInlineNd CMutArrProxy CMutVal::AsArr() const { return CMutArrProxy(*this); }
EckInlineNd CMutObjProxy CMutVal::AsObj() const { return CMutObjProxy(*this); }
ECK_JSON_NAMESPACE_END
ECK_NAMESPACE_END

#pragma pop_macro("free")
#pragma pop_macro("malloc")
#pragma pop_macro("realloc")