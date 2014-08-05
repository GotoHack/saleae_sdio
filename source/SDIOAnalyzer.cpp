#include "SDIOAnalyzer.h"
#include "SDIOAnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include <AnalyzerHelpers.h>

SDIOAnalyzer::SDIOAnalyzer()
:	Analyzer(),  
	mSettings( new SDIOAnalyzerSettings() ),
	mSimulationInitilized( false ),
    currentSampleNo(0)
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
	DataBuilder cmdData;
    U64 cmdValue;
    U32 i;
    BitState cmdBitState;

    
	SetAnalyzerResults( mResults.get() );
	//mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mClockChannel );

	mSampleRateHz = GetSampleRate();

	//mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );
	mClock = GetAnalyzerChannelData( mSettings->mClockChannel );
	mCmd = GetAnalyzerChannelData( mSettings->mCmdChannel );
	mData0 = GetAnalyzerChannelData( mSettings->mD0Channel );
	mData1 = GetAnalyzerChannelData( mSettings->mD1Channel );
	mData2 = GetAnalyzerChannelData( mSettings->mD2Channel );
	mData3 = GetAnalyzerChannelData( mSettings->mD3Channel );

    // wait for CMD to be high
	if( mCmd->GetBitState() == BIT_LOW ) {
		mCmd->AdvanceToNextEdge();
    }

    cmdData.Reset(&cmdValue, AnalyzerEnums::MsbFirst, 48);

	// U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
	// U32 samples_to_first_center_of_first_data_bit = U32( 1.5 * double( mSampleRateHz ) / double( mSettings->mBitRate ) );

	for( ; ; )
	{
        // we know cmd is high, wait till the next transition, which should be the start bit
        AdvanceAllLinesToNextStartBit();
        cmdData.Reset(&cmdValue, AnalyzerEnums::MsbFirst, 48);
        
        // now we're at the start bit, we start using our clock as our sample generator
        for (i = 0; i < 48; i++)
        {
            cmdBitState = mCmd->GetBitState();
            cmdData.AddBit(cmdBitState);

            
            if( mClock->GetBitState() == BIT_LOW ) {
                mClock->AdvanceToNextEdge();
            }
            mClock->AdvanceToNextEdge();

            // now get our position, based on clock
            currentSampleNo = mClock->GetSampleNumber();
            mCmd->AdvanceToAbsPosition(currentSampleNo);

        }

        currentSampleNo = mClock->GetSampleNumber();

        // move all the data lines to current sample
        mData0->AdvanceToAbsPosition(currentSampleNo);
        mData1->AdvanceToAbsPosition(currentSampleNo);
        mData2->AdvanceToAbsPosition(currentSampleNo);
        mData3->AdvanceToAbsPosition(currentSampleNo);
        
        Frame frame = ParseCurrentCommand(cmdValue, currentSampleNo);

	}
}

Frame SDIOAnalyzer::ParseCurrentCommand(U64 cmdValue, U64 currentSample)
{
    //we have a byte to save. 
    Frame frame;
    U64 tmp = cmdValue;
    frame.mData1 = cmdValue;
    frame.mFlags = 0;
    frame.mStartingSampleInclusive = frameStartSample;
    frame.mEndingSampleInclusive = currentSampleNo;

    tmp >>= 40;
    frame.mType = (U8)tmp & 0x3f;
    if (tmp & 0x40) {
        frame.mFlags = 1;
    }
    else {
        frame.mFlags = 0;
    }



    mResults->AddFrame( frame );
    mResults->CommitResults();
    ReportProgress( frame.mEndingSampleInclusive );
}

U64 SDIOAnalyzer::AdvanceAllLinesToNextStartBit()
{
    // make sure we're on a high bit
	if( mCmd->GetBitState() == BIT_LOW ) {
		mCmd->AdvanceToNextEdge();
    }

    // now advance to start bit which is a falling bit
    mCmd->AdvanceToNextEdge();

    currentSampleNo = mCmd->GetSampleNumber();
    frameStartSample = currentSampleNo;

    // but we want to sample on the rising edge of the clock.  Advance the clock to
    // our sample number, check the clock, it should be low.  advance the clock to next edge
    mClock->AdvanceToAbsPosition(currentSampleNo);
	if( mClock->GetBitState() == BIT_LOW ) {
		mClock->AdvanceToNextEdge();
    }

    // now get our position, based on clock
    currentSampleNo = mClock->GetSampleNumber();

    // move all the lines to current sample
    mCmd->AdvanceToAbsPosition(currentSampleNo);
    mData0->AdvanceToAbsPosition(currentSampleNo);
    mData1->AdvanceToAbsPosition(currentSampleNo);
    mData2->AdvanceToAbsPosition(currentSampleNo);
    mData3->AdvanceToAbsPosition(currentSampleNo);

    return currentSampleNo;
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
	//return mSettings->mBitRate * 4;
	return 1000000;
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
