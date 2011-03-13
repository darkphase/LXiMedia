!include "MUI2.nsh"

Name "LeX-Interactive MediaCenter"
OutFile "..\..\bin\LXiMediaCenterSetup.exe"
SetCompressor /SOLID lzma

InstallDir "$PROGRAMFILES\LXiMediaCenter"
RequestExecutionLevel admin

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_LICENSE "..\..\COPYING"
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
  ExecWait 'taskkill /F /T /IM lximcbackend.exe'
  ExecWait 'taskkill /F /IM lximctrayicon.exe'
SectionEnd

Section "-Shared Files" SecShared
  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File ..\..\COPYING
  File ..\..\VERSION
  File ..\..\README
  File ..\..\bin\libgcc_s_dw2-1.dll
  File ..\..\bin\mingwm10.dll
  File ..\..\bin\Qt*.dll
  File ..\..\bin\LXiMediaCenter?.dll
  File ..\..\bin\LXiServer?.dll
  File ..\..\bin\LXiStream?.dll
  File ..\..\bin\LXiStreamGui?.dll

  SetOutPath $INSTDIR\imageformats
  SetOverwrite ifnewer
  File ..\..\bin\imageformats\*.dll

  SetOutPath $INSTDIR\sqldrivers
  SetOverwrite ifnewer
  File ..\..\bin\sqldrivers\*.dll

  SetOutPath $INSTDIR\liblxistream
  SetOverwrite ifnewer
  File ..\..\bin\liblxistream\*.dll

  ; For backwards compatibility with 0.1.x versions, can be removed in the future.
  Rename "$%ALLUSERSPROFILE%\Application Data\lximc" "$%ALLUSERSPROFILE%\Application Data\LXiMediaCenter"

  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter" "DisplayName" "LeX-Interactive MediaCenter"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Backend service" SecBackend
  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File ..\..\bin\lximcbackend.exe

  SetOutPath $INSTDIR\lximediacenter
  SetOverwrite ifnewer
  File ..\..\bin\lximediacenter\*.dll

  nsisFirewall::AddAuthorizedApplication "$INSTDIR\lximcbackend.exe" "LeX-Interactive MediaCenter - Backend service"

  ExecWait '"$INSTDIR\lximcbackend.exe" --install'
  ExecWait 'net start "LXiMediaCenter Backend"'
SectionEnd

Section "Systemtray icon" SecTrayIcon
  ExecWait 'taskkill /F /IM lximctrayicon.exe'

  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File ..\..\bin\lximctrayicon.exe

  nsisFirewall::AddAuthorizedApplication "$INSTDIR\lximctrayicon.exe" "LeX-Interactive MediaCenter - Tray icon"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "LXiMediaCenterTrayIcon" '"$INSTDIR\lximctrayicon.exe"'
  Exec '"$INSTDIR\lximctrayicon.exe"'
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"

  ExecWait 'net stop "LXiMediaCenter Backend"'
  ExecWait 'taskkill /F /T /IM lximcbackend.exe'
  ExecWait 'taskkill /F /IM lximctrayicon.exe'

  ExecWait '"$INSTDIR\lximcbackend.exe" --uninstall'
  
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "LXiMediaCenterTrayIcon"

  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\lximcbackend.exe"
  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\lximctrayicon.exe"

  RMDir /r /REBOOTOK "$INSTDIR"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter"
  
  MessageBox MB_YESNO "Would you like to keep the existing settings and databases? Note that building a new database might take several hours." /SD IDYES IDYES noRemoveDb
    RMDir /r "$%ALLUSERSPROFILE%\Application Data\LXiMediaCenter"
  noRemoveDb:
  
SectionEnd
