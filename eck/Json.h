#pragma once
#include "CString.h"
#include "CByteBuffer.h"

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

EckInline BOOL YyLocateStringPosition(PCSTR pszText, size_t cchText, size_t ocbPos,
    size_t& nLine, size_t& nCol, size_t& nChar) noexcept
{
    return yyjson_locate_pos(pszText, cchText, ocbPos, &nLine, &nCol, &nChar);
}

namespace Priv
{
    EckInline auto JsonValueAt(auto& This, PCSTR pszKey, size_t cchKey = SizeTMax) noexcept
    {
        return This.AtValue(pszKey, cchKey);
    }

    template<class TThis, class T>
    EckInline auto JsonValueAtType(TThis& This, const T& x) noexcept
    {
        using T1 = std::remove_cvref_t<T>;
        if constexpr (std::is_integral_v<T1>)
            return This.ArrAt(x);
        else if constexpr (std::is_convertible_v<T1, PCCH> ||
            std::is_convertible_v<T1, PCBYTE> ||
            std::is_convertible_v<T1, const char8_t*>)
        {
            return JsonValueAt(This, (PCSTR)x);
        }
        else if constexpr (IsSameTemplate<CStringT, T1>::V)
            return JsonValueAt(This, x.Data(), x.Size());
        else if constexpr (IsSameTemplate<std::basic_string, T1>::V &&
            sizeof(typename T1::value_type) == 1)
            return JsonValueAt(This, x.c_str(), x.size());
        else if constexpr (IsSameTemplate<std::basic_string_view, T1>::V &&
            sizeof(typename T1::value_type) == 1)
            return JsonValueAt(This, x.data(), x.size());
        else
            static_assert(false, "Unsupported type.");
    }

    template<class T>
    EckInline bool EqualIterator(const T& x, const T& y) noexcept
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

    inline CStringW WriteW(PSTR pszU8, size_t cchU8, _In_opt_ YyAlc* pAlc) noexcept
    {
        if (!pszU8)
            return {};
        CStringW rs{ StrX2W(pszU8, (int)cchU8, CP_UTF8) };
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
        constexpr CValBase(void* p) noexcept : m_pVal{ p } {}
        EckInlineNd YyType GetType() const noexcept { return unsafe_yyjson_get_type(m_pVal); }
        EckInlineNd YySubType GetSubType() const noexcept { return unsafe_yyjson_get_subtype(m_pVal); }
        EckInlineNd uint8_t GetTag() const noexcept { return unsafe_yyjson_get_tag(m_pVal); }
        EckInlineNd PCSTR GetTypeDescription() const noexcept { return yyjson_get_type_desc((yyjson_val*)m_pVal); }

        EckInline bool EqualString(PCSTR pszStr) const noexcept { return yyjson_equals_str((yyjson_val*)m_pVal, pszStr); }
        EckInline bool EqualString(PCSTR pszStr, size_t cchStr) const noexcept { return yyjson_equals_strn((yyjson_val*)m_pVal, pszStr, cchStr); }

        EckInlineNd bool IsNull() const noexcept { return unsafe_yyjson_is_null(m_pVal); }
        EckInlineNd bool IsTrue() const noexcept { return unsafe_yyjson_is_true(m_pVal); }
        EckInlineNd bool IsFalse() const noexcept { return unsafe_yyjson_is_false(m_pVal); }
        EckInlineNd bool IsBool() const noexcept { return unsafe_yyjson_is_bool(m_pVal); }
        EckInlineNd bool IsUInt64() const noexcept { return unsafe_yyjson_is_uint(m_pVal); }
        EckInlineNd bool IsInt64() const noexcept { return unsafe_yyjson_is_sint(m_pVal); }
        EckInlineNd bool IsInt() const noexcept { return unsafe_yyjson_is_int(m_pVal); }
        EckInlineNd bool IsReal() const noexcept { return unsafe_yyjson_is_real(m_pVal); }
        EckInlineNd bool IsNumber() const noexcept { return unsafe_yyjson_is_num(m_pVal); }
        EckInlineNd bool IsString() const noexcept { return unsafe_yyjson_is_str(m_pVal); }
        EckInlineNd bool IsArray() const noexcept { return unsafe_yyjson_is_arr(m_pVal); }
        EckInlineNd bool IsObject() const noexcept { return unsafe_yyjson_is_obj(m_pVal); }
        EckInlineNd bool IsContainer() const noexcept { return unsafe_yyjson_is_ctn(m_pVal); }
        EckInlineNd bool IsRaw() const noexcept { return unsafe_yyjson_is_raw(m_pVal); }

        EckInlineNd PCSTR GetRaw() const noexcept { return yyjson_get_raw((yyjson_val*)m_pVal); }
        EckInlineNd bool GetBool() const noexcept { return yyjson_get_bool((yyjson_val*)m_pVal); }
        EckInlineNd uint64_t GetUInt64() const noexcept { return yyjson_get_uint((yyjson_val*)m_pVal); }
        EckInlineNd int64_t GetInt64() const noexcept { return yyjson_get_sint((yyjson_val*)m_pVal); }
        EckInlineNd int GetInt() const noexcept { return yyjson_get_int((yyjson_val*)m_pVal); }
        EckInlineNd double GetReal() const noexcept { return yyjson_get_real((yyjson_val*)m_pVal); }
        EckInlineNd double GetNumber() const noexcept { return yyjson_get_num((yyjson_val*)m_pVal); }
        EckInlineNd PCSTR GetString() const noexcept { return yyjson_get_str((yyjson_val*)m_pVal); }
        EckInlineNd size_t GetLength() const noexcept { return yyjson_get_len((yyjson_val*)m_pVal); }
        EckInlineNd CStringW GetStringW() const noexcept { return StrX2W(GetString(), (int)GetLength(), CP_UTF8); }
    };
}

