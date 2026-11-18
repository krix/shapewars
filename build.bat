@echo off
echo Building ShapeWars with MinGW-w64...

REM Set the compiler path
set CC=C:\mingw64\bin\gcc.exe

REM Game source files
set GAME_SRC=src/main.c src/entity.c src/my_engine.c src/spacetime.c src/sounds.c src/embedded_assets.c src/embedded_font_qoi.c src/embedded_font_json.c src/particle.c src/touch_input.c src/scenes/game.c src/scenes/start_screen.c src/entities/player.c src/entities/enemy.c src/entities/bullet.c src/entities/gambo.c src/entities/explosion.c src/entities/mine.c src/entities/blackhole.c src/entities/powerup.c

REM High Impact source files (only the standalone modules we need)
set HI_SRC=high_impact/src/alloc.c high_impact/src/font.c high_impact/src/image.c high_impact/src/input.c high_impact/src/render.c high_impact/src/platform.c high_impact/src/sound.c high_impact/src/utils.c

REM Include directories
set INCLUDES=-Ihigh_impact/src -Isokol -Isrc

REM Compiler flags
set CFLAGS=-std=gnu99 -Wall -Wstrict-prototypes -Wno-unused-variable -Wno-unused-function -DPLATFORM_SOKOL -DRENDER_GL -O2

REM Linker flags for Windows with Sokol
set LFLAGS=-lgdi32 -luser32 -lshell32 -lole32 -lopengl32 -lglu32 -lmingw32 -lwinmm -lxinput -ldinput8 -ldxguid

REM Create build directory
if not exist build mkdir build

REM Compile and link
%CC% %GAME_SRC% %HI_SRC% %INCLUDES% %CFLAGS% %LFLAGS% -o build\shapewars.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful! Run build\shapewars.exe to play.
) else (
    echo Build failed with error code %ERRORLEVEL%.
)