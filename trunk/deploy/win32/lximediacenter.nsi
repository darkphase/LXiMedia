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
  File libgcc_s_dw2-1.dll
  File mingwm10.dll
  File QtCore4.dll
  File QtGui4.dll
  File QtNetwork4.dll
  File QtXml4.dll
  File LXiCore.dll
  File LXiServer.dll

  SetOutPath $INSTDIR\imageformats
  SetOverwrite ifnewer
  File imageformats\qjpeg4.dll
  File imageformats\qtiff4.dll

  ; For backwards compatibility with 0.2.x versions, can be removed in the future.
  RMDir /r "$%ALLUSERSPROFILE%\..\LocalService\Local Settings\Application Data\LXiMediaCenter"

  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter" "DisplayName" "LeX-Interactive MediaCenter"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Backend service" SecBackend
  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File QtScript4.dll
  File LXiMediaCenter.dll
  File LXiStream.dll
  File LXiStreamGui.dll
  File dcraw.exe
  File lximcbackend.exe

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
  File phonon4.dll
  File QtWebKit4.dll
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
