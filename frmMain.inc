#include once "frmMain.bi"
#include once "clsDoubleBuffer.bi"


' Last event reported by the callbacks, echoed at the bottom of the window.
dim shared gwszLastEvent as DWSTRING


' ========================================================================================
' Shared layout numbers so WM_PAINT (labels) and the position sub agree. All values are
' raw design pixels; DPI-scale at the point of use.
' ========================================================================================
#define DEMO_LEFT        20
#define DEMO_BOX_HEIGHT  32
#define DEMO_BOX1_TOP    46
#define DEMO_BOX2_TOP    122
#define DEMO_BOX3_TOP    198


' ========================================================================================
' Process WM_PAINT message for window/dialog: frmMain
' ========================================================================================
function frmMain_OnPaint( byval hwnd as HWND ) as LRESULT
    dim as clsDoubleBuffer b
    dim pWindow as CWindow ptr = AfxCWindowPtr(hwnd)
    if pWindow = null then return 0

    b.BeginDoubleBuffer(hwnd)
    b.SetBackColor( theme.BackColorWindow )
    b.PaintClientRect()

    ' Captions above each textbox, and the last callback event at the bottom.
    dim as RECT rc = b.rcClient()
    dim as RECT rcText
    b.SetFont( ghFont(GUIFONT_9) )
    b.SetForeColor( theme.ForeColorLabel )

    SetRect( @rcText, pWindow->ScaleX(DEMO_LEFT), pWindow->ScaleY(DEMO_BOX1_TOP - 22), _
             rc.right, pWindow->ScaleY(DEMO_BOX1_TOP) )
    b.PaintText( "Cue banner, rounded focus border, all callbacks (ESC clears via MessageCallback):", @rcText, DT_LEFT )

    SetRect( @rcText, pWindow->ScaleX(DEMO_LEFT), pWindow->ScaleY(DEMO_BOX2_TOP - 22), _
             rc.right, pWindow->ScaleY(DEMO_BOX2_TOP) )
    b.PaintText( "Read-only, wide margins, select-all on focus (context menu offers Copy only):", @rcText, DT_LEFT )

    SetRect( @rcText, pWindow->ScaleX(DEMO_LEFT), pWindow->ScaleY(DEMO_BOX3_TOP - 22), _
             rc.right, pWindow->ScaleY(DEMO_BOX3_TOP) )
    b.PaintText( "Password char, 8-character limit, borderless:", @rcText, DT_LEFT )

    if len(gwszLastEvent) then
        SetRect( @rcText, pWindow->ScaleX(DEMO_LEFT), rc.bottom - pWindow->ScaleY(30), _
                 rc.right, rc.bottom )
        b.SetForeColor( theme.ForeColorEvent )
        b.PaintText( gwszLastEvent, @rcText, DT_LEFT )
    end if

    b.EndDoubleBuffer()

    function = 0
end function


' ========================================================================================
' Position the three textboxes. The control has no opinion about its own placement --
' the host positions it like any window.
' ========================================================================================
sub frmMain_PositionTextBoxes( byval hwnd as HWND )
    dim as RECT rc: GetClientRect( hwnd, @rc )
    dim pWindow as CWindow ptr = AfxCWindowPtr(hwnd)
    if pWindow = null then exit sub

    dim as long nLeft   = pWindow->ScaleX(DEMO_LEFT)
    dim as long nWidth  = (rc.right - rc.left) - (nLeft * 2)
    dim as long nHeight = pWindow->ScaleY(DEMO_BOX_HEIGHT)
    if nWidth < 50 then nWidth = 50

    dim as HWND hCtl = GetDlgItem( hwnd, IDC_FRMMAIN_TEXTBOX1 )
    if IsWindow(hCtl) then SetWindowPos( hCtl, 0, nLeft, pWindow->ScaleY(DEMO_BOX1_TOP), nWidth, nHeight, SWP_NOZORDER or SWP_SHOWWINDOW )
    hCtl = GetDlgItem( hwnd, IDC_FRMMAIN_TEXTBOX2 )
    if IsWindow(hCtl) then SetWindowPos( hCtl, 0, nLeft, pWindow->ScaleY(DEMO_BOX2_TOP), nWidth, nHeight, SWP_NOZORDER or SWP_SHOWWINDOW)
    hCtl = GetDlgItem( hwnd, IDC_FRMMAIN_TEXTBOX3 )
    if IsWindow(hCtl) then SetWindowPos( hCtl, 0, nLeft, pWindow->ScaleY(DEMO_BOX3_TOP), nWidth, nHeight, SWP_NOZORDER or SWP_SHOWWINDOW)
end sub


' ========================================================================================
' Process WM_SIZE message for window/dialog: frmMain
' ========================================================================================
function frmMain_OnSize( _
            byval hwnd as HWND, _
            byval state as UINT, _
            byval cx as long, _
            byval cy as long _
            ) as LRESULT
    if state <> SIZE_MINIMIZED then
        frmMain_PositionTextBoxes( hwnd )
        AfxRedrawWindow( hwnd )
    end if
    function = 0
