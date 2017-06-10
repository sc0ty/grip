@echo off
for %%a in (%*) do set "dst=%%a"
set "dst=%dst:/=\%"

:loop
	set "src=%1"
	set "src=%src:/=\%"
	copy "%src%" %dst%"
	shift
if not "%~2"=="" goto loop