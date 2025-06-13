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


setlocal

echo Checking results

rem — Count PASS lines (find is lighter than findstr here)
for /f "tokens=2 delims=:" %%A in ('
    find /c " PASS|" test.log
') do set PASS_COUNT=%%A

rem — Count FAIL lines
for /f "tokens=2 delims=:" %%A in ('
    find /c " FAIL|" test.log
') do set FAIL_COUNT=%%A

>>test.log echo TOTAL  %FAIL_COUNT% FAIL  :  %PASS_COUNT% PASS

endlocal

echo All programs have run. Output is in test.log.
