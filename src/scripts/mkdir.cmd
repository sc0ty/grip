@echo off
:loop
	set "p=%1"
	set "p=%p:/=\%"
	if not exist "%p%" mkdir "%p%"
	shift
if not "%~1"=="" goto loop