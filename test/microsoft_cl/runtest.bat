::  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
::  SPDX-License-Identifier: MIT

@echo off
:: -----------------------------------------------
:: runtest
:: Usage:
::   runtest        - Compile & run all t_*.c from parent, collecting stderr in test.log
::   runtest clean  - ::ove all generated executables, object files, and test.log
:: -----------------------------------------------

:: Check for "clean" argument (case-insensitive)
if /I "%~1"=="clean" (
    echo Cleaning up artifacts...

    :: Quietly delete test.log, executables, and object files (suppress “file not found” errors)
    del /Q test.log 2>nul
    del /Q t_*.exe  2>nul
    del /Q t_*.obj  2>nul

    goto :eof
)

:: -----------------------------------------------------------
:: No argument: compile each t_*.c from parent and run them
:: -----------------------------------------------------------

:: Initialize test.log
echo START %DATE% %TIME% > test.log

:: Compile all t_*.c from parent directory, adding include paths and using C11
for %%F in ("..\t_*.c") do (
    echo Compiling %%~nxF...
    cl /W4 /O2 /std:c11 /I ".." /I "..\..\src" "%%~fF"
)

:: Run each compiled t_*.exe and append stderr to test.log
for %%E in (t_*.exe) do (
    echo Running %%~nxE...
    "%%~fE" 2>>test.log
)

echo END %DATE% %TIME% >> test.log

:: Count PASS/FAIL

setlocal

echo Checking results

::   — Count PASS lines (find is lighter than findstr here)
for /f "tokens=2 delims=:" %%A in ('
    find /c " PASS|" test.log
') do set PASS_COUNT=%%A

::   — Count FAIL lines
for /f "tokens=2 delims=:" %%A in ('
    find /c " FAIL|" test.log
') do set FAIL_COUNT=%%A

echo TOTAL  %FAIL_COUNT% FAIL  :  %PASS_COUNT% PASS >>test.log 

endlocal

echo All programs have run. Output is in test.log.
