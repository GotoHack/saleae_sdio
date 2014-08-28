#include "SdioCmd5.h"
#include "SDIOParser.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


const char * SdioCmd5::getDetailedString()
{
    char format[1024] = {0};
    U32 occr = getOCCR();

    sprintf (format, "0x%012llX, CMD5 OCCR: 0x%06X ", cmdData, occr);

    strcat (format, parse_CMD5_OCR(occr));

    string str(format);
    const char * chr = str.c_str();

    return chr;
}

U32 SdioCmd5::getOCCR()
{
    U32 occr = (U32)CMD5_OCR(cmdData);
    return occr;
}

const char* SdioCmd5Resp::getShortString()
{
    char format[1024] = {0};
    U32 occr = getOCCR();

    sprintf (format, "0x%012llX, CMD5 Resp OCR: 0x%06X", cmdData, occr);

    string str(format);
    const char * chr = str.c_str();

    return chr;
}
const char* SdioCmd5Resp::getDetailedString()
{
    char format[1024] = {0};
    U32 flags;

    sprintf (format, "0x%012llX, CMD5 Rsp Card Ready: %d, Num Functions: %d, Memory Present: %d, OCCR: 0x%06X",
            cmdData, isCardReady(), numIOFunctions(), isMemoryPresent(), getOCCR());


    string str(format);
    const char * chr = str.c_str();

    return chr;
}

