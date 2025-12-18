#define AppVer GetVersionNumbersString('..\bin\gitmaster.exe')

[Setup]
AppName=GitMaster
AppVerName=GitMaster version {#AppVer}
AppPublisher=Boris Barbulovski
AppPublisherURL=https://github.com/bokic/gitmaster
AppVersion={#AppVer}
ArchitecturesInstallIn64BitMode=x64os
DefaultDirName={commonpf}\GitMaster
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

; Qt library
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\Qt6Core.dll"; DestDir: "{app}";
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\Qt6Gui.dll"; DestDir: "{app}";
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\Qt6Widgets.dll"; DestDir: "{app}";
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\Qt6Network.dll"; DestDir: "{app}";
Source: "{#%QT_PATH}\{#%QT_KIT}\bin\Qt6Svg.dll"; DestDir: "{app}";

; Qt platform
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms";

; Qt Style
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\styles\qmodernwindowsstyle.dll"; DestDir: "{app}\styles";

; Qt image icon engines
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\iconengines\qsvgicon.dll"; DestDir: "{app}\iconengines";

; Qt image formats
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\imageformats\qgif.dll"; DestDir: "{app}\imageformats";
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\imageformats\qico.dll"; DestDir: "{app}\imageformats";
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\imageformats\qjpeg.dll"; DestDir: "{app}\imageformats";
Source: "{#%QT_PATH}\{#%QT_KIT}\plugins\imageformats\qsvg.dll"; DestDir: "{app}\imageformats";

; App
Source: "..\bin\Release\gitmaster.exe"; DestDir: "{app}";

; libgit2 library
Source: "..\bin\git2.dll"; DestDir: "{app}";
