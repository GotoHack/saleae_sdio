#include <LogicPublicTypes.h>
#include <SdioCmd.h>
#include <CIA.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#include <stdio.h>


vector<U64> *readFile(char* fileName);
void parse(vector <U64>* vec);
int main (int argc, char** argv)
{
    int i;

    // validate that we have an argument
    if (argc <= 1)
    {
        printf("usage: %s <input file>\n\n", argv[0]);
    }

    // read the file into a vector
    vector<U64> *readData = readFile(argv[1]);

    // parse the vector
    parse(readData);
    
    return 0;
}
vector<U64> *readFile(char* fileName)
{
    vector<U64> *data = new vector<U64>;
    ifstream inStream(fileName, fstream::in);
    U64 tmp;

    if (inStream.is_open())
    {
        while (!inStream.eof())
        {
            inStream  >> hex >> tmp;
            data->push_back(tmp);
        }
        inStream.close();
    }
    // to debug the vector, in gdb, type: pvector data

    return data;
}

void parse(vector <U64>* vec)
{
    // U64 data1 = 0x740040580005;
    // U64 data2 = 0x340000100037;
    // SdioCmd *tmp = SdioCmd::CreateSdioCmd(data1);
    SdioCmd *tmp;
    U64 tmpData = 0;
    vector<U64>::iterator it;

    for (it = vec->begin(); it != vec->end(); it++)
    {
        tmpData = *it;
        tmp = SdioCmd::CreateSdioCmd(*it);
        if (tmp->getCmd() == 52)
        {
            CCCR *cccr = CCCR::BuildCCCR(*it);
            //printf ("%s\n", tmp->getDetailedString());
            cout << tmp->getDetailedString()->c_str()<<endl;
        }
        else
        {
            printf("0x%012llX\n", tmpData);
        }
    }

    CCCR::DumpCCCRTable();
    CCCR::DumpFBRTable();
    CCCR::CleanupCCCR();

}
