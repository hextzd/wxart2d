@echo off
for /R . %%D in (*.bak) do call :doit %%D
goto :eof

:doit %1
del %~dpn1
move %1 %~dpn1
goto :eof
