#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDIOParser.h"

using namespace::std;


string* parse_str(U64 val)
{
    ostringstream stream;
    char format[200] = {0};
    //const char* str = "";
    string  *str;
    
    switch(CMD_VAL(val))
    {
        case 0:
            break;
        case 3:
            str = parse_CMD3(val);
            break;
        case 5:
        case 0x3f:
            str = parse_CMD5(val);
            break;
        case 52:
            str = parse_CMD52(val);
            break;
        case 53:
            str = parse_CMD53(val);
            break;
        default:
            break;
    }
    return str;
}
string* parse_CMD52(U64 val)
{
    string* str;
    // if the direction bit indicates command (set)
    if (CMD_DIR(val) == DIR_FROM_HOST)
    {
        str = parse_CMD52_COMMAND (val);
    }
    else
    {
        // direction bit == 0 ,indicates response
        str = parse_CMD52_RESP (val);
    }
    return str;
}

string* parse_CMD52_COMMAND(U64 val)
{
    ostringstream stream;
    char format[200] = {0};

    if(CMD52_RW(val))
    {
        stream << " Write -- ";
    }
    else
    {
        stream << " Read  -- ";
    }

    stream  << "Function: " << CMD52_FUN(val)<<" ";
    stream  << "RAW: " << CMD52_RAW(val)<<" ";

    sprintf(format, "0x%05llX", CMD52_ADDRESS(val));
    stream << "Address: " << format << " ";

    sprintf(format, "0x%02llX", CMD52_DATA(val));
    stream << "Data: " << format << " ";

    string *str = new string (stream.str());
    return str;
    // const char * chr = str.c_str();

    // return chr;
}
string* parse_CMD52_RESP(U64 val)
{
    ostringstream stream;
    char format[200] = {0};
    unsigned char flags = (unsigned char)CMD52_RESP_FLAGS(val);

    stream << " Response ";

    // if any error flags, let's parse them
    if (flags & 0xCF)
    {
        stream << "CMD52_Response ERRORs: ";
        if (flags & 0x80)
        {
            stream << "COM_CRC_ERROR ";
        }
        if (flags & 0x40)
        {
            stream << "ILLEGAL_COMMAND ";
        }
        if (flags & 0x08)
        {
            stream << "General ERROR ";
        }
        if (flags & 0x04)
        {
            stream << "RFU ";
        }
        if (flags & 0x02)
        {
            stream << "FUNCTION_NUMBER ";
        }
        if (flags & 0x01)
        {
            stream << "OUT_OF_RANGE ";
        }
    }
    stream << "Flags: IO_CURRENT_STATE: ";
    switch (flags & 0x30)
    {
        case 0: 
            stream << "DISabled ";
            break;
        case 1: 
            stream << "CMD - executing CMD52 ";
            break;
        case 2: 
            stream << "TRN - Data xfer using Data lines ";
            break;
        case 3: 
            stream << "RFU ";
            break;
    }


    sprintf(format, "0x%02llX", CMD52_RESP_DATA(val));
    stream << "Data: " << format << " ";

    string *str = new string (stream.str());
    return str;
    // const char * chr = str.c_str();

    // return chr;
}

string* parse_CMD53(U64 val)
{
    string* str;
    // if the direction bit indicates command (set)
    if (CMD_DIR(val) == DIR_FROM_HOST)
    {
        str = parse_CMD53_COMMAND (val);
    }
    else
    {
        // direction bit == 0 ,indicates response
        str = parse_CMD53_RESP (val);
    }
    return str;
}
string* parse_CMD53_COMMAND(U64 val)
{
    ostringstream stream;
    char format[200] = {0};

    if(CMD53_RW(val))
    {
        stream << " Write -- ";
    }
    else
    {
        stream << " Read  -- ";
    }

    stream  << "Function: " << CMD53_FUN(val)<<" ";
    stream  << "Block Mode: " << CMD53_BLOCK_MODE(val)<<" ";
    if ( CMD53_BLOCK_MODE(val) == 0)
    {
        stream  << "(transmit in BYTE Mode) ";
    }
    else
    {
        stream  << "(transmit in BLOCK Mode) ";
    }

    stream  << "OP Code: " << CMD53_OP_CODE(val)<<" ";
    if( CMD53_OP_CODE(val) == 0)
    {
        stream  << "(Multi byte R/W to fixed address) ";
    }
    else
    {
        stream  << "(Multi byte R/W to incrementing address) ";
    }

    sprintf(format, "0x%05llX", CMD53_ADDRESS(val));
    stream << "Address: " << format << " ";

    stream  << "Byte/Block Count: " << hex << CMD53_COUNT(val)<<" ";
    if ( CMD53_BLOCK_MODE(val) == 0 )
    {
        // byte mode 
        if (CMD53_COUNT(val) == 0)
        {
            stream  << "(Count: 512 bytes) ";
        }
        else
        {
            stream << "Count: " << CMD53_COUNT(val) << " bytes) ";
        }
    }
    else
    {
        // block mode 
        if (CMD53_COUNT(val) == 0)
        {
            stream  << "(Count: infinite blocks) ";
        }
        else
        {
            stream << "Count: " << CMD53_COUNT(val) << " blocks) ";
        }

    }


    string *str = new string (stream.str());
    return str;
    // const char * chr = str.c_str();

    // return chr;
}
string* parse_CMD53_RESP(U64 val)
{
    return parse_CMD52_RESP(val);
}

