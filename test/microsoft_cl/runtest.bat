@echo off
rem -----------------------------------------------
rem runtest.bat
rem Usage:
rem   runtest.bat        - Compile & run all t_*.c from parent, collecting stderr in test.log
rem   runtest.bat clean  - Remove all generated executables, object files, and test.log
rem -----------------------------------------------

rem Check for "clean" argument (case-insensitive)
if /I "%~1"=="clean" (
    echo Cleaning up artifacts...

    rem Quietly delete test.log, executables, and object files (suppress “file not found” errors)
    del /Q test.log 2>nul
    del /Q t_*.exe  2>nul
    del /Q t_*.obj  2>nul

    goto :eof
)

rem -----------------------------------------------------------
rem No argument: compile each t_*.c from parent and run them
rem -----------------------------------------------------------

rem Initialize test.log
echo START %DATE% %TIME% > test.log

rem Step 1: Compile all t_*.c from parent directory, adding include paths and using C11
for %%F in ("..\t_*.c") do (
    echo Compiling %%~nxF...
    cl /W4 /O2 /std:c11 /I ".." /I "..\..\src" "%%~fF"
)

rem Step 2: Run each compiled t_*.exe and append stderr to test.log
for %%E in (t_*.exe) do (
    echo Running %%~nxE...
    "%%~fE" 2>>test.log
)

echo END %DATE% %TIME% >> test.log

rem Step 3: Count PASS/FAIL
echo Checking results
set /a PASS_COUNT=0
set /a FAIL_COUNT=0

for /f "delims=" %%L in (test.log) do (
    :: echo %%L
    echo "%%L" | findstr /c:" PASS|" >nul && set /a PASS_COUNT+=1
    echo "%%L" | findstr /c:" FAIL|" >nul && set /a FAIL_COUNT+=1
)

echo TOTAL: %FAIL_COUNT% FAIL : %PASS_COUNT% PASS >> test.log

echo All programs have run.  Stderr output (if any) is in test.log.
