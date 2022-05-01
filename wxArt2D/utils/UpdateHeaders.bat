@echo off
echo Update Headers Diff > diff.txt
for /R . %%D in (*.cpp;*.h) do call :doit %%D
pause
goto :eof

:doit %1
rem echo Processing %1
perl updateheaders.pl %1 %1.tmp
if %ERRORLEVEL%==1 (
   pause
   del %1.tmp
   goto :eof
)
diff %1 %1.tmp >> diff.txt
if %ERRORLEVEL%==1 (
   echo Changes in %1 >> diff.txt
   move %1 %1.bak
   move %1.tmp %1
) else (
   del %1.tmp
)
goto :eof
