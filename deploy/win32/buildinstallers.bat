@set /p VERSION= < ..\..\VERSION

@makensis lximediacenter.nsi
@move /y ..\..\bin\LXiMediaCenterSetup.exe ..\..\bin\LXiMediaCenter-%VERSION%.exe

@makensis lximediaplayer.nsi
@move /y ..\..\bin\LXiMediaPlayerSetup.exe ..\..\bin\LXiMediaPlayer-%VERSION%.exe
