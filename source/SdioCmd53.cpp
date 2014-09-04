#include "SdioCmd53.h"
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

string* SdioCmd53::getShortString()
{
    char dummy[1024] = {0};
    char format[1024] = {0};

    sprintf (format, "0x%012llX, CMD53 ", cmdData);
    if (getRead()) strcat (format, "R:");
    else strcat (format, "W:");

    sprintf (dummy, "0x%02X ", getXferCount());
    strcat (format, dummy);

    if (isBlockMode()) strcat (format, " Blks");
    else strcat (format, " Bytes");

    string *str = new string(format);
    // const char * chr = str.c_str();
    // return chr;
    return str;
}

string* SdioCmd53::getDetailedString()
{
    char dummy[1024] = {0};
    char format[1024] = {0};

    sprintf (format, "0x%012llX, CMD53 Function: %d ", cmdData, getFunctionNumber());
    if (getRead()) strcat (format, "Read, ");
    else strcat (format, "Write, ");

    sprintf (dummy, "0x%03X ", getXferCount());
    strcat (format, dummy);

    if (isBlockMode()) strcat (format,"Blocks, ");
    else strcat (format, "Bytes, ");

    sprintf (dummy, "From Address: 0x%05X, ", getRegisterAddress());
    strcat (format, dummy);

    if (isIncrementingAddress()) strcat (format, "using Incrementing Addresses");
    else strcat (format, "using Fixed Addressing");

    string *str = new string(format);
    // const char * chr = str.c_str();
    // return chr;
    return str;
}
bool SdioCmd53::getRead()
{
    bool retVal = true;
    if (CMD53_RW(cmdData))
    {
        retVal = false;
    }
    return retVal;
}

bool SdioCmd53::getWrite()
{
    return !getRead();
}

U32 SdioCmd53::getFunctionNumber()
{
    U32 fun = (U32)CMD53_FUN(cmdData);
	return fun;
}

bool SdioCmd53::isBlockMode()
{
    bool retVal = false;
    if (CMD53_BLOCK_MODE(cmdData))
    {
        retVal = true;
    }
    return retVal;
}
bool SdioCmd53::isByteMode()
{
    return !isBlockMode();
}
bool SdioCmd53::isIncrementingAddress()
{
    bool retVal = false;
    if (CMD53_OP_CODE(cmdData))
    {
        retVal = true;
    }
    return retVal;
}
bool SdioCmd53::isFixedAddress()
{
    return !isIncrementingAddress();
}

U32 SdioCmd53::getRegisterAddress()
{
    U32 address = CMD53_ADDRESS(cmdData);
    return address;
}

U32 SdioCmd53::getXferCount()
{
    U32 count = (U32)CMD53_COUNT(cmdData);
    return count;
}

string* SdioCmd53Resp::getShortString()
{
    char format[200] = {0};

    sprintf(format, "0x%012llX, CMD53 Resp", cmdData);

    string *str = new string(format);
    // const char * chr = str.c_str();
    // return chr;
    return str;
}
string* SdioCmd53Resp::getDetailedString()
{
    char dummy[1024] = {0};
    char format[1024] = {0};
    U32 flags;

    sprintf (format, "0x%012llX, CMD53 Rsp Data: 0x%02X ", cmdData, getData());

    flags = getResponseBitFlags();

    sprintf (dummy, "Flags: 0x%02X:",flags);
    strcat (format, dummy);

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
    

    string *str = new string(format);
    // const char * chr = str.c_str();
    // return chr;
    return str;
}
