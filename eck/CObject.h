#pragma once
#include "ObjectAttribute.h"

ECK_NAMESPACE_BEGIN
#define ECK_RTTI(Cls, Base)         \
    inline static ::eck::ClassInfo s_ClassInfo_##Cls \
    {                               \
        L###Cls,                    \
        &Base::s_ClassInfo_##Base,  \
        RttiDefaultNewObject<Cls>   \
    };                              \
    constexpr ::eck::ClassInfo* RttiGetClass() const noexcept override \
    { return &s_ClassInfo_##Cls; }	\
    static constexpr ::eck::ClassInfo* RttiThisClass() \
    { return &s_ClassInfo_##Cls; }  \
    inline static ::eck::Priv::ClassRegister s_ClassRegister_##Cls{ &Cls::s_ClassInfo_##Cls };


class CObject;
struct ClassInfo
{
    using FNewObject = CObject * (*)();

    std::wstring_view svClassName;
    ClassInfo* pBaseClass;
    FNewObject pfnNewObject;

    EckInlineNd CObject* NewObject() const noexcept { return pfnNewObject(); }
};

namespace Priv
{
    struct ClassRegister { ClassRegister(ClassInfo* pInfo) noexcept; };
}

// 绝对不能直接实例化此类
class __declspec(novtable) CObject
{
    friend struct Priv::ClassRegister;
public:
    inline static ClassInfo s_ClassInfo_CObject{ L"CObject"sv };
private:
    inline static Priv::ClassRegister s_ClassRegister_CObject{ &s_ClassInfo_CObject };
public:
    virtual ~CObject() = default;

    EckInlineNdCe static ClassInfo* RttiThisClass() noexcept { return &s_ClassInfo_CObject; }

    EckInlineNd static auto& RttiClassMap() noexcept
    {
        static std::unordered_map<std::wstring_view, ClassInfo*> s_hsClassInfo{};
        return s_hsClassInfo;
    }

    EckInlineNdCe virtual ClassInfo* RttiGetClass() const noexcept
    {
        return &s_ClassInfo_CObject;
    }

    EckNfInlineNdCe BOOL RttiIsKindOf(const ClassInfo* pInfo) const noexcept
    {
        auto p = RttiGetClass();
        do
        {
            if (p == pInfo)
                return TRUE;
            p = p->pBaseClass;
        } while (p);
        return FALSE;
    }

    template<class T>
    EckInlineNdCe BOOL RttiIsKindOf() const noexcept
    {
        return RttiIsKindOf(T::RttiThisClass());
    }

    virtual ObjAttrErr GetSetAttribute(std::wstring_view svName,
        std::wstring_view svValue, CRefStrW& rsValue, BOOL bSet) noexcept
    {
        return ObjAttrErr::InvalidAttr;
    }

    EckInline ObjAttrErr GetAttribute(std::wstring_view svName, CRefStrW& rsValue) noexcept
    {
        return GetSetAttribute(svName, {}, rsValue, FALSE);
    }
    EckInline ObjAttrErr SetAttribute(std::wstring_view svName, std::wstring_view svValue) noexcept
    {
        CRefStrW Dummy{};
        return GetSetAttribute(svName, svValue, Dummy, TRUE);
    }
};

inline Priv::ClassRegister::ClassRegister(ClassInfo* pInfo) noexcept
{
    EckAssert(!CObject::RttiClassMap().contains(pInfo->svClassName));
    CObject::RttiClassMap().emplace(pInfo->svClassName, pInfo);
}

template<class T>
inline CObject* RttiDefaultNewObject() noexcept
{
    if constexpr (std::is_abstract_v<T>)
        return nullptr;
    else
        return new T{};
}

EckInline ClassInfo* RttiGetClass(std::wstring_view svClassName) noexcept
{
    const auto it = CObject::RttiClassMap().find(svClassName);
    if (it == CObject::RttiClassMap().end())
        return nullptr;
    return it->second;
}

EckInlineNd CObject* RttiNewObject(std::wstring_view svClassName) noexcept
{
    const auto pInfo = RttiGetClass(svClassName);
    if (!pInfo)
        return nullptr;
    return pInfo->NewObject();
}
ECK_NAMESPACE_END