[Setup]
AppId={{DA1CF62B-D790-488B-8A5E-49A8EDBBF1E3}
AppName=BetterAngle Pro
AppVersion={#AppVer}
AppPublisher=Mahan
AppPublisherURL=https://github.com/MahanYTT/BetterAngle
AppSupportURL=https://github.com/MahanYTT/BetterAngle
AppUpdatesURL=https://github.com/MahanYTT/BetterAngle
DefaultDirName={autopf}\BetterAngle Pro
DisableProgramGroupPage=yes
PrivilegesRequired=admin
OutputDir=bin
OutputBaseFilename=BetterAngle_Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "build\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\BetterAngle Pro"; Filename: "{app}\BetterAngle.exe"
Name: "{autodesktop}\BetterAngle Pro"; Filename: "{app}\BetterAngle.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\BetterAngle.exe"; Description: "{cm:LaunchProgram,BetterAngle Pro}"; Flags: nowait postinstall skipifsilent
