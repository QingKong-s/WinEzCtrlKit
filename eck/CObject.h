/*
* WinEzCtrlKit Library
*
* CObject.h : RTTI基类
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CObject;
struct ClassInfo
{
	using FNewObject = CObject * (*)();

	PCWSTR pszClassName;
	ClassInfo* pBaseClass;
	FNewObject pfnNewObject;

	ClassInfo* pNext;

	EckInline [[nodiscard]] CObject* NewObject() const { return pfnNewObject(); }
};

extern ClassInfo* g_pClassInfo;

struct ClassInfoRegister
{
	constexpr ClassInfoRegister(ClassInfo* pInfo)
	{
		auto pPrev = g_pClassInfo;
		pInfo->pNext = pPrev;
		g_pClassInfo = pInfo;
	}
};

// 绝对不能直接实例化此类
class __declspec(novtable) CObject
{
private:
	static ClassInfoRegister s_ClassInfoRegister_CObject;
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
};
inline ClassInfo CObject::s_ClassInfo_CObject = { L"CObject" };
inline ClassInfoRegister CObject::s_ClassInfoRegister_CObject(&CObject::s_ClassInfo_CObject);

template<class T>
inline CObject* RttiStdNewObject()
{
	return new T{};
}

#define ECK_RTTI(Cls) \
	static ClassInfo s_ClassInfo_##Cls; \
	constexpr ClassInfo* RttiGetClassInfo() const override { return &s_ClassInfo_##Cls; } \
	static constexpr ClassInfo* RttiClassInfo() { return &s_ClassInfo_##Cls; } \
	static ClassInfoRegister s_ClassInfoRegister_##Cls;

#define ECK_RTTI_IMPL_BASE(Cls, Base) \
	ClassInfo Cls::s_ClassInfo_##Cls = { L###Cls, &Base::s_ClassInfo_##Base, RttiStdNewObject<Cls> }; \
	ClassInfoRegister Cls::s_ClassInfoRegister_##Cls(&Cls::s_ClassInfo_##Cls);

#define ECK_RTTI_IMPL_BASE_INLINE(Cls, Base) \
	inline ClassInfo Cls::s_ClassInfo_##Cls = { L###Cls, &Base::s_ClassInfo_##Base, RttiStdNewObject<Cls> }; \
	inline ClassInfoRegister Cls::s_ClassInfoRegister_##Cls(&Cls::s_ClassInfo_##Cls);

#define ECK_RTTI_IMPL(Cls) ECK_RTTI_IMPL_BASE(Cls, CObject)

#define ECK_RTTI_IMPL_INLINE(Cls) ECK_RTTI_IMPL_BASE_INLINE(Cls, CObject)
ECK_NAMESPACE_END