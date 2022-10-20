//Written in 2022 by Adam Klotblixt (adam.klotblixt@gmail.com)
//
//To the extent possible under law, the author have dedicated all
//copyright and related and neighboring rights to this software to the
//public domain worldwide.
//This software is distributed without any warranty.
//
//You should have received a copy of the CC0 Public Domain Dedication
//along with this software. If not, see
//<http://creativecommons.org/publicdomain/zero/1.0/>.

processor "z80"

section code, "VideoMain", 16516
{       
JpLoadImage:
        jp      LoadImage //+0
JpEnableChroma:        
        jp      EnableChroma //+3
JpDisableChroma:        
        jp      DisableChroma //+6
JpEnableHires:
        jp      EnableHires //+9
JpDisableHires:
        jp      DisableHires //+12
JpOpenAStringFile:
        jp      OpenAStringFile //+15
JpMoveAttributes:
        jp      MoveAttributes //+18
        
        include "zxpand.inc"
        include "video.inc"
}

//--------------------------
section code, "SCRSTACK.BIN", 32768
{
        ScreenStackStart:        
        
        ScreenStackEven:
        {
                var PIXEL_PTR = SCREEN_PIXELS
                var NOP_PTR = SCREEN_ATTR
                repeat SCREEN_HEIGHT_RASTERS_PER_FIELD - 1
                {
                        define word[3] = {PIXEL_PTR, NOP_PTR, GenerateLines}
                        PIXEL_PTR += SCREEN_WIDTH_BYTES * 2
                        NOP_PTR += SCREEN_ATTR_BYTES * 2
                }
                define word[3] = {PIXEL_PTR, NOP_PTR, ContinueHires}
                define word[1] = {GenerateVsyncOdd}
                define word[1] = {ScreenStackOdd}
        }

        ScreenStackOdd:
        {
                PIXEL_PTR = SCREEN_PIXELS + SCREEN_WIDTH_BYTES
                NOP_PTR = SCREEN_ATTR + SCREEN_ATTR_BYTES
                repeat SCREEN_HEIGHT_RASTERS_PER_FIELD - 1
                {
                        define word[3] = {PIXEL_PTR, NOP_PTR, GenerateLines}
                        PIXEL_PTR += SCREEN_WIDTH_BYTES * 2
                        NOP_PTR += SCREEN_ATTR_BYTES * 2
                }
                define word[3] = {PIXEL_PTR, NOP_PTR, ContinueHires}
                define word[1] = {GenerateVsyncEven}
                define word[1] = {ScreenStackEven}
        }
}
