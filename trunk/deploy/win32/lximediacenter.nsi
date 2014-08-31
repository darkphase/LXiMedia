!include "MUI2.nsh"

Name "LeX-Interactive MediaCenter"
OutFile "LXiMediaCenterSetup.exe"
SetCompressor /SOLID lzma

InstallDir "$PROGRAMFILES\LXiMediaCenter"
RequestExecutionLevel admin

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
  
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
  
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "-Files" SecFiles
  ExecWait 'net stop "LXiMediaCenter Backend"'
  ExecWait 'cmd /C "taskkill /F /IM lximcbackend.exe || tskill lximcbackend.exe /A"'
  ExecWait 'cmd /C "taskkill /F /IM lximclauncher.exe || tskill lximclauncher.exe /A"'

  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File lximcbackend.exe
  File lximclauncher.exe
  File libvlc.dll
  File libvlccore.dll

  SetOutPath $INSTDIR\plugins
  File /r plugins\*

  nsisFirewall::AddAuthorizedApplication "$INSTDIR\lximcbackend.exe" "LeX-Interactive MediaCenter - Backend service"
  nsisFirewall::AddAuthorizedApplication "$INSTDIR\lximclauncher.exe" "LeX-Interactive MediaCenter - Launcher"

  ExecWait '"$INSTDIR\lximcbackend.exe" --install'
  CreateShortCut "$SMPROGRAMS\LXiMediaCenter.lnk" "$INSTDIR\lximclauncher.exe" "" "$INSTDIR\lximclauncher.exe" 0
  Exec '"$INSTDIR\lximclauncher.exe"'

  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter" "DisplayName" "LeX-Interactive MediaCenter"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"

  ExecWait '"$INSTDIR\lximcbackend.exe" --uninstall'
  ExecWait 'cmd /C "taskkill /F /IM lximcbackend.exe || tskill lximcbackend.exe /A"'
  ExecWait 'cmd /C "taskkill /F /IM lximclauncher.exe || tskill lximclauncher.exe /A"'

  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\lximcbackend.exe"
  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\lximclauncher.exe"

  Delete "$SMPROGRAMS\LXiMediaCenter.lnk"

  RMDir /r /REBOOTOK "$INSTDIR"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter"
SectionEnd
