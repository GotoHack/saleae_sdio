#ifndef CIA_H
#define CIA_H

#include "LogicPublicTypes.h"
#include "SDIOParser.h"
#include "SdioCmd52.h"
#include <string.h>
#include <list>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <map>
#include <string>

using namespace std;

#ifndef _MSC_VER
#define PACKED __attribute__ ((__packed__))
#else
#define PACKED
#pragma pack(push, 1)
#endif

#define PRINTF_BIT_PATTERN "| %x%x%x%x | %x%x%x%x |"
#define PRINTF_BIT(x) (1 & x >> 7), \
                      (1 & x >> 6), \
                      (1 & x >> 5), \
                      (1 & x >> 4), \
                      (1 & x >> 3), \
                      (1 & x >> 2), \
                      (1 & x >> 1), \
                      (1 & x )


// this is arbitrary -- just looking at the map in 6.7 of the simplified spec
#define CIS_MAX_SIZE 0xFFF
#define CIA_PTR_INIT_VAL    0xDEADBF
#define CIS_AREA_START      0x1000
#define CIS_AREA_END        0x7FFF

#define CISTPL_MANFID       0x20
#define CISTPL_FUNCID       0x21
#define CISTPL_FUNCE        0x22

typedef struct PACKED 
{
    unsigned char CCCR_SDIO_Revision;
    unsigned char SD_Specification_Revision;
    unsigned char IO_Enable;
    unsigned char IO_Ready;
    unsigned char Int_Enable;
    unsigned char Int_Pending;
    unsigned char IO_Abort;
    unsigned char Bus_Interface_Control;
    unsigned char Card_Capatility;
    unsigned char Common_CIS_Pointer[3];
    unsigned char Bus_Suspend;
    unsigned char Function_Select;
    unsigned char Exec_Flags;
    unsigned char Ready_Flags;
    unsigned char FN0_Block_Size[2];
    unsigned char Power_Control;
    unsigned char High_Speed;
    unsigned char RFU[0xDC];
    unsigned char Reserved_for_Vendors[0x0F];
} CCCR_t;

extern const char *CCCR_NAMES[];
#define CCCR_ADDRESS_START  0x00
#define CCCR_ADDRESS_END    0xFF
#define NUM_CCCR_ELEMENTS   0x14



// the FBR is an array of 256 bytes for functions 1-7, each function addressed at the next 100 (hex)
// comments for fn1, but they apply to all the fbr's
typedef struct PACKED
{
    unsigned char FN_CSA;                                       // 0x100
    unsigned char FN_extended_SDIO_fn_interface_code;           // 0x101
    unsigned char RFU[7];                                       // 0x102--0x108
    unsigned char CIS_ptr[3];                                   // 0x109-0x10b
    unsigned char CSA_ptr[3];                                   // 0x10c--10e
    unsigned char Data_Access_Window_To_CSA;                    // 0x10f
    unsigned char IO_block_size_for_fn[2];                      // 0x110--0x111
    unsigned char RFU2[0xEE];                                   // 0x112--0x1ff
} FBR_t;;

extern const char* FBR_NAMES[];
#define FBR_ADDRESS_START   0x100
#define FBR_ADDRESS_END     0x7ff
#define NUM_FBR_ELEMENTS    0x12

class TUPLE
{
    //private:
    public:
        U32 address;
        U32 tplCode;
        U32 size;
        list <U32> body;
        map <int, string> tupleNames;
    public:
        TUPLE(U32 addr=0);
        void setAddress(U32 addr) {address = addr;}
        void setTplCode(U32 code) {tplCode = code;}
        void setSize(U32 sz) {size = sz;}
        void addData(U32 data) { body.push_back(data);}
        void dump(bool cisTupleFlag = false);
        void dump(std::ostream &stream, bool cisTupleFlag = false);
    private:
        void dumpBody(std::ostream &stream, bool cisTupleFlag = false, int i=0);
        void dumpBody_MANFID(std::ostream &stream, bool cisTupleFlag = false, int i=0);
        void dumpBody_FUNCID(std::ostream &stream, bool cisTupleFlag = false, int i=0);
        void dumpBody_FUNCE(std::ostream &stream, bool cisTupleFlag = false, int i=0);
        void dumpBody_GENERIC(std::ostream &stream, bool cisTupleFlag = false, int i=0, char *strings[]=0, int maxStrings=0);


};

class CCCR
{
    public:
        static CCCR* BuildCCCR(U64 data);
        static bool AddCmd52ToCCCR(U64 data);
        static void DumpCCCRTable(void);
        static void DumpCCCRTable(std::ostream &stream);
        static void DumpFBRTable(void);
        static void DumpFBRTable(std::ostream &stream);
        static void CleanupCCCR();
        U32 getCisAddress();

    protected:
        class TupleChain
        {
            private:
                U32 lastTupleAddress;
                U32 cisAddress;
                bool newTuplePending;
                bool tupleComplete;
                list <TUPLE> tuples;
                bool cisTupleFlag;

            public:
                TupleChain() :lastTupleAddress(0), newTuplePending(false), tupleComplete(false), cisTupleFlag(false), tuples() {};
                void setCisAddress(U32 address);
                void addDataToTuple(U64 data);
                void dump();
                void dump(std::ostream &stream);
                void setCisTupleFlag() {cisTupleFlag = true;};
        };

        class FBR
        {
            public:
                FBR(U32 number = 1);
                void DumpFBR();
                void DumpFBR(std::ostream &stream);
                void addData(U64 data);
                U32 getCisAddress(void);
            private:
                FBR_t fbr_data;
                U32 functionNumber;
                bool fbrDataPopulated;
                TupleChain tupleChain;

        };

    private:
        // variables
        CCCR_t cccr_data;
        // flags to indicate if data is populated, 
        // don't want to waste output space w/ unpopulated tables
        bool cccrDataPopulated;
        SdioCmd52 *lastHostCmd52;
        static CCCR* theCCCR;
        FBR fbr[7];
        TupleChain tupleChain;
        

        // functions
        CCCR();
        ~CCCR() {};
        CCCR(const CCCR& c) {};
        CCCR& operator=(const CCCR&)  {};

        bool HandleCmd52Request(U64 data);
        bool HandleCmd52Response(U64 data);

};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif /* CIA_H */
