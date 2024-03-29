#include "CIA.h"
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
#include <list>
#include <iterator>

using namespace std;

const char *CCCR_NAMES[] = 
{
    "0x00 CCCR/SDIO Revision           |SDIO|SDIO|SDIO|SDIO | CCCR|CCCR|CCCR|CCCR|    ",
    "0x01 SD Specification Revision    |RFU |RFU |RFU |RFU  |  SD | SD | SD | SD |    ",
    "0x02 I/O Enable                   |IOE7|IOE6|IOE5|IOE4 | IOE3|IOE2|IOE1|RFU |    ",
    "0x03 I/O Ready                    |IOR7|IOR6|IOR5|IOR4 | IOR3|IOR2|IOR1|RFU |    ",
    "0x04 Int Enable                   |IEN7|IEN6|IEN5|IEN4 | IEN3|IEN2|IEN1|IENM|    ",
    "0x05 Int Pending                  |INT7|INT6|INT5|INT4 | INT3|INT2|INT1|RFU |    ",
    "0x06 I/O Abort                    |RFU |RFU |RFU |RFU  | RES |AS2 |AS1 |AS0 |    ",
    "0x07 Bus Interface Control        |CD d|SCSI|ECSI|RFU  | RFU |RFU |BW 1|BW 0|    ",
    "0x08 Card Capability              |4BLS|LSC |E4MI|S4MI | SBS |SRW |SMB |SDC |    ",
    "0x09 Common CIS Ptr [0]           |    |    |    |     |     |    |    |    |    ",
    "0x0A Common CIS Ptr [1]           |    |    |    |     |     |    |    |    |    ",
    "0x0B Common CIS Ptr [2]           |    |    |    |     |     |    |    |    |    ",
    "0x0C Bus Suspend                  |RFU |RFU |RFU |RFU  | RFU |RFU | BR | BS |    ",
    "0x0D Function Select              | DF |RFU |RFU |RFU  | FS3 |FS2 |FS1 |FS1 |    ",
    "0x0E Exec Flags                   |EX7 |EX6 |EX5 |EX4  | EX3 |EX2 |EX1 |EXM |    ",
    "0x0F Ready Flags                  |RF7 |RF6 |RF5 |RF4  | RF3 |RF2 |RF1 |RFM |    ",
    "0x10 FN0 Block Size [0]           |    |    |    |     |     |    |    |    |    ",
    "0x11 FN0 Block Size [1]           |    |    |    |     |     |    |    |    |    ",
    "0x12 Power Control                |RFU |RFU |RFU |RFU  | RFU |RFU |EMPC|SMPC|    ",
    "0x13 High Speed                   |RFU |RFU |RFU |RFU  | RFU |RFU |EHS |SHS |    "
};

const char* FBR_NAMES[] = 
{
    "0x00 CSA Support/FN Intrf Code    |CSAE|CSAS|RFU |RFU  | Func Interface Code|    ",
    "0x01 Extended Intf Code           |    |    |    |     |     |    |    |    |    ",
    "0x02 Power Selection              |RFU |RFU |RFU |RFU  | RFU |RFU |EPS |SPS |    ",
    "0x03 RFU                          |RFU |RFU |RFU |RFU  | RFU |RFU |RFU |RFU |    ",
    "0x04 RFU                          |RFU |RFU |RFU |RFU  | RFU |RFU |RFU |RFU |    ",
    "0x05 RFU                          |RFU |RFU |RFU |RFU  | RFU |RFU |RFU |RFU |    ",
    "0x06 RFU                          |RFU |RFU |RFU |RFU  | RFU |RFU |RFU |RFU |    ",
    "0x07 RFU                          |RFU |RFU |RFU |RFU  | RFU |RFU |RFU |RFU |    ",
    "0x08 RFU                          |RFU |RFU |RFU |RFU  | RFU |RFU |RFU |RFU |    ",
    "0x09 CIS PTR [0]                  |    |    |    |     |     |    |    |    |    ",
    "0x0A CIS PTR [1]                  |    |    |    |     |     |    |    |    |    ",
    "0x0B CIS PTR [2]                  |    |    |    |     |     |    |    |    |    ",
    "0x0C CSA PTR [0]                  |    |    |    |     |     |    |    |    |    ",
    "0x0D CSA PTR [1]                  |    |    |    |     |     |    |    |    |    ",
    "0x0E CSA PTR [2]                  |    |    |    |     |     |    |    |    |    ",
    "0x0F Data Access Window (CSA)     |    |    |    |     |     |    |    |    |    ",
    "0x10 I/O Block Size [0]           |    |    |    |     |     |    |    |    |    ",
    "0x11 I/O Block Size [1]           |    |    |    |     |     |    |    |    |    ",
};

