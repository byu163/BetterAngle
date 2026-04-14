### BetterAngle Pro v4.27.166
- **ROI Selection Hotfix**: Fixed critical bug where "Select prompt stage 1" (ROI selection) was completely broken after ESC key implementation. The issue was caused by a "SAFETY GUARD" timer that forcibly re-enabled window transparency every 16.7ms, preventing mouse interaction during ROI drag.
- **Window Transparency Logic**: Modified the safety guard to skip transparency enforcement when in ROI/Color selection mode, allowing proper mouse capture and drag functionality.
- **ROI Validation**: Added proper validation in WM_LBUTTONUP handler to check if ROI rectangle is valid (non-zero size) before transitioning to color selection stage.

### BetterAngle Pro v4.27.164
- **Default Value Fix**: Changed the default value of "Dive to glide threshold match limit %" slider from 5% to 9% as requested. Both glide and freefall thresholds now default to 9% for new installations.

### BetterAngle Pro v4.27.163
- **Maintenance Release**: Version bump for deployment pipeline synchronization and clean build environment.

### BetterAngle Pro v4.27.162
- **ROI Selection Fix**: Added proper mouse capture (`SetCapture`/`ReleaseCapture`) during ROI drag operations to prevent losing mouse messages when dragging outside the window.
- **ESC Key Support**: Pressing ESC now cancels ROI/color selection and returns to normal mode, with proper cleanup of mouse capture and screen snapshot.
- **Decimal UI Movement Fix**: Resolved bug where decimal UI couldn't be moved even when Fortnite was not in focus by refining drag logic condition ordering.

### BetterAngle Pro v4.27.165
- **Progressive Pulse Animation**: Re-engineered the crosshair pulse to use 1.2-second smooth fades. This creates a 'glide' transition into and out of transparency.
- **Micro-Hold Logic**: The crosshair now stays fully transparent for exactly **0.3 seconds**, providing a brief, rhythmic visual pause before fading back in.

### BetterAngle Pro v4.27.161
- **Final Branding Restored**: Re-integrated the characteristic **">" symbol** into the center of the master cyan orb assets.
- **Deep Shave & Luma Scrub**: Applied a refined 6% margin shave and darkness-threshold filter to the new branding, ensuring absolute transparency with zero black artifacts.
- **Universal Icon Refresh (v161)**: Implemented a cache-busting transition to `BetterAngle_v161.ico` to force Windows to update all desktop and start menu shortcuts immediately.

### BetterAngle Pro v4.27.157
- **UI Nomenclature Update**: Renamed the calibration slider in the General tab to **"Dive to glide threshold match limit %"** per user feedback.
- **Improved Layout**: Cleaned up redundant descriptive text in the General tab for a higher-precision dashboard layout.

### BetterAngle Pro v4.27.154
- **Branding Transparency Hardened**: Programmatically scrubbed the AI-generated checkerboard patterns from the master logo assets, ensuring 100% pure transparency for the cyan orb.
- **Icon Cache-Busting**: Implemented a transition to `BetterAngle_v153.ico` (v154 build). This forces the Windows shell to ignore its old cached icons and display the new high-fidelity assets across the desktop, taskbar, and Control Panel immediately.
- **Universal Asset Injection**: Updated the RC resource script and the Inno Setup configuration to ensure the uninstaller and all program shortcuts use the latest refined branding.
