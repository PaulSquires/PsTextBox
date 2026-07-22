# CTextBox

A reusable textbox custom control for FreeBASIC / Win32, built on AfxNova — single-line
by default, optionally **multiline** (word-wrapped; a creation-time choice).
The editing engine is a **RichEdit50W** child window wrapped in a `CWindow` container —
RichEdit rather than EDIT because it supports vertically centered text (`EM_SETRECT`)
and per-control colors (`EM_SETCHARFORMAT` / `EM_SETBKGNDCOLOR`) without owner-draw.
Any number of instances can coexist; each owns all of its state.

- **Cue banner** with its own forecolor and (optionally) its own font, drawn whenever
  the buffer is empty — focused or not, the caret blinks over it
- One user-set **forecolor / backcolor** for all text (never per-character; the child
  runs in `TM_PLAINTEXT` mode, so pasted text sheds any rich formatting)
- **Border chrome**: width (0 = none), color, a separate focus color, small **rounded
  corners**, and a settable outer fill so the corners blend into the host background
- **Left/right margins** (`EM_SETMARGINS`) and automatic horizontal scroll
  (`ES_AUTOHSCROLL`)
- Dedicated **event callbacks** (change / focus / Enter / scroll-changed) plus the
  sibling-standard observe-with-veto message callback
- Built-in, localizable right-click **Cut/Copy/Paste/Select All menu**, drawn by
  **CPopupMenu** so it can be themed to match the host's other menus (see *Context menu*
  — it needs one line in your message pump)
- Optional **multiline mode** (`CTextBox_Create(..., true)`) — word wrap, ENTER inserts
  a newline, TAB inserts a tab character, no native scrollbar: an external scrollbar
  (CVScrollBar) is driven through `ScrollChangedCallback` / `CTextBox_GetVScrollInfo` /
  `CTextBox_ScrollToLine` (see *Multiline mode* below)
- Optional **select-all on focus** (`CTextBox_SetSelectOnFocus`) — selects everything
  when focus arrives via Tab or a programmatic `SetFocus`; a mouse click still places
  the caret at the click point
- Optional **numeric-only mode** (`CTextBox_SetNumericMode`) — accepts digits, one
  leading minus and one decimal separator, typed *or pasted*, with a configurable
  number of decimal places (`CTextBox_SetDecimalPlaces`, 0 = integers only). On focus
  loss the value is reformatted to exactly that many places (empty stays empty so the
  cue can show — or set `CTextBox_SetZeroWhenEmpty` to display the formatted zero,
  e.g. `0.00`, instead). `CTextBox_GetValue` / `CTextBox_SetValue` trade the text as
  a double.
- A **message door**: classic edit-control messages sent to the control are forwarded
  to the RichEdit child

## Files

