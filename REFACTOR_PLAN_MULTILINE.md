# CTextBox multiline mode — refactor plan

Goal: allow the embedded RichEdit50W child to be created as **multiline**, so tiko's Output
form can replace its two raw richedits (Notes, Compiler Log) with CTextBox instances.

Decisions taken (2026-07-20, with Paul):

- **Multiline is a creation-time option** — `ES_MULTILINE` cannot be toggled on a live
  window, so it is an optional parameter on `CTextBox_Create` (default false; the three
  existing single-line call sites in tiko compile unchanged).
- **Scrollbar**: tiko's Output form keeps its custom-painted vscroll panel *for now*, but
  the CTextBox notification contract is shaped for **CVScrollBar**, which will replace that
  panel later. Units are LINES in both directions:
  `ScrollChangedCallback` fires → host calls `CTextBox_GetVScrollInfo(total, page, pos)` →
  pushes straight into `CVScrollBar_SetRange(total, page, pos)`; CVScrollBar's
  `ScrollCallback(newPos)` → `CTextBox_ScrollToLine(newPos)`.
- **Tab in multiline inserts a tab character** (matches current Notes behavior; the
  MessageCallback veto still lets a host repurpose it).
- **Enter in multiline inserts a newline**; `EnterPressedCallback` never fires in multiline.
- **Context menu gains a built-in, localizable Select All** (4th item, both modes),
  set via an optional 4th parameter on `CTextBox_SetMenuText`.
- **Numeric mode is single-line only** (`SetNumericMode` no-ops on a multiline control).
- **Dev flow**: Phase A here in the standalone repo (test harness), Phase B copies the
  finished `CTextBox.bi/.inc` into `tiko\src` and converts frmOutput.

## Phase A — the control (this repo)

Type `CTEXTBOX` additions:
- `bMultiline as boolean = false` — fixed at creation.
- `wszMenuSelectAll as DWSTRING` — 4th built-in menu label (default "Select All").
- `ScrollChangedCallback as TXT_ScrollChangedCallbackSub` — relayed from the RichEdit's
  `EN_UPDATE` (mask `ENM_UPDATE`, multiline only). Deliberately **not** gated by
  `bInternalChange`: a programmatic SetText changes the line count and a scrollbar must
  hear about it.

API:
- `CTextBox_Create( hWndParent, CtrlID, byval bMultiline as boolean = false )`
  Multiline child styles: `ES_MULTILINE or ES_AUTOVSCROLL or ES_WANTRETURN`, **no**
  `ES_AUTOHSCROLL` (word wrap). No scrollbar styles — a themed scrollbar is external.
- `CTextBox_GetMultiline( hTextBox ) as boolean`
- `CTextBox_GetVScrollInfo( hTextBox, byref nTotalLines, byref nLinesPerPage, byref nFirstVisibleLine )`
  — `EM_GETLINECOUNT` / `EM_GETFIRSTVISIBLELINE`; lines-per-page = formatting-rect height \
  line height from the text font's TEXTMETRIC (no fudge factors — trace-verified).
- `CTextBox_ScrollToLine( hTextBox, nLine )` — delta `EM_LINESCROLL` from current first line.
- `CTextBox_SetScrollChangedCallback( hTextBox, usersub )`
- `CTextBox_SetMenuText( hTextBox, Cut, Copy, Paste, SelectAll = "" )` — empty keeps the
  current Select All label, so 3-argument callers are unaffected.

Behavior, gated on `bMultiline`:
- Child subclass: `WM_CHAR` 13 / `WM_KEYDOWN` VK_RETURN pass through; VK_TAB / char 9 pass
  through (tab character); ESC still swallowed in both modes (beep suppression).
- Cue banner: drawn top-left with `DT_WORDBREAK` (no `DT_SINGLELINE`, no vertical centering).
- `CTextBox_CenterSingleLineText` early-exits on multiline (no `EM_SETRECT`; the RichEdit's
  default formatting rect tracks the client). Margins still applied via `EM_SETMARGINS`.
