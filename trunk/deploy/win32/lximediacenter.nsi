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
  ExecWait 'tskill lximcbackend /A'
  ExecWait 'tskill lximctrayicon /A'
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
  File QtSql4.dll
  File QtXml4.dll
  File LXiCore.dll
  File LXiMediaCenter.dll
  File LXiServer.dll
  File LXiStream.dll
  File LXiStreamGui.dll

  SetOutPath $INSTDIR\imageformats
  SetOverwrite ifnewer
  File imageformats\qjpeg4.dll
  File imageformats\qtiff4.dll

  SetOutPath $INSTDIR\sqldrivers
  SetOverwrite ifnewer
  File sqldrivers\qsqlite4.dll

  SetOutPath $INSTDIR\lximedia
  SetOverwrite ifnewer
  File lximedia\lxistream_*.dll

  ; For backwards compatibility with 0.1.x versions, can be removed in the future.
  RMDir /r /REBOOTOK "$%ALLUSERSPROFILE%\Application Data\lximc"
  RMDir /r /REBOOTOK "$INSTDIR\liblxistream"
  RMDir /r /REBOOTOK "$INSTDIR\lximediacenter"

  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter" "DisplayName" "LeX-Interactive MediaCenter"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Backend service" SecBackend
  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File lximcbackend.exe

  nsisFirewall::AddAuthorizedApplication "$INSTDIR\lximcbackend.exe" "LeX-Interactive MediaCenter - Backend service"

  SetOutPath $INSTDIR\lximedia
  SetOverwrite ifnewer
  File lximedia\lximediacenter_*.dll

  ExecWait '"$INSTDIR\lximcbackend.exe" --install'
  ExecWait 'net start "LXiMediaCenter Backend"'
SectionEnd

Section "Systemtray icon" SecTrayIcon
  ExecWait 'tskill lximctrayicon /A'

  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File lximctrayicon.exe

  nsisFirewall::AddAuthorizedApplication "$INSTDIR\lximctrayicon.exe" "LeX-Interactive MediaCenter - Tray icon"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "LXiMediaCenterTrayIcon" '"$INSTDIR\lximctrayicon.exe"'
  Exec '"$INSTDIR\lximctrayicon.exe"'
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"

  ExecWait 'net stop "LXiMediaCenter Backend"'
  ExecWait 'tskill lximcbackend /A'
  ExecWait 'tskill lximctrayicon /A'

  ExecWait '"$INSTDIR\lximcbackend.exe" --uninstall'
  
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "LXiMediaCenterTrayIcon"

  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\lximcbackend.exe"
  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\lximctrayicon.exe"

  RMDir /r /REBOOTOK "$INSTDIR"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaCenter"
  
  MessageBox MB_YESNO "Would you like to keep the existing settings and databases? Note that building a new database might take several hours." /SD IDYES IDYES noRemoveDb
    RMDir /r "$%ALLUSERSPROFILE%\..\LocalService\Local Settings\Application Data\LXiMediaCenter"
  noRemoveDb:
  
SectionEnd
