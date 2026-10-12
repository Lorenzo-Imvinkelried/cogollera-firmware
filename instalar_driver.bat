@echo off
:: Solicitar permisos de administrador automáticamente si no los tiene
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
if '%errorlevel%' NEQ '0' (
    echo Solicitando permisos de Administrador...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    echo UAC.ShellExecute "%~s0", "", "", "runas", 1 >> "%temp%\getadmin.vbs"
    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"

echo ===================================================================
echo     INSTALANDO CONTROLADOR CP210X PARA ESP32 (SILICON LABS)
echo ===================================================================
echo.
pnputil /add-driver "%~dp0driver-cp210x\silabser.inf" /install
echo.
echo ===================================================================
echo Si arriba dice "Paquetes de controladores agregados: 1", ya esta listo!
echo Puedes conectar tu ESP32 o ejecutar: pio run --target upload
echo ===================================================================
pause