CCCR* CCCR::theCCCR = 0;

CCCR::CCCR() 
    :lastHostCmd52(0) ,
    cccrDataPopulated(false),
    tupleChain()
{
    int i;
    memset(&cccr_data, 0, sizeof(CCCR_t));
    cccr_data.Common_CIS_Pointer[0] = (CIA_PTR_INIT_VAL ) & 0xff;
    cccr_data.Common_CIS_Pointer[1] = (CIA_PTR_INIT_VAL >> 8) & 0xff;
    cccr_data.Common_CIS_Pointer[2] = (CIA_PTR_INIT_VAL >> 16) & 0xff;

    for (i = 0; i < 7; i++)
    {
        fbr[i] = FBR(i+1);
    }
}
void CCCR::CleanupCCCR()
{
    if (theCCCR != 0)
    {
        if (theCCCR->lastHostCmd52 != 0)
        {
            delete theCCCR->lastHostCmd52;
        }
        theCCCR->cccrDataPopulated = false;
        theCCCR->tupleChain = TupleChain();

        int i;
        memset(&theCCCR->cccr_data, 0, sizeof(CCCR_t));
        theCCCR->cccr_data.Common_CIS_Pointer[0] = (CIA_PTR_INIT_VAL ) & 0xff;
        theCCCR->cccr_data.Common_CIS_Pointer[1] = (CIA_PTR_INIT_VAL >> 8) & 0xff;
        theCCCR->cccr_data.Common_CIS_Pointer[2] = (CIA_PTR_INIT_VAL >> 16) & 0xff;

        for (i = 0; i < 7; i++)
        {
            theCCCR->fbr[i] = FBR(i+1);
        }
    }
}

CCCR* CCCR::BuildCCCR(U64 data)
{
    if (theCCCR == 0)
    {
        theCCCR = new CCCR();
    }
    theCCCR->AddCmd52ToCCCR(data);
    return theCCCR;
}

bool CCCR::AddCmd52ToCCCR(U64 data)
{
    bool retVal = false;
    if (theCCCR != 0)
    {
        // first check if this is cmd 52
        if (CMD_VAL(data) == 52)
        {
            if (CMD_DIR(data) == DIR_FROM_HOST)
            {
                retVal = theCCCR->HandleCmd52Request(data);
            }
            else
            {
                retVal = theCCCR->HandleCmd52Response(data);
            }
        }
    }
    return retVal;

}
bool CCCR::HandleCmd52Request(U64 data)
{
    SdioCmd52 *c52 = new SdioCmd52(data);
    U32 address = c52->getRegisterAddress();
    
    // first clean up our last host CMD 52
    if (lastHostCmd52 != 0)
    {
        delete lastHostCmd52;
        lastHostCmd52 = 0;
    }
    // make sure we are using funtion 0
    // actaully, scratch that, we pretty much stuff every CMD52 request into the lastHostCmd52
    // so long as its tied to Function 0
    // leaving logic here to remind me of documentation
    if (c52->getFunctionNumber() == 0)
    {
        // make sure the register address is within CCCR bounds
        if (address >= CCCR_ADDRESS_START && address <= CCCR_ADDRESS_END)
        {
            lastHostCmd52 = c52;
        }
        // check if this is an FBR.  
        // they are addressed from 0x100--0x1FF for F1, 0x200--0x2ff for F2, etc
        else if (address >= 0x100 && address <= 0x7ff)
        {
            lastHostCmd52 = c52;
        }
        else
        {
            U32 tmp = getCisAddress();
            lastHostCmd52 = c52;
        }
    }
    else
    {
        cout <<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        cout <<"!!!!!!!!!           CMD52 for something other F0         !!!!!!!!!!!!!!!!!!!!!!!\n";
        cout <<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    }


    return true;
}
bool CCCR::HandleCmd52Response(U64 data)
{
    SdioCmd52Resp *c52Resp = new SdioCmd52Resp(data);
    U32 regAddress = 0;
    U32 c_data = 0;
    U8 *cccrData_ptr = (U8*)&cccr_data;
    U32 fbrOffset = 0;
    U8 *fbrData_ptr = 0;
    int i;
    
    if (lastHostCmd52 != 0)
    {
        // handle function 0 stuff
        // Actaully all lastHostCmd52's should be Function 0, but 
        // just make sure
        if (lastHostCmd52->getFunctionNumber() == 0)
        {
            regAddress = lastHostCmd52->getRegisterAddress();
            c_data = (U8)c52Resp->getData();

            if (regAddress >= CCCR_ADDRESS_START && regAddress <= CCCR_ADDRESS_END )
            {
                // indicate to the dumper, that CCCR data has been read/written,
                // that way we don't dump uninitialized garbage
                cccrDataPopulated = true;
                cccrData_ptr[regAddress] = c_data;
            }
        }
        // see if this is the common CIS within the CIS
        if (regAddress >= CIS_AREA_START && regAddress <= CIS_AREA_END)
        {
            tupleChain.setCisAddress(getCisAddress());
            tupleChain.setCisTupleFlag();
            tupleChain.addDataToTuple(data);
        }
        // try to add to all FBR's, they will check addressing, and recursively add the 
        // CIS and CSA pointer addresses
        for (i = 0; i < 7; i++)
        {
            fbr[i].addData(data);
        }

        delete lastHostCmd52;
        lastHostCmd52 = 0;
    }

    delete c52Resp;
    return true;
}

