#pragma once
class CApp
{
public:
	constexpr static PCWSTR ClipboardFormat = L"Eck.Designer.CF.Ctrl";
private:
	UINT m_cfCtrl{};
public:
	void Init();

	EckInlineNdCe UINT GetClipboardFormat() const noexcept { return m_cfCtrl; }
};

extern CApp* App;