- Container `WM_COMMAND`: `EN_UPDATE` → `ScrollChangedCallback` (ungated); `EN_CHANGE`
  relay unchanged (gated, user-only).
- Context menu: Select All appended (separator before it when other items exist), enabled
  whenever the control has text; `EM_SETSEL 0,-1` on pick.

Harness (`frmMain`): two new boxes — an editable multiline with cue banner +
Change/ScrollChanged callbacks, and a read-only multiline preloaded with 40 numbered lines.
Programmatic asserts printed at startup (env-gate `CTEXTBOX_SMOKE=1` auto-closes for an
unattended run): `GetVScrollInfo` numbers vs hand-counted expectations, and
`ScrollToLine(10)` → first visible line reads back 10.

## Phase B — tiko frmOutput conversion (`tiko\src`)

1. Copy finished `CTextBox.bi/.inc` over (copies are content-identical baseline).
2. `frmOutput_Show`: both `AddControl("RICHEDIT")` blocks → `CTextBox_Create(…, true)`.
   Log: `SetReadOnly(true)`. Notes: `SetChangeCallback` (saves gApp notes — replaces the
   `EN_CHANGE` branch in `frmOutput_OnCommand`). Both: `SetBorderWidth(0)`,
   `SetMargins(ScaleX(20), 0)` (replaces the `EM_SETRECT` +20px hacks),
   `SetScrollChangedCallback` → thumb recalc (replaces the `EN_UPDATE` branch),
   `SetMenuText` with localized labels incl. Select All. Drop the `EM_SHOWSCROLLBAR` calls.
3. Delete `frmOutput_RichEdit_SubclassProc` + `MSG_USER_RICHEDIT_SELECTALL` handling.
4. `frmOutput_SetControlColors`: `EM_SETCHARFORMAT`/`EM_SETBKGNDCOLOR` pairs →
   `CTextBox_SetForeColor`/`SetBackColor`.
5. Scroll glue: `frmOutputVScroll_calcVThumbRect` → `CTextBox_GetVScrollInfo`; delete
   `GetVisibleLineCount` (its `EM_GETCHARFORMAT` would NOT pass the message door — outside
   the forwarded `EM_GETSEL..EM_GETIMESTATUS` range). Scroll writes → `CTextBox_ScrollToLine`
   so the code is already CVScrollBar-shaped. (`EM_LINESCROLL` etc. would keep working
   through the door; the rewrite is for the later swap.)
6. Focus routing, `frmMainEdit.inc` lines ~30/55/79/165: wrap the Notes/Log
   `GetDlgItem(...)` in `CTextBox_GetRichEditHandle(...)` — same pattern the adjacent
   Find/Replace cases already use (focus sits on the child).
7. Untouched (verified safe — text get/set goes through the forwarded message door):
   `frmMain.inc:40`, `frmMainFile.inc:414`, `frmMainProject.inc:86`,
   `modCompileErrors.inc:303`, `clsConfig.inc:447`, frmOutput's reset/show functions.

## Phase C — later, separate task

Swap the Output form's custom vscroll panel for CVScrollBar: create it, wire
`ScrollChangedCallback → GetVScrollInfo → CVScrollBar_SetRange` and
`ScrollCallback → ScrollToLine`; delete `frmOutputVScroll_*` + `OUTPUT_VSCROLL_TYPE`.

## Verification

- `fbc64.exe -i "C:\dev" main.bas` (harness) and tiko's build — clean, zero new warnings.
- Geometry asserted by trace, never by eye (GetVScrollInfo vs hand counts, two heights).
- Interactive pass (author): typing/Enter/Tab in Notes, wheel + thumb drag, readonly Log,
  context menu incl. Select All, Edit-menu routing, theme switch, notes persistence across
  project switch. Single-line regression: Find/Replace/Search Symbol still behave.
- State explicitly what was NOT verified at the end of each phase.

## Status

