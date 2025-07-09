#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
#if !ECKCXX20
#pragma push_macro("ccpIsComInterface")
#define ccpIsComInterface class
#endif

template <ccpIsComInterface T>
class ComPtr
{
	template<ccpIsComInterface U>
	friend class ComPtr;
public:
	using TInterface = T;
private:
	TInterface* p{};

	EckInline void ReleaseIt() noexcept
	{
		if (p)
		{
			TInterface* t{};
			std::swap(p, t);
			t->Release();
		}
	}

	EckInline ULONG ReleaseItRet() noexcept
	{
		if (p)
		{
			TInterface* t{};
			std::swap(p, t);
			return t->Release();
		}
		else
			return 0;
	}
public:
	ComPtr() = default;

	constexpr ComPtr(std::nullptr_t) noexcept {}

	template<ccpIsComInterface U>
#if ECKCXX20
		requires std::is_convertible_v<U*, TInterface*>
#endif
	ComPtr(U* x) noexcept :p{ x }
	{
		if (p)
			p->AddRef();
	}

	ComPtr(const ComPtr& x) noexcept : p{ x.p }
	{
		if (p)
			p->AddRef();
	}

	constexpr ComPtr(ComPtr&& x) noexcept
	{
		std::swap(p, x.p);
	}

	template<ccpIsComInterface U>
#if ECKCXX20
		requires std::is_convertible_v<U*, TInterface*>
#endif
	ComPtr(const ComPtr<U>& x) noexcept : p{ x.p }
	{
		if (p)
			p->AddRef();
	}

	template<ccpIsComInterface U>
#if ECKCXX20
		requires std::is_convertible_v<U*, TInterface*>
#endif
	constexpr ComPtr(ComPtr<U>&& x) noexcept
	{
		std::swap(p, x.p);
	}

	ComPtr(REFCLSID clsid, HRESULT* phr = nullptr)
	{
		const auto hr = CoCreateInstance(clsid, nullptr,
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p));
		if (phr) *phr = hr;
	}

	~ComPtr() noexcept
	{
		ReleaseIt();
	}

	ComPtr& operator=(std::nullptr_t) noexcept
	{
		ReleaseIt();
		return *this;
	}

	template<ccpIsComInterface U>
#if ECKCXX20
		requires std::is_convertible_v<U*, TInterface*>
#endif
	constexpr ComPtr& operator=(U* x) noexcept
	{
		if (x != p)
			ComPtr(x).Swap(*this);
		return *this;
	}

	constexpr ComPtr& operator=(const ComPtr& x) noexcept
	{
		if (p != x.p)
			ComPtr(x).Swap(*this);
		return *this;
	}

	constexpr ComPtr& operator=(ComPtr&& x) noexcept
	{
		std::move(x).Swap(*this);
		return *this;
	}

	template<ccpIsComInterface U>
#if ECKCXX20
		requires std::is_convertible_v<U*, TInterface*>
#endif
	constexpr ComPtr& operator=(const ComPtr<U>& x) noexcept
	{
		ComPtr(x).Swap(*this);
		return *this;
	}

	EckInline constexpr [[nodiscard]] TInterface* const* operator&() const noexcept
	{
		return &p;
	}

	EckInline constexpr [[nodiscard]] TInterface** operator&() noexcept
	{
		return &p;
	}

	EckInline constexpr [[nodiscard]] TInterface* Get() const noexcept
	{
		return p;
	}

	EckInline constexpr [[nodiscard]] TInterface* operator->() const noexcept
	{
		return p;
	}

	EckInline constexpr [[nodiscard]] TInterface* const* AddrOf() const noexcept
	{
		return &p;
	}

	EckInline constexpr [[nodiscard]] TInterface** AddrOf() noexcept
	{
		return &p;
	}

	EckInline [[nodiscard]] TInterface** AddrOfClear() noexcept
	{
		ReleaseIt();
		return &p;
	}

	EckInline constexpr [[nodiscard]] TInterface*& RefOf() noexcept
	{
		return p;
	}

	EckInline constexpr [[nodiscard]] TInterface*& RefOfClear() noexcept
	{
		ReleaseIt();
		return p;
	}

	EckInline constexpr [[nodiscard]] TInterface* Detach() noexcept
	{
		TInterface* t{};
		std::swap(p, t);
		return t;
	}

	EckInline void Attach(TInterface* x) noexcept
	{
		if (p != nullptr)
		{
#ifdef _DEBUG
			EckAssert(p->Release() || p != x);
#else
			p->Release();
#endif
		}
		p = x;
	}

	EckInline ULONG Clear() noexcept
	{
		return ReleaseItRet();
	}

	template<ccpIsComInterface U>
	EckInline HRESULT As(ComPtr<U>& x) const noexcept
	{
		return p->QueryInterface(__uuidof(U), (void**)x.AddrOfClear());
	}

	EckInline HRESULT As(REFIID riid, ComPtr<IUnknown>& x) const noexcept
	{
		return p->QueryInterface(riid, (void**)x.AddrOfClear());
	}

	template<ccpIsComInterface U>
	EckInline HRESULT As(U*& x) const noexcept
	{
		return p->QueryInterface(&x);
	}

	EckInline constexpr void Swap(ComPtr& x) noexcept
	{
		std::swap(p, x.p);
	}

	EckInline constexpr void Swap(ComPtr&& x) noexcept
	{
		std::swap(p, x.p);
	}

	HRESULT CreateInstance(REFCLSID clsid) noexcept
	{
		ReleaseIt();
		return CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p));
	}
};
#if !ECKCXX20
#undef ccpIsComInterface
#pragma pop_macro("ccpIsComInterface")
#endif
ECK_NAMESPACE_END

namespace std
{
#if ECKCXX20
	template<::eck::ccpIsComInterface T>
#else
	template<class T>
#endif
	constexpr void swap(::eck::ComPtr<T>& a, ::eck::ComPtr<T>& b)
	{
		a.Swap(b);
	}
}