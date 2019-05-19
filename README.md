# kblayout
Simple keyboard layout indicator. GNU/Linux

Build:

    gcc -O2 -s -lX11 -lXft `pkg-config --cflags freetype2` -o kblayout kblayout.c
