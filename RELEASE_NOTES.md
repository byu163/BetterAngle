### BetterAngle Pro v4.10.7: "Hotfix: JSON Registry Exception Crash"
v4.10.7: Fixed a fatal initialization runtime exception (`std::invalid_argument`) triggered on startup when parsing standard config spaces. The data parser previously choked on trailing colons during memory loading (`settings.json`), causing an immediate UI crash immediately after the startup splash sequence. The JSON engine now securely wraps all metadata injection points in proper `try-catch` structures with correct spatial bounds, ensuring total startup stability regardless of config formatting.

### BetterAngle Pro v4.10.6: "Ergonomic Calibration & Window Tuning"
v4.10.6: Substantially overhauled the standalone `BetterAngleConfig.exe` sequence. The calibration UI window bounds were scaled down significantly to minimize screen obstruction during active tracking. Furthermore, the progression hotkey for completing the live 120-degree spin was dynamically unbound from `ENTER` and explicitly re-routed natively to `F10`, preventing players from taking their hands off their active gameplay stance while measuring FOV boundaries. Lastly, fixed a historical trace buffer offset error causing negative/inverted coordinate conversions.

### BetterAngle Pro v4.10.5: "True Optical FOV Calibration Layer"
v4.10.5: Completely restructured the engine overriding logic that was artificially snapping the active prompt ROI box (Red/Green state) based on raw angle thresholds instead of genuine algorithm target color matching. Added a new native "Prompt Detection Threshold" slider/modifier within the Control Panel Simulation tab, allowing absolute micro-calibration of the exact spatial ratio required for the software to dynamically shift between Normal and Skydiving sensitivities. Re-wired these values natively into the local `settings.json` registry to permanently persist custom sensitivities alongside native profiles.

### BetterAngle Pro v4.10.4: "Dynamic Cloud Release Logs"
v4.10.4: Native implementation mapped to `msbuild.yml` that seamlessly parses and extracts the latest `RELEASE_NOTES.md` changelog array, injecting it directly into the GitHub Actions active context body at compile time instead of printing static generic build text.

### BetterAngle Pro v4.10.3: "Bugfix: Angle Trajectory Anchor & Config Sub-linking"
v4.10.3: Re-anchored the core tracking geometry (`m_baseAngle`) whenever Sensitivity natively changes state (i.e. changing into Skydiving Mode). Prevents visual target angles from snapping. Hard-linked the remaining missing `State.cpp` and `Logic.cpp` footprint into `BetterAngleConfig` to satisfy GitHub Actions' aggressive strict execution limits on the Input thread wrapper. Repaired the custom Zero Angle keybind which wasn't pushing memory to the calculation core layer. 

### BetterAngle Pro v4.10.2: "Hotfix: Secondary Executable Linker Isolation"
v4.10.2: Resolved a GitHub Actions build abort (`LNK2019`) triggered when the CI system blindly compiled the global `ControlPanel.cpp` UI layer into the lightweight `BetterAngleConfig.exe` footprint. The secondary executable now strict-links only its necessary core logic components (`Input.cpp`, `Profile.cpp`), permanently isolating it from the main software namespace wrapper.

### BetterAngle Pro v4.10.1: "Hotfix: Compiler Scope Isolation"
v4.10.1: Native hotfix applied to patch undeclared `g_currentProfile` scoping limits for the MSVC strict compiler inside `ControlPanel.cpp`, and isolated a `wchar_t` string conversion loop that was tripping C4244 strict errors on Github Actions. Build environment stabilized.

### BetterAngle Pro v4.10.0: "The Calibration Suite & Cloud Command Center"
v4.10.0: Fully introduced `BetterAngleConfig.exe`, a standalone HWND_MESSAGE backed wizard to guide users through 0-120 degree tracking, typing Sens & DPI natively, and outputting standardized JSONs. Completely rewired Control Panel to support dynamic Cloud-Profile syncing directly from the GitHub Repo directory. Completely overhauled Keybinds to be interactively customizable inside the Command Center and persisted to local metadata automatically.

### BetterAngle Pro v4.9.41: "Unbreakable Input Telemetry & Edge-Case Bootloader"
v4.9.41: Added direct dx_sum output to Control Panel diagnostics to track exact mouse buffer intake. Added hardware RawInput VM/Hypervisor support (`MOUSE_MOVE_ABSOLUTE` processing for Parallels/VMware). Engineered an auto-fallback scale generator and profile factory to prevent zero-scale divides when launching externally.

### BetterAngle Pro v4.9.40: "Message Pump & Corrupt JSON Purge"
v4.9.40: Re-routed RawInput via a dedicated invisible HWND_MESSAGE window to bypass Windows 11 transparent layer dropping blocks. Implemented pre-parse comma swapping to restore already-corrupted profiles saving 0,0 float metrics.

### BetterAngle Pro v4.9.39: "Angle Calculation & Simulation Overhaul"
v4.9.39: Critical fix for Angle remaining 0 (RawInput memory padding bug resolved). Fixed locale-dependent JSON parsing that broke float scales on European OS settings. Debug Mode now perfectly overrides all Fortnite gating including cursor lock.

### BetterAngle Pro v4.9.38: "Zero-Copy Optics & Advanced Debug Engine"
v4.9.38: Fixed memory-lock color match bugs via CreateDIBSection. Fully functional Angle Logic globally active with Fortnite override bypass in new Debug tab.

### BetterAngle Pro v4.9.37: "Fixed angle and color picker bug"
v4.9.31: Fixed angle and color picker bug.

### BetterAngle Pro v4.9.30: "Fixing auto update"
v4.9.30: Fixed auto update.

### BetterAngle Pro v4.9.24: "Fixed angle calculation & updated version"
v4.9.24: Fixed angle calculation bug and updated version.

### BetterAngle Pro v4.9.23: "Fixed some bugs"
v4.9.23: Bug fixes.

### BetterAngle Pro v4.9.20: "Removed old stuff"
v4.9.20: Removed old stuff.

### BetterAngle Pro v4.9.16: "Added Set Zero Functionality (NOT TESTED YET)"
v4.9.16: Added a "Set Zero" button but can't compile so someone please test it.
### BetterAngle Pro v4.9.15: "Release Workflow Fix"
v4.9.15: Fixed the GitHub Actions release flow so tag pushes create a GitHub Release and upload BetterAngle.exe automatically.

### BetterAngle Pro v4.9.14: "Zero-Touch Installer & Identity Fix"
v4.9.14: Implemented Zero-Touch Auto-Installer and Relauncher. Fixed Git contribution identity. Improved full-screen coordinate accuracy.

### BetterAngle Pro v4.9.13: "Snapshot Logic & Global Workspace"
💎 v4.9.13: Implemented Full-Screen Snapshot selection. Screen now dims during selection for clarity. Fixed color picking to bypass dark overlay via memory-bitmap retrieval.
💎 **[NEW] Dynamic Live ROI Visualizer:** Persistent visual box that dynamically shifts from GREEN (Gliding) to RED (Diving).
🤖 **[NEW] 60FPS UI Engine:** Control panel rendering boosted to 60 FPS (16ms) for buttery-smooth window interaction.
🔴 **[NEW] Professional Tabbed Dashboard:** Separated General/Binds and Software/Updates for a cleaner, modern workspace.
🔍 **[IMPROVED] Cinematic Calibration Tool:** Full-screen cinematic dark dimming when pressing Ctrl+R to focus entirely on the selection area.
📌 **[FIXED] ROI Visibility Toggle:** Added F9 to instantly toggle the visual scanning box on or off.
