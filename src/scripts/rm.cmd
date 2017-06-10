@echo off
:loop
	set "p=%1"
	set "p=%p:/=\%"
	if exist "%p%" del /Q "%p%"
	shift
if not "%~1"=="" goto loop