!include "MUI2.nsh"

Name "LeX-Interactive MediaPlayer"
OutFile "..\..\bin\LXiMediaPlayerSetup.exe"
SetCompressor /SOLID lzma

InstallDir "$PROGRAMFILES\LXiMediaPlayer"
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
  ExecWait 'taskkill /F /T /IM lximediaplayer.exe'
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
  File ..\..\bin\LXiStream?.dll
  File ..\..\bin\LXiStreamGui?.dll
  File ..\..\bin\lxistreamsandbox.exe

  SetOutPath $INSTDIR\imageformats
  SetOverwrite ifnewer
  File ..\..\bin\imageformats\*.dll

  SetOutPath $INSTDIR\liblxistream
  SetOverwrite ifnewer
  File ..\..\bin\liblxistream\*.dll

  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaPlayer" "DisplayName" "LeX-Interactive MediaPlayer"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaPlayer" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Media player" SecPlayer
  SetOutPath $INSTDIR
  SetOverwrite ifnewer
  File ..\..\bin\lximediaplayer.exe
SectionEnd

Section "Start Menu Shortcuts"
  CreateShortCut "$SMPROGRAMS\LXiMediaPlayer.lnk" "$INSTDIR\lximediaplayer.exe" "" "$INSTDIR\lximediaplayer.exe" 0
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"

  ExecWait 'taskkill /F /T /IM lximediaplayer.exe'

  RMDir /r /REBOOTOK "$INSTDIR"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LXiMediaPlayer"  
SectionEnd
