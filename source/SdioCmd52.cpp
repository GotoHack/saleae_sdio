#include "SdioCmd52.h"
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

const char * SdioCmd52::getShortString()
{
    char format[1024] = {0};
    char dummy[1024] = {0};

    sprintf(format, "0x%012llX, CMD52 ", cmdData);
    if (getRead()) strcat (format, "R: ");
    else strcat (format, "W: ");

    sprintf (dummy, "0x%02X ", getData());
    strcat (format, dummy);


    if (getRead()) sprintf (dummy, "from: 0x%05X", getRegisterAddress());
    if (getWrite()) sprintf (dummy, " to: 0x%05X", getRegisterAddress());
    strcat(format, dummy);

    string str(format);
    const char * chr = str.c_str();

    return chr;
}

const char * SdioCmd52::getDetailedString()
{
    char format[1024] = {0};
    char dummy[1024] = {0};

    sprintf(format, "0x%012llX, CMD52 Function %d ", cmdData, getFunctionNumber());
    if (getRead()) strcat (format, "READ ");
    else strcat (format, "WRITE ");


    sprintf(dummy, "0x%02X ", getData());
    strcat (format, (const char*)dummy);

    if (getRead()) strcat (format, "from ");
    if (getWrite()) strcat (format, "to ");
    sprintf(dummy, "0x%05X ", getRegisterAddress());
    strcat (format, (const char*)dummy);

    if (isReadAfterWrite()) strcat (format, "-- RAW is set");
    else strcat (format, "-- RAW is not set");


    string str (format);
    const char * chr = str.c_str();

    return chr;
}
bool SdioCmd52::getRead()
{
    bool retVal = true;
    if (CMD52_RW(cmdData))
    {
        retVal = false;
    }
    return retVal;
}

bool SdioCmd52::getWrite()
{
    return !getRead();
}

U32 SdioCmd52::getFunctionNumber()
{
    U8 fun = (U8)CMD52_FUN(cmdData);
}

bool SdioCmd52::isReadAfterWrite()
{
    bool retVal = false;
    if (CMD52_RAW(cmdData))
    {
        retVal = true;
    }
    return retVal;
}


U32 SdioCmd52::getRegisterAddress()
{
    U32 address = CMD52_ADDRESS(cmdData);
    return address;
}

U32 SdioCmd52::getData()
{
    U32 data = (U32)CMD52_DATA(cmdData);
    return data;
}


const char* SdioCmd52Resp::getShortString()
{
    char format[1024] = {0};


    sprintf (format, "0x%012llX, CMD52 Resp: 0x%02X", cmdData, getData());


    string str(format);
    const char * chr = str.c_str();

    return chr;
}
const char* SdioCmd52Resp::getDetailedString()
{
    char format[1024] = {0};
    char dummy[1024] = {0};
    U32 flags;

    sprintf (format, "0x%012llX, CMD52 Rsp Data: 0x%02X ", cmdData, getData());

    flags = getResponseBitFlags();
    sprintf (dummy, "Flags: 0x%02X:", flags);
    strcat(format, dummy);

    if (flags & 0x80) strcat (format, "COM_CRC_ERROR:");
    if (flags & 0x40) strcat (format, "ILLEGAL_COMMAND:");

    switch (flags & 0x30)
    {
        case 0x00:
            strcat (format, "IO_CURRENT_STATE - DIS:");
            break;
        case 0x10:
            strcat (format, "IO_CURRENT_STATE - CMD:");
            break;
        case 0x20:
            strcat (format, "IO_CURRENT_STATE - TRN:");
            break;
        case 0x30:
            strcat (format, "IO_CURRENT_STATE - RFU:");
            break;
        case 0x08:
            strcat (format, "ERROR:");
            break;
        case 0x02:
            strcat (format, "FUNCTION_NUMBER invalid:");
            break;
        case 0x01:
            strcat (format, "OUT_OF_RANGE:");
            break;
    }

    

    string str(format);
    const char * chr = str.c_str();

    return chr;
}

U32 SdioCmd52Resp::getResponseBitFlags()
{
    U32 flags = (U32)CMD52_RESP_FLAGS(cmdData);
    return flags;
}
U32 SdioCmd52Resp::getData()
{
    U32  data = (U32)CMD52_RESP_DATA(cmdData);
    return (U8)data;
}
