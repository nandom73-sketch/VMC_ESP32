@echo off
echo.
echo ==========================
echo Aggiornamento SmartVMC
echo ==========================
echo.

git pull origin main

if errorlevel 1 (
    echo.
    echo ==========================
    echo ERRORE: aggiornamento fallito
    echo ==========================
    pause
    exit /b
)

echo.
echo ==========================
echo Pull completato!
echo ==========================
pause