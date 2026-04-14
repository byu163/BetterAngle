### BetterAngle Pro v4.27.231
- **Crosshair Thickness Clamp Fix**: Added additional validation in `setCrossThickness` to clamp values between 0.1px and 10.0px, ensuring the crosshair thickness can properly go below 1px as intended by the UI slider. This provides an extra safeguard beyond the profile loading logic.
- **Focus-Steal Transition Guard (replaces BlockInput)**: On dive/glide state transitions, BetterAngle now temporarily steals foreground focus to its own transparent HUD window (invisible to the user). Fortnite stops processing camera input for the animation duration (250ms glide→dive, 1000ms dive→glide), then focus is returned instantly. Keyboard input is unaffected throughout.
- **Admin Requirement Removed**: No longer requires Administrator privileges. The UAC manifest flag and elevation check have been removed since `BlockInput` is no longer used.
- feat: Restore Reset Crosshair button and backend API (v4.27.229)
- chore: auto-increment version for release

### BetterAngle Pro v4.27.230
- **Focus-Steal Transition Guard (replaces BlockInput)**: On dive/glide state transitions, BetterAngle now temporarily steals foreground focus to its own transparent HUD window (invisible to the user). Fortnite stops processing camera input for the animation duration (250ms glide→dive, 1000ms dive→glide), then focus is returned instantly. Keyboard input is unaffected throughout.
- **Admin Requirement Removed**: No longer requires Administrator privileges. The UAC manifest flag and elevation check have been removed since `BlockInput` is no longer used.

### BetterAngle Pro v4.27.229
- **Admin Elevation Enforced**: BetterAngle now requires and verifies Administrator privileges at startup. If not elevated, a clear error dialog is shown and the app does not launch. The CMake manifest now embeds `requireAdministrator` so Windows auto-prompts UAC on double-click.
- **Input Freeze on State Transition**: Added `BlockInput` system-level input suspension on diving/gliding FOV animation transitions. Glide→Dive suspends all mouse+keyboard for 250ms; Dive→Glide for 1000ms to prevent FOV animation noise from corrupting the angle accumulator.
- **Screen Capture Exclusion**: The HUD overlay window is now excluded from Win32 `BitBlt`-based screen capture via `SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE)`, preventing the UI from blocking the detector's pixel reads.
- **Color Selection Dot Removed**: Removed the red dot from the Stage 2 magnifier cursor tip that was obscuring the pixel being aimed at.

Generating release notes from commit range: v4.27.226..HEAD ### BetterAngle Pro v4.27.228
- fix: Crosshair sub-pixel thickness and reset button behavior

Generating release notes from commit range: v4.27.224..HEAD

### BetterAngle Pro v4.27.227
- **Crosshair Reset Button Fix**: Updated the reset crosshair button to properly turn the crosshair OFF and reset all crosshair settings to their default values (thickness: 1.0px, color: red, offsets: 0, pulse: off), matching fresh installation behavior.
- **Crosshair Sub-Pixel Thickness Fix**: Fixed crosshair thickness loading logic to properly support sub-pixel values down to 0.1px. Previously, extremely small values (<0.01) were reset to 1.0px; now they're clamped to the UI minimum of 0.1px, ensuring the crosshair can be made visibly thinner.

### BetterAngle Pro v4.27.226
- **Breathing Pulse Animation**: Implemented a sophisticated 3.0-second crosshair pulse cycle. Features a 1.2s smooth fade-out, a distinct **0.3s transparency pause**, and a 1.5s slow return to full opacity for a premium "alive" look.

### BetterAngle Pro v4.27.225
- **Breathing Pulse Animation**: Implemented a sophisticated 3.0-second crosshair pulse cycle. Features a 1.2s smooth fade-out, a distinct **0.3s transparency pause**, and a 1.5s slow return to full opacity for a premium "alive" look.

### BetterAngle Pro v4.27.224
- feat: Restore diveGlideMatch bindings and inject real-time GDI+ debug performance overlay
- **Crosshair Sub-Pixel Thickness Fix**: Fixed crosshair thickness loading logic to properly support sub-pixel values down to 0.1px. Previously, extremely small values (<0.01) were reset to 1.0px; now they're clamped to the UI minimum of 0.1px, ensuring the crosshair can be made visibly thinner.

### BetterAngle Pro v4.27.223
- **Trigger Calibration Restore**: Fixed the 'NaN%' render issue on the Dive-to-Glide Match Limit slider in the frontend. The Qt binding has been fully restored and directly hooks into the master C++ detector logic arrays, bypassing hardcoded magic bounds.
- **Advanced Diagnostic Sub-Overlay**: Introduced a new toggle in the DEBUG tab allowing users to project an analytics glass sub-panel beneath the master Win32 Angle HUD on-screen. This renders real-time engine tracking metrics including dynamic FPS, algorithmic core thread detection lag bounds, and precision hardware state synchronization (Fortnite context matching).


### BetterAngle Pro v4.27.221
- **Angle Wrap Formatting**: Fixed angle calculation UI logic so that the value dynamically formats and correctly resets precisely at the wrap point (359.9 to 0.0), ensuring the angle is faithfully bounded and doesn't display as 360.0.
- **Debug Diagnostics Tab**: Added a new 'DEBUG' tab to the QML Dashboard providing real-time hardware and operating system hooks to monitor Fortnite process detection, window focus, and mouse hidden states. This operates entirely independent of the core angle calculation logic.