end function


' ========================================================================================
' Process WM_CLOSE message for window/dialog: frmMain
' ========================================================================================
function frmMain_OnClose( byval hwnd as HWND ) as LRESULT
    DestroyWindow(hwnd)
    function = 0
end Function


' ========================================================================================
' Process WM_DESTROY message for window/dialog: frmMain
' ========================================================================================
function frmMain_OnDestroy( byval hwnd as HWND ) as LRESULT
    PostQuitMessage(0)
    function = 0
end Function


' ========================================================================================
' frmMain Window procedure
' ========================================================================================
function frmMain_WndProc( _
            byval hwnd   as HWND, _
            byval uMsg   as UINT, _
            byval wParam as WPARAM, _
            byval lParam as LPARAM _
            ) as LRESULT

    select case uMsg
        HANDLE_MSG (hwnd, WM_SIZE,          frmMain_OnSize)
        HANDLE_MSG (hwnd, WM_PAINT,         frmMain_OnPaint)
        HANDLE_MSG (hwnd, WM_CLOSE,         frmMain_OnClose)
        HANDLE_MSG (hwnd, WM_DESTROY,       frmMain_OnDestroy)

        case WM_NCDESTROY
            ' delete the allocated memory for window
            dim as CWindow ptr pWindow = AfxCWindowPtr(hwnd)
            if pWindow then delete(pWindow)
            HWND_FRMMAIN = 0
            return 0

        case WM_ERASEBKGND
            return true      ' prevents painting the background

    end select

    ' for messages that we don't deal with
    function = DefWindowProc(HWnd, uMsg, wParam, lParam)

end Function


' ========================================================================================
' Report a callback event: console (tracing) plus the in-window status line.
' Trace output goes through a plain string first -- printing a DWSTRING (or a
' string & wstring concat) interleaves nulls (see C:\dev\Learnings.md).
' ========================================================================================
sub Demo_ReportEvent( byval wszEvent as DWSTRING )
    gwszLastEvent = wszEvent
    dim as string s = gwszLastEvent
    print s
    if IsWindow(HWND_FRMMAIN) then InvalidateRect( HWND_FRMMAIN, null, true )
end sub


' ========================================================================================
' CTextBox callbacks (all wired to the first textbox).
' ========================================================================================
sub TextBox_ChangeCallback( byval hTextBox as HWND )
    Demo_ReportEvent( "changed: " & CTextBox_GetText(hTextBox) )
end sub

sub TextBox_FocusCallback( byval hTextBox as HWND, byval bGotFocus as boolean )
    dim as long id = GetDlgCtrlID( hTextBox )
    Demo_ReportEvent( "focus " & iif(bGotFocus, "gained", "lost") & ": textbox id " & id )
end sub

sub TextBox_EnterPressedCallback( byval hTextBox as HWND )
    Demo_ReportEvent( "ENTER pressed: [" & CTextBox_GetText(hTextBox) & "]" )
end sub

function TextBox_MessageCallback( byval m as CTEXTBOX_MESSAGEINFO ptr ) as boolean
    ' ESC clears the box. CTextBox_Clear is programmatic, so no ChangeCallback fires --
    ' the status line below is this callback's own doing.
    if (m->uMsg = WM_KEYDOWN) andalso (m->wParam = VK_ESCAPE) then
        CTextBox_Clear( m->hTextBox )
        Demo_ReportEvent( "ESC: cleared (silently - no change event)" )
        return true
    end if
    return false
end function


