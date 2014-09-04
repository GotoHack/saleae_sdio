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


string* SdioCmd5::getDetailedString()
{
    char format[1024] = {0};
    U32 occr = getOCCR();

    sprintf (format, "0x%012llX, CMD5 OCCR: 0x%06X ", cmdData, occr);

	string *s = parse_CMD5_OCR(occr);
	strcat (format, s->c_str());
	delete s;


    string* str = new string(format);
    //const char * chr = str.c_str();
    //return chr;
    return str;
}

U32 SdioCmd5::getOCCR()
{
    U32 occr = (U32)CMD5_OCR(cmdData);
    return occr;
}

string* SdioCmd5Resp::getShortString()
{
    char format[1024] = {0};
    U32 occr = getOCCR();

    sprintf (format, "0x%012llX, CMD5 Resp OCR: 0x%06X", cmdData, occr);

    string *str = new string(format);
    // const char * chr = str.c_str();
    // return chr;
    return str;
}
string* SdioCmd5Resp::getDetailedString()
{
    char format[1024] = {0};

    sprintf (format, "0x%012llX, CMD5 Rsp Card Ready: %d, Num Functions: %d, Memory Present: %d, OCCR: 0x%06X",
            cmdData, isCardReady(), numIOFunctions(), isMemoryPresent(), getOCCR());


    string *str = new string(format);
    // const char * chr = str.c_str();
    // return chr;
    return str;
}

