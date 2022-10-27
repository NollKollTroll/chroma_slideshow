# Chroma slideshow

ZX81 program to show pictures, 256x384i in colour.

## General info

This software enables a ZX81 with both Chroma- and ZXpand-interface to do a slideshow of hires colour pictures, all programmed in BASIC.<br>
The pictures are 15 colours 256x384 pixels, and can be created with the included converter.

## Example usage

Put all the files from the release-binary folder on an SD-card, load the file **show.p** and then sit back and enjoy.

## Creating your own slideshow

First, gather the pictures and resize them to 256x384 with whatever program of your choice. Save the files as uncompressed 24-bit BMP.<br>
Use the command line program **bmp2nkt** to convert the BMP-files to the format usable for the ZX81 slideshow.<br>
Open the file **show.asm** in ZX-IDE and change the BASIC-lines loading the pictures, but do not change the load address.<br>
Compile the program.<br>
Put all the converted pictures and the file **show.p** on an SD-card.<br>
**NOTE:** 50hz interlaced is VERY flickery, choose 60Hz if at all possible!

## Emulators

My trusty go-to emulator EightyOne v1.29 works well with this program but needs some specific settings:<br>
- RAM Pack: 48k<br>
- High resolution: WRX<br>
- Colour: Chroma<br>
- ZXpand+: ON<br>
- TV-emulation / Advanced effects: OFF

## Building bmp2nkt for Linux

Developing and building bmp2nkt I used Kdevelop in Linux Mint, load the bmp2nkt project folder and compile.

## Building bmp2nkt for Windows

For anyone (future me) that wants to build the bmp2nkt program for windows, it requires Mingw-w64. Follow these steps:

```
cd bmp2nkt
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../win64_cross_compile_toolchain.txt -DCMAKE_BUILD_TYPE=Release ..
make
```

You will also need the MingW dynamic link libraries found here in Linux Mint:

```
/usr/lib/gcc/x86_64-w64-mingw32/9.3-win32/libgcc_s_seh-1.dll
/usr/lib/gcc/x86_64-w64-mingw32/9.3-win32/libstdc++-6.dll
```

## History

This is a program I originally wrote in 2019. It was shown at a Swedish retro computer exhibition, and then I forgot about it. The time has finally come to release it to the public.<br>

## Future

I hope many slideshows will be created and shown using our lovely little door-stoppers!<br>
Maybe someone wants to create a version of the converter with a GUI?

## License

The code I created for this project is licensed under Creative Commons CC0 1.0 Universal.

## Contact

Mail: <adam.klotblixt@gmail.com><br>
Github: <https://github.com/NollKollTroll/chroma_slideshow>