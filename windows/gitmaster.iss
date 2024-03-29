#define AppVer GetFileVersion('..\bin\gitmaster.exe')

[Setup]
AppName=GitMaster
AppVerName=GitMaster version {#AppVer}
AppPublisher=Boris Barbulovski
AppPublisherURL=https://github.com/bokic/gitmaster
AppVersion={#AppVer}
ArchitecturesInstallIn64BitMode=x64
DefaultDirName={pf}\GitMaster
DefaultGroupName=GitMaster
UninstallDisplayIcon={app}\GitMaster.exe
Compression=lzma
SolidCompression=yes
OutputBaseFilename={#ReadIni(SourcePath + "installer.ini", "installer", "target_name ")}
OutputDir=.
VersionInfoVersion={#AppVer}
VersionInfoDescription={#ReadIni(SourcePath + "installer.ini", "installer", "description")}


[Dirs]
Name: "{app}";
Name: "{app}\platforms";
Name: "{app}\imageformats";
Name: "{app}\iconengines";

[Files]
; MinGW runtime
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\libwinpthread-1.dll"; DestDir: "{app}";
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\libgcc_s_seh-1.dll"; DestDir: "{app}";
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\libstdc++-6.dll"; DestDir: "{app}";

; OpenSSL
Source: "{#%OPENSSL_PATH}\bin\libcrypto-1_1-x64.dll"; DestDir: "{app}";
Source: "{#%OPENSSL_PATH}\bin\libssl-1_1-x64.dll"; DestDir: "{app}";

; Qt library
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\Qt6Core.dll"; DestDir: "{app}";
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\Qt6Gui.dll"; DestDir: "{app}";
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\Qt6Widgets.dll"; DestDir: "{app}";
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\Qt6Network.dll"; DestDir: "{app}";
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\Qt6Svg.dll"; DestDir: "{app}";

; Qt platform
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms";

; Qt image icon engines
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\iconengines\qsvgicon.dll"; DestDir: "{app}\iconengines";

; Qt image formats
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\imageformats\qgif.dll"; DestDir: "{app}\imageformats";
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\imageformats\qico.dll"; DestDir: "{app}\imageformats";
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\imageformats\qjpeg.dll"; DestDir: "{app}\imageformats";
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\imageformats\qsvg.dll"; DestDir: "{app}\imageformats";

; App
Source: "..\bin\gitmaster.exe"; DestDir: "{app}";

; git library
Source: "..\bin\libgit2.dll"; DestDir: "{app}";
