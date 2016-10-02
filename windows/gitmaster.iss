[Setup]
AppName=GitMaster
AppVerName=GitMaster version 0.1
AppPublisher=Boris Barbulovski
AppPublisherURL=https://github.com/bokic/gitmaster
AppVersion=0.1
DefaultDirName={pf}\GitMaster
DefaultGroupName=GitMaster
UninstallDisplayIcon={app}\GitMaster.exe
Compression=lzma
SolidCompression=yes
OutputBaseFilename=gitmaster_0.1
OutputDir=.
VersionInfoVersion=0.1
VersionInfoDescription=Git GUI client(Qt 5.7)


[Dirs]
Name: "{app}";
Name: "{app}\platforms";
Name: "{app}\imageformats";
Name: "{app}\iconengines";

[Files]
; MinGW runtime
Source: "C:\Qt\5.7\mingw53_32\bin\libwinpthread-1.dll"; DestDir: "{app}";
Source: "C:\Qt\5.7\mingw53_32\bin\libgcc_s_dw2-1.dll"; DestDir: "{app}";
Source: "C:\Qt\5.7\mingw53_32\bin\libstdc++-6.dll"; DestDir: "{app}";

; Qt library
Source: "C:\Qt\5.7\mingw53_32\bin\Qt5Core.dll"; DestDir: "{app}";
Source: "C:\Qt\5.7\mingw53_32\bin\Qt5Gui.dll"; DestDir: "{app}";
Source: "C:\Qt\5.7\mingw53_32\bin\Qt5Widgets.dll"; DestDir: "{app}";
Source: "C:\Qt\5.7\mingw53_32\bin\Qt5Network.dll"; DestDir: "{app}";
Source: "C:\Qt\5.7\mingw53_32\bin\Qt5Svg.dll"; DestDir: "{app}";

; Qt platform
Source: "C:\Qt\5.7\mingw53_32\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms";

; Qt image icon engines
Source: "C:\Qt\5.7\mingw53_32\plugins\iconengines\qsvgicon.dll"; DestDir: "{app}\iconengines";

; Qt image formats
Source: "C:\Qt\5.7\mingw53_32\plugins\imageformats\qgif.dll"; DestDir: "{app}\imageformats";
Source: "C:\Qt\5.7\mingw53_32\plugins\imageformats\qico.dll"; DestDir: "{app}\imageformats";
Source: "C:\Qt\5.7\mingw53_32\plugins\imageformats\qjpeg.dll"; DestDir: "{app}\imageformats";
Source: "C:\Qt\5.7\mingw53_32\plugins\imageformats\qsvg.dll"; DestDir: "{app}\imageformats";
Source: "C:\Qt\5.7\mingw53_32\plugins\imageformats\qtiff.dll"; DestDir: "{app}\imageformats";

; App
Source: "..\bin\gitmaster.exe"; DestDir: "{app}";

; git library
Source: "..\bin\libgit2.dll"; DestDir: "{app}";