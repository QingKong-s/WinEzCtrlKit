#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CObject;
struct ClassInfo
{
	using FNewObject = CObject * (*)();

	std::wstring_view svClassName;
	ClassInfo* pBaseClass;
	FNewObject pfnNewObject;

	EckInline [[nodiscard]] CObject* NewObject() const { return pfnNewObject(); }
};

namespace Priv
{
	struct ClassInfoRegister { ClassInfoRegister(ClassInfo* pInfo); };
}

// 绝对不能直接实例化此类
class __declspec(novtable) CObject
{
	friend struct Priv::ClassInfoRegister;
private:
	static Priv::ClassInfoRegister s_ClassInfoRegister_CObject;
public:
	static ClassInfo s_ClassInfo_CObject;

	virtual ~CObject() = default;

	static constexpr ClassInfo* RttiClassInfo() { return &s_ClassInfo_CObject; }

	[[nodiscard]] virtual constexpr ClassInfo* RttiGetClassInfo() const
	{
		return &s_ClassInfo_CObject;
	}

	[[nodiscard]] constexpr BOOL RttiIsKindOf(const ClassInfo* pInfo) const
	{
		auto pThisInfo = RttiGetClassInfo();
		do
		{
			if (pThisInfo == pInfo)
				return TRUE;
			pThisInfo = pThisInfo->pBaseClass;
		} while (pThisInfo);
		return FALSE;
	}

	template<class T>
	EckInline [[nodiscard]] constexpr BOOL RttiIsKindOf() const { return RttiIsKindOf(T::RttiClassInfo()); }

	EckInlineNd static auto& RttiClassInfoMap()
	{
		static std::unordered_map<std::wstring_view, ClassInfo*> s_hsClassInfo{};
		return s_hsClassInfo;
	}
};
inline ClassInfo CObject::s_ClassInfo_CObject{ std::wstring_view{ L"CObject" } };
inline Priv::ClassInfoRegister CObject::s_ClassInfoRegister_CObject{ CObject::RttiClassInfo() };

inline Priv::ClassInfoRegister::ClassInfoRegister(ClassInfo* pInfo)
{
	//EckAssert(!CObject::s_hsClassInfo.contains(pInfo->svClassName));
	CObject::RttiClassInfoMap().emplace(pInfo->svClassName, pInfo);
}

template<class T>
inline CObject* RttiStdNewObject()
{
	if constexpr (std::is_abstract_v<T>)
		return nullptr;
	else
		return new T{};
}

#define ECK_RTTI(Cls)							\
	static ::eck::ClassInfo s_ClassInfo_##Cls;	\
	constexpr ::eck::ClassInfo* RttiGetClassInfo() const override { return &s_ClassInfo_##Cls; }	\
	static constexpr ::eck::ClassInfo* RttiClassInfo() { return &s_ClassInfo_##Cls; }				\
	static ::eck::Priv::ClassInfoRegister s_ClassInfoRegister_##Cls;

#define ECKPRIV_RTTI_IMPL1(Cls, Base)			\
	::eck::ClassInfo Cls::s_ClassInfo_##Cls		\
	{ L###Cls, &Base::s_ClassInfo_##Base, RttiStdNewObject<Cls> };
#define ECKPRIV_RTTI_IMPL2(Cls, Base)			\
	::eck::Priv::ClassInfoRegister Cls::s_ClassInfoRegister_##Cls(&Cls::s_ClassInfo_##Cls);

// 非CObject直接子类
#define ECK_RTTI_IMPL_BASE(Cls, Base)			\
	ECKPRIV_RTTI_IMPL1(Cls, Base)				\
	ECKPRIV_RTTI_IMPL2(Cls, Base)
// 非CObject直接子类，并inline
#define ECK_RTTI_IMPL_BASE_INLINE(Cls, Base)	\
	inline ECKPRIV_RTTI_IMPL1(Cls, Base)		\
	inline ECKPRIV_RTTI_IMPL2(Cls, Base)

// CObject直接子类
#define ECK_RTTI_IMPL(Cls) ECK_RTTI_IMPL_BASE(Cls, CObject)
// CObject直接子类，并inline
#define ECK_RTTI_IMPL_INLINE(Cls) ECK_RTTI_IMPL_BASE_INLINE(Cls, CObject)


#define ECKPRIV_RTTI_IMPL1_PREFIX(Cls, Base, Prefix)	\
	::eck::ClassInfo Cls::s_ClassInfo_##Cls				\
	{ ECKTOSTRW(Prefix##::##Cls), &Base::s_ClassInfo_##Base, RttiStdNewObject<Cls> };

// 带名称前缀的非CObject直接子类
#define ECK_RTTI_IMPL_BASE_PREFIX(Cls, Base, Prefix)	\
	ECKPRIV_RTTI_IMPL1_PREFIX(Cls, Base, Prefix)		\
	ECKPRIV_RTTI_IMPL2(Cls, Base)
// 带名称前缀的非CObject直接子类，并inline
#define ECK_RTTI_IMPL_BASE_INLINE_PREFIX(Cls, Base)		\
	inline ECKPRIV_RTTI_IMPL1_PREFIX(Cls, Base, Prefix)	\
	inline ECKPRIV_RTTI_IMPL2(Cls, Base)

// 带名称前缀的CObject直接子类
#define ECK_RTTI_IMPL_PREFIX(Cls, Prefix) ECK_RTTI_IMPL_BASE_PREFIX(Cls, CObject, Prefix)
// 带名称前缀的CObject直接子类，并inline
#define ECK_RTTI_IMPL_INLINE_PREFIX(Cls, Prefix) ECK_RTTI_IMPL_BASE_INLINE_PREFIX(Cls, CObject, Prefix)

EckInline ClassInfo* RttiGetClassInfo(std::wstring_view svClassName)
{
	const auto it = CObject::RttiClassInfoMap().find(svClassName);
	if (it == CObject::RttiClassInfoMap().end())
		return nullptr;
	return it->second;
}

EckInlineNd CObject* RttiNewObject(std::wstring_view svClassName)
{
	const auto pInfo = RttiGetClassInfo(svClassName);
	if (!pInfo)
		return nullptr;
	return pInfo->NewObject();
}
ECK_NAMESPACE_END