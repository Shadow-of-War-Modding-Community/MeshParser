@echo off

:: Set the size of the console window
mode con: cols=150 lines=40

:: Print the executable to run with the first command argument
<nul set /p "=MeshUnpacker.exe unpack "

:: Use PowerShell to open a file selection dialog and get the selected .mesh file path
for /f "delims=" %%I in ('powershell -NoProfile -Command "Add-Type -AssemblyName System.Windows.Forms; $f = New-Object System.Windows.Forms.OpenFileDialog; $f.Filter = 'MESH Files (*.mesh)|*.mesh'; $f.Title = 'Select a .mesh file'; if ($f.ShowDialog() -eq 'OK') { $f.FileName }"') do set MESH_FILE=%%I

:: Check if a .mesh file was selected
if "%MESH_FILE%"=="" (
    echo:
    echo:
    echo No file selected. Exiting...
    timeout /t 5 >nul
    exit /b
)

:: Print the selected .mesh file as the second command argument
<nul set /p "=%MESH_FILE%"

:: Extract the directory from the .mesh file path
for %%I in ("%MESH_FILE%") do set OUTPUT_DIR=%%~dpI

:: Execute MeshUnpacker.exe with the selected file
MeshUnpacker.exe unpack "%MESH_FILE%"

:: Print the output file
timeout /t 1 >nul
echo:
echo:
echo Output: %OUTPUT_DIR%out.fbx

pause