void CCCR::DumpCCCRTable(void)
{
    CCCR::DumpCCCRTable(cout);
}
void CCCR::DumpCCCRTable(std::ostream &stream)
// static function -- available to anyone
{
    U32 i;
    U8* tmp = 0;
    char buffer[400] = {0};

    // since this is static, we cannot do anything unless we have a CCCR
    if (theCCCR != 0)
    {

        tmp = (U8*)&(theCCCR->cccr_data);
        if (theCCCR->cccrDataPopulated)
        {
            stream << endl << "CCCR TABLE  Address range: 0x0000--0x00FF  (All multi-byte values are Little Endian)" << endl;
            stream << "=========================================================================================================" << endl;
            for (i = 0; i < NUM_CCCR_ELEMENTS; i++)
            {
                // sprintf(buffer, "ADDR 0x%02X: 0x%02X -- " PRINTF_BIT_PATTERN,
                //         i, tmp[i], PRINTF_BIT(tmp[i]));
                sprintf(buffer, "%s 0x%02X -- " PRINTF_BIT_PATTERN, CCCR_NAMES[i], tmp[i], PRINTF_BIT(tmp[i]));

                stream << buffer << endl;
            }
            stream << "=========================================================================================================" << endl;
            theCCCR->tupleChain.dump(stream);
        }
    }
}

// static function
void CCCR::DumpFBRTable(void)
{
    CCCR::DumpFBRTable(cout);
}
void CCCR::DumpFBRTable(std::ostream &stream)
{
    U32 funcIndex;
    char buffer[400] = {0};

    if (theCCCR != 0)
    {
        for (funcIndex = 0; funcIndex < 7; funcIndex++)
        {
            theCCCR->fbr[funcIndex].DumpFBR(stream);
        }
    }
}

U32 CCCR::getCisAddress()
{
    U32 address = 0;
    if ( ( cccr_data.Common_CIS_Pointer[0] == (CIA_PTR_INIT_VAL ) & 0xff)       ||
         ( cccr_data.Common_CIS_Pointer[1] == (CIA_PTR_INIT_VAL >> 8) & 0xff)   ||
         ( cccr_data.Common_CIS_Pointer[2] == (CIA_PTR_INIT_VAL >> 16) & 0xff)  )
    {
        address = CIA_PTR_INIT_VAL;
    }
    else
    {
        address = (cccr_data.Common_CIS_Pointer[0] << 0) | (cccr_data.Common_CIS_Pointer[1] << 8) | (cccr_data.Common_CIS_Pointer[2] << 16);
    }
    return address;
}

