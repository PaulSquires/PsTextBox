# CTextBox

A reusable single-line textbox custom control for FreeBASIC / Win32, built on AfxNova.
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
- Dedicated **event callbacks** (change / focus / Enter) plus the sibling-standard
  observe-with-veto message callback
- Built-in, localizable right-click **Cut/Copy/Paste menu**
- A **message door**: classic edit-control messages sent to the control are forwarded
  to the RichEdit child

## Files

| File | Purpose |
|---|---|
| `CTextBox.bi` / `.inc` | The control. `CTextBox.bi` is the documented public header. |
| `clsDoubleBuffer.bi` / `.inc` | Flicker-free drawing helper (chrome painting) |
| `main.bas`, `frmMain.bi` / `.inc` | Demo / test harness (three instances) |

`CTextBox.bi` pulls in `AfxNova\AfxRichEdit.inc` itself (RichEdit definitions and
helpers). Include order:

```freebasic
#include once "clsDoubleBuffer.inc"
#include once "CTextBox.inc"
```

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
- **Programmatic setters are silent.** `SetText` / `ReplaceSel` / `Clear` (and a
  forwarded `WM_SETTEXT`) never fire the ChangeCallback; only user edits do — typing,
  cut/paste, undo. A host can safely call a setter from inside its own handler.
- **The message door.** The container forwards the classic edit-message range
  (`EM_GETSEL` .. `EM_GETIMESTATUS`) plus `WM_SETTEXT` / `WM_GETTEXT` /
  `WM_GETTEXTLENGTH` / `WM_CUT` / `WM_COPY` / `WM_PASTE` / `WM_CLEAR` / `WM_UNDO`, so
  `SendMessage( hBox, EM_SETSEL, 0, -1 )` works from a separate control or window.
  One implementation, two doors: message for a separate control, function
  (`CTextBox_SetSel` etc.) for an in-process host.
- **Single line, enforced.** No `ES_MULTILINE`; ENTER is swallowed (no newline, no
  beep) and reported through the EnterPressedCallback; a multiline paste keeps only
  what fits on one line.
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
| `TB_ChangeCallbackSub` | after a USER edit changed the text (programmatic = silent) |
| `TB_FocusCallbackSub` | RichEdit gained / lost keyboard focus (border already repainted) |
| `TB_EnterPressedCallbackSub` | ENTER pressed (the keypress itself is always swallowed) |
| `TB_MessageCallbackFunc` | key / mouse / focus / context-menu messages, before the control acts; return TRUE to suppress. Suppressing `WM_SETFOCUS` / `WM_KILLFOCUS` also suppresses the RichEdit's caret handling — only do that on purpose. |

## Context menu

Right-click shows Cut / Copy / Paste, filtered by state (read-only offers Copy only;
nothing eligible = no menu). Localize the labels with
`CTextBox_SetMenuText( hBox, "Ausschneiden", "Kopieren", "Einfügen" )`. The
MessageCallback sees `WM_CONTEXTMENU` first — return TRUE to suppress or replace it.

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