class CVal : public Priv::CValBase
{
public:
    constexpr CVal(YyVal* pVal) noexcept : CValBase{ (void*)pVal } {}
    EckInlineNdCe auto GetPointer() const noexcept { return (YyVal*)m_pVal; }
    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_pVal; }

    EckInline bool SetRaw(PCSTR pszRaw, size_t cchRaw) const noexcept { return yyjson_set_raw(GetPointer(), pszRaw, cchRaw); }
    EckInline bool SetNull() const noexcept { return yyjson_set_null(GetPointer()); }
    EckInline bool SetBool(bool bVal) const noexcept { return yyjson_set_bool(GetPointer(), bVal); }
    EckInline bool SetUInt64(uint64_t uVal) const noexcept { return yyjson_set_uint(GetPointer(), uVal); }
    EckInline bool SetInt64(int64_t iVal) const noexcept { return yyjson_set_sint(GetPointer(), iVal); }
    EckInline bool SetInt(int iVal) const noexcept { return yyjson_set_int(GetPointer(), iVal); }
    EckInline bool SetReal(double dVal) const noexcept { return yyjson_set_real(GetPointer(), dVal); }
    EckInline bool SetString(PCSTR pszVal) const noexcept { return yyjson_set_str(GetPointer(), pszVal); }
    EckInline bool SetString(PCSTR pszVal, size_t cchVal) const noexcept { return yyjson_set_strn(GetPointer(), pszVal, cchVal); }

    EckInlineNd size_t ArrSize() const noexcept { return yyjson_arr_size(GetPointer()); }
    EckInlineNd CVal ArrAt(size_t idx) const noexcept { return CVal(yyjson_arr_get(GetPointer(), idx)); }
    EckInlineNd CVal ArrFront() const noexcept { return CVal(yyjson_arr_get_first(GetPointer())); }
    EckInlineNd CVal ArrBack() const noexcept { return CVal(yyjson_arr_get_last(GetPointer())); }
    EckInlineNd size_t ObjSize() const noexcept { return yyjson_obj_size(GetPointer()); }
    EckInlineNd CVal ObjAt(PCSTR pszKey) const noexcept { return CVal(yyjson_obj_get(GetPointer(), pszKey)); }
    EckInlineNd CVal ObjAt(PCSTR pszKey, size_t cchKey) const noexcept { return CVal(yyjson_obj_getn(GetPointer(), pszKey, cchKey)); }
    EckInlineNd CVal ObjGetVal(CVal Key) const noexcept { return CVal(yyjson_obj_iter_get_val(Key.GetPointer())); }

    EckInlineNd PSTR Write(size_t* pcchOut, YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) noexcept
    {
        return yyjson_val_write_opts(GetPointer(), uFlags, pAlc, pcchOut, pErr);
    }
    EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) noexcept
    {
        return yyjson_val_write_file(pszFile, GetPointer(), uFlags, pAlc, pErr);
    }
    CStringW WriteW(YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) noexcept
    {
        size_t cchU8{};
        PSTR pszU8 = Write(&cchU8, uFlags, pAlc, pErr);
        return Priv::WriteW(pszU8, cchU8, pAlc);
    }

    EckInlineNd CVal AtValue(PCSTR pszPtr, size_t cchPtr = SizeTMax,
        YyPtrErr* pErr = nullptr) const noexcept
    {
        return CVal(yyjson_ptr_getx(GetPointer(), pszPtr,
            cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pErr));
    }
    [[nodiscard]] CVal operator[](const auto& x) const noexcept { return Priv::JsonValueAtType(*this, x); }

    EckInlineNd CArrProxy AsArray() const noexcept;
    EckInlineNd CObjProxy AsObject() const noexcept;
};

class CDoc
{
private:
    YyDoc* m_pDoc{};
public:
    ECK_DISABLE_COPY_DEF_CONS(CDoc);
    CDoc(PCSTR pszJson, size_t cchJson = SizeTMax, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr) noexcept
    {
        m_pDoc = yyjson_read_opts((PSTR)pszJson, cchJson == SizeTMax ? strlen(pszJson) : cchJson,
            uFlags, pAlc, pErr);
    }

    template<class TAllocator>
    CDoc(const CByteBufferT<TAllocator>& rb, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr) noexcept
        : CDoc((PCSTR)rb.Data(), rb.Size(), uFlags, pAlc, pErr)
    {
    }

    template<class TTraits, class TAllocator>
    CDoc(const CStringT<CHAR, TTraits, TAllocator>& rs, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr) noexcept
        : CDoc(rs.Data(), rs.Size(), uFlags, pAlc, pErr)
    {
    }

    template<class TTraits, class TAllocator>
    CDoc(const std::basic_string<CHAR, TTraits, TAllocator>& s, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr) noexcept
        : CDoc(s.c_str(), s.size(), uFlags, pAlc, pErr)
    {
    }

    template<class TTraits>
    CDoc(const std::basic_string_view<CHAR, TTraits>& sv, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr) noexcept
        : CDoc(sv.data(), sv.size(), uFlags, pAlc, pErr)
    {
    }

    CDoc(const char8_t* pszJson, size_t cchJson = SizeTMax, YyReadFlag uFlags = 0,
        const YyAlc* pAlc = nullptr, YyReadErr* pErr = nullptr) noexcept
        :CDoc((PCSTR)pszJson, cchJson, uFlags, pAlc, pErr)
    {
    }

