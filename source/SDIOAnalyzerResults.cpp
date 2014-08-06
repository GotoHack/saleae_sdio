#include "SDIOAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "SDIOAnalyzer.h"
#include "SDIOAnalyzerSettings.h"
#include <iostream>
#include <fstream>
#include "SDIOParser.h"

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

	char number_str[128];
	//AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
	AnalyzerHelpers::GetNumberString( frame.mData1, Hexadecimal, 8, number_str, 128 );
	AddResultString( number_str );
}

void SDIOAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();
    double timeStamp = 0.0;

	file_stream << "Time [s],Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
        U64 cmdType = frame.mData1;
		
		char time_str[128];
		// AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );
        timeStamp = frame.mStartingSampleInclusive;
        timeStamp -= trigger_sample;
        timeStamp /= sample_rate;
        sprintf(time_str, "%012lld %0.10f", frame.mStartingSampleInclusive,timeStamp);

		char number_str[128];
		char number_str2[128];
        char dir_str[128];
		//AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
		// AnalyzerHelpers::GetNumberString( frame.mData1, Hexadecimal, 8, number_str, 128 );
		// AnalyzerHelpers::GetNumberString( frame.mType, Decimal, 8, number_str2, 128 );
        sprintf(number_str, "%012llX", frame.mData1);
        sprintf(number_str2, "%02d", frame.mType);
        if (frame.mFlags == 1){
            sprintf(dir_str, "Host -> Card");
        }else {
            sprintf(dir_str, "Response ");
        }


		file_stream << time_str << "," << "\t"<<number_str <<", CMD: "<< number_str2 <<",     "<<dir_str << " -- "<<parse_str(frame.mData1) << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

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
}
