#include "pch.h"

#include "CWndMain.h"

void CWndMain::DmNewDpi(int iDpi) noexcept
{
    const int iDpiOld = m_iDpi;
    m_iDpi = iDpi;
    m_Layout.LoOnDpiChanged(iDpi);
    eck::ReCreateAndApplyFontForDpiChanged(HWnd, m_hFont, iDpi, iDpiOld);
    DmUpdateFixedSize();
}

void CWndMain::DmUpdateFixedSize() noexcept
{
}

void CWndMain::OnDestory() noexcept
{
    DeleteObject(m_hFont);
    m_hFont = nullptr;
}

LRESULT CWndMain::OnCreate(CREATESTRUCT* pcs) noexcept
{
    eck::PtcCurrent()->UpdateDefaultColor();

    m_iDpi = eck::GetDpi(HWnd);
    m_hFont = eck::DftCreate(m_iDpi);

    constexpr DWORD Style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
    constexpr DWORD GroupStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP;

    const eck::LYTMARGINS Mar{ .b = DpiScale((eck::TLytCoord)8, m_iDpi) };
    const auto cy = DpiScale(30, m_iDpi);
    const auto cxED = DpiScale(300, m_iDpi);

    m_CBRememberPassword.DbgTag = L"CBRememberPassword";
    m_CBShowPassword.DbgTag = L"CBShowPassword";

    Ui::ERR_CTX ErrCtx{};
    const auto r = Ui::Create(this, ErrCtx,
        Ui::VBox{
            m_Layout,
            Ui::Default{
                Ui::Style{ Style },
                Ui::Font{ m_hFont },
            },
            Ui::Local{
                Ui::Margin{ .b = 10 },
            },

            Ui::Window{
                m_EDUserName,
                Ui::ExStyle{ WS_EX_CLIENTEDGE },
                Ui::Flags{ eck::LF_IDEAL_HEIGHT }
            },
            Ui::HBox{
                m_LytPassword,
                Ui::Flags{ eck::LF_IDEAL_HEIGHT },

                Ui::Window{
                    m_EDPassword,
                    Ui::ExStyle{ WS_EX_CLIENTEDGE },
                    Ui::Flags{ eck::LF_IDEAL_HEIGHT },
                    Ui::Weight{ 1 },
                },
                Ui::Window{
                    m_CBShowPassword, L"●",
                    Ui::Flags{ eck::LF_IDEAL_WIDTH }
                }
            },
            Ui::HBox{
                m_LytOption,
                Ui::Flags{ eck::LF_IDEAL | eck::LF_ALIGN_CENTER },

                Ui::Window{
                    m_CBRememberPassword, L"记住密码",
                    Ui::Flags{ eck::LF_IDEAL, Ui::Replace },
                    Ui::Weight{ 1 },
                    Ui::Style{ BS_AUTOCHECKBOX },
                },
                Ui::Window{
                    m_CBLoginAuto, L"自动登录",
                    Ui::Flags{ eck::LF_IDEAL },
                    Ui::Style{ BS_AUTOCHECKBOX },
                },
            },
            Ui::Window{
                m_BTLogin, L"登录",
                Ui::Flags{ eck::LF_IDEAL | eck::LF_ALIGN_CENTER }
            },
        });
    EckAssert(r == Ui::Result::Ok);
    m_Layout.LoInitializeDpi(m_iDpi);
    return 0;
}

LRESULT CWndMain::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (uMsg)
    {
    case WM_SIZE:
        m_Layout.Arrange(
            0,
            eck::DpiScale((eck::TLytCoord)70, m_iDpi),
            (eck::TLytCoord)LOWORD(lParam),
            (eck::TLytCoord)HIWORD(lParam));
        break;

    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            if ((LPARAM)m_BTLogin.HWnd == lParam)
                MessageBoxW(HWnd, L"Hello World!", L"Hello", MB_OK);
            return 0;
        }
    }
    break;

    case WM_CREATE:
        return OnCreate((CREATESTRUCT*)lParam);
    case WM_DESTROY:
        OnDestory();
        PostQuitMessage(0);
        return 0;

    case WM_SYSCOLORCHANGE:
        eck::MsgOnSystemColorChangeMainWindow(HWnd, wParam, lParam);
        break;
    case WM_SETTINGCHANGE:
        eck::MsgOnSettingChangeMainWindow(HWnd, wParam, lParam);
        break;
    case WM_DPICHANGED:
        DmNewDpi(HIWORD(wParam));
        eck::MsgOnDpiChanged(HWnd, lParam);
        return 0;
    }
    return __super::OnMessage(uMsg, wParam, lParam);
}

BOOL CWndMain::PreTranslateMessage(const MSG& Msg) noexcept
{
    if (IsDialogMessageW(HWnd, (MSG*)&Msg))
        return TRUE;
    return __super::PreTranslateMessage(Msg);
}