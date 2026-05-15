@echo off
echo Building ShapeWars for WASM...

set EMCC=C:\Users\krix\AppData\Local\Temp\opencode\emsdk\upstream\emscripten\emcc.bat

set GAME_SRC=src/main.c src/entity.c src/my_engine.c src/spacetime.c src/sounds.c src/embedded_assets.c src/embedded_font_qoi.c src/embedded_font_json.c src/scenes/game.c src/scenes/start_screen.c src/entities/player.c src/entities/enemy.c src/entities/bullet.c src/entities/gambo.c src/entities/explosion.c src/entities/mine.c src/entities/blackhole.c src/entities/powerup.c

set HI_SRC=high_impact/src/alloc.c high_impact/src/font.c high_impact/src/image.c high_impact/src/input.c high_impact/src/render.c high_impact/src/platform.c high_impact/src/sound.c high_impact/src/utils.c

set INCLUDES=-Ihigh_impact/src -Isokol -Isrc

set CFLAGS=-std=gnu99 -Wall -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-function -DPLATFORM_SOKOL -DRENDER_GL -DRENDER_RESIZE_MODE=0 -O2

set LFLAGS=-lGL -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=web -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -sEXPORTED_FUNCTIONS=_main

REM Create build directory
if not exist build_wasm mkdir build_wasm

copy /Y src\wasm_shell.html build_wasm\index.html

%EMCC% %GAME_SRC% %HI_SRC% %INCLUDES% %CFLAGS% %LFLAGS% -o build_wasm\shapewars.js

if %ERRORLEVEL% EQU 0 (
    echo Build successful! Files: build_wasm\index.html, build_wasm\shapewars.js, build_wasm\shapewars.wasm
) else (
    echo Build failed with error code %ERRORLEVEL%.
)
