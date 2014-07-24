#include "SDIOAnalyzer.h"
#include "SDIOAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

SDIOAnalyzer::SDIOAnalyzer()
:	Analyzer(),  
	mSettings( new SDIOAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

SDIOAnalyzer::~SDIOAnalyzer()
{
	KillThread();
}

void SDIOAnalyzer::WorkerThread()
{
	mResults.reset( new SDIOAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );

	mSampleRateHz = GetSampleRate();

	mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );
	mClock = GetAnalyzerChannelData( mSettings->mClockChannel );
	mCmd = GetAnalyzerChannelData( mSettings->mCmdChannel );
	mData0 = GetAnalyzerChannelData( mSettings->mD0Channel );
	mData1 = GetAnalyzerChannelData( mSettings->mD1Channel );
	mData2 = GetAnalyzerChannelData( mSettings->mD2Channel );
	mData3 = GetAnalyzerChannelData( mSettings->mD3Channel );

	if( mSerial->GetBitState() == BIT_LOW )
		mSerial->AdvanceToNextEdge();

	U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
	U32 samples_to_first_center_of_first_data_bit = U32( 1.5 * double( mSampleRateHz ) / double( mSettings->mBitRate ) );

	for( ; ; )
	{
		U8 data = 0;
		U8 mask = 1 << 7;
		
		mSerial->AdvanceToNextEdge(); //falling edge -- beginning of the start bit

		U64 starting_sample = mSerial->GetSampleNumber();

		mSerial->Advance( samples_to_first_center_of_first_data_bit );

		for( U32 i=0; i<8; i++ )
		{
			//let's put a dot exactly where we sample this bit:
			mResults->AddMarker( mSerial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );

			if( mSerial->GetBitState() == BIT_HIGH )
				data |= mask;

			mSerial->Advance( samples_per_bit );

			mask = mask >> 1;
		}


		//we have a byte to save. 
		Frame frame;
		frame.mData1 = data;
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = starting_sample;
		frame.mEndingSampleInclusive = mSerial->GetSampleNumber();

		mResults->AddFrame( frame );
		mResults->CommitResults();
		ReportProgress( frame.mEndingSampleInclusive );
	}
}

bool SDIOAnalyzer::NeedsRerun()
{
	return false;
}

U32 SDIOAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 SDIOAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 4;
}

const char* SDIOAnalyzer::GetAnalyzerName() const
{
	return "SDIO Analyzer";
}

const char* GetAnalyzerName()
{
	return "SDIO Analyzer";
}

Analyzer* CreateAnalyzer()
{
	return new SDIOAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}