void CCCR::TupleChain::setCisAddress(U32 address)
{
    if ((address != CIA_PTR_INIT_VAL) && (address >= CIS_AREA_START) && (address <= CIS_AREA_END))
    {
        cisAddress = address;
    }
}
void CCCR::TupleChain::addDataToTuple(U64 data)
{
    SdioCmd52 *c52 = theCCCR->lastHostCmd52;
    SdioCmd52Resp *c52Resp = new SdioCmd52Resp(data);
    U32 regAddress = 0;
    U32 c_data = 0;
    list<TUPLE>::iterator it = tuples.end();
    TUPLE tuple;

    regAddress = c52->getRegisterAddress();
    c_data = (U8)c52Resp->getData();

    if (tupleComplete == false)
    {
        if (regAddress == cisAddress)
        {
            // we are building the CIS now, this is the first step.  we need to extract data and setup our
            // end addresses for the range checking, etc.
            newTuplePending = true;
            tuple = TUPLE(regAddress);
            tuple.setTplCode(c_data);
            tuples.push_back(tuple);

        }
        else if (regAddress == lastTupleAddress)
        {
            tuple = TUPLE(regAddress);
            tuple.setTplCode(c_data);

            if (c_data == 0xff)
            {
                // done with tuple walk
                tuple.setSize(0);
                newTuplePending = false;
                tupleComplete = true;
            }
            else
            {
                newTuplePending = true;
            }
            tuples.push_back(tuple);
        }
        else if ((regAddress < lastTupleAddress) && (regAddress > cisAddress))
        {
            //TUPLE t = tuples.front();
            it--;
            // examine in gdb using: plist it->body int, after the next call
            it->addData(c_data);
        }
        else
        {
            if (newTuplePending)
            {
                it--;
                it->setSize(c_data);
                newTuplePending = false;
                lastTupleAddress = regAddress + c_data + 1;
            }
        }
    }
    else
    {
        // in this case, we need to merely update our tuple
        if (regAddress >= cisAddress && regAddress <= lastTupleAddress)
        {
            bool stop = false;
            // we need to go through our tuple chain to update
            it = tuples.begin();

            while (stop == false && it != tuples.end())
            {
                // if this is the tuple code, verify it
                if (regAddress == it->address)
                {
                    // we have a matching address w/ existing tuple
                    // if the data doesn't match, just stop
                    if (it->tplCode != c_data)
                    {
                        // don't update, just freak out
                        stop = true;
                    }
					stop = true;
                }

                // if this is the tuple size, verify it as best we can
                else if (regAddress == it->address + 1)
                {
                    if (it->size != c_data)
                    {
                        // don't update, just freak out
                        stop = true;
                    }
					stop = true;
                }

                if ((regAddress >= it->address + 2) && (regAddress <= (it->address + it->size + 1)))
                {
                    list<U32>::iterator dataIt = it->body.begin();
					// the tuple only has the data in the list, thus the first 2 bytes are encapsulated 
					// in the class itself, so we have to work our way around it.  upon doing this,
					// it seems not that optimal, but i'm tired of messing with this 
                    int k = regAddress - (it->address + 2);
                    int j;

                    for (j = 0; j < k; j++)
                    {
                        dataIt++;
                    }
                    *dataIt = c_data;

                    stop = true;
                }
                it++;
            }
        }
    }

}

void CCCR::TupleChain::dump()
{
    dump(cout);
}
void CCCR::TupleChain::dump(std::ostream &stream)
{
    stream << "Tuples for address: 0x" << hex << cisAddress <<"   (All multi-byte values are Little Endian)"<< endl;
    stream << "================================================================================" << endl;
    list<TUPLE>::iterator tupleIterator;

    for (tupleIterator = tuples.begin(); tupleIterator != tuples.end(); tupleIterator++)
    {
        tupleIterator->dump(stream, cisTupleFlag);
    }

}

