#ifndef SDIOCMD_H
#define SDIOCMD_H

#include "LogicPublicTypes.h"
#include "SDIOParser.h"
#include "string"

using namespace std;


class SdioCmd
{
public:
    SdioCmd(U64 data) { cmdData = data;}

    virtual string* getShortString();
    virtual string* getDetailedString();

    virtual U32 getCmd() { return CMD_VAL(cmdData); };
    virtual U32 getDir() { return CMD_DIR(cmdData); };

    static SdioCmd* CreateSdioCmd(U64 data);



protected:
    U64 cmdData;
    static SdioCmd* lastSdioHostCmd;
};


#endif /* SDIOCMD_H */
