!include "MUI2.nsh"

Name "LeX-Interactive MediaServer"
OutFile "LXiMediaServerSetup.exe"
SetCompressor /SOLID lzma

InstallDir "$PROGRAMFILES\LXiMediaServer"
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
  ExecWait 'cmd /C "taskkill /F /IM lximediaserver.exe || tskill lximediaserver.exe /A"'

  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File lximediaserver.exe
  File *.dll

  SetOutPath $INSTDIR\plugins
  File /r plugins\*

  nsisFirewall::AddAuthorizedApplication "$INSTDIR\lximediaserver.exe" "LeX-Interactive MediaServer"

  CreateShortCut "$SMPROGRAMS\LXiMediaServer.lnk" "$INSTDIR\lximediaserver.exe" "" "$INSTDIR\lximediaserver.exe" 0
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "LXiMediaServer" '"$INSTDIR\lximediaserver.exe" --run'

  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaServer" "DisplayName" "LeX-Interactive MediaServer"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaServer" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"

  ExecWait 'cmd /C "taskkill /F /IM lximediaserver.exe || tskill lximediaserver.exe /A"'
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "LXiMediaServer"

  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\lximediaserver.exe"

  Delete "$SMPROGRAMS\LXiMediaServer.lnk"

  RMDir /r /REBOOTOK "$INSTDIR"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaServer"
SectionEnd