CCCR::FBR::FBR(U32 number)
: functionNumber(number),
    fbrDataPopulated(false),
    tupleChain()
{
    memset(&fbr_data, 0, sizeof (FBR_t));
    fbr_data.CIS_ptr[0] = (CIA_PTR_INIT_VAL ) & 0xff;
    fbr_data.CIS_ptr[1] = (CIA_PTR_INIT_VAL >> 8) & 0xff;
    fbr_data.CIS_ptr[2] = (CIA_PTR_INIT_VAL >> 16) & 0xff;

    fbr_data.CSA_ptr[0] = (CIA_PTR_INIT_VAL ) & 0xff;
    fbr_data.CSA_ptr[1] = (CIA_PTR_INIT_VAL >> 8) & 0xff;
    fbr_data.CSA_ptr[2] = (CIA_PTR_INIT_VAL >> 16) & 0xff;
}

void CCCR::FBR::DumpFBR()
{
    DumpFBR(cout);
}
void CCCR::FBR::DumpFBR(std::ostream &stream)
{
    U32 i,funcNo;
    U8* tmp = (U8*)&fbr_data;
    char buffer[400] = {0};

    if (fbrDataPopulated)
    {
        funcNo = functionNumber;
        tmp = (U8*)&fbr_data;

        stream << endl << "FBR TABLE for Function " << funcNo << "  Address range: 0x0"<< funcNo <<"00--0x0" << funcNo <<"FF  (All multi-byte values are Little Endian)" << endl;
        stream << "=========================================================================================================" << endl;
        for (i = 0; i < NUM_FBR_ELEMENTS; i++)
        {
            sprintf(buffer, "%s 0x%02X -- " PRINTF_BIT_PATTERN, FBR_NAMES[i], tmp[i], PRINTF_BIT(tmp[i]));

            stream << buffer << endl;
        }
        stream << "=========================================================================================================" << endl;
        stream << "CIS Address is 0x:" << getCisAddress() << endl;
        tupleChain.dump(stream);
    }
}

void CCCR::FBR::addData(U64 data)
{
    SdioCmd52 *c52 = theCCCR->lastHostCmd52;
    SdioCmd52Resp *c52Resp = new SdioCmd52Resp(data);
    U32 regAddress = 0;
    U32 c_data = 0;
    U8 *fbrData_ptr = 0;
    U32 reqFunctionNumber = 0;
    U32 cisAddress = 0;

    regAddress = c52->getRegisterAddress();
    c_data = (U8)c52Resp->getData();

    // first see if this is a part of the FBR itself, i.e. addressed within
    // the range of this FBR
    if (regAddress >= FBR_ADDRESS_START && regAddress <= FBR_ADDRESS_END)
    {
        reqFunctionNumber = (regAddress >> 8);
        if (reqFunctionNumber == functionNumber)
        {
            fbrData_ptr = (U8*)&fbr_data;
            fbrData_ptr [regAddress & 0xff] = c_data;

            // indicate that the fbr for this function is now populated
            fbrDataPopulated = true;
        }
    }

    // Now, we need to check if this falls into the CIS portion of the FBR.
    // The FBR contains a ptr to the CIS, and this is our jumping off point.
    // we maintain the initial and the last address to use for our range checkers
    cisAddress = getCisAddress();

    // make sure CIS Address has been set to a value, and not the initialization value
    // if this is the case, try to populate
    if ((cisAddress != CIA_PTR_INIT_VAL) && (cisAddress >= CIS_AREA_START) && (cisAddress <= CIS_AREA_END))
    {
        tupleChain.setCisAddress(cisAddress);
        tupleChain.addDataToTuple(data);
    }

}
U32 CCCR::FBR::getCisAddress(void)
{
    U32 address = 0;
    if ( ( fbr_data.CIS_ptr[0] == (CIA_PTR_INIT_VAL ) & 0xff)       ||
         ( fbr_data.CIS_ptr[1] == (CIA_PTR_INIT_VAL >> 8) & 0xff)   ||
         ( fbr_data.CIS_ptr[2] == (CIA_PTR_INIT_VAL >> 16) & 0xff)  )
    {
        address = CIA_PTR_INIT_VAL;
    }
    else
    {
        address = (fbr_data.CIS_ptr[0] << 0) | (fbr_data.CIS_ptr[1] << 8) | (fbr_data.CIS_ptr[2] << 16);
    }
    return address;
}