| File | Purpose |
|---|---|
| `CTextBox.bi` / `.inc` | The control. `CTextBox.bi` is the documented public header. |
| `CPopupMenu.bi` / `.inc` | The right-click menu (vendored from `C:\dev\CMenuBar`, its canonical home — sync from there, don't edit here) |
| `clsDoubleBuffer.bi` / `.inc` | Flicker-free drawing helper (chrome painting) |
| `main.bas`, `frmMain.bi` / `.inc` | Demo / test harness (five instances, incl. two multiline; `CTEXTBOX_SMOKE=1` runs the startup asserts and exits) |

`CTextBox.bi` pulls in `AfxNova\AfxRichEdit.inc` itself (RichEdit definitions and
helpers) and `CPopupMenu.bi`. Include order:

```freebasic
#include once "clsDoubleBuffer.inc"
#include once "CPopupMenu.inc"
#include once "CTextBox.inc"
```

An app already hosting `CMenuBar` has `CPopupMenu.inc` included once already — that is
the same file, so nothing is duplicated.

## Quick start

```freebasic
' Create, then position it like any window.
dim as HWND hBox = CTextBox_Create( hWndParent, IDC_MYTEXTBOX )
SetWindowPos( hBox, 0, x, y, cx, 32, SWP_NOZORDER or SWP_SHOWWINDOW )

CTextBox_SetFont( hBox, hMyFont )                      ' you keep ownership
CTextBox_SetForeColor( hBox, BGR(215,218,224) )
CTextBox_SetBackColor( hBox, BGR(44,49,58) )

CTextBox_SetCueBannerText( hBox, "Search anything..." )
CTextBox_SetCueBannerColor( hBox, BGR(128,134,143) )   ' independent of the forecolor
CTextBox_SetCueBannerFont( hBox, hItalicFont )         ' optional; defaults to the text font

CTextBox_SetBorderColor( hBox, BGR(60,66,77) )
CTextBox_SetFocusBorderColor( hBox, BGR(86,156,214) )  ' lights up while editing
CTextBox_SetCornerRadius( hBox, pWindow->ScaleX(4) )   ' keep it small (plain GDI arcs)
CTextBox_SetOuterBackColor( hBox, BGR(33,37,43) )      ' host background: corners blend in
CTextBox_SetMargins( hBox, pWindow->ScaleX(8), pWindow->ScaleX(8) )

CTextBox_SetChangeCallback( hBox, @MyChangeCallback )
CTextBox_SetEnterPressedCallback( hBox, @MyEnterCallback )
```

## Concepts worth knowing

- **The handle is the control.** `CTextBox_Create` returns a real `HWND` carrying the
  `CtrlID` (`GetDlgItem` finds it). Position it with `SetWindowPos`; the RichEdit child
  is derived geometry (inset by the border width) and is never positioned by the host.
  `CTextBox_GetRichEditHandle` exists as an escape hatch for RichEdit-specific messages.
  Keyboard focus sits on the child, so `GetFocus() = hBox` is always false — use
  `CTextBox_HasFocus( hBox )` for focus tests.
- **Programmatic setters are silent.** `SetText` / `ReplaceSel` / `Clear` (and a
  forwarded `WM_SETTEXT`) never fire the ChangeCallback; only user edits do — typing,
  cut/paste, undo. A host can safely call a setter from inside its own handler.
- **The message door.** The container forwards the classic edit-message range
  (`EM_GETSEL` .. `EM_GETIMESTATUS`) plus `WM_SETTEXT` / `WM_GETTEXT` /
  `WM_GETTEXTLENGTH` / `WM_CUT` / `WM_COPY` / `WM_PASTE` / `WM_CLEAR` / `WM_UNDO`, so
  `SendMessage( hBox, EM_SETSEL, 0, -1 )` works from a separate control or window.
  One implementation, two doors: message for a separate control, function
  (`CTextBox_SetSel` etc.) for an in-process host.
- **The line mode is fixed at creation.** `ES_MULTILINE` cannot be toggled on a live
  window, so `CTextBox_Create`'s `bMultiline` parameter decides it once. Single-line
  (the default): ENTER is swallowed (no newline, no beep) and reported through the
  EnterPressedCallback; a multiline paste keeps only what fits on one line. Multiline:
  ENTER and TAB are ordinary editing keys, text starts at the top (no vertical
  centering), and the EnterPressedCallback never fires.
- **Fonts are caller-owned.** The control converts the text `HFONT` to a `CHARFORMATW`
  (face, size, charset, bold/italic/underline/strikeout + forecolor) internally and
  re-derives vertical centering on every size or font change. It never deletes an HFONT.
- **No mouse capture.** The RichEdit manages its own selection-drag capture; the
  container takes none, so the MessageCallback veto is honored uniformly — there is no
  press state a suppressed message could strand.
- **Tab navigation is built in.** The RichEdit child carries `WS_TABSTOP` and the
  container `WS_EX_CONTROLPARENT`, and the control handles `VK_TAB` itself
  (`GetNextDlgTabItem`, Shift+Tab for backwards) — so tabbing between controls works
  even without `IsDialogMessage` in the host's loop. Veto `VK_TAB` in the
  MessageCallback to repurpose Tab (e.g. to drive a picker list).

## Callbacks

| Callback | Fires |
|---|---|
| `TXT_ChangeCallbackSub` | after a USER edit changed the text (programmatic = silent) |
| `TXT_FocusCallbackSub` | RichEdit gained / lost keyboard focus (border already repainted) |
| `TXT_EnterPressedCallbackSub` | ENTER pressed (single-line only; the keypress itself is always swallowed) |
| `TXT_MessageCallbackFunc` | key / mouse / focus / context-menu messages, before the control acts; return TRUE to suppress. Suppressing `WM_SETFOCUS` / `WM_KILLFOCUS` also suppresses the RichEdit's caret handling — only do that on purpose. |
| `TXT_ScrollChangedCallbackSub` | multiline only: the vertical scroll state may have changed (typing, programmatic SetText, wheel/keys). Unlike the ChangeCallback it is NOT silenced for programmatic changes — a scrollbar must hear about those too. |

## Context menu

Right-click shows Cut / Copy / Paste / Select All, filtered by state (read-only offers
Copy and Select All only; Select All appears whenever the control holds text; nothing
eligible = no menu). Localize the labels with
`CTextBox_SetMenuText( hBox, "Ausschneiden", "Kopieren", "Einfügen", "Alles auswählen" )`
— the fourth argument is optional; empty keeps the current Select All label. The
MessageCallback sees `WM_CONTEXTMENU` first — return TRUE to suppress or replace it.

The menu is a **CPopupMenu** (vendored from `C:\dev\CMenuBar`), not `TrackPopupMenu` —
owner-drawn, so it can be themed to match the rest of your menus instead of showing a
native grey menu in the middle of a dark UI.

**This puts one obligation on the host, and it is not optional.** A CPopupMenu is *not*
modal: keyboard navigation and dismissal on an outside click both live in a message
filter. Skip the filter and the menu opens and paints, but arrow keys do nothing and it
never closes when you click elsewhere.

```freebasic
do while GetMessage(@uMsg, null, 0, 0)
    if CTextBox_FilterMessage( @uMsg ) then continue do
    TranslateMessage @uMsg
    DispatchMessage @uMsg
loop
```

One call serves every CTextBox in the application — only one menu chain can be open at a
time and the filter finds it. An app that also hosts `CMenuBar` calls that filter too;
the two are independent and each stands down while the other's menu is up.

Styling is yours: `CTextBox_GetContextMenu( hBox )` returns the popup, so the usual
`CPopupMenu_SetColors` / `SetFonts` / `SetGlyphs` / `SetItemHeight` apply. The handle is
stable for the control's lifetime, so theme it once after `CTextBox_Create` — labels are
rebuilt per open, colors and fonts are not. Left alone it renders with CPopupMenu's own
defaults. `CTextBox_CloseContextMenu()` dismisses an open menu (silently) for hosts with
a global "close every menu" moment, such as app deactivation.

> **Upgrading:** this replaced an inline `TrackPopupMenu` that ran the commands in the
> same stack frame as `WM_CONTEXTMENU`. They now run from the popup's select callback,
> after the menu closes. Existing hosts must add the `CTextBox_FilterMessage` line and
> include `CPopupMenu.inc`.

## Multiline mode

Create with `CTextBox_Create( hWndParent, CtrlID, true )`. The child gets
`ES_MULTILINE or ES_AUTOVSCROLL or ES_WANTRETURN` and **no** `ES_AUTOHSCROLL`, so text
word-wraps at the control width. No scrollbar styles are added — the control scrolls
content but shows no native scrollbar, by design: pair it with an external themed
scrollbar (CVScrollBar) through three pieces, all in LINE units:

```freebasic
' 1. The control says "my scroll state may have changed" (fires on typing,
'    programmatic SetText, wheel/keyboard scrolling):
sub MyScrollChanged( byval hTextBox as HWND )
    dim as integer nTotal, nPage, nFirst
    CTextBox_GetVScrollInfo( hTextBox, nTotal, nPage, nFirst )   ' 2. read the range
    CVScrollBar_SetRange( hMyScrollBar, nTotal, nPage, nFirst )  '    push it
end sub
CTextBox_SetScrollChangedCallback( hBox, @MyScrollChanged )

' 3. The scrollbar says "the user dragged to newPos":
sub MyScrollCallback( byval hScrollBar as HWND, byval newPos as integer )
    CTextBox_ScrollToLine( hBox, newPos )
end sub
```

**Wheel scrolling is implemented by the control itself** — the RichEdit's native
`WM_MOUSEWHEEL` handling ignored the message in this configuration even when shown,
focused and styled `WS_VSCROLL`, so the subclass converts the (accumulated, signed)
wheel delta to `EM_LINESCROLL`, honoring the system wheel-lines setting including
page-per-notch. A user wheel scroll fires the ScrollChangedCallback; the programmatic
`ScrollToLine` stays silent — the same user-interaction-notifies split as the other
callbacks.

`GetVScrollInfo`'s lines-per-page derives from the formatting-rect height and the text
font's line height — no fudge factors; a partial line at the bottom is not counted.
Caveat: a programmatic `SetText` into a control that has never been sized/shown may not
fire the callback (the RichEdit skips the display update) — hosts that fill a hidden
control should refresh their scrollbar when showing it. Numeric mode is single-line
only (`SetNumericMode` no-ops on a multiline control); the cue banner word-wraps.

## Building

The demo builds with the FreeBASIC toolchain, 64-bit, with the workspace root on the
include path (sources include AfxNova as `AfxNova\...`):

```
fbc64.exe -i "C:\dev" main.bas
```

## Design notes

- **Pixel values are raw.** Margins, border width and corner radius are device pixels;
  DPI-scale at the call site (`pWindow->ScaleX`). The control never scales a caller's
  value, so nothing double-scales and `EM_GETMARGINS` always reads back what was set.

- **A message loop caution:** `IsDialogMessage` swallows `VK_RETURN` / `VK_ESCAPE`
  before the textbox ever sees them, which defeats the EnterPressedCallback (and any
  ESC handling in a MessageCallback). The demo's loop deliberately omits it.
- The formatting rect (`EM_SETRECT` vertical centering) and the margins
  (`EM_SETMARGINS`) are set through different messages; the control stores the margins
  and re-applies them after every rect change so they survive font and size changes.
- `EM_SETMARGINS` sent through the message door is intercepted for the same reason:
  the values are stored control-side before forwarding.
- The corner radius maps to a plain GDI `RoundRect` (curvature = radius × 2). Corners
  are aliased; the radius is meant to stay small. `SetOuterBackColor` decides what the
  pixels outside the arcs show.
