#include once "frmMain.bi"
#include once "CBufferPaint.bi"


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
#define DEMO_BOX4_TOP    274                ' multiline editable
#define DEMO_BOX5_TOP    416                ' multiline read-only
#define DEMO_MULTI_HEIGHT 110


' ========================================================================================
' Process WM_PAINT message for window/dialog: frmMain
' ========================================================================================
function frmMain_OnPaint( byval hwnd as HWND ) as LRESULT
    dim as CBufferPaint b
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
    b.PaintText( "Numeric only, 2 decimal places, formats on focus loss, empty shows 0.00:", @rcText, DT_LEFT )

    SetRect( @rcText, pWindow->ScaleX(DEMO_LEFT), pWindow->ScaleY(DEMO_BOX4_TOP - 22), _
             rc.right, pWindow->ScaleY(DEMO_BOX4_TOP) )
    b.PaintText( "Multiline, editable: ENTER = newline, TAB = tab char, cue wraps, scroll events below:", @rcText, DT_LEFT )

    SetRect( @rcText, pWindow->ScaleX(DEMO_LEFT), pWindow->ScaleY(DEMO_BOX5_TOP - 22), _
             rc.right, pWindow->ScaleY(DEMO_BOX5_TOP) )
    b.PaintText( "Multiline, read-only, 40 preloaded lines (wheel-scroll; menu offers Copy + Select All):", @rcText, DT_LEFT )

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

    dim as long nMultiHeight = pWindow->ScaleY(DEMO_MULTI_HEIGHT)
    hCtl = GetDlgItem( hwnd, IDC_FRMMAIN_TEXTBOX4 )
    if IsWindow(hCtl) then SetWindowPos( hCtl, 0, nLeft, pWindow->ScaleY(DEMO_BOX4_TOP), nWidth, nMultiHeight, SWP_NOZORDER or SWP_SHOWWINDOW)
    hCtl = GetDlgItem( hwnd, IDC_FRMMAIN_TEXTBOX5 )
    if IsWindow(hCtl) then SetWindowPos( hCtl, 0, nLeft, pWindow->ScaleY(DEMO_BOX5_TOP), nWidth, nMultiHeight, SWP_NOZORDER or SWP_SHOWWINDOW)
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

sub TextBox_ScrollChangedCallback( byval hTextBox as HWND )
    ' The CVScrollBar wiring pattern: on every fire, read the range and (there) push it
    ' into CVScrollBar_SetRange( total, page, pos ). Here it goes to the status line.
    dim as integer nTotal, nPage, nFirst
    CTextBox_GetVScrollInfo( hTextBox, nTotal, nPage, nFirst )
    Demo_ReportEvent( "scroll: id " & GetDlgCtrlID(hTextBox) & _
                      "  total=" & nTotal & "  page=" & nPage & "  first=" & nFirst )
