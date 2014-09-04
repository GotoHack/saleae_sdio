#ifndef SDIOCMD53_H
#define SDIOCMD53_H

#include "SdioCmd.h"
#include "SdioCmd52.h"

class SdioCmd53 : public SdioCmd
{
public:
    SdioCmd53(U64 data):SdioCmd(data){};
    virtual string *getShortString();
    virtual string *getDetailedString();

    bool getRead(); 
    bool getWrite(); 
    U32  getFunctionNumber();
    bool isBlockMode();
    bool isByteMode();
    bool isFixedAddress();
    bool isIncrementingAddress();
    U32  getRegisterAddress();
    U32  getXferCount();

};

class SdioCmd53Resp : public SdioCmd52Resp
{
public:
    SdioCmd53Resp(U64 data):SdioCmd52Resp(data){};
    virtual string *getShortString();
    virtual string *getDetailedString();
};


#endif /* SDIOCMD53_H */
