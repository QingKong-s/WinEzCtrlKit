#pragma once
/*
* ��Ҫʹ��DPI��֪������嵥�ļ�".\eck\Others\DpiAwarePreMonV2.manifest"��
* ��ʹ��'ϵͳDPI��֪'��'ÿ��ʾ��DPI��֪V2'����ʹ��'ÿ��ʾ��DPI��֪V1'������Ҳ
* δ��������
*/

#ifndef ECKMACRO_NO_COMCTL60
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifndef ECKMACRO_NO_AUTO_ADD_LIB
#pragma comment(lib,"Gdiplus.lib")
#pragma comment(lib,"ComCtl32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"UxTheme.lib")
#pragma comment(lib,"Dwmapi.lib")
#endif

// ECKMACRO_NO_WIN11_22621  SDK�汾�Ƿ�>=22621