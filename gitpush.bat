@echo off
echo.
set /p MSG=Messaggio commit: 

git add .

git commit -m "%MSG%"

if errorlevel 1 (
    echo.
    echo Nessun commit eseguito oppure errore.
    pause
    exit /b
)

git push

echo.
echo ==========================
echo Push completato!
echo ==========================
pause