- Phase A: **done** (2026-07-20) — build clean, zero warnings
  (`fbc64.exe -i "C:\dev" main.bas`). Startup asserts all pass in the smoke run
  (`CTEXTBOX_SMOKE=1 main.exe`): total lines 40, first visible 0, lines-per-page 6
  (110px box, Segoe UI 10, no fudge factor), `ScrollToLine(10)` reads back 10,
  numeric mode refused on multiline, `GetMultiline` correct for both modes, and the
  message door still answers on a single-line box.
  Notes: (1) `EN_UPDATE` (→ ScrollChangedCallback) did NOT fire for a programmatic
  SetText into a still-0x0/hidden control — it fired once the control was sized and
  painted. Hosts filling a hidden control must refresh their scrollbar on show (tiko's
  frmOutput already does). Documented in README. (2) Mixed `DWSTRING &` concat chains
  are ambiguous-overload errors — harness builds long text in a plain `string` first.
  Wheel-scroll fix (same day, after Paul's report -- real wheel did not scroll):
  msftedit's own WM_MOUSEWHEEL handling never scrolled, even shown + focused +
  WS_VSCROLL + real coords (EM_SCROLL worked throughout, isolating the wheel path).
  Fix: the subclass implements wheel scrolling itself -- accumulated signed delta ->
  EM_LINESCROLL, honoring SPI_GETWHEELSCROLLLINES incl. page-per-notch -- and fires
  ScrollChangedCallback (user interaction notifies; programmatic ScrollToLine stays
  silent). Asserted: one notch -> first visible = 3 with the callback firing live.
  NOT verified (author's interactive pass): typing/ENTER/TAB feel, cue-banner wrap
  pixels, real-hardware wheel + live scroll events, context menu incl. Select All,
  read-only behavior, focus border, single-line regressions by hand, DPI > 100%.
- Phase B: **done** (2026-07-20) — CTextBox.bi/.inc synced into tiko\src; frmOutput
  converted per plan (both richedits -> multiline CTextBox; RichEdit subclass proc,
  MSG_USER_RICHEDIT_SELECTALL case, EN_UPDATE/EN_CHANGE WM_COMMAND handlers,
  GetVisibleLineCount + "-4" fudge, and the EM_SETRECT indent hacks all deleted;
  thumb math on GetVScrollInfo, scroll writes on ScrollToLine); frmMainEdit focus
  tests wrapped in CTextBox_GetRichEditHandle. Merged --no-ff into tiko's
  `development` (a48d70f). Build clean via _compile.bat, zero warnings; app
  launches and runs (5s smoke, main window up, then killed).
  NOT verified (author's interactive pass): Notes typing/saving across project
  switches, log fill on compile, wheel + custom thumb drag/page-click, context
  menu, Edit-menu routing, theme switch, panel resize/minimize, DPI > 100%.
- Phase C: **done** (2026-07-20) — frmOutput's CPanelWindow scrollbar (WndProc,
  calcVThumbRect, OUTPUT_VSCROLL_TYPE/gOutputVScroll) deleted, replaced by
  CVScrollBar with the planned three-piece glue (ScrollChangedCallback ->
  GetVScrollInfo -> SetRange, gated to the visible textbox and re-pushed on tab
  switch; ScrollCallback -> ScrollToLine). Paint callback preserves the old
  flat-track/divider visuals and reads theme colors live. PositionWindows computes
  the ScaleX(12) width instead of querying the (0x0-created) control. Net -127
  lines. Merged --no-ff into tiko `development` (f6c50c8). Build clean, zero
  warnings; 5s launch smoke ok.
  NOT verified (author's interactive pass): thumb drag, track-click paging +
  auto-repeat, hover state, wheel/thumb staying in sync, tab switches between
  Log/Notes, theme switch, panel resize/minimize, DPI > 100%.
  Bonus CVScrollBar gains over the old panel: track-click auto-repeat, hover
  tracking, capture-loss cancel.