end sub


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
            100, 100, 700, 650 _
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

    ' --- Textbox 3: numeric-only (2 decimal places), borderless. ----------------------
    hCtl = CTextBox_Create( HWND_FRMMAIN, IDC_FRMMAIN_TEXTBOX3 )
    CTextBox_SetFont( hCtl, ghFont(GUIFONT_10) )
    CTextBox_SetForeColor( hCtl, theme.ForeColor )
    CTextBox_SetBackColor( hCtl, theme.BackColorBox )
    CTextBox_SetBorderWidth( hCtl, 0 )
    CTextBox_SetOuterBackColor( hCtl, theme.BackColorWindow )
    CTextBox_SetCueBannerText( hCtl, "Amount (digits, minus, one decimal)" )
    CTextBox_SetCueBannerColor( hCtl, theme.ForeColorCue )
    CTextBox_SetMargins( hCtl, pWindow->ScaleX(8), pWindow->ScaleX(8) )
    CTextBox_SetNumericMode( hCtl, true )
    CTextBox_SetDecimalPlaces( hCtl, 2 )
    CTextBox_SetSelectOnFocus( hCtl, true )    ' tab in, type a fresh amount
    CTextBox_SetZeroWhenEmpty( hCtl, true )    ' shows 0.00 instead of the cue

    ' --- Textbox 4: MULTILINE, editable. ENTER inserts a newline (no EnterPressed
    '     event exists in multiline), TAB inserts a tab character, the cue banner
    '     word-wraps, and every scroll-state change reports through the
    '     ScrollChangedCallback (the CVScrollBar wiring point). -----------------------
    hCtl = CTextBox_Create( HWND_FRMMAIN, IDC_FRMMAIN_TEXTBOX4, true )
    CTextBox_SetFont( hCtl, ghFont(GUIFONT_10) )
    CTextBox_SetForeColor( hCtl, theme.ForeColor )
    CTextBox_SetBackColor( hCtl, theme.BackColorBox )
    CTextBox_SetBorderColor( hCtl, theme.BorderColor )
    CTextBox_SetFocusBorderColor( hCtl, theme.FocusAccent )
    CTextBox_SetOuterBackColor( hCtl, theme.BackColorWindow )
    CTextBox_SetCueBannerText( hCtl, "Type multiline notes here... ENTER starts a new line, TAB indents, and this cue banner word-wraps instead of running off the right edge of the box." )
    CTextBox_SetCueBannerColor( hCtl, theme.ForeColorCue )
    CTextBox_SetCueBannerFont( hCtl, ghFont(CUEFONT_10) )
    CTextBox_SetMargins( hCtl, pWindow->ScaleX(8), pWindow->ScaleX(8) )
    CTextBox_SetChangeCallback( hCtl, @TextBox_ChangeCallback )
    CTextBox_SetScrollChangedCallback( hCtl, @TextBox_ScrollChangedCallback )
    CTextBox_SetNumericMode( hCtl, true )      ' must be REFUSED (multiline) - asserted below

    ' --- Textbox 5: MULTILINE, read-only, preloaded with 40 numbered lines (the
    '     Compiler Log shape). The scroll callback is set BEFORE the SetText so the
    '     programmatic fire proves the callback is not gated like ChangeCallback. -----
    hCtl = CTextBox_Create( HWND_FRMMAIN, IDC_FRMMAIN_TEXTBOX5, true )
    CTextBox_SetFont( hCtl, ghFont(GUIFONT_10) )
    CTextBox_SetForeColor( hCtl, theme.ForeColor )
    CTextBox_SetBackColor( hCtl, theme.BackColorBox )
    CTextBox_SetBorderColor( hCtl, theme.BorderColor )
    CTextBox_SetFocusBorderColor( hCtl, theme.FocusAccent )
    CTextBox_SetOuterBackColor( hCtl, theme.BackColorWindow )
    CTextBox_SetMargins( hCtl, pWindow->ScaleX(8), pWindow->ScaleX(8) )
    CTextBox_SetScrollChangedCallback( hCtl, @TextBox_ScrollChangedCallback )
    ' Built in a plain string (mixed DWSTRING & chains are ambiguous overloads), then
    ' converted once on assignment -- pure-ASCII content, so the ANSI conversion is safe.
    dim as string sLines
    for i as long = 1 to 40
        sLines &= "Line " & i & " of 40 - preloaded read-only multiline content"
        if i < 40 then sLines &= chr(13, 10)
    next
    dim as DWSTRING wszLines = sLines
    CTextBox_SetText( hCtl, wszLines )
    CTextBox_SetReadOnly( hCtl, true )

    frmMain_PositionTextBoxes( HWND_FRMMAIN )

    ' --- Startup asserts (geometry is asserted, never eyeballed): the boxes are sized
    '     now, so the scroll numbers are real. --------------------------------------
    dim as HWND hMulti = GetDlgItem( HWND_FRMMAIN, IDC_FRMMAIN_TEXTBOX5 )
    dim as integer nTotal, nPage, nFirst
    CTextBox_GetVScrollInfo( hMulti, nTotal, nPage, nFirst )
    s = "ASSERT total lines = 40: got " & nTotal & iif( nTotal = 40, "  [ok]", "  [FAIL]" )
    print s
    s = "ASSERT first visible = 0: got " & nFirst & iif( nFirst = 0, "  [ok]", "  [FAIL]" )
    print s
    ' Page count depends on DPI/font metrics; assert a sane band and print the exact
    ' value for the hand-check (box is 110 design px tall, minus 1px border each side).
    s = "ASSERT 3 <= page <= 12: got " & nPage & iif( (nPage >= 3) and (nPage <= 12), "  [ok]", "  [FAIL]" )
    print s
    CTextBox_ScrollToLine( hMulti, 10 )
    CTextBox_GetVScrollInfo( hMulti, nTotal, nPage, nFirst )
    s = "ASSERT ScrollToLine(10) -> first = 10: got " & nFirst & iif( nFirst = 10, "  [ok]", "  [FAIL]" )
    print s
    CTextBox_ScrollToLine( hMulti, 0 )
    dim as boolean bNum = CTextBox_GetNumericMode( GetDlgItem(HWND_FRMMAIN, IDC_FRMMAIN_TEXTBOX4) )
    s = "ASSERT numeric mode refused on multiline: " & iif( bNum = false, "[ok]", "[FAIL]" )
    print s
    dim as boolean bMulti4 = CTextBox_GetMultiline( GetDlgItem(HWND_FRMMAIN, IDC_FRMMAIN_TEXTBOX4) )
    dim as boolean bMulti1 = CTextBox_GetMultiline( GetDlgItem(HWND_FRMMAIN, IDC_FRMMAIN_TEXTBOX1) )
    s = "ASSERT GetMultiline box4/box1 = true/false: " & _
        iif( (bMulti4 = true) andalso (bMulti1 = false), "[ok]", "[FAIL]" )
    print s

    UpdateWindow( pWindow->hWindow )
    ShowWindow( pWindow->hWindow, SW_SHOW )

    ' Wheel scrolling assert runs against the SHOWN window (the RichEdit's wheel
    ' handling is tested in its real state): one downward notch straight to the
    ' RichEdit child (delta -120 in the wParam high word) must move the first line.
    ' One downward wheel notch (signed delta -120 in the wParam high word) must move
    ' the first visible line -- the control's own wheel handler, so a plain SendMessage
    ' exercises the real path (the RichEdit's native handling is bypassed entirely).
    dim as long wheelParam = cast(long, cast(ushort, -120)) shl 16
    SendMessage( CTextBox_GetRichEditHandle(hMulti), WM_MOUSEWHEEL, cast(WPARAM, wheelParam), 0 )
    CTextBox_GetVScrollInfo( hMulti, nTotal, nPage, nFirst )
    s = "ASSERT wheel notch down -> first > 0: got " & nFirst & iif( nFirst > 0, "  [ok]", "  [FAIL]" )
    print s
    CTextBox_ScrollToLine( hMulti, 0 )

    ' Unattended smoke run: CTEXTBOX_SMOKE=1 closes the window right away so the process
    ' exits cleanly and a redirected stdout flushes (a killed FB process loses every
    ' print -- see C:\dev\Learnings.md).
    if environ("CTEXTBOX_SMOKE") <> "" then PostMessage( HWND_FRMMAIN, WM_CLOSE, 0, 0 )

    ' Process windows events
    dim uMsg as MSG

    ' Message loop. No IsDialogMessage here on purpose: it swallows VK_RETURN/VK_ESCAPE
    ' before the textbox sees them, which would defeat EnterPressedCallback and the ESC
    ' handling in the MessageCallback. Tab still navigates between the boxes -- the
    ' control handles VK_TAB itself (GetNextDlgTabItem), no dialog loop needed.
    do while GetMessage(@uMsg, null, 0, 0)
        if uMsg.message = WM_QUIT then exit do
        ' MANDATORY once a CTextBox is on screen: its right-click menu is a CPopupMenu,
        ' which is not modal -- keyboard navigation and dismissal on an outside click
        ' both live in this filter. One call covers every textbox in the app.
        if CTextBox_FilterMessage( @uMsg ) then continue do
        TranslateMessage @uMsg    ' Translates virtual-key messages into character messages.
        DispatchMessage @uMsg     ' Dispatches a message to a window procedure.
    loop  ' message loop

    function = uMsg.wParam

    for i as long = 0 to MAXFONTS - 1
        if ghFont(i) then DeleteObject(ghFont(i))
    next

end function
