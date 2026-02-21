#pragma once
#include "CString.h"
#include "CSignal.h"
#include "CObject.h"

#include <variant>

ECK_NAMESPACE_BEGIN
enum class DDXType : UINT
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
    std::variant<int, LONGLONG, float, double, BYTE, COLORREF, CStringW,
        std::vector<int>, std::vector<LONGLONG>, std::vector<float>, std::vector<double>,
        std::vector<BYTE>, std::vector<COLORREF>, std::vector<CStringW>>
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

    EckInlineNdCe BOOL IsEmpty() const { return Data.index() == 4; }
};

class __declspec(novtable) CDdx : public CObject
{
public:
    ECK_RTTI(CDdx, CObject);
protected:
    DDXData m_Data{};
    DDXRange m_Range{};
    CSignal<NoIntercept_T, void, CDdx&> m_Sig{};

    virtual void OnDataChanged() = 0;

    template<DDXType Type>
    void Clamp() noexcept
    {
        std::get<(size_t)Type>(m_Data.Data) = std::clamp(std::get<(size_t)Type>(m_Data.Data),
            std::get<(size_t)Type>(m_Range.Data).Min, std::get<(size_t)Type>(m_Range.Data).Max);
    }

    template<DDXType Type>
    void ClampVector() noexcept
    {
        auto& v = std::get<(size_t)Type + (size_t)DDXType::PRIV_Count>(m_Data.Data);
        for (auto& e : v)
            e = std::clamp(e, std::get<(size_t)Type>(m_Range.Data).Min,
                std::get<(size_t)Type>(m_Range.Data).Max);
    }

    void ClampToRange() noexcept
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