string* parse_CMD5(U64 val)
{
    string* str;
    // if the direction bit indicates command (set)
    if (CMD_DIR(val) == DIR_FROM_HOST)
    {
        str = parse_CMD5_COMMAND (val);
    }
    else
    {
        // direction bit == 0 ,indicates response
        str = parse_CMD5_RESP (val);
    }
    return str;
}
string* parse_CMD5_COMMAND(U64 val)
{
    ostringstream stream;
    char format[200] = {0};
    unsigned int ocr = CMD5_OCR(val);

    sprintf(format, "0x%06X", ocr);
    stream << " CMD 5 OCR: " << format << " ";
	string *s = parse_CMD5_OCR(ocr);
    stream << s;
	delete s;

    string *str = new string (stream.str());
    return str;
    // const char * chr = str.c_str();

    // return chr;
}
string* parse_CMD5_RESP(U64 val)
{
    ostringstream stream;
    char format[200] = {0};
    unsigned int ocr = CMD5_RESP_OCR(val);

    stream << " CMD5 Response: ";
    stream << "Card Ready Bit: " << CMD5_RESP_C(val) << ", ";
    stream << "Number of I/O Functions: " << CMD5_RESP_NO_FUN(val) << ", ";
    stream << "Memory Present: " << CMD5_RESP_MEM(val) << " ";

    sprintf(format, "0x%06X", ocr);
    stream << "OCR: " << format << " ";
    stream << parse_CMD5_OCR(ocr);

    string *str = new string (stream.str());
    return str;
    // const char * chr = str.c_str();

    // return chr;
}
string* parse_CMD5_OCR(unsigned int ocr)
{
    char format[1024] = {0};

    sprintf(format, "OCR Values: ");
    if (ocr & 0xff)
    {
        strcat (format, "Error -- OCR is Reserved value, ");
    }
    if (ocr & 0x100)
    {
        strcat (format, "2.0-2.1 Volts, ");
    }
    if (ocr & 0x200)
    {
        strcat (format,"2.1-2.2 Volts, ");
    }
    if (ocr & 0x400)
    {
        strcat (format, "2.2-2.3 Volts, ");
    }
    if (ocr & 0x800)
    {
        strcat (format, "2.3-2.4 Volts, ");
    }
    if (ocr & 0x1000)
    {
        strcat (format, "2.4-2.5 Volts, ");
    }
    if (ocr & 0x2000)
    {
        strcat (format, "2.5-2.6 Volts, ");
    }
    if (ocr & 0x4000)
    {
        strcat (format, "2.6-2.7 Volts, ");
    }
    if (ocr & 0x8000)
    {
        strcat (format, "2.7-2.8 Volts, ");
    }
    if (ocr & 0x10000)
    {
        strcat (format, "2.8-2.9 Volts, ");
    }
    if (ocr & 0x20000)
    {
        strcat (format, "2.9-3.0 Volts, ");
    }
    if (ocr & 0x40000)
    {
        strcat (format, "3.0-3.1 Volts, ");
    }
    if (ocr & 0x80000)
    {
        strcat (format, "3.1-3.2 Volts, ");
    }
    if (ocr & 0x100000)
    {
        strcat (format, "3.2-3.3 Volts, ");
    }
    if (ocr & 0x200000)
    {
        strcat (format, "3.3-3.4 Volts, ");
    }
    if (ocr & 0x400000)
    {
        strcat (format, "3.4-3.5 Volts, ");
    }
    if (ocr & 0x800000)
    {
        strcat (format, "3.5-3.6 Volts, ");
    }

    string *str = new string(format);
    return str;
    //const char * chr = str.c_str();

    //return chr;
}

string* parse_CMD3(U64 val)
{
    string* str;
    // if the direction bit indicates command (set)
    if (CMD_DIR(val) == DIR_FROM_HOST)
    {
        str = parse_CMD3_COMMAND (val);
    }
    else
    {
        // direction bit == 0 ,indicates response
        str = parse_CMD3_RESP (val);
    }
    return str;
}
string* parse_CMD3_COMMAND(U64 val)
{
    ostringstream stream;

    stream << " CMD 3 -- stuff bits only.... ";

    string *str = new string (stream.str());
    return str;
    // const char * chr = str.c_str();

    // return chr;
}
string* parse_CMD3_RESP(U64 val)
{
    ostringstream stream;
    char format[200] = {0};
    unsigned int status = CMD3_RESP_STATUS(val);

    stream << " CMD3 Response: ";
    sprintf(format, "RCA: 0x%04llX", CMD3_RESP_RCA(val));
    stream << format << ", ";

    sprintf(format, "0x%04X", status);
    stream << "Card status: " << format << ": ";

    // now do the status bits.  Per the spec, for cmd3 response, the bits
    // are mapped: 23, 22, 19, 12-0, of the status bit table 4-36 in the physical
    // layer specification.  
    // However, per the SDIO spec, bits 12-0 should be only 0 for SDIO cards
    
    // mapped to bit 23
    if (status & 0x8000)
    {
        stream << "COM_CRC_ERROR, ";
    }
    // mapped to bit 22
    if (status & 0x4000)
    {
        stream << "ILLEGAL_COMMAND, ";
    }
    // mapped to bit 19
    if (status & 0x2000)
    {
        stream << "ERROR, ";
    }
    // verify that bits 12-0 are 0 for SDIO
    if (status & 0x1fff)
    {
        stream << "ERROR -- status bits 12:0 should be all 0, but something is set ";
    }
    
    string *str = new string (stream.str());
    return str;
    // const char * chr = str.c_str();

    // return chr;
}
