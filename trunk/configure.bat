@echo off
if exist "%1" (
  set QTDIR=%1
	"%1\bin\qmake" -recursive
) else (
	echo Usage:
	echo   configure ^<QtDir^>
) 
