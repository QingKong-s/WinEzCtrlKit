/*
* WinEzCtrlKit Library
*
* DDX.h : 动态数据交换
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CRefStr.h"
#include "CSignal.h"
#include "CObject.h"

#include <variant>

ECK_NAMESPACE_BEGIN
enum class DDXType :UINT
{
	// 不要改动顺序

	Int,
	LongLong,
	Float,
	Double,
	Bool,
	Color,
	String,

	PRIV_Count,

	Array = 1u << 31,
};
ECK_ENUM_BIT_FLAGS(DDXType);

struct DDXData
{
	DDXType eType;
	std::variant<int, LONGLONG, float, double, BYTE, COLORREF, CRefStrW,
		std::vector<int>, std::vector<LONGLONG>, std::vector<float>, std::vector<double>,
		std::vector<BYTE>, std::vector<COLORREF>, std::vector<CRefStrW>>
		Data;// 不要改动顺序
};

struct DDXRange
{
	template<class T>
	struct Range
	{
		T Min;
		T Max;
	};

	struct Dummy {};

	std::variant<Range<int>, Range<LONGLONG>, Range<float>, Range<double>, Dummy>
		Data{ Dummy{} };// 不要改动顺序

	EckInline [[nodiscard]] constexpr BOOL IsEmpty() const { return Data.index() == 4; }
};

class __declspec(novtable) CDdx :public CObject
{
public:
	ECK_RTTI(CDdx);
protected:
	DDXData m_Data{};
	DDXRange m_Range{};
	CSignal<NoIntercept_T, void, CDdx&> m_Sig{};

	virtual void OnDataChanged() = 0;

	template<DDXType Type>
	void Clamp()
	{
		std::get<(size_t)Type>(m_Data.Data) = std::clamp(std::get<(size_t)Type>(m_Data.Data),
			std::get<(size_t)Type>(m_Range.Data).Min, std::get<(size_t)Type>(m_Range.Data).Max);
	}

	template<DDXType Type>
	void ClampVector()
	{
		auto& v = std::get<(size_t)Type + (size_t)DDXType::PRIV_Count>(m_Data.Data);
		for (auto& e : v)
			e = std::clamp(e, std::get<(size_t)Type>(m_Range.Data).Min,
				std::get<(size_t)Type>(m_Range.Data).Max);
	}

	void ClampToRange()
	{
		if (m_Range.IsEmpty())
			return;

		switch (m_Data.eType)
		{
		case DDXType::Int:
			Clamp<DDXType::Int>();
			break;
		case DDXType::LongLong:
			Clamp<DDXType::LongLong>();
			break;
		case DDXType::Float:
			Clamp<DDXType::Float>();
			break;
		case DDXType::Double:
			Clamp<DDXType::Double>();
			break;
		case DDXType::Array | DDXType::Int:
			ClampVector<DDXType::Int>();
			break;
		case DDXType::Array | DDXType::LongLong:
			ClampVector<DDXType::LongLong>();
			break;
		case DDXType::Array | DDXType::Float:
			ClampVector<DDXType::Float>();
			break;
		case DDXType::Array | DDXType::Double:
			ClampVector<DDXType::Double>();
			break;
		}
	}

	void IntSet(int Value)
	{
		m_Data.eType = DDXType::Int;
		m_Data.Data = Value;
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(LONGLONG Value)
	{
		m_Data.eType = DDXType::LongLong;
		m_Data.Data = Value;
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(float Value)
	{
		m_Data.eType = DDXType::Float;
		m_Data.Data = Value;
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(double Value)
	{
		m_Data.eType = DDXType::Double;
		m_Data.Data = Value;
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(bool Value)
	{
		m_Data.eType = DDXType::Bool;
		m_Data.Data = Value;
		m_Sig.Emit(*this);
	}

	void IntSet(COLORREF Value)
	{
		m_Data.eType = DDXType::Color;
		m_Data.Data = Value;
		m_Sig.Emit(*this);
	}

	void IntSet(const CRefStrW& Value)
	{
		m_Data.eType = DDXType::String;
		m_Data.Data = Value;
		m_Sig.Emit(*this);
	}

	void IntSet(const std::vector<int>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Int;
		m_Data.Data = Value;
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(const std::vector<LONGLONG>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::LongLong;
		m_Data.Data = Value;
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(const std::vector<float>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Float;
		m_Data.Data = Value;
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(const std::vector<double>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Double;
		m_Data.Data = Value;
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(const std::vector<BYTE>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Bool;
		m_Data.Data = Value;
		m_Sig.Emit(*this);
	}

	void IntSet(const std::vector<COLORREF>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Color;
		m_Data.Data = Value;
		m_Sig.Emit(*this);
	}

	void IntSet(const std::vector<CRefStrW>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::String;
		m_Data.Data = Value;
		m_Sig.Emit(*this);
	}

	void IntSet(std::vector<int>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Int;
		m_Data.Data = std::move(Value);
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(std::vector<LONGLONG>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::LongLong;
		m_Data.Data = std::move(Value);
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(std::vector<float>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Float;
		m_Data.Data = std::move(Value);
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(std::vector<double>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Double;
		m_Data.Data = std::move(Value);
		ClampToRange();
		m_Sig.Emit(*this);
	}

	void IntSet(std::vector<BYTE>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Bool;
		m_Data.Data = std::move(Value);
		m_Sig.Emit(*this);
	}

	void IntSet(std::vector<COLORREF>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Color;
		m_Data.Data = std::move(Value);
		m_Sig.Emit(*this);
	}

	void IntSet(std::vector<CRefStrW>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::String;
		m_Data.Data = std::move(Value);
		m_Sig.Emit(*this);
	}
public:
	EckInline [[nodiscard]] constexpr auto& GetSignal() { return m_Sig; }

	EckInline [[nodiscard]] constexpr const DDXData& Get() const { return m_Data; }

	EckInline [[nodiscard]] constexpr int GetInt() const { return std::get<(size_t)DDXType::Int>(m_Data.Data); }

	EckInline [[nodiscard]] constexpr LONGLONG GetLongLong() const { return std::get<(size_t)DDXType::LongLong>(m_Data.Data); }

	EckInline [[nodiscard]] constexpr float GetFloat() const { return std::get<(size_t)DDXType::Float>(m_Data.Data); }

	EckInline [[nodiscard]] constexpr double GetDouble() const { return std::get<(size_t)DDXType::Double>(m_Data.Data); }

	EckInline [[nodiscard]] constexpr bool GetBool() const { return std::get<(size_t)DDXType::Bool>(m_Data.Data); }

	EckInline [[nodiscard]] constexpr COLORREF GetColor() const { return std::get<(size_t)DDXType::Color>(m_Data.Data); }

	EckInline [[nodiscard]] const CRefStrW& GetString() const { return std::get<(size_t)DDXType::String>(m_Data.Data); }

	EckInline [[nodiscard]] const std::vector<int>& GetIntArray() const { return std::get<(size_t)DDXType::PRIV_Count>(m_Data.Data); }

	EckInline [[nodiscard]] const std::vector<LONGLONG>& GetLongLongArray() const { return std::get<(size_t)DDXType::PRIV_Count + 1>(m_Data.Data); }

	EckInline [[nodiscard]] const std::vector<float>& GetFloatArray() const { return std::get<(size_t)DDXType::PRIV_Count + 2>(m_Data.Data); }

	EckInline [[nodiscard]] const std::vector<double>& GetDoubleArray() const { return std::get<(size_t)DDXType::PRIV_Count + 3>(m_Data.Data); }

	EckInline [[nodiscard]] const std::vector<BYTE>& GetBoolArray() const { return std::get<(size_t)DDXType::PRIV_Count + 4>(m_Data.Data); }

	EckInline [[nodiscard]] const std::vector<COLORREF>& GetColorArray() const { return std::get<(size_t)DDXType::PRIV_Count + 5>(m_Data.Data); }

	EckInline [[nodiscard]] const std::vector<CRefStrW>& GetStringArray() const { return std::get<(size_t)DDXType::PRIV_Count + 6>(m_Data.Data); }

	void Set(const DDXData& Data)
	{
		m_Data = Data;
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void Set(DDXData&& Data)
	{
		m_Data = std::move(Data);
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetInt(int Value)
	{
		m_Data.eType = DDXType::Int;
		m_Data.Data = Value;
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetLongLong(LONGLONG Value)
	{
		m_Data.eType = DDXType::LongLong;
		m_Data.Data = Value;
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetFloat(float Value)
	{
		m_Data.eType = DDXType::Float;
		m_Data.Data = Value;
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetDouble(double Value)
	{
		m_Data.eType = DDXType::Double;
		m_Data.Data = Value;
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetBool(bool Value)
	{
		m_Data.eType = DDXType::Bool;
		m_Data.Data = Value;
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetColor(COLORREF Value)
	{
		m_Data.eType = DDXType::Color;
		m_Data.Data = Value;
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetString(const CRefStrW& Value)
	{
		m_Data.eType = DDXType::String;
		m_Data.Data = Value;
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetIntArray(const std::vector<int>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Int;
		m_Data.Data = Value;
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetLongLongArray(const std::vector<LONGLONG>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::LongLong;
		m_Data.Data = Value;
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetFloatArray(const std::vector<float>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Float;
		m_Data.Data = Value;
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetDoubleArray(const std::vector<double>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Double;
		m_Data.Data = Value;
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetBoolArray(const std::vector<BYTE>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Bool;
		m_Data.Data = Value;
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetColorArray(const std::vector<COLORREF>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Color;
		m_Data.Data = Value;
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetStringArray(const std::vector<CRefStrW>& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::String;
		m_Data.Data = Value;
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetString(CRefStrW&& Value)
	{
		m_Data.eType = DDXType::String;
		m_Data.Data = std::move(Value);
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetIntArray(std::vector<int>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Int;
		m_Data.Data = std::move(Value);
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetLongLongArray(std::vector<LONGLONG>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::LongLong;
		m_Data.Data = std::move(Value);
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetFloatArray(std::vector<float>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Float;
		m_Data.Data = std::move(Value);
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetDoubleArray(std::vector<double>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Double;
		m_Data.Data = std::move(Value);
		ClampToRange();
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetBoolArray(std::vector<BYTE>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Bool;
		m_Data.Data = std::move(Value);
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetColorArray(std::vector<COLORREF>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::Color;
		m_Data.Data = std::move(Value);
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	void SetStringArray(std::vector<CRefStrW>&& Value)
	{
		m_Data.eType = DDXType::Array | DDXType::String;
		m_Data.Data = std::move(Value);
		OnDataChanged();
		m_Sig.Emit(*this);
	}

	CDdx& operator=(const DDXData& Data)
	{
		Set(Data);
		return *this;
	}

	CDdx& operator=(DDXData&& Data)
	{
		Set(std::move(Data));
		return *this;
	}

	CDdx& operator=(int Value)
	{
		SetInt(Value);
		return *this;
	}

	CDdx& operator=(LONGLONG Value)
	{
		SetLongLong(Value);
		return *this;
	}

	CDdx& operator=(float Value)
	{
		SetFloat(Value);
		return *this;
	}

	CDdx& operator=(double Value)
	{
		SetDouble(Value);
		return *this;
	}

	CDdx& operator=(bool Value)
	{
		SetBool(Value);
		return *this;
	}

	CDdx& operator=(COLORREF Value)
	{
		SetColor(Value);
		return *this;
	}

	CDdx& operator=(const CRefStrW& Value)
	{
		SetString(Value);
		return *this;
	}

	CDdx& operator=(CRefStrW&& Value)
	{
		SetString(std::move(Value));
		return *this;
	}

	CDdx& operator=(PCWSTR Value)
	{
		SetString(CRefStrW(Value));
		return *this;
	}

	CDdx& operator=(const std::vector<int>& Value)
	{
		SetIntArray(Value);
		return *this;
	}

	CDdx& operator=(std::vector<int>&& Value)
	{
		SetIntArray(std::move(Value));
		return *this;
	}

	CDdx& operator=(const std::vector<LONGLONG>& Value)
	{
		SetLongLongArray(Value);
		return *this;
	}

	CDdx& operator=(std::vector<LONGLONG>&& Value)
	{
		SetLongLongArray(std::move(Value));
		return *this;
	}

	CDdx& operator=(const std::vector<float>& Value)
	{
		SetFloatArray(Value);
		return *this;
	}

	CDdx& operator=(std::vector<float>&& Value)
	{
		SetFloatArray(std::move(Value));
		return *this;
	}

	CDdx& operator=(const std::vector<double>& Value)
	{
		SetDoubleArray(Value);
		return *this;
	}

	CDdx& operator=(std::vector<double>&& Value)
	{
		SetDoubleArray(std::move(Value));
		return *this;
	}

	CDdx& operator=(const std::vector<BYTE>& Value)
	{
		SetBoolArray(Value);
		return *this;
	}

	CDdx& operator=(std::vector<BYTE>&& Value)
	{
		SetBoolArray(std::move(Value));
		return *this;
	}

	CDdx& operator=(const std::vector<COLORREF>& Value)
	{
		SetColorArray(Value);
		return *this;
	}

	CDdx& operator=(std::vector<COLORREF>&& Value)
	{
		SetColorArray(std::move(Value));
		return *this;
	}

	CDdx& operator=(const std::vector<CRefStrW>& Value)
	{
		SetStringArray(Value);
		return *this;
	}

	CDdx& operator=(std::vector<CRefStrW>&& Value)
	{
		SetStringArray(std::move(Value));
		return *this;
	}

	void SetRange(const DDXRange& Range)
	{
		m_Range = Range;
		ClampToRange();
	}

	void SetRange(int Min, int Max)
	{
		m_Range.Data = DDXRange::Range{ Min, Max };
		ClampToRange();
	}

	void SetRange(LONGLONG Min, LONGLONG Max)
	{
		m_Range.Data = DDXRange::Range{ Min, Max };
		ClampToRange();
	}

	void SetRange(float Min, float Max)
	{
		m_Range.Data = DDXRange::Range{ Min, Max };
	}

	void SetRange(double Min, double Max)
	{
		m_Range.Data = DDXRange::Range{ Min, Max };
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CDdx, CObject);
ECK_NAMESPACE_END