    void IntSet(int Value) noexcept
    {
        m_Data.eType = DDXType::Int;
        m_Data.Data = Value;
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(LONGLONG Value) noexcept
    {
        m_Data.eType = DDXType::LongLong;
        m_Data.Data = Value;
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(float Value) noexcept
    {
        m_Data.eType = DDXType::Float;
        m_Data.Data = Value;
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(double Value) noexcept
    {
        m_Data.eType = DDXType::Double;
        m_Data.Data = Value;
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(bool Value) noexcept
    {
        m_Data.eType = DDXType::Bool;
        m_Data.Data = Value;
        m_Sig.Emit(*this);
    }

    void IntSet(COLORREF Value) noexcept
    {
        m_Data.eType = DDXType::Color;
        m_Data.Data = Value;
        m_Sig.Emit(*this);
    }

    void IntSet(const CStringW& Value) noexcept
    {
        m_Data.eType = DDXType::String;
        m_Data.Data = Value;
        m_Sig.Emit(*this);
    }

    void IntSet(const std::vector<int>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Int;
        m_Data.Data = Value;
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(const std::vector<LONGLONG>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::LongLong;
        m_Data.Data = Value;
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(const std::vector<float>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Float;
        m_Data.Data = Value;
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(const std::vector<double>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Double;
        m_Data.Data = Value;
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(const std::vector<BYTE>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Bool;
        m_Data.Data = Value;
        m_Sig.Emit(*this);
    }

    void IntSet(const std::vector<COLORREF>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Color;
        m_Data.Data = Value;
        m_Sig.Emit(*this);
    }

    void IntSet(const std::vector<CStringW>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::String;
        m_Data.Data = Value;
        m_Sig.Emit(*this);
    }

    void IntSet(std::vector<int>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Int;
        m_Data.Data = std::move(Value);
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(std::vector<LONGLONG>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::LongLong;
        m_Data.Data = std::move(Value);
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(std::vector<float>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Float;
        m_Data.Data = std::move(Value);
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(std::vector<double>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Double;
        m_Data.Data = std::move(Value);
        ClampToRange();
        m_Sig.Emit(*this);
    }

    void IntSet(std::vector<BYTE>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Bool;
        m_Data.Data = std::move(Value);
        m_Sig.Emit(*this);
    }

    void IntSet(std::vector<COLORREF>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Color;
        m_Data.Data = std::move(Value);
        m_Sig.Emit(*this);
    }

    void IntSet(std::vector<CStringW>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::String;
        m_Data.Data = std::move(Value);
        m_Sig.Emit(*this);
    }
public:
    EckInlineNdCe auto& GetSignal() noexcept { return m_Sig; }

    EckInlineNdCe const DDXData& Get() const noexcept { return m_Data; }
    EckInlineNdCe int GetInt() const noexcept { return std::get<(size_t)DDXType::Int>(m_Data.Data); }
    EckInlineNdCe LONGLONG GetLongLong() const noexcept { return std::get<(size_t)DDXType::LongLong>(m_Data.Data); }
    EckInlineNdCe float GetFloat() const noexcept { return std::get<(size_t)DDXType::Float>(m_Data.Data); }
    EckInlineNdCe double GetDouble() const noexcept { return std::get<(size_t)DDXType::Double>(m_Data.Data); }
    EckInlineNdCe bool GetBool() const noexcept { return std::get<(size_t)DDXType::Bool>(m_Data.Data); }
    EckInlineNdCe COLORREF GetColor() const noexcept { return std::get<(size_t)DDXType::Color>(m_Data.Data); }

    EckInlineNdCe const CStringW& GetString() const noexcept { return std::get<(size_t)DDXType::String>(m_Data.Data); }
    EckInlineNdCe const std::vector<int>& GetIntArray() const noexcept { return std::get<(size_t)DDXType::PRIV_Count>(m_Data.Data); }
    EckInlineNdCe const std::vector<LONGLONG>& GetLongLongArray() const noexcept { return std::get<(size_t)DDXType::PRIV_Count + 1>(m_Data.Data); }
    EckInlineNdCe const std::vector<float>& GetFloatArray() const noexcept { return std::get<(size_t)DDXType::PRIV_Count + 2>(m_Data.Data); }
    EckInlineNdCe const std::vector<double>& GetDoubleArray() const noexcept { return std::get<(size_t)DDXType::PRIV_Count + 3>(m_Data.Data); }
    EckInlineNdCe const std::vector<BYTE>& GetBoolArray() const noexcept { return std::get<(size_t)DDXType::PRIV_Count + 4>(m_Data.Data); }
    EckInlineNdCe const std::vector<COLORREF>& GetColorArray() const noexcept { return std::get<(size_t)DDXType::PRIV_Count + 5>(m_Data.Data); }
    EckInlineNdCe const std::vector<CStringW>& GetStringArray() const noexcept { return std::get<(size_t)DDXType::PRIV_Count + 6>(m_Data.Data); }

    void Set(const DDXData& Data) noexcept
    {
        m_Data = Data;
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void Set(DDXData&& Data) noexcept
    {
        m_Data = std::move(Data);
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetInt(int Value) noexcept
    {
        m_Data.eType = DDXType::Int;
        m_Data.Data = Value;
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetLongLong(LONGLONG Value) noexcept
    {
        m_Data.eType = DDXType::LongLong;
        m_Data.Data = Value;
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetFloat(float Value) noexcept
    {
        m_Data.eType = DDXType::Float;
        m_Data.Data = Value;
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetDouble(double Value) noexcept
    {
        m_Data.eType = DDXType::Double;
        m_Data.Data = Value;
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetBool(bool Value) noexcept
    {
        m_Data.eType = DDXType::Bool;
        m_Data.Data = Value;
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetColor(COLORREF Value) noexcept
    {
        m_Data.eType = DDXType::Color;
        m_Data.Data = Value;
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetString(const CStringW& Value) noexcept
    {
        m_Data.eType = DDXType::String;
        m_Data.Data = Value;
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetIntArray(const std::vector<int>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Int;
        m_Data.Data = Value;
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetLongLongArray(const std::vector<LONGLONG>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::LongLong;
        m_Data.Data = Value;
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetFloatArray(const std::vector<float>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Float;
        m_Data.Data = Value;
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetDoubleArray(const std::vector<double>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Double;
        m_Data.Data = Value;
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetBoolArray(const std::vector<BYTE>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Bool;
        m_Data.Data = Value;
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetColorArray(const std::vector<COLORREF>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Color;
        m_Data.Data = Value;
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetStringArray(const std::vector<CStringW>& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::String;
        m_Data.Data = Value;
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetString(CStringW&& Value) noexcept
    {
        m_Data.eType = DDXType::String;
        m_Data.Data = std::move(Value);
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetIntArray(std::vector<int>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Int;
        m_Data.Data = std::move(Value);
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetLongLongArray(std::vector<LONGLONG>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::LongLong;
        m_Data.Data = std::move(Value);
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetFloatArray(std::vector<float>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Float;
        m_Data.Data = std::move(Value);
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetDoubleArray(std::vector<double>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Double;
        m_Data.Data = std::move(Value);
        ClampToRange();
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetBoolArray(std::vector<BYTE>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Bool;
        m_Data.Data = std::move(Value);
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetColorArray(std::vector<COLORREF>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::Color;
        m_Data.Data = std::move(Value);
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    void SetStringArray(std::vector<CStringW>&& Value) noexcept
    {
        m_Data.eType = DDXType::Array | DDXType::String;
        m_Data.Data = std::move(Value);
        OnDataChanged();
        m_Sig.Emit(*this);
    }

    CDdx& operator=(const DDXData& Data) noexcept
    {
        Set(Data);
        return *this;
    }

    CDdx& operator=(DDXData&& Data) noexcept
    {
        Set(std::move(Data));
        return *this;
    }

    CDdx& operator=(int Value) noexcept
    {
        SetInt(Value);
        return *this;
    }

    CDdx& operator=(LONGLONG Value) noexcept
    {
        SetLongLong(Value);
        return *this;
    }

    CDdx& operator=(float Value) noexcept
    {
        SetFloat(Value);
        return *this;
    }

    CDdx& operator=(double Value) noexcept
    {
        SetDouble(Value);
        return *this;
    }

    CDdx& operator=(bool Value) noexcept
    {
        SetBool(Value);
        return *this;
    }

    CDdx& operator=(COLORREF Value) noexcept
    {
        SetColor(Value);
        return *this;
    }

    CDdx& operator=(const CStringW& Value) noexcept
    {
        SetString(Value);
        return *this;
    }

    CDdx& operator=(CStringW&& Value) noexcept
    {
        SetString(std::move(Value));
        return *this;
    }

    CDdx& operator=(PCWSTR Value) noexcept
    {
        SetString(CStringW(Value));
        return *this;
    }

    CDdx& operator=(const std::vector<int>& Value) noexcept
    {
        SetIntArray(Value);
        return *this;
    }

    CDdx& operator=(std::vector<int>&& Value) noexcept
    {
        SetIntArray(std::move(Value));
        return *this;
    }

    CDdx& operator=(const std::vector<LONGLONG>& Value) noexcept
    {
        SetLongLongArray(Value);
        return *this;
    }

    CDdx& operator=(std::vector<LONGLONG>&& Value) noexcept
    {
        SetLongLongArray(std::move(Value));
        return *this;
    }

    CDdx& operator=(const std::vector<float>& Value) noexcept
    {
        SetFloatArray(Value);
        return *this;
    }

    CDdx& operator=(std::vector<float>&& Value) noexcept
    {
        SetFloatArray(std::move(Value));
        return *this;
    }

    CDdx& operator=(const std::vector<double>& Value) noexcept
    {
        SetDoubleArray(Value);
        return *this;
    }

    CDdx& operator=(std::vector<double>&& Value) noexcept
    {
        SetDoubleArray(std::move(Value));
        return *this;
    }

    CDdx& operator=(const std::vector<BYTE>& Value) noexcept
    {
        SetBoolArray(Value);
        return *this;
    }

    CDdx& operator=(std::vector<BYTE>&& Value) noexcept
    {
        SetBoolArray(std::move(Value));
        return *this;
    }

    CDdx& operator=(const std::vector<COLORREF>& Value) noexcept
    {
        SetColorArray(Value);
        return *this;
    }

    CDdx& operator=(std::vector<COLORREF>&& Value) noexcept
    {
        SetColorArray(std::move(Value));
        return *this;
    }

    CDdx& operator=(const std::vector<CStringW>& Value) noexcept
    {
        SetStringArray(Value);
        return *this;
    }

    CDdx& operator=(std::vector<CStringW>&& Value) noexcept
    {
        SetStringArray(std::move(Value));
        return *this;
    }

    void SetRange(const DDXRange& Range) noexcept
    {
        m_Range = Range;
        ClampToRange();
    }

    void SetRange(int Min, int Max) noexcept
    {
        m_Range.Data = DDXRange::Range{ Min, Max };
        ClampToRange();
    }

    void SetRange(LONGLONG Min, LONGLONG Max) noexcept
    {
        m_Range.Data = DDXRange::Range{ Min, Max };
        ClampToRange();
    }

    void SetRange(float Min, float Max) noexcept
    {
        m_Range.Data = DDXRange::Range{ Min, Max };
    }

    void SetRange(double Min, double Max) noexcept
    {
        m_Range.Data = DDXRange::Range{ Min, Max };
    }
};
ECK_NAMESPACE_END