    constexpr CDoc(YyDoc* pDoc) noexcept : m_pDoc{ pDoc } {}
    constexpr CDoc(CDoc&& x) noexcept : m_pDoc{ x.Detach() } {}
    constexpr CDoc& operator=(CDoc&& x) noexcept
    {
        std::swap(m_pDoc, x.m_pDoc);
        return *this;
    }
    ~CDoc() { Free(); }

    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_pDoc; }
    EckInlineNdCe YyDoc* GetPointer() const noexcept { return m_pDoc; }
    EckInline void Free() noexcept
    {
        if (m_pDoc)
        {
            yyjson_doc_free(m_pDoc);
            m_pDoc = nullptr;
        }
    }
    EckInlineNdCe YyDoc* Detach() noexcept
    {
        YyDoc* pDoc = m_pDoc;
        m_pDoc = nullptr;
        return pDoc;
    }
    EckInlineCe YyDoc* Attach(YyDoc* pDoc) noexcept
    {
        YyDoc* pOldDoc = m_pDoc;
        m_pDoc = pDoc;
        return pOldDoc;
    }

    EckInlineNd CVal GetRoot() const noexcept { return CVal(yyjson_doc_get_root(m_pDoc)); }
    EckInlineNd size_t GetReadSize() const noexcept { return yyjson_doc_get_read_size(m_pDoc); }
    EckInlineNd size_t GetValueCount() const noexcept { return yyjson_doc_get_val_count(m_pDoc); }
    EckInlineNd CVal AtValue(PCSTR pszPtr, size_t cchPtr = SizeTMax,
        YyPtrErr* pErr = nullptr) const noexcept
    {
        return CVal(yyjson_doc_ptr_getx(m_pDoc, pszPtr,
            cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pErr));
    }
    EckInlineNd PSTR Write(size_t* pcchOut, YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const noexcept
    {
        return yyjson_write_opts(m_pDoc, uFlags, pAlc, pcchOut, pErr);
    }
    EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const noexcept
    {
        return yyjson_write_file(pszFile, m_pDoc, uFlags, pAlc, pErr);
    }
    CStringW WriteW(YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const noexcept
    {
        size_t cchU8{};
        PSTR pszU8 = Write(&cchU8, uFlags, pAlc, pErr);
        return Priv::WriteW(pszU8, cchU8, pAlc);
    }
    EckInlineNd CMutDoc Clone(const YyAlc* pAlc = nullptr) const noexcept;
    EckInlineNd CVal operator[](PCSTR pszKey) const noexcept
    {
        return Priv::JsonValueAt(*this, pszKey);
    }
};

struct CArrIter
{
    YyArrIter m_iter{};

    CArrIter() = default;
    constexpr CArrIter(YyArrIter iter) noexcept : m_iter{ iter } {}
    CArrIter(CVal val) noexcept : m_iter{ yyjson_arr_iter_with(val.GetPointer()) } {}

    EckInline void FromValue(CVal val) noexcept { m_iter = yyjson_arr_iter_with(val.GetPointer()); }
    EckInlineNd BOOL HasNext() const noexcept { return yyjson_arr_iter_has_next((YyArrIter*)&m_iter); }
    EckInlineNd CVal Next() noexcept { return CVal(yyjson_arr_iter_next(&m_iter)); }
    EckInlineCe CVal GetCurrent() const noexcept { return CVal(m_iter.cur); }
    EckInline CArrIter& operator++() noexcept { Next(); return *this; }
    EckInlineCe CVal operator*() const noexcept { return GetCurrent(); }
};
EckInline bool operator==(const CArrIter& x, const CArrIter& y) noexcept
{
    return Priv::EqualIterator<CArrIter>(x, y);
}

struct CObjIter
{
    YyObjIter m_iter{};

    CObjIter() = default;
    constexpr CObjIter(YyObjIter iter) noexcept : m_iter{ iter } {}
    CObjIter(CVal val) noexcept : m_iter{ yyjson_obj_iter_with(val.GetPointer()) } {}

    EckInline void FromValue(CVal val) noexcept { m_iter = yyjson_obj_iter_with(val.GetPointer()); }
    EckInlineNd BOOL HasNext() const noexcept { return yyjson_obj_iter_has_next((YyObjIter*)&m_iter); }
    EckInlineNd CVal Next() noexcept { return CVal(yyjson_obj_iter_next(&m_iter)); }
    EckInlineNd CVal Get(PCSTR pszKey) noexcept { return yyjson_obj_iter_get(&m_iter, pszKey); }
    EckInlineNd CVal Get(PCSTR pszKey, size_t cchKey) noexcept { return yyjson_obj_iter_getn(&m_iter, pszKey, cchKey); }
    EckInlineCe CVal GetCurrent() const noexcept { return CVal(m_iter.cur); }
    EckInline CObjIter& operator++() noexcept { Next(); return *this; }
    EckInlineCe CVal operator*() const noexcept { return GetCurrent(); }
};
EckInline bool operator==(const CObjIter& x, const CObjIter& y) noexcept
{
    return Priv::EqualIterator<CObjIter>(x, y);
}

struct CArrProxy
{
    CVal Val;
    EckInline CArrIter begin() const noexcept { return CArrIter{ Val }; }
    EckInline CArrIter end() const noexcept { return CArrIter{}; }
};
struct CObjProxy
{
    CVal Val;
    EckInline CObjIter begin() const noexcept { return CObjIter{ Val }; }
    EckInline CObjIter end() const noexcept { return CObjIter{}; }
};

class CMutVal : public Priv::CValBase
{
private:
    const CMutDoc* m_pDoc{};
public:
    constexpr CMutVal(YyMutVal* pVal, const CMutDoc* pDoc = nullptr) noexcept
        : CValBase{ pVal }, m_pDoc{ pDoc }
    {
    }
    EckInlineNdCe auto Ptr() const noexcept { return (YyMutVal*)m_pVal; }
    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_pVal; }

    EckInline void SetRaw(PCSTR pszRaw, size_t cchRaw) const noexcept { return unsafe_yyjson_set_raw(Ptr(), pszRaw, cchRaw); }
    EckInline void SetNull() const noexcept { return unsafe_yyjson_set_null(Ptr()); }
    EckInline void SetBool(bool bVal) const noexcept { return unsafe_yyjson_set_bool(Ptr(), bVal); }
    EckInline void SetUInt64(uint64_t uVal) const noexcept { return unsafe_yyjson_set_uint(Ptr(), uVal); }
    EckInline void SetInt64(int64_t iVal) const noexcept { return unsafe_yyjson_set_sint(Ptr(), iVal); }
    EckInline void SetInt(int iVal) const noexcept { return unsafe_yyjson_set_sint(Ptr(), iVal); }
    EckInline void SetReal(double dVal) const noexcept { return unsafe_yyjson_set_real(Ptr(), dVal); }
    EckInline void SetString(PCSTR pszVal) const noexcept { return unsafe_yyjson_set_str(Ptr(), pszVal); }
    EckInline void SetString(PCSTR pszVal, size_t cchVal) const noexcept { return unsafe_yyjson_set_strn(Ptr(), pszVal, cchVal); }
    EckInline void SetArray(size_t c = 0) const noexcept { return unsafe_yyjson_set_arr(Ptr(), c); }
    EckInline void SetObject(size_t c = 0) const noexcept { return unsafe_yyjson_set_obj(Ptr(), c); }

    EckInlineNd size_t ArrSize() const noexcept { return yyjson_mut_arr_size(Ptr()); }
    EckInlineNd CMutVal ArrAt(size_t idx) const noexcept { return CMutVal(yyjson_mut_arr_get(Ptr(), idx), m_pDoc); }
    EckInlineNd CMutVal ArrFront() const noexcept { return CMutVal(yyjson_mut_arr_get_first(Ptr()), m_pDoc); }
    EckInlineNd CMutVal ArrBack() const noexcept { return CMutVal(yyjson_mut_arr_get_last(Ptr()), m_pDoc); }
    EckInline BOOL ArrInsert(size_t idx, CMutVal Val) const noexcept
    {
        return yyjson_mut_arr_insert(Ptr(), Val.Ptr(), idx);
    }
    EckInline BOOL ArrPushBack(CMutVal Val) const noexcept { return yyjson_mut_arr_append(Ptr(), Val.Ptr()); }
    EckInline BOOL ArrPushFront(CMutVal Val) const noexcept { return yyjson_mut_arr_prepend(Ptr(), Val.Ptr()); }
    EckInline CMutVal ArrReplace(size_t idx, CMutVal Val) const noexcept
    {
        return CMutVal(yyjson_mut_arr_replace(Ptr(), idx, Val.Ptr()), m_pDoc);
    }
    EckInline CMutVal ArrRemove(size_t idx) const noexcept { return CMutVal(yyjson_mut_arr_remove(Ptr(), idx), m_pDoc); }
    EckInline BOOL ArrRemove(size_t idx, size_t c) const noexcept
    {
        return yyjson_mut_arr_remove_range(Ptr(), idx, c);
    }
    EckInline CMutVal ArrPopBack() const noexcept { return CMutVal(yyjson_mut_arr_remove_last(Ptr()), m_pDoc); }
    EckInline CMutVal ArrPopFront() const noexcept { return CMutVal(yyjson_mut_arr_remove_first(Ptr()), m_pDoc); }
    EckInline BOOL ArrClear() const noexcept { return yyjson_mut_arr_clear(Ptr()); }
    EckInline BOOL ArrRotate(size_t idx) const noexcept { return yyjson_mut_arr_rotate(Ptr(), idx); }

    EckInlineNd size_t ObjSize() const noexcept { return yyjson_mut_obj_size(Ptr()); }
    EckInlineNd CMutVal ObjAt(PCSTR pszKey) const noexcept { return CMutVal(yyjson_mut_obj_get(Ptr(), pszKey), m_pDoc); }
    EckInlineNd CMutVal ObjAt(PCSTR pszKey, size_t cchKey) const noexcept
    {
        return CMutVal(yyjson_mut_obj_getn(Ptr(), pszKey, cchKey), m_pDoc);
    }
    EckInline BOOL ObjInsert(size_t idx, CMutVal Key, CMutVal Val) const noexcept
    {
        return yyjson_mut_obj_insert(Ptr(), Key.Ptr(), Val.Ptr(), idx);
    }
    EckInline CMutVal ObjRemove(CMutVal Key) const noexcept
    {
        return CMutVal(yyjson_mut_obj_remove(Ptr(), Key.Ptr()), m_pDoc);
    }
    EckInline CMutVal ObjRemove(PCSTR pszKey) const noexcept
    {
        return CMutVal(yyjson_mut_obj_remove_key(Ptr(), pszKey), m_pDoc);
    }
    EckInline BOOL ObjClear() const noexcept { return yyjson_mut_obj_clear(Ptr()); }
    EckInline BOOL ObjReplace(CMutVal Key, CMutVal Val) const noexcept
    {
        return yyjson_mut_obj_replace(Ptr(), Key.Ptr(), Val.Ptr());
    }
    EckInline BOOL ObjRotate(size_t idx) const noexcept { return yyjson_mut_obj_rotate(Ptr(), idx); }

    EckInlineNd PSTR Write(size_t& cchOut, YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const noexcept
    {
        return yyjson_mut_val_write_opts(Ptr(), uFlags, pAlc, &cchOut, pErr);
    }
    EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const noexcept
    {
        return yyjson_mut_val_write_file(pszFile, Ptr(), uFlags, pAlc, pErr);
    }
    EckInlineNd CStringW WriteW(YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const noexcept
    {
        size_t cchOut;
        const auto pszU8 = Write(cchOut, uFlags, pAlc, pErr);
        return Priv::WriteW(pszU8, cchOut, pAlc);
    }

    EckInlineNd CMutVal AtValue(PCSTR pszPtr, size_t cchPtr = SizeTMax,
        YyPtrCtx* pCtx = nullptr, YyPtrErr* pErr = nullptr) const noexcept
    {
        return CMutVal(yyjson_mut_ptr_getx(Ptr(), pszPtr,
            cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pCtx, pErr));
    }

    EckInlineNd CMutVal operator[](const auto& x) const noexcept { return Priv::JsonValueAtType(*this, x); }
    EckInline const CMutVal& operator=(Priv::CInitProxy x) const noexcept;
    EckInlineCe void SetParentDocument(const CMutDoc* pDoc) { m_pDoc = pDoc; }

    EckInlineNd CMutArrProxy AsArray() const noexcept;
    EckInlineNd CMutObjProxy AsObject() const noexcept;
};

class CMutDoc
{
private:
    YyMutDoc* m_pDoc{};
public:
    ECK_DISABLE_COPY(CMutDoc);

    CMutDoc() : m_pDoc{ yyjson_mut_doc_new(nullptr) } {}
    explicit CMutDoc(const CDoc& Doc, const YyAlc* pAlc = nullptr) noexcept
        : m_pDoc{ yyjson_doc_mut_copy(Doc.GetPointer(), pAlc) }
    {
    }
    explicit CMutDoc(const CMutDoc& Doc, const YyAlc* pAlc = nullptr) noexcept
        : m_pDoc{ yyjson_mut_doc_mut_copy(Doc.GetPointer(), pAlc) }
    {
    }
    explicit constexpr CMutDoc(YyMutDoc* pDoc) noexcept : m_pDoc{ pDoc } {}

    constexpr CMutDoc(CMutDoc&& x) noexcept : m_pDoc{ x.Detach() } {}
    CMutDoc& operator=(CMutDoc&& x) noexcept
    {
        std::swap(m_pDoc, x.m_pDoc);
        return *this;
    }
    ~CMutDoc() { Free(); }

    EckInline void Create(const YyAlc* pAlc = nullptr) noexcept
    {
        Free();
        m_pDoc = yyjson_mut_doc_new(pAlc);
    }
    EckInline void Create(const CDoc& Doc, const YyAlc* pAlc = nullptr) noexcept
    {
        Free();
        m_pDoc = yyjson_doc_mut_copy(Doc.GetPointer(), pAlc);
    }
    EckInline void Create(const CMutDoc& Doc, const YyAlc* pAlc = nullptr) noexcept
    {
        Free();
        m_pDoc = yyjson_mut_doc_mut_copy(Doc.GetPointer(), pAlc);
    }

    EckInlineNdCe BOOL IsValid() const noexcept { return !!m_pDoc; }
    EckInlineNdCe YyMutDoc* GetPointer() const noexcept { return m_pDoc; }

    void Free() noexcept
    {
        if (m_pDoc)
        {
            yyjson_mut_doc_free(m_pDoc);
            m_pDoc = nullptr;
        }
    }
    EckInlineNdCe YyMutDoc* Detach()
    {
        const auto pDoc = m_pDoc;
        m_pDoc = nullptr;
        return pDoc;
    }
    EckInlineCe YyMutDoc* Attach(YyMutDoc* pDoc)
    {
        const auto pOldDoc = m_pDoc;
        m_pDoc = pDoc;
        return pOldDoc;
    }

    EckInlineNd CMutVal GetRoot() const noexcept { return CMutVal(yyjson_mut_doc_get_root(m_pDoc)); }
    EckInline void SetRoot(CMutVal Val) const noexcept { yyjson_mut_doc_set_root(m_pDoc, Val.Ptr()); }
    EckInline BOOL SetStringPoolSize(size_t cb) const noexcept { return yyjson_mut_doc_set_str_pool_size(m_pDoc, cb); }
    EckInline BOOL SetValuePoolSize(size_t cb) const noexcept { return yyjson_mut_doc_set_val_pool_size(m_pDoc, cb); }
    EckInlineNd CMutVal AtValue(PCSTR pszPtr, size_t cchPtr = SizeTMax,
        YyPtrCtx* pCtx = nullptr, YyPtrErr* pErr = nullptr) const noexcept
    {
        return CMutVal(yyjson_mut_doc_ptr_getx(m_pDoc, pszPtr,
            cchPtr == SizeTMax ? strlen(pszPtr) : cchPtr, pCtx, pErr), this);
    }
    EckInlineNd PSTR Write(size_t& cchOut, YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const noexcept
    {
        return yyjson_mut_write_opts(m_pDoc, uFlags, pAlc, &cchOut, pErr);
    }
    EckInline BOOL Write(PCSTR pszFile, YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const noexcept
    {
        return yyjson_mut_write_file(pszFile, m_pDoc, uFlags, pAlc, pErr);
    }
    EckInlineNd CStringW WriteW(YyWriteFlag uFlags = 0,
        YyAlc* pAlc = nullptr, YyWriteErr* pErr = nullptr) const noexcept
    {
        size_t cchOut;
        const auto pszU8 = Write(cchOut, uFlags, pAlc, pErr);
        return Priv::WriteW(pszU8, cchOut, pAlc);
    }

    EckInlineNd CMutDoc Clone() const noexcept { return CMutDoc(yyjson_mut_doc_mut_copy(m_pDoc, nullptr)); }
    EckInlineNd CDoc CloneImmutable() const noexcept { return CDoc(yyjson_mut_doc_imut_copy(m_pDoc, nullptr)); }

    EckInlineNd CMutVal NewRaw(PCSTR pszRaw, size_t cchRaw = SizeTMax) const noexcept
    {
        return CMutVal(yyjson_mut_rawn(m_pDoc, pszRaw, cchRaw == SizeTMax ? strlen(pszRaw) : cchRaw), this);
    }
    EckInlineNd CMutVal NewRawCopy(PCSTR pszRaw, size_t cchRaw = SizeTMax) const noexcept
    {
        return CMutVal(yyjson_mut_rawncpy(m_pDoc, pszRaw, cchRaw == SizeTMax ? strlen(pszRaw) : cchRaw), this);
    }
    EckInlineNd CMutVal NewNull() const noexcept { return CMutVal(yyjson_mut_null(m_pDoc), this); }
    EckInlineNd CMutVal NewTrue() const noexcept { return CMutVal(yyjson_mut_true(m_pDoc), this); }
    EckInlineNd CMutVal NewFalse() const noexcept { return CMutVal(yyjson_mut_false(m_pDoc), this); }
    EckInlineNd CMutVal NewBool(bool bVal) const noexcept { return CMutVal(yyjson_mut_bool(m_pDoc, bVal), this); }
    EckInlineNd CMutVal NewUInt64(uint64_t uVal) const noexcept { return CMutVal(yyjson_mut_uint(m_pDoc, uVal), this); }
    EckInlineNd CMutVal NewInt64(int64_t iVal) const noexcept { return CMutVal(yyjson_mut_sint(m_pDoc, iVal), this); }
    EckInlineNd CMutVal NewInt(int iVal) const noexcept { return CMutVal(yyjson_mut_int(m_pDoc, iVal), this); }
    EckInlineNd CMutVal NewReal(double dVal) const noexcept { return CMutVal(yyjson_mut_real(m_pDoc, dVal), this); }
    EckInlineNd CMutVal NewString(PCSTR pszVal, size_t cchVal = SizeTMax) const noexcept
    {
        return CMutVal(yyjson_mut_strn(m_pDoc, pszVal, cchVal == SizeTMax ? strlen(pszVal) : cchVal), this);
    }
    EckInlineNd CMutVal NewStringCopy(PCSTR pszVal, size_t cchVal = SizeTMax) const noexcept
    {
        return CMutVal(yyjson_mut_strncpy(m_pDoc, pszVal, cchVal == SizeTMax ? strlen(pszVal) : cchVal), this);
    }
    EckInlineNd CMutVal NewStringCopy(PCWSTR pszVal, size_t cchVal = SizeTMax) const noexcept
    {
        if (cchVal == SizeTMax)
            cchVal = wcslen(pszVal);
        const auto u8 = StrW2X(pszVal, (int)cchVal, CP_UTF8);
        return NewStringCopy(u8.Data(), u8.Size());
    }
    EckInlineNd CMutVal NewArray() const noexcept { return CMutVal(yyjson_mut_arr(m_pDoc), this); }
    EckInlineNd CMutVal NewArray(const bool* pbVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_bool(m_pDoc, pbVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const int8_t* piVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_sint8(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const int16_t* piVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_sint16(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const int32_t* piVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_sint32(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const int64_t* piVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_sint64(m_pDoc, piVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const uint8_t* puVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_uint8(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const uint16_t* puVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_uint16(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const uint32_t* puVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_uint32(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const uint64_t* puVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_uint64(m_pDoc, puVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const float* pVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_float(m_pDoc, pVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const double* pdVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_real(m_pDoc, pdVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const char** ppszVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_str(m_pDoc, ppszVals, cVals), this);
    }
    EckInlineNd CMutVal NewArray(const char** ppszVals, const size_t* pcch, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_strn(m_pDoc, ppszVals, pcch, cVals), this);
    }
    EckInlineNd CMutVal NewArrayCopy(const char** ppszVals, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_strcpy(m_pDoc, ppszVals, cVals), this);
    }
    EckInlineNd CMutVal NewArrayCopy(const char** ppszVals, const size_t* pcch, size_t cVals) const noexcept
    {
        return CMutVal(yyjson_mut_arr_with_strncpy(m_pDoc, ppszVals, pcch, cVals), this);
    }

    EckInlineNd CMutVal NewObject() const noexcept { return CMutVal(yyjson_mut_obj(m_pDoc), this); }
    EckInlineNd CMutVal NewObject(const char** ppszKeys, const char** pVals, size_t cPairs) const noexcept
    {
        return CMutVal(yyjson_mut_obj_with_str(m_pDoc, ppszKeys, pVals, cPairs), this);
    }
    EckInlineNd CMutVal NewObject(const char** ppszKV, size_t cPairs) const noexcept
    {
        return CMutVal(yyjson_mut_obj_with_kv(m_pDoc, ppszKV, cPairs), this);
    }

    [[nodiscard]] CMutVal operator[](PCSTR pszKey) const noexcept
    {
        return Priv::JsonValueAt(*this, pszKey);
    }
    EckInline const CMutDoc& operator=(Priv::CInitProxy x) const noexcept;
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

        CInitProxy(std::nullptr_t) noexcept : eType{ Type::Null } { v.pObj = nullptr; }
        CInitProxy(bool b) noexcept : eType{ Type::Bool } { v.b = b; }
        CInitProxy(uint64_t u64) noexcept : eType{ Type::UInt64 } { v.u64 = u64; }
        CInitProxy(int64_t i64) noexcept : eType{ Type::Int64 } { v.i64 = i64; }
        CInitProxy(int i) noexcept : eType{ Type::Int } { v.i = i; }
        CInitProxy(UINT i) noexcept : CInitProxy(uint64_t(i)) {}
        CInitProxy(double d) noexcept : eType{ Type::Real } { v.d = d; }
        CInitProxy(const char* s) noexcept : eType{ Type::String } { v.s = s; }
        CInitProxy(const char8_t* s) noexcept : eType{ Type::String } { v.s = (PCSTR)s; }
        CInitProxy(const wchar_t* ws) noexcept : eType{ Type::StringW } { v.ws = ws; }
        CInitProxy(JArr_T) noexcept : eType{ Type::ArrayMark } { v.pObj = nullptr; }
        CInitProxy(const CMutVal& Val) noexcept : eType{ Type::JVal } { v.jVal = Val.Ptr(); }
        CInitProxy(const std::initializer_list<CInitProxy>& il) noexcept : eType{ Type::Object } { v.pObj = &il; }

        template<CcpEnum T>
        CInitProxy(T e) noexcept : CInitProxy(std::underlying_type_t<T>(e)) {}

        template<class TTraits, class TAllocator>
        CInitProxy(const std::basic_string<char, TTraits, TAllocator>& s) noexcept : eType{ Type::String }
        {
            v.s = s.data(); cch = (UINT)s.size();
        }
        template<class TTraits, class TAllocator>
        CInitProxy(const std::basic_string<char8_t, TTraits, TAllocator>& s) noexcept : eType{ Type::String }
        {
            v.s = (PCSTR)s.data(); cch = (UINT)s.size();
        }
        template<class TTraits, class TAllocator>
        CInitProxy(const std::basic_string<WCHAR, TTraits, TAllocator>& s) noexcept : eType{ Type::StringW }
        {
            v.ws = s.data(); cch = (UINT)s.size();
        }
        template<class TTraits>
        CInitProxy(const std::basic_string_view<char, TTraits>& s) noexcept : eType{ Type::String }
        {
            v.s = s.data(); cch = (UINT)s.size();
        }
        template<class TTraits>
        CInitProxy(const std::basic_string_view<char8_t, TTraits>& s) noexcept : eType{ Type::String }
        {
            v.s = (PCSTR)s.data(); cch = (UINT)s.size();
        }
        template<class TTraits>
        CInitProxy(const std::basic_string_view<WCHAR, TTraits>& s) noexcept : eType{ Type::StringW }
        {
            v.ws = s.data(); cch = (UINT)s.size();
        }
        template<class TTraits, class TAllocator>
        CInitProxy(const CStringT<char, TTraits, TAllocator>& rs) noexcept : eType{ Type::String }
        {
            v.s = rs.Data(); cch = (UINT)rs.Size();
        }
        template<class TTraits, class TAllocator>
        CInitProxy(const CStringT<char8_t, TTraits, TAllocator>& rs) noexcept : eType{ Type::String }
        {
            v.s = (PCSTR)rs.Data(); cch = (UINT)rs.Size();
        }
        template<class TTraits, class TAllocator>
        CInitProxy(const CStringT<WCHAR, TTraits, TAllocator>& rs) noexcept : eType{ Type::StringW }
        {
            v.ws = rs.Data(); cch = (UINT)rs.Size();
        }
        template<class TAllocator>
        CInitProxy(const CByteBufferT<TAllocator>& rb) noexcept : eType{ Type::String }
        {
            v.s = (PCSTR)rb.Data(); cch = (UINT)rb.Size();
        }

        inline CMutVal ToMutableValue(const CMutDoc& Doc) const noexcept
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
                    return Doc.NewString("", 0);
                else
                    return Doc.NewStringCopy(v.s, cch == UINT_MAX ? strlen(v.s) : cch);
            case Type::StringW:
            {
                const auto u8 = StrW2X(v.ws, (int)cch, CP_UTF8);
                return Doc.NewStringCopy(u8.Data(), u8.Size());
            }
            ECK_UNREACHABLE;
            case Type::JVal:    return CMutVal{ v.jVal };
            case Type::Object:
            {
                if (v.pObj->begin()->eType == Type::ArrayMark)
                {
                    CMutVal Ret{ Doc.NewArray() };
                    for (auto it = v.pObj->begin() + 1; it != v.pObj->end(); ++it)
                        Ret.ArrPushBack(it->ToMutableValue(Doc));
                    return Ret;
                }
                else
                {
                    EckAssert(v.pObj->size() % 2 == 0);// 必须为偶数
                    CMutVal Ret{ Doc.NewObject() };
                    CMutVal Key{ nullptr };
                    for (size_t i{}; const auto& e : *v.pObj)
                    {
                        if (i % 2 == 0)
                            Key = e.ToMutableValue(Doc);
                        else
                            Ret.ObjInsert(i / 2, Key, e.ToMutableValue(Doc));
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
        inline void ReplaceMutValue(const CMutDoc& Doc, const CMutVal& Val) const noexcept
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
                    Val.SetString("", 0);
                else
                    Val.SetString(v.s, cch == UINT_MAX ? strlen(v.s) : cch);
            }
            break;
            case Type::StringW:
            {
                const auto u8 = StrW2X(v.ws, (int)cch, CP_UTF8);
                Val.SetString(u8.Data(), u8.Size());
            }
            break;
            case Type::Object:
            {
                if (v.pObj->begin()->eType == Type::ArrayMark)
                {
                    Val.SetArray();
                    for (auto it = v.pObj->begin() + 1; it != v.pObj->end(); ++it)
                        Val.ArrPushBack(it->ToMutableValue(Doc));
                }
                else
                {
                    EckAssert(v.pObj->size() % 2 == 0);// 必须为偶数
                    Val.SetObject();
                    CMutVal Key{ nullptr };
                    for (size_t i{}; const auto& e : *v.pObj)
                    {
                        if (i % 2 == 0)
                            Key = e.ToMutableValue(Doc);
                        else
                            Val.ObjInsert(i / 2, Key, e.ToMutableValue(Doc));
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

    EckInline void FromValue(CMutVal val) { m_iter = yyjson_mut_arr_iter_with(val.Ptr()); }
    EckInlineNd BOOL HasNext() const noexcept { return yyjson_mut_arr_iter_has_next((YyMutArrIter*)&m_iter); }
    EckInlineNd CMutVal Next() { return CMutVal(yyjson_mut_arr_iter_next(&m_iter)); }
    EckInline CMutVal Remove() { return yyjson_mut_arr_iter_remove(&m_iter); }
    EckInlineCe CMutVal GetCurrent() const noexcept { return CMutVal(m_iter.cur); }
    EckInline CMutArrIter& operator++() { Next(); return *this; }
    EckInlineCe CMutVal operator*() const noexcept { return GetCurrent(); }
};
EckInline bool operator==(const CMutArrIter& x, const CMutArrIter& y) { return Priv::EqualIterator<CMutArrIter>(x, y); }

struct CMutObjIter
{
    YyMutObjIter m_iter;

    CMutObjIter() = default;
    constexpr CMutObjIter(const YyMutObjIter& iter) : m_iter{ iter } {}
    CMutObjIter(CMutVal val) :m_iter{ yyjson_mut_obj_iter_with(val.Ptr()) } {}

    EckInline void FromValue(CMutVal val) { m_iter = yyjson_mut_obj_iter_with(val.Ptr()); }
    EckInlineNd BOOL HasNext() const noexcept { return yyjson_mut_obj_iter_has_next((YyMutObjIter*)&m_iter); }
    EckInlineNd CMutVal Next() { return CMutVal(yyjson_mut_obj_iter_next(&m_iter)); }
    EckInlineNd CMutVal Get(PCSTR pszKey) { return yyjson_mut_obj_iter_get(&m_iter, pszKey); }
    EckInlineNd CMutVal Get(PCSTR pszKey, size_t cchKey) { return yyjson_mut_obj_iter_getn(&m_iter, pszKey, cchKey); }
    EckInline CMutVal Remove() { return yyjson_mut_obj_iter_remove(&m_iter); }
    EckInlineCe CMutVal GetCurrent() const noexcept { return CMutVal(m_iter.cur); }
    EckInline CMutObjIter& operator++() { Next(); return *this; }
    EckInlineCe CMutVal operator*() const noexcept { return GetCurrent(); }
};
EckInline bool operator==(const CMutObjIter& x, const CMutObjIter& y) { return Priv::EqualIterator<CMutObjIter>(x, y); }

struct CMutArrProxy
{
    CMutVal Val;
    EckInline CMutArrIter begin() const noexcept { return CMutArrIter{ Val }; }
    EckInline CMutArrIter end() const noexcept { return CMutArrIter{}; }
};
struct CMutObjProxy
{
    CMutVal Val;
    EckInline CMutObjIter begin() const noexcept { return CMutObjIter{ Val }; }
    EckInline CMutObjIter end() const noexcept { return CMutObjIter{}; }
};

EckInline const CMutVal& CMutVal::operator=(Priv::CInitProxy x) const noexcept
{
    x.ReplaceMutValue(*m_pDoc, *this);
    return *this;
}
EckInline const CMutDoc& CMutDoc::operator=(Priv::CInitProxy x) const noexcept
{
    SetRoot(x.ToMutableValue(*this));
    return *this;
}
EckInlineNd CMutDoc CDoc::Clone(const YyAlc* pAlc) const noexcept { return CMutDoc(yyjson_doc_mut_copy(m_pDoc, pAlc)); }
EckInlineNd CArrProxy CVal::AsArray() const noexcept { return CArrProxy(*this); }
EckInlineNd CObjProxy CVal::AsObject() const noexcept { return CObjProxy(*this); }
EckInlineNd CMutArrProxy CMutVal::AsArray() const noexcept { return CMutArrProxy(*this); }
EckInlineNd CMutObjProxy CMutVal::AsObject() const noexcept { return CMutObjProxy(*this); }
ECK_JSON_NAMESPACE_END
ECK_NAMESPACE_END

#pragma pop_macro("free")
#pragma pop_macro("malloc")
#pragma pop_macro("realloc")