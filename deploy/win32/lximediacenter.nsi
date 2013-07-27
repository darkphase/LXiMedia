!include "MUI2.nsh"

Name "LeX-Interactive MediaCenter"
OutFile "LXiMediaCenterSetup.exe"
SetCompressor /SOLID lzma

InstallDir "$PROGRAMFILES\LXiMediaCenter"
RequestExecutionLevel admin

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_LICENSE "COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
  
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
  
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "-Stop Apps" SecStop
  ExecWait 'net stop "LXiMediaCenter Backend"'
  ExecWait 'cmd /C "taskkill /F /IM lximcbackend.exe || tskill lximcbackend.exe /A"'
  ExecWait 'cmd /C "taskkill /F /IM lximcfrontend.exe || tskill lximcfrontend.exe /A"'
SectionEnd

Section "-Shared Files" SecShared
  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File COPYING
  File VERSION
  File README
  File D3DCompiler_43.dll
  File icudt49.dll
  File icuin49.dll
  File icuuc49.dll
  File libEGL.dll
  File libGLESv2.dll
  File Qt5Concurrent.dll
  File Qt5Core.dll
  File Qt5Gui.dll
  File Qt5Network.dll
  File Qt5OpenGL.dll
  File Qt5Widgets.dll
  File Qt5Xml.dll
  File LXiCore.dll
  File LXiServer.dll

  ; Only needed when compiled with MinGW
  File /nonfatal libgcc_s_sjlj-1.dll
  File /nonfatal libstdc++-6.dll
  File /nonfatal libwinpthread-1.dll

  SetOutPath $INSTDIR\imageformats
  SetOverwrite ifnewer
  File imageformats\qjpeg.dll
  File imageformats\qtiff.dll

  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter" "DisplayName" "LeX-Interactive MediaCenter"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Backend service" SecBackend
  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File LXiMediaCenter.dll
  File LXiStream.dll
  File LXiStreamGui.dll
  File lximcbackend.exe

  File /nonfatal dcraw.exe

  SetOutPath $INSTDIR\lximedia
  SetOverwrite ifnewer
  File lximedia\lxistream_*.dll

  nsisFirewall::AddAuthorizedApplication "$INSTDIR\lximcbackend.exe" "LeX-Interactive MediaCenter - Backend service"

  SetOutPath $INSTDIR\lximedia
  SetOverwrite ifnewer
  File lximedia\lximediacenter_*.dll

  ExecWait '"$INSTDIR\lximcbackend.exe" --install'
  ExecWait 'net start "LXiMediaCenter Backend"'
SectionEnd

Section "Frontend application" SecFrontend
  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File Qt5Multimedia.dll
  File Qt5MultimediaWidgets.dll
  File Qt5PrintSupport.dll
  File Qt5Qml.dll
  File Qt5Quick.dll
  File Qt5Sql.dll
  File Qt5WebKit.dll
  File Qt5WebKitWidgets.dll
  File lximcfrontend.exe

  CreateShortCut "$SMPROGRAMS\LXiMediaCenter Frontend.lnk" "$INSTDIR\lximcfrontend.exe" "" "$INSTDIR\lximcfrontend.exe" 0

  nsisFirewall::AddAuthorizedApplication "$INSTDIR\lximcfrontend.exe" "LeX-Interactive MediaCenter - Frontend application"

  Exec '"$INSTDIR\lximcfrontend.exe" --welcome'
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"

  ExecWait 'net stop "LXiMediaCenter Backend"'
  ExecWait 'cmd /C "taskkill /F /IM lximcbackend.exe || tskill lximcbackend.exe /A"'
  ExecWait 'cmd /C "taskkill /F /IM lximcfrontend.exe || tskill lximcfrontend.exe /A"'

  ExecWait '"$INSTDIR\lximcbackend.exe" --uninstall'

  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\lximcbackend.exe"
  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\lximcfrontend.exe"

  Delete "$SMPROGRAMS\LXiMediaCenter Frontend.lnk"

  RMDir /r /REBOOTOK "$INSTDIR"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter"
SectionEnd
