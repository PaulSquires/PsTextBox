' ========================================================================================
' CTextBox demo
' ========================================================================================

#define UNICODE
#define _WIN32_WINNT &h0602

#include once "windows.bi"
#include once "AfxNova\CWindow.inc"
#include once "AfxNova\AfxStr.inc"

using AfxNova


#define APPNAME          wstr("Custom TextBox")
#define APPCLASSNAME     wstr("custom_textbox_class")

#DEFINE GUIFONT          wstr("Segoe UI")

#DEFINE GUIFONT_9        0
#DEFINE GUIFONT_10       1
#DEFINE CUEFONT_10       2
#DEFINE MAXFONTS         3

dim shared ghFont(MAXFONTS) as HFONT

dim shared as HWND HWND_FRMMAIN


type THEME_TYPE
    BackColorWindow    as COLORREF = BGR(33,37,43)
    BackColorBox       as COLORREF = BGR(44,49,58)
    ForeColor          as COLORREF = BGR(215,218,224)
    ForeColorCue       as COLORREF = BGR(128,134,143)
    ForeColorLabel     as COLORREF = BGR(140,146,155)
    ForeColorEvent     as COLORREF = BGR(152,195,121)
    BorderColor        as COLORREF = BGR(60,66,77)
    FocusAccent        as COLORREF = BGR(86,156,214)
end type
dim shared theme as THEME_TYPE



#include once "clsDoubleBuffer.inc"
#include once "CPopupMenu.inc"    ' CTextBox's built-in right-click menu runs on it
#include once "CTextBox.inc"
#include once "frmMain.inc"


' ========================================================================================
' WinMain
' ========================================================================================
function WinMain( _
            byval hInstance     as HINSTANCE, _
            byval hPrevInstance as HINSTANCE, _
            byval szCmdLine     as zstring ptr, _
            byval nCmdShow      as long _
            ) as long

    ' Initialize the COM library
    CoInitialize(null)

    ' Show the main form
    ' Initialize GDI+ (clsDoubleBuffer's rendering backend -- see DBUF_GDIPLUS). Must be
    ' running before the first WM_PAINT builds a buffer, and must outlive every one of
    ' them, so it brackets frmMain_Show.
    dim as ULONG_PTR gdipToken = AfxGdipInit()

    function = frmMain_Show( 0 )

    ' Uninitialize the COM library
    ' Every window is destroyed and every clsDoubleBuffer has run its destructor by here,
    ' so no CGp* object can still be alive. Precedes CoUninitialize: GDI+ leans on COM.
    AfxGdipShutdown( gdipToken )

    CoUninitialize

end function


' ========================================================================================
' Main program entry point
' ========================================================================================
end WinMain( GetModuleHandle(null), null, command(), SW_NORMAL )