' ========================================================================================
' frmMain_Show
' ========================================================================================
function frmMain_Show( byval hWndParent as HWND ) as LRESULT

    '  Create the main window and child controls
    dim pWindow as CWindow ptr = new CWindow(APPCLASSNAME)

    ghFont(GUIFONT_9)       = pWindow->CreateFont(GUIFONT, 9, FW_NORMAL)
    ghFont(GUIFONT_10)      = pWindow->CreateFont(GUIFONT, 10, FW_NORMAL)
    ghFont(CUEFONT_10)      = pWindow->CreateFont(GUIFONT, 10, FW_NORMAL, CTRUE)   ' italic cue

    HWND_FRMMAIN = pWindow->Create( _
            null, APPNAME, @frmMain_WndProc, _
            100, 100, 700, 330 _
            )

    ' --- Textbox 1: cue banner with its own color and italic font, rounded border that
    '     lights up on focus, margins, and every callback wired. -----------------------
    dim as HWND hCtl = CTextBox_Create( HWND_FRMMAIN, IDC_FRMMAIN_TEXTBOX1 )
    CTextBox_SetFont( hCtl, ghFont(GUIFONT_10) )
    CTextBox_SetForeColor( hCtl, theme.ForeColor )
    CTextBox_SetBackColor( hCtl, theme.BackColorBox )
    CTextBox_SetCueBannerText( hCtl, "Search anything..." )
    CTextBox_SetCueBannerColor( hCtl, theme.ForeColorCue )
    CTextBox_SetCueBannerFont( hCtl, ghFont(CUEFONT_10) )
    CTextBox_SetBorderColor( hCtl, theme.BorderColor )
    CTextBox_SetFocusBorderColor( hCtl, theme.FocusAccent )
    ' All CTextBox pixel values are raw device pixels: DPI-scale at the call site.
    CTextBox_SetCornerRadius( hCtl, pWindow->ScaleX(4) )
    CTextBox_SetOuterBackColor( hCtl, theme.BackColorWindow )   ' corners blend into the form
    CTextBox_SetMargins( hCtl, pWindow->ScaleX(8), pWindow->ScaleX(8) )
    CTextBox_SetChangeCallback( hCtl, @TextBox_ChangeCallback )
    CTextBox_SetFocusCallback( hCtl, @TextBox_FocusCallback )
    CTextBox_SetEnterPressedCallback( hCtl, @TextBox_EnterPressedCallback )
    CTextBox_SetMessageCallback( hCtl, @TextBox_MessageCallback )

    ' --- Textbox 2: read-only with wide margins; its context menu offers Copy only. ---
    hCtl = CTextBox_Create( HWND_FRMMAIN, IDC_FRMMAIN_TEXTBOX2 )
    CTextBox_SetFont( hCtl, ghFont(GUIFONT_10) )
    CTextBox_SetForeColor( hCtl, theme.ForeColor )
    CTextBox_SetBackColor( hCtl, theme.BackColorBox )
    CTextBox_SetBorderColor( hCtl, theme.BorderColor )
    CTextBox_SetFocusBorderColor( hCtl, theme.FocusAccent )
    CTextBox_SetOuterBackColor( hCtl, theme.BackColorWindow )
    CTextBox_SetMargins( hCtl, pWindow->ScaleX(16), pWindow->ScaleX(16) )
    CTextBox_SetText( hCtl, "Read-only text set programmatically (no change event fired)" )
    CTextBox_SetReadOnly( hCtl, true )
    CTextBox_SetSelectOnFocus( hCtl, true )    ' Tab into this box selects everything
    ' The message door: classic edit messages sent to the CONTROL reach the RichEdit.
    dim as long nDoorLen = SendMessage( hCtl, WM_GETTEXTLENGTH, 0, 0 )
    dim as string s = "message door: WM_GETTEXTLENGTH on the control returned " & nDoorLen
    print s
    SendMessage( hCtl, EM_SETSEL, 0, 9 )    ' select "Read-only" through the same door

    ' --- Textbox 3: password char, 8-character limit, borderless. ---------------------
    hCtl = CTextBox_Create( HWND_FRMMAIN, IDC_FRMMAIN_TEXTBOX3 )
    CTextBox_SetFont( hCtl, ghFont(GUIFONT_10) )
    CTextBox_SetForeColor( hCtl, theme.ForeColor )
    CTextBox_SetBackColor( hCtl, theme.BackColorBox )
    CTextBox_SetBorderWidth( hCtl, 0 )
    CTextBox_SetOuterBackColor( hCtl, theme.BackColorWindow )
    CTextBox_SetCueBannerText( hCtl, "PIN (max 8)" )
    CTextBox_SetCueBannerColor( hCtl, theme.ForeColorCue )
    CTextBox_SetMargins( hCtl, pWindow->ScaleX(8), pWindow->ScaleX(8) )
    CTextBox_SetLimitText( hCtl, 8 )
    CTextBox_SetPasswordChar( hCtl, asc("*") )

    frmMain_PositionTextBoxes( HWND_FRMMAIN )

    UpdateWindow( pWindow->hWindow )
    ShowWindow( pWindow->hWindow, SW_SHOW )

    ' Process windows events
    dim uMsg as MSG

    ' Message loop. No IsDialogMessage here on purpose: it swallows VK_RETURN/VK_ESCAPE
    ' before the textbox sees them, which would defeat EnterPressedCallback and the ESC
    ' handling in the MessageCallback. Tab still navigates between the boxes -- the
    ' control handles VK_TAB itself (GetNextDlgTabItem), no dialog loop needed.
    do while GetMessage(@uMsg, null, 0, 0)
        if uMsg.message = WM_QUIT then exit do
        TranslateMessage @uMsg    ' Translates virtual-key messages into character messages.
        DispatchMessage @uMsg     ' Dispatches a message to a window procedure.
    loop  ' message loop

    function = uMsg.wParam

    for i as long = 0 to MAXFONTS - 1
        if ghFont(i) then DeleteObject(ghFont(i))
    next

end function
