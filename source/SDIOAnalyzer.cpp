#include "SDIOAnalyzer.h"
#include "SDIOAnalyzerSettings.h"
#include "SDIOParser.h"
#include <AnalyzerChannelData.h>
#include <AnalyzerHelpers.h>
#include <iostream>

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
	DataBuilder dataData;
    U64 cmdValue;
    U64 dataValue;
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

            
            // advance to next rising edge (i.e. full clock cycle)
            mClock->AdvanceToNextEdge();
            mClock->AdvanceToNextEdge();

            // now get our position, based on clock
            currentSampleNo = mClock->GetSampleNumber();
            mCmd->AdvanceToAbsPosition(currentSampleNo);

        }
        Frame frame = ParseCurrentCommand(cmdValue, currentSampleNo);

            
        // advance clock 1 cycle
        mClock->AdvanceToNextEdge();
        mClock->AdvanceToNextEdge();
        mClock->AdvanceToNextEdge();
        mClock->AdvanceToNextEdge();

        currentSampleNo = mClock->GetSampleNumber();

        // move all the data lines to current sample
        mCmd->AdvanceToAbsPosition(currentSampleNo);
        mData0->AdvanceToAbsPosition(currentSampleNo);
        mData1->AdvanceToAbsPosition(currentSampleNo);
        mData2->AdvanceToAbsPosition(currentSampleNo);
        mData3->AdvanceToAbsPosition(currentSampleNo);
        // std::cout << "Current Sample Number (b): " << currentSampleNo << "mCmd->GetBitState(): "<< mCmd->GetBitState() << std::endl;
        

        // check if this is a command 53 ack from the card, we need to look for data
        if ( (CMD_VAL(cmdValue) == 53) && (CMD_DIR(cmdValue) == 0))
        {
            // for now assume we are 4 bit.  we need a setting to choose 1/4 bit
	        U64 absoluteLastSample = mCmd->GetSampleOfNextEdge();	//without moving, get the sample of the next transition. 

            std::cout << "currentSampleNo: " << currentSampleNo << ", absoluteLastSample: "<< absoluteLastSample<< std::endl;
            // move data lines to start bit
            AdvanceDataLinesToStartBit();

            std::cout << "CMD53 Data: " ;
            while ((currentSampleNo < absoluteLastSample) && 
                    ( mData0->WouldAdvancingToAbsPositionCauseTransition(absoluteLastSample) == true))
            {
                dataData.Reset(&dataValue, AnalyzerEnums::MsbFirst, 8);
                for (i = 0; i < 2; i++)
                {
                    // assume we are on the rising edge of the clock, so take a sample and advance for the  next one
                    dataData.AddBit(mData3->GetBitState());
                    dataData.AddBit(mData2->GetBitState());
                    dataData.AddBit(mData1->GetBitState());
                    dataData.AddBit(mData0->GetBitState());
                    
                    // advance to next rising edge (i.e. full clock cycle)
                    mClock->AdvanceToNextEdge();
                    mClock->AdvanceToNextEdge();

                    // now get our position, based on clock
                    currentSampleNo = mClock->GetSampleNumber();
                    mData0->AdvanceToAbsPosition(currentSampleNo);
                    mData1->AdvanceToAbsPosition(currentSampleNo);
                    mData2->AdvanceToAbsPosition(currentSampleNo);
                    mData3->AdvanceToAbsPosition(currentSampleNo);
                }
                std::cout << std::hex << dataValue << ":";
                Frame dataFrame = ParseCurrentCommand(dataValue, currentSampleNo, FRAME_TYPE_DATA_DATA);
// #define FRAME_TYPE_CMD_DATA             0x01
// #define FRAME_TYPE_DATA_DATA            0x02
// #define FRAME_FLAGS_CMD53_DATA_START    0x01
// #define FRAME_FLAGS_CMD53_DATA_END      0x02
            }
            std::cout << std::endl;
        }
	}
}

Frame SDIOAnalyzer::ParseCurrentCommand(U64 cmdValue, U64 currentSample, U8 type, U8 flags)
{
    //we have a byte to save. 
    Frame frame;
    U64 tmp = cmdValue;
    frame.mData1 = cmdValue;
    frame.mFlags = flags;
    frame.mType = type;
    frame.mStartingSampleInclusive = frameStartSample;
    frame.mEndingSampleInclusive = currentSampleNo;


    // tmp >>= 40;
    // frame.mType = (U8)tmp & 0x3f;
    // if (tmp & 0x40) {
    //     frame.mFlags = 1;
    // }
    // else {
    //     frame.mFlags = 0;
    // }



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
	if( mClock->GetBitState() == BIT_HIGH ) {
		mClock->AdvanceToNextEdge();
    }
    // now advance to rising edge
    mClock->AdvanceToNextEdge();

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

U64 SDIOAnalyzer::AdvanceDataLinesToStartBit()
{
    // make sure we're on the start bit
	if( mData0->GetBitState() == BIT_HIGH ) {
		mData0->AdvanceToNextEdge();
    }
	if( mData1->GetBitState() == BIT_HIGH ) {
		mData1->AdvanceToNextEdge();
    }
	if( mData2->GetBitState() == BIT_HIGH ) {
		mData2->AdvanceToNextEdge();
    }
	if( mData3->GetBitState() == BIT_HIGH ) {
		mData3->AdvanceToNextEdge();
    }
    // advance clock to our sample number, and then advance to the
    // rising edge
    currentSampleNo = mData0->GetSampleNumber();
    mClock->AdvanceToAbsPosition(currentSampleNo);

    if (mClock->GetBitState() == BIT_HIGH ) {
		mClock->AdvanceToNextEdge();
    }
    // now advance to rising edge
    mClock->AdvanceToNextEdge();
    mClock->AdvanceToNextEdge();
    mClock->AdvanceToNextEdge();
    currentSampleNo = mClock->GetSampleNumber();
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
