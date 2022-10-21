#include <iostream>     // std::cout, std::fixed
#include <fstream>
#include <array>
#include <cmath>
    
#include "libbmp-master/CPP/libbmp.h"
#include "optionparser-1.7/src/optionparser.h"

using namespace std;

#define NR_OF_FILTER_ROWS 3
#define NR_OF_FILTER_COLS 5
#define IMAGE_SIZE_X 256
#define IMAGE_SIZE_Y 384

struct filter_t {
    array <array <float, NR_OF_FILTER_COLS>, NR_OF_FILTER_ROWS> filterData;
    string name;
};

array <filter_t, 4> filter = {
    {
        {
            {
                {            
                    {     0,      0,      0, 8/42.0, 4/42.0},
                    {2/42.0, 4/42.0, 8/42.0, 4/42.0, 2/42.0},
                    {1/42.0, 2/42.0, 4/42.0, 2/42.0, 1/42.0}
                }
                
            },
            "Stucki"
        },
        {
            {
                {
                    {     0,     0,       0, 7/48.0, 5/48.0},
                    {3/48.0, 5/48.0, 7/48.0, 5/48.0, 3/48.0},
                    {1/48.0, 3/48.0, 5/48.0, 3/48.0, 1/48.0}
                }
            },
            "JaJuNi"
        },
        {
            {
                {
                    {     0,      0,      0, 7/16.0,      0},
                    {     0, 3/16.0, 5/16.0, 1/16.0,      0},

                    {     0,      0,      0,      0,      0}
                }
            },
            "Floyd-Steinberg"
        },
        {
            {
                {
                    {     0,      0,      0,  1/8.0,  1/8.0},
                    {     0,  1/8.0,  1/8.0,  1/8.0,      0},
                    {     0,      0,  1/8.0,      0,      0}
                }
            },
            "Atkinson"
        }
    }
};

struct rgb_t {
    uint8_t R;
    uint8_t G;
    uint8_t B;    
};

struct palette_t {
    array <rgb_t, 16> entry;
    uint8_t first;
    uint8_t last;
    string name;
};

array <palette_t, 4> palette = {
    {
        {
            {
                {
                    {0,0,0},
                    {0,0,0xaa},         
                    {0xaa,0,0},
                    {0xaa,0,0xaa},
                    {0,0xaa,0},
                    {0,0xaa,0xaa},
                    {0xaa,0xaa,0},
                    {0xaa,0xaa,0xaa},
                    {0,0,0},            
                    {0,0,0xff},
                    {0xff,0,0},
                    {0xff,0,0xff},
                    {0,0xff,0},
                    {0,0xff,0xff},
                    {0xff,0xff,0},
                    {0xff,0xff,0xff}
                }
            },
            1,
            15,
            "chroma15"
        },
        {
            {
                {
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},            
                    {0,0,0xff},
                    {0xff,0,0},
                    {0xff,0,0xff},
                    {0,0xff,0},
                    {0,0xff,0xff},
                    {0xff,0xff,0},
                    {0xff,0xff,0xff}
                }
            },
            8,
            15,
            "chroma8"
        },
        {
            {
                {
                    {0,0,0},
                    {19,19,19},         
                    {51,51,51},
                    {70,70,70},
                    {100,100,100},
                    {119,119,119},
                    {151,151,151},
                    {170,170,170},
                    {0,0,0},            
                    {28,28,28},
                    {77,77,77},
                    {105,105,105},
                    {150,150,150},
                    {179,179,179},
                    {227,227,227},
                    {255,255,255}
                }
            },
            1,
            15,
            "chromaG15"
        },
        {
            {
                {
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},
                    {0,0,0},            
                    {28,28,28},
                    {77,77,77},
                    {105,105,105},
                    {150,150,150},
                    {179,179,179},
                    {227,227,227},
                    {255,255,255}
                }
            },
            8,
            15,
            "chromaG8"
        }        
    }
};

struct rgbError_t {
    float R;
    float G;
    float B;    
};

struct workingError_t {
    array <array <rgbError_t, (IMAGE_SIZE_X + NR_OF_FILTER_COLS - 1)>, NR_OF_FILTER_ROWS>row;
} workingError;

struct workingOctet_t {
    array <rgb_t, 8> pos;
} workingOctet, bestWorkingOctet;

struct workingErrorOctet_t {
    array <array <rgbError_t, 8 + NR_OF_FILTER_COLS - 1>, NR_OF_FILTER_ROWS>row;
    float total;
} workingErrorOctet, bestWorkingErrorOctet;

struct currentError_t {
    rgbError_t rgbError;
    float total;
} currentError, currentBGError, currentFGError;

string fileNameIn;
string fileNameOut;
uint8_t paletteNr;
uint8_t filterNr;
float filterStrength;
bool grey;

