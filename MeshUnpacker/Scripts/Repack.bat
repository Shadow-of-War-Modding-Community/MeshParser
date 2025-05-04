@echo off

:: Set the size of the console window
mode con: cols=150 lines=40

:: Print the executable to run with the first command argument
<nul set /p "=MeshUnpacker.exe repack "

:: Use PowerShell to open a file selection dialog and get the selected .fbx file path
for /f "delims=" %%I in ('powershell -NoProfile -Command "Add-Type -AssemblyName System.Windows.Forms; $f = New-Object System.Windows.Forms.OpenFileDialog; $f.Filter = 'FBX Files (*.fbx)|*.fbx'; $f.Title = 'Select a .fbx file'; if ($f.ShowDialog() -eq 'OK') { $f.FileName }"') do set FBX_FILE=%%I

:: Check if a .fbx file was selected
if "%FBX_FILE%"=="" (
    echo:
    echo:
    echo No file selected. Exiting...
    timeout /t 5 >nul
    exit /b
)

:: Print the selected .fbx file as the second command argument
<nul set /p "=%FBX_FILE% "

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

:: Print the selected .mesh file as the third command argument
<nul set /p "=%MESH_FILE%"

:: Extract the directory from the .fbx file path
for %%I in ("%FBX_FILE%") do set OUTPUT_DIR=%%~dpI

:: Execute MeshUnpacker.exe with the selected files
MeshUnpacker.exe repack "%FBX_FILE%" "%MESH_FILE%"

:: Print the output file
timeout /t 1 >nul
echo:
echo:
echo Output: %OUTPUT_DIR%out.mesh

pause