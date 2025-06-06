@echo off
rem -----------------------------------------------
rem runtest.bat (for Pelles C)
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

rem Delete any existing log so we start fresh (silent if it doesn't exist)
del /Q test.log 2>nul

rem Step 1: Compile all t_*.c from parent directory, adding include paths and using optimization + high warnings
for %%F in ("..\t_*.c") do (
    echo Compiling %%~nxF...
    cc /O2 /W2 /I".." /I"..\..\src" /Fo".\%%~nF.obj" "%%~F" 
)

rem Step 2: Run each compiled t_*.exe and append stderr to test.log
for %%E in (t_*.exe) do (
    echo Running %%~nxE...
    "%%~fE" 2>>test.log
)

echo All programs have run.  Stderr output (if any) is in test.log.
