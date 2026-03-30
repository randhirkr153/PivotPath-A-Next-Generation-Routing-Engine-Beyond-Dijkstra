@echo off
echo ===================================================
echo      SmartRoute: High-Performance Navigation
echo ===================================================
echo.
echo Starting the Python Backend Server...
start "SmartRoute Backend" cmd /k "cd backend && uvicorn main:app --reload"

echo Starting the React Frontend Server...
start "SmartRoute Frontend" cmd /k "cd frontend && npm run dev"

echo.
echo Both servers are starting! 
echo.
echo Once they load, you can access your web app any time by opening your browser to:
echo     http://localhost:5173
echo.
echo Note: Keep the two black command prompt windows open while you are using the app. 
echo       To stop the app, simply close those two windows.
echo.
pause