array<uint8_t, IMAGE_SIZE_Y * IMAGE_SIZE_X / 8> pixelData;
array<uint8_t, IMAGE_SIZE_Y * IMAGE_SIZE_X / 8> attributeData;
rgb_t currentPixel;
rgb_t currentFGColour; 
rgb_t currentBGColour;
uint8_t pixelOctet, bestPixelOctet, bestAttribute;
int16_t cStart, cStop, cStep;
int16_t xStart, xStop, xStep;
uint32_t dataPtr;
bool serpentine;
uint8_t requiredOptionsCount;

struct Arg: public option::Arg
{
    static void printError(const char* msg1, const option::Option& opt, const char* msg2)
    {
        fprintf(stderr, "%s", msg1);
        fwrite(opt.name, opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }
    static option::ArgStatus Unknown(const option::Option& option, bool msg)
    {
        if (msg) printError("Unknown option '", option, "'\n\n");
        return option::ARG_ILLEGAL;
    }
    static option::ArgStatus Numeric(const option::Option& option, bool msg)
    {
        char* endptr = 0;
        if (option.arg != 0 && strtol(option.arg, &endptr, 10)){};
        if (endptr != option.arg && *endptr == 0)
        return option::ARG_OK;
        if (msg) printError("Option '", option, "' requires a numeric argument\n\n");
        return option::ARG_ILLEGAL;
    }
};

enum  optionIndex { UNKNOWN, HELP, NUMERIC, PALETTE, FILTER, STRENGTH, SERP };
const option::Descriptor usage[] =
{
    {UNKNOWN, 0,"" , ""    ,option::Arg::None,     "USAGE: bmp2nkt [options] inFile outFile\n\n"
                                                   "Options:"},
    {HELP,    0,"h", "help",option::Arg::None,     "  -h --help  Print usage and exit.\n"},
    {PALETTE, 0,"p",     "",     Arg::Numeric,     "  -p <num>   Palette type:\n"
                                                   "             0 = chroma 15 colours\n"
                                                   "             1 = chroma 8 colours (default)\n"
                                                   "             2 = chroma 15 grey shades\n"
                                                   "             3 = chroma 8 grey shades\n"},
    {FILTER, 0,"f",      "",     Arg::Numeric,     "  -f <num>   Filter type:\n"
                                                   "             0 = Stucki (default)\n"
                                                   "             1 = JaJuNi\n"
                                                   "             2 = Floyd-Steinberg\n"
                                                   "             3 = Atkinson\n"},
    {STRENGTH, 0,"F",    "",     Arg::Numeric,     "  -F <num>   Filter strength in percent.\n"
                                                   "             Defaults to 100%.\n"},
    {SERP,     0,"s",    "",    Arg::Optional,     "  -s         Serpentine parsing, optional.\n"
                                                   "             Default is left-to-right parsing.\n"},                                                   
    {UNKNOWN,  0,"" ,    "",option::Arg::None,     "inFile:\n"
                                                   "  Full path to the file to convert.\n"
                                                   "  Requires an uncompressed 24-bit BMP-file at 256x384 pixels.\n"
                                                   "\n"
                                                   "outFile:\n"
                                                   "  Full path and filename, but without extension.\n"
                                                   "  Both .bmp and .nkt will be created.\n"
                                                   "\n"
                                                   "Examples:\n"
                                                   "  bmp2nkt file1.bmp file2\n"
                                                   "  bmp2nkt -p0 -f2 -F95 -s Pictures/file1.bmp Chroma/file2\n"},
    {0,0,0,0,0,0}
};

int main(int argc, char **argv)
{   

    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(usage, argc, argv);
    option::Option options[stats.options_max], buffer[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);

    if (parse.error())
    {
        option::printUsage(cout, usage);
        return 1;
    }
    
    if (options[HELP] || argc == 0)
    {
        option::printUsage(cout, usage);
        return 0;
    }
    
    if (options[SERP])
    {
        serpentine = true;
    }
    else
    {
        serpentine = false;
    }
    
    if (options[PALETTE])
    {
        paletteNr = atoi(options[PALETTE].arg);
        if ((paletteNr == 2) || (paletteNr == 3))
        {
            grey = true;
        }
        else
        {
            grey = false;
        }
    }
    else
    {
        paletteNr = 1;
    }
    
    if (options[FILTER])
    {
        filterNr = atoi(options[FILTER].arg);
    }
    else
    {
        filterNr = 0;
    }
    
    if (options[STRENGTH])
    {
        filterStrength = atoi(options[STRENGTH].arg) / 100.0;
    }
    else
    {
        filterStrength = 1.00;
    }
    
    if (parse.nonOptionsCount() == 2)
    {        
        fileNameIn = parse.nonOption(0);
        fileNameOut = parse.nonOption(1);     
    }
    else if (parse.nonOptionsCount() == 1)
    {
        cout << "Missing outFile" << endl;
        //option::printUsage(cout, usage);
        return 1;
    }
    else if (parse.nonOptionsCount() == 0)
    {
        cout << "Missing inFile and outFile" << endl;
        //option::printUsage(cout, usage);
        return 1;
    }
    else
    {
        option::printUsage(cout, usage);
        return 0;
    }
    
    BmpImg img;
    img.read (fileNameIn);
    if(img.get_height() != IMAGE_SIZE_Y)
    {
        cout << "ERROR: got " << to_string(img.get_height()) << "high, expected " << to_string(IMAGE_SIZE_Y) << endl;
        return -1;
    }
    if(img.get_width() != IMAGE_SIZE_X)
    {
        cout << "ERROR: got " << to_string(img.get_width()) << "wide, expected " << to_string(IMAGE_SIZE_X) << endl;
        return -1;
    }    
    
    for (int16_t y = 0; y < IMAGE_SIZE_Y; ++y)
    {
        //cout << "processing line " << to_string(y) << endl;
        workingError.row[0] = workingError.row[1];
        workingError.row[1] = workingError.row[2];
        for (int16_t i = 0; i < (IMAGE_SIZE_X + NR_OF_FILTER_COLS - 1); ++i)
        {
            workingError.row[2][i] = {0.0, 0.0, 0.0};
        }
                
        if((serpentine == true) && (y & 1))
        {
            cStart = (int16_t)(IMAGE_SIZE_X / 8) - 1;
            cStop = -1;
            cStep = -1;
            
            xStart = 8 - 1;
            xStop = -1;
            xStep = -1;
        }
        else
        {
            cStart = 0;
            cStop = (int16_t)(IMAGE_SIZE_X / 8);
            cStep = 1;
            
            xStart = 0;
            xStop = 8;
            xStep = 1;
        }
        
        for (int16_t column = cStart; column != cStop; column += cStep)
        {            
            //cout << to_string(column) << endl;
            bestWorkingErrorOctet.total = HUGE_VAL_F32; 
            for (int16_t FGCount = palette[paletteNr].first; FGCount <= palette[paletteNr].last; ++FGCount)
            {
                for (int16_t BGCount = FGCount + 1; BGCount <= palette[paletteNr].last; ++BGCount)
                {                    
                    //cout << to_string(FGCount) << ":" << to_string(BGCount) << endl;
                    currentFGColour = palette[paletteNr].entry[FGCount];
                    currentBGColour = palette[paletteNr].entry[BGCount];
                    for (int16_t i = 0; i < (8 + NR_OF_FILTER_COLS - 1); ++i)                        
                    {
                        workingErrorOctet.row[0][i] = workingError.row[0][column * 8 + i]; 
                        workingErrorOctet.row[1][i] = workingError.row[1][column * 8 + i];
                        workingErrorOctet.row[2][i] = workingError.row[2][column * 8 + i];
                    }
                    //calculate errors and pixels for 8 pixels, with current FG and BG colours
                    workingErrorOctet.total = 0.0;
                    pixelOctet = 0;
                    for (int16_t x = xStart; x != xStop; x += xStep)
                    {               
                        if (grey == true)
                        {
                            currentPixel.R = round((0.30 * img.red_at((column * 8) + x, y)) +
                                               (0.59 * img.green_at((column * 8) + x, y)) +
                                               (0.11 * img.blue_at((column * 8) + x, y)));
                            currentPixel.G = currentPixel.R;
                            currentPixel.B = currentPixel.R;
                        }
                        else
                        {
                            currentPixel.R = img.red_at((column * 8) + x, y);
                            currentPixel.G = img.green_at((column * 8) + x, y);
                            currentPixel.B = img.blue_at((column * 8) + x, y);
                        }
                        currentBGError.rgbError.R = currentPixel.R + workingErrorOctet.row[0][x + 2].R - currentBGColour.R;
                        currentBGError.rgbError.G = currentPixel.G + workingErrorOctet.row[0][x + 2].G - currentBGColour.G;
                        currentBGError.rgbError.B = currentPixel.B + workingErrorOctet.row[0][x + 2].B - currentBGColour.B;
                        currentBGError.total = ((currentBGError.rgbError.R * currentBGError.rgbError.R) + 
                                                (currentBGError.rgbError.G * currentBGError.rgbError.G) + 
                                                (currentBGError.rgbError.B * currentBGError.rgbError.B));
                        currentFGError.rgbError.R = currentPixel.R + workingErrorOctet.row[0][x + 2].R - currentFGColour.R;
                        currentFGError.rgbError.G = currentPixel.G + workingErrorOctet.row[0][x + 2].G - currentFGColour.G;
                        currentFGError.rgbError.B = currentPixel.B + workingErrorOctet.row[0][x + 2].B - currentFGColour.B;
                        currentFGError.total = ((currentFGError.rgbError.R * currentFGError.rgbError.R) + 
                                                (currentFGError.rgbError.G * currentFGError.rgbError.G) + 
                                                (currentFGError.rgbError.B * currentFGError.rgbError.B));                        
                        if (currentBGError.total < currentFGError.total)
                        {
                            currentError = currentBGError;
                            workingErrorOctet.total += currentBGError.total;
                            workingOctet.pos[x] = currentBGColour;
                            //no need to change pixel-bit, already 0
                        }
                        else
                        {
                            currentError = currentFGError;
                            workingErrorOctet.total += currentFGError.total;
                            workingOctet.pos[x] = currentFGColour;
                            pixelOctet += (128 >> x);                            
                        }
                        if((serpentine == true) && (y & 1))
                        {
                            
                            for (int16_t FilterRow = 0; FilterRow < NR_OF_FILTER_ROWS; ++FilterRow)
                            {
                                for (int16_t FilterCol = (NR_OF_FILTER_COLS - 1); FilterCol >= 0; --FilterCol)
                                {
                                    workingErrorOctet.row[FilterRow][x + FilterCol].R += currentError.rgbError.R * filter[filterNr].filterData[FilterRow][4 - FilterCol] * filterStrength;
                                    workingErrorOctet.row[FilterRow][x + FilterCol].G += currentError.rgbError.G * filter[filterNr].filterData[FilterRow][4 - FilterCol] * filterStrength;
                                    workingErrorOctet.row[FilterRow][x + FilterCol].B += currentError.rgbError.B * filter[filterNr].filterData[FilterRow][4 - FilterCol] * filterStrength;
                                }
                            }
                        }
                        else
                        {
                            for (int16_t FilterRow = 0; FilterRow < NR_OF_FILTER_ROWS; ++FilterRow)
                            {
                                for (int16_t FilterCol = 0; FilterCol < NR_OF_FILTER_COLS; ++FilterCol)
                                {
                                    workingErrorOctet.row[FilterRow][x + FilterCol].R += currentError.rgbError.R * filter[filterNr].filterData[FilterRow][FilterCol] * filterStrength;
                                    workingErrorOctet.row[FilterRow][x + FilterCol].G += currentError.rgbError.G * filter[filterNr].filterData[FilterRow][FilterCol] * filterStrength;
                                    workingErrorOctet.row[FilterRow][x + FilterCol].B += currentError.rgbError.B * filter[filterNr].filterData[FilterRow][FilterCol] * filterStrength;
                                }
                            }                            
                        }                        
                    }                    
                    //are the current colours best so far for this octet?
                    if (workingErrorOctet.total < bestWorkingErrorOctet.total)
                    {
                        bestPixelOctet = pixelOctet;
                        bestAttribute = (BGCount << 4) + FGCount;
                        bestWorkingErrorOctet = workingErrorOctet;
                        bestWorkingOctet = workingOctet;
                    }                                
                }            
            }
            //colours chosen, now set pixels and errors
            for (int16_t i = 0; i < 8; ++i)
            {
                img.set_pixel(column * 8 + i, y, bestWorkingOctet.pos[i].R, bestWorkingOctet.pos[i].G, bestWorkingOctet.pos[i].B);
            }
            for (int16_t i = 0; i < (8 + NR_OF_FILTER_COLS - 1); ++i)
            {
                workingError.row[0][column * 8 + i] = bestWorkingErrorOctet.row[0][i];
                workingError.row[1][column * 8 + i] = bestWorkingErrorOctet.row[1][i];
                workingError.row[2][column * 8 + i] = bestWorkingErrorOctet.row[2][i];
            }
            
            dataPtr = column + (y * IMAGE_SIZE_X / 8);
            pixelData[dataPtr] = bestPixelOctet;
            attributeData[dataPtr] = bestAttribute;                    
        }
        //cout << to_string(y) << "done!" << endl;
    }    
        
    fileNameOut += "_p" + to_string(paletteNr);
    fileNameOut += "_f" + to_string(filterNr);
    fileNameOut += "_F" + to_string((int)(filterStrength * 100));
    if (serpentine)
    {
        fileNameOut += "_s1";
    }
    else
    {
        fileNameOut += "_s0";
    }   
    img.write(fileNameOut + ".bmp");

    ofstream outFile;
    outFile.open(fileNameOut + ".nkt", ios::binary|ios::out);
    outFile.write((char *)&pixelData, sizeof(pixelData));
    outFile.write((char *)&attributeData, sizeof(attributeData));
    outFile.close();
    
    cout << endl << "Done!" << endl;
    return 0;
}
