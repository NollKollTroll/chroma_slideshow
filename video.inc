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

const SCREEN_WIDTH_PIXELS             = 256
const SCREEN_WIDTH_BYTES              = SCREEN_WIDTH_PIXELS / 8  //x-resolution in bytes
const SCREEN_HEIGHT_RASTERS           = 384 //y-resolution in lines
const SCREEN_HEIGHT_RASTERS_PER_FIELD = SCREEN_HEIGHT_RASTERS / 2
const TV_STD_50HZ                     = 312
const TV_STD_60HZ                     = 262
const VSYNC_RASTERS                   = 4
const WASTED_RASTERS                  = 2
const TOTAL_BLANK_RASTERS             = TV_STD - WASTED_RASTERS - SCREEN_HEIGHT_RASTERS_PER_FIELD - VSYNC_RASTERS
const TOP_BLANK_RASTERS               = TOTAL_BLANK_RASTERS / 2
const BOTTOM_BLANK_RASTERS            = TOTAL_BLANK_RASTERS - TOP_BLANK_RASTERS
const SCREEN_PIXELS                   = ($c000 - $3000)
const SCREEN_ATTR                     = (NOPList + $8000)
const SCREEN_ATTR_BYTES               = (2 + SCREEN_WIDTH_BYTES + 1)

FrameCounter: define byte = 0

//--------------------------
subroutine EnableChroma
{
        //Enable chroma mode 1 colour
        //Lower nibble is border colour
        ld      a,%00110000
        ld      bc,$7fef
        out     (c),a
        ret
}

//--------------------------
subroutine DisableChroma
{
        //Disable chroma colour mode
        ld      a,%00001111
        ld      bc,$7fef
        out     (c),a
        ret
}

//--------------------------
subroutine EnableHires
{
        //synchronise the change to hires
/*
        ld      hl,Frames                       //Frames counter
        ld      a,(hl)                          //get old Frames
        {
                cp      (hl)                    //compare to new Frames
                jr      z,@loop                 //exit after a change is detected
        }
*/
        //change to hires
        ld      ix,GenerateVsyncEven
        ret
}

//--------------------------
subroutine DisableHires
{
        //restore ROM character patterns
        ld      a,$1e
        ld      i,a
        //restore standard ROM video mode
        ld      ix,$0281
        ret
}

//--------------------------
subroutine FastVideo
{
        //start VSync, disable NMI
        in      a,($fe)
        ret
}

//--------------------------
subroutine SlowVideo
{
        //end VSync, enable NMI
        out     ($fe),a 
        ret
}

//--------------------------
subroutine GenerateVsyncOdd
{
        //waste time        
        ld	a,0				//7
        ld	a,0				//7
        ld	b,6				//7
        djnz    *                               //13/8        
	jp	GenerateVsyncEven		//10
}

//--------------------------
subroutine GenerateVsyncEven
{
        //waste time
        repeat 5
        {
                nop                             //4
        }
        scf                                     //4
        //start VSync, disable NMI
        in      a,($fe)                         //11
        //waste time to create VSync, 441 cycles
        ld      b,32                            //7
        djnz    *                               //13/8
        ret     nc                              //5
        ret     nc                              //5        
        //prepare top border
        ld      ix,GenerateHiresScreen          //14
        ld      a,256 - TOP_BLANK_RASTERS       //7
        ex      af,af'                          //4
        //restore registers
        pop     hl                              //10
        pop     de                              //10
        pop     bc                              //10
        pop     af                              //10
        //end VSync, enable NMI
        out     ($fe),a                         //11
        //return to do some useful work, and generate top blank lines
        ret                                     //10
}

//--------------------------
subroutine GenerateHiresScreen
{
        //init all registers needed for hires display
        ld      (.StackSave),sp
        ld      sp,.ScreenStack: ScreenStackEven
        xor     a
        //waste time to synchronize picture-start
        ld      b,5
        djnz    *
        ret     nz
        //generate screen
GenerateLines:
        //waste time
        add     hl,bc                           //11
        add     hl,bc                           //11
        //prepare pixel pointer
        pop     hl                              //10
        ld      a,h                             //4
        ld      i,a                             //9
        ld      a,l                             //4
        //jump to generate pixels and attributes
        ret                                     //10 == 59
        //returns here when the whole screen is drawn
ContinueHires:
        //increment FrameCounter
        ld      hl,FrameCounter
        inc     (hl)        
        //Prepare bottom border
        ld      a,256 - BOTTOM_BLANK_RASTERS
        ex      af,af'
        pop     ix
        //prepare next field
        pop     hl
        ld      (.ScreenStack),hl
        //restore registers
        ld      sp,.StackSave: 0
        pop     hl
        pop     de
        pop     bc
        pop     af
        //enable NMI
        out     ($fe),a 		        
        //return to do some useful work, and generate bottom blank lines
        ret
}

//--------------------------
//sync to hires screen update
subroutine WaitScreen
{
        ld      hl,FrameCounter
        ld      a,(hl)
        {
                cp      (hl)
                jp      z,@loop
        }
        ret
}

//--------------------------
//Requires an opened file to stream from.
subroutine LoadImage
{
        //Load pixels
        ld      bc,SCREEN_HEIGHT_RASTERS
        ld      hl,SCREEN_PIXELS
        ld      de,SCREEN_WIDTH_BYTES
        {
                push    bc
                push    de
                push    hl
                ld      a,SCREEN_WIDTH_BYTES                
                call    ReadBytesFromFileToHL
                pop     hl
                pop     de
                pop     bc
                add     hl,de
                dec     bc
                ld      a,b
                or      c                
                jp      nz,@loop
        }
        //Load attributes
        ld      bc,SCREEN_HEIGHT_RASTERS
        ld      hl,SCREEN_ATTR + 2
        ld      de,SCREEN_ATTR_BYTES
        {
                push    bc
                push    de
                push    hl
                ld      a,SCREEN_WIDTH_BYTES
                call    ReadBytesFromFileToHL
                pop     hl
                pop     de
                pop     bc
                add     hl,de
                dec     bc
                ld      a,b
                or      c                
                jp      nz,@loop
        }
        ret
}

//--------------------------
subroutine MoveAttributes
{
        ld      hl,($c000 + $3000) - 1
        ld      de,(NOPListEnd + $8000) - 2
        ld      bc,384
        {        
                push    bc
                ld      bc,32
                lddr
                dec     de
                dec     de
                dec     de
                pop     bc
                dec     bc
                ld      a,b
                or      c
                jp      nz,@loop
        }
        ret
}

//--------------------------
NOPList:
{
        repeat SCREEN_HEIGHT_RASTERS
        {	
                ld      r,a                     //9
                repeat SCREEN_WIDTH_BYTES
                {
                        nop                     //4 * 32
                }
                ret     z                       //11 == 148
        }
NOPListEnd:
}
//print("NOPList {} bytes\n", sizeof(NOPList))
