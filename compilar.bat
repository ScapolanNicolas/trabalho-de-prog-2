@echo off
REM ===========================================================================
REM  compilar.bat - Compila o Sistema de Controle de Estoque no Windows
REM  Uso: de dois cliques neste arquivo (precisa ter o GCC instalado).
REM  Gera o estoque.exe nesta pasta, com o SQLite embutido (amalgamation).
REM ===========================================================================
cd /d "%~dp0"

where gcc >nul 2>nul
if errorlevel 1 (
    echo [ERRO] GCC nao encontrado neste computador.
    echo Instale o MSYS2 ^(https://www.msys2.org^) ou baixe o estoque.exe pronto
    echo na secao "Releases" do repositorio no GitHub.
    pause
    exit /b 1
)

if not exist sqlite3.o (
    echo Compilando o SQLite ^(apenas na primeira vez, pode levar 1 minuto^)...
    gcc -O2 -c sqlite3.c -o sqlite3.o
    if errorlevel 1 goto erro
)

echo Compilando o projeto...
gcc -O2 -static -I. main.c models/produto.c algoritmos/busca.c algoritmos/ordenacao.c persistencia/banco.c sqlite3.o -o estoque.exe
if errorlevel 1 goto erro

echo.
echo Compilado com sucesso! Para executar, de dois cliques em estoque.exe
pause
exit /b 0

:erro
echo.
echo [ERRO] A compilacao falhou. Veja as mensagens acima.
pause
exit /b 1