TUPLE::TUPLE(U32 addr)
:address(addr), 
    tplCode(0), 
    size(0), 
    body() 
{
    tupleNames[0x00]= string("CISTPL_NULL");
    tupleNames[0x10]= string("CISTPL_CHECKSUM");
    tupleNames[0x15]= string("CISTPL_VERS_1");
    tupleNames[0x16]= string("CISTPL_ALTSTR");
    tupleNames[0x20]= string("CISTPL_MANFID");
    tupleNames[0x21]= string("CISTPL_FUNCID");
    tupleNames[0x22]= string("CISTPL_FUNCE");
    tupleNames[0x91]= string("CISTPL_SDIO_STD");
    tupleNames[0x92]= string("CISTPL_SDIO_EXT");
    tupleNames[0xFF]= string("CISTPL_END");
}

void TUPLE::dump(bool cisTupleFlag)
{
    dump(cout, cisTupleFlag);
}
void TUPLE::dump(std::ostream &stream, bool cisTupleFlag)
{
    //list<U32>::iterator it = body.begin();
    int i = 0;
    stream << "\tTuple: 0x" << setw(2) << setfill('0') << hex << tplCode 
        << ", located at 0x" << setw(4) << setfill('0') << hex << address 
        << ", size: 0x" << setw(4) << setfill('0') << hex << size << endl;
    stream << "\t=================================================" << endl;


    // print the TUPLE ID and name
    stream << "\t0x" << setw(4) << setfill('0') << hex << (address + i) <<"\t" <<
        setw(2) << setfill('0') << hex << "(0x" << i << ")" << "\t\t" <<
        setw(2) << setfill('0') << tplCode << "\t\t" << tupleNames[tplCode] << endl;
    i++;

    // print the TPL_LINK (i.e. the nex tupl number)
    stream << "\t0x" << setw(4) << setfill('0') << hex << (address + i) <<"\t" <<
        setw(2) << setfill('0') << hex << "(0x" << i << ")" << "\t\t" <<
        setw(2) << setfill('0') << size << endl;
    i++;

    // now dump the body
    dumpBody(stream, cisTupleFlag, i);
    // // now dump the body
    // for (it = body.begin(); it != body.end(); it++)
    // {
    //     stream << "\t0x" << setw(4) << setfill('0') << hex << (address + i) <<"\t" <<
    //         setw(2) << setfill('0') << hex << "(0x" << i << ")" << "\t\t" <<
    //         setw(2) << setfill('0') << *it << endl;
    //     i++;
    // }
    stream << "\t=================================================" << endl;
}
void TUPLE::dumpBody(std::ostream &stream, bool cisTupleFlag, int i )
{
    switch (tplCode)
    {
        case CISTPL_MANFID:
            dumpBody_MANFID(stream, cisTupleFlag, i);
            break;
        case CISTPL_FUNCID:
            dumpBody_FUNCID(stream, cisTupleFlag, i);
            break;
        case CISTPL_FUNCE:
            dumpBody_FUNCE(stream, cisTupleFlag, i);
            break;
        default:
            dumpBody_GENERIC(stream, cisTupleFlag, i, 0, 0);
            break;
    }
}
void TUPLE::dumpBody_MANFID(std::ostream &stream, bool cisTupleFlag, int i)
{
    int j = 0;
    int maxJ = 4;
    char* strings[] = {
        "\t\tSDIO Card Manufacturer Code",
        "\t\tSDIO Card Manufacturer Code",
        "\t\tSDIO Card Manufacturer Information (Part#/Rev)",
        "\t\tSDIO Card Manufacturer Information (Part#/Rev)",
    };

    dumpBody_GENERIC(stream, cisTupleFlag, i, strings, maxJ);
}
void TUPLE::dumpBody_FUNCID(std::ostream &stream, bool cisTupleFlag, int i)
{
    int j = 0;
    int maxJ = 2;
    char* strings[] = {
        "\t\tTPLFID_FUNCTION Card Function Code (should be 0x0C per spec)",
        "\t\tTPLFID_SYSINIT bit mask (not used, should be 0x00)",
    };

    dumpBody_GENERIC(stream, cisTupleFlag, i, strings, maxJ);
}
void TUPLE::dumpBody_FUNCE(std::ostream &stream, bool cisTupleFlag, int i)
{
    int j = 0;
    int maxCisStrings = 4;
    char* cisStrings[] = {
        "\t\tTPLFE_TYPE of extended data (0x00)",
        "\t\tTPLFE_FN_0_BLK_SIZE (low byte)",
        "\t\tTPLFE_FN_0_BLK_SIZE (hi byte)",
        "\t\tTPLFE_MAX_TRAN_SPEED",
    };
    int maxFbrStrings = 42;
    char* fbrStrings[] = {
        "\t\tTPLFE_TYPE of extended data (0x01)",
        "\t\tTPLFE_FUNCTION_INFO",
        "\t\tTPLFE_STD_IO_REV",
        "\t\tTPLFE_CARD_PSN (low)",
        "\t\tTPLFE_CARD_PSN (mid)",
        "\t\tTPLFE_CARD_PSN (mid)",
        "\t\tTPLFE_CARD_PSN (hi)",
        "\t\tTPLFE_CSA_SIZE (low)",
        "\t\tTPLFE_CSA_SIZE (mid)",
        "\t\tTPLFE_CSA_SIZE (mid)",
        "\t\tTPLFE_CSA_SIZE (hi)",
        "\t\tTPLFE_CSA_PROPERTY",
        "\t\tTPLFE_MAX_BLK_SIZE (low)",
        "\t\tTPLFE_MAX_BLK_SIZE (hi)",
        "\t\tTPLFE_OCR (low)",
        "\t\tTPLFE_OCR (mid)",
        "\t\tTPLFE_OCR (mid)",
        "\t\tTPLFE_OCR (hi)",
        "\t\tTPLFE_OP_MIN_PWR",
        "\t\tTPLFE_OP_AVG_PWR",
        "\t\tTPLFE_OP_MAX_PWR",
        "\t\tTPLFE_SB_MIN_PWR",
        "\t\tTPLFE_SB_AVG_PWR",
        "\t\tTPLFE_SB_MAX_PWR",
        "\t\tTPLFE_MIN_BW (low)",
        "\t\tTPLFE_MIN_BW (hi)",
        "\t\tTPLFE_OPT_BW (low)",
        "\t\tTPLFE_OPT_BW (hi)",
        "\t\tTPLFE_ENABLE_TIMEOUT_VAL (low)",
        "\t\tTPLFE_ENABLE_TIMEOUT_VAL (hi)",
        "\t\tTPLFE_SP_AVG_PWR_3.3V (low)",
        "\t\tTPLFE_SP_AVG_PWR_3.3V (hi)",
        "\t\tTPLFE_SP_MAX_PWR_3.3V (low)",
        "\t\tTPLFE_SP_MAX_PWR_3.3V (hi)",
        "\t\tTPLFE_HP_AVG_PWR_3.3V (low)",
        "\t\tTPLFE_HP_AVG_PWR_3.3V (hi)",
        "\t\tTPLFE_HP_MAX_PWR_3.3V (low)",
        "\t\tTPLFE_HP_MAX_PWR_3.3V (hi)",
        "\t\tTPLFE_LP_AVG_PWR_3.3V (low)",
        "\t\tTPLFE_LP_AVG_PWR_3.3V (hi)",
        "\t\tTPLFE_LP_MAX_PWR_3.3V (low)",
        "\t\tTPLFE_LP_MAX_PWR_3.3V (hi)",
    };

    if (cisTupleFlag)
    {
        dumpBody_GENERIC(stream, cisTupleFlag, i, cisStrings, maxCisStrings);
    }
    else
    {
        dumpBody_GENERIC(stream, cisTupleFlag, i, fbrStrings, maxFbrStrings);
    }
}
void TUPLE::dumpBody_GENERIC(std::ostream &stream, bool cisTupleFlag, int i, char* strings[], int maxStrings)
{
    list<U32>::iterator it = body.begin();
    int j = 0;
    for (it = body.begin(); it != body.end(); it++)
    {
        stream << "\t0x" << setw(4) << setfill('0') << hex << (address + i) <<"\t" <<
            setw(2) << setfill('0') << hex << "(0x" << i << ")" << "\t\t" <<
            setw(2) << setfill('0') << *it;
        if (j >= maxStrings)
        {
            stream << endl;
        }
        else
        {
            stream << strings[j++] << endl;
        }
        i++;
    }
}
