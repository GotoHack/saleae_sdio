#include "SDIOAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "SDIOAnalyzer.h"
#include "SDIOAnalyzerSettings.h"
#include <iostream>
#include <fstream>
#include "SDIOParser.h"
#include "SdioCmd.h"
#include "CIA.h"

SDIOAnalyzerResults::SDIOAnalyzerResults( SDIOAnalyzer* analyzer, SDIOAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

SDIOAnalyzerResults::~SDIOAnalyzerResults()
{
}

void SDIOAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );

    // if this is command data
    if (frame.mType == FRAME_TYPE_CMD_DATA)
    {
        SdioCmd *tmp = SdioCmd::CreateSdioCmd(frame.mData1);
        string *s = tmp->getShortString();
		AddResultString( s->c_str() );
        delete s;
    }
    // else, this is data line data, should be partitioned into bytes
    else
    {
        char number_str[128] = {0};
        sprintf(number_str, "0x%02X", frame.mData1);
        AddResultString( number_str );
    }
}

void SDIOAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();
    double timeStamp = 0.0;


    if ( (export_type_user_id == SDIO_EXPORT_FULL) ||(export_type_user_id == SDIO_EXPORT_SHORT))
    {
        // if you want sample numbers
        // file_stream << "Sample Number, Time [s] ,Value ,Description" << std::endl;
        file_stream << "Time [s] ,Value ,Description" << std::endl;
    }
    else
    {
        // if you want sample numbers
        file_stream << "Sample Number, Time [s] ,Value ,Description" << std::endl;
        // file_stream << "Time [s] ,Value"<< std::endl;
    }

	U64 num_frames = GetNumFrames();
    

	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
        U64 cmdType = frame.mData1;
		
		char time_str[128];
        timeStamp = (double)frame.mStartingSampleInclusive;
        timeStamp -= trigger_sample;
        timeStamp /= sample_rate;

        if ( (export_type_user_id == SDIO_EXPORT_FULL) ||(export_type_user_id == SDIO_EXPORT_SHORT))
        {
            // only if we want sample numbers
            // sprintf(time_str, "%012lld %0.10f", frame.mStartingSampleInclusive,timeStamp);
            sprintf(time_str, "%012lld", timeStamp);
        }
        else
        {
            // only if we want sample numbers
            // sprintf(time_str, "%012lld %0.10f", frame.mStartingSampleInclusive,timeStamp);
            sprintf(time_str, "%012lld", timeStamp);
        }

        // if regular CMD line data
        if (frame.mType == FRAME_TYPE_CMD_DATA)
        {
            SdioCmd *tmp = SdioCmd::CreateSdioCmd(frame.mData1);

            if (tmp->getCmd() == 52)
            {

                // set up the CCCR for exporting
                // the cccr will be static, so no need to free it
                CCCR *cccr = CCCR::BuildCCCR(frame.mData1);
            }
           
            if (export_type_user_id == SDIO_EXPORT_FULL)
            {
				// const char* detail = tmp->getDetailedString();
				string *s = tmp->getDetailedString();
				file_stream << time_str << "," << "\t" << s->c_str() << endl;
                delete s;
            }
            else if (export_type_user_id == SDIO_EXPORT_SHORT)
            {
                string *s = tmp->getShortString();
                file_stream << time_str << "," << "\t" << s->c_str() << endl;
                delete s;
            }
            else
            {
                // just data
                char number_str[128];
                sprintf(number_str, "0x%012llX", frame.mData1);
                file_stream << time_str << "," << "\t" << number_str << endl;
            }
        }
        else
        {
            char number_str[128];
            // this is DATA Line Data
            sprintf(number_str, "0x%02llX", frame.mData1);
            file_stream << time_str << "," << "\t"<<number_str <<std::endl;
        }

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}
    CCCR::DumpCCCRTable(file_stream);
    CCCR::DumpFBRTable(file_stream);
    CCCR::CleanupCCCR();


	file_stream.close();
}

void SDIOAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
	Frame frame = GetFrame( frame_index );
	ClearResultStrings();

	char number_str[128];
	//AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
	AnalyzerHelpers::GetNumberString( frame.mData1, Hexadecimal, 8, number_str, 128 );
	AddResultString( number_str );
}

void SDIOAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}

void SDIOAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}

char* SDIOAnalyzerResults::ParseCommand(U64 data)
{
    return (char* )parse_str(data);
}
char* SDIOAnalyzerResults::ParseCmd52(U64 data)
{
	return "";
}
