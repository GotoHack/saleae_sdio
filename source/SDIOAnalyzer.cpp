#include "SDIOAnalyzer.h"
#include "SDIOAnalyzerSettings.h"
#include "SDIOParser.h"
#include <AnalyzerChannelData.h>
#include <AnalyzerHelpers.h>
#include <iostream>
#include <stdio.h>

SDIOAnalyzer::SDIOAnalyzer()
:	Analyzer(),  
	mSettings( new SDIOAnalyzerSettings() ),
	mSimulationInitilized( false ),
    currentSampleNo(0),
    numSamplesInHalfClock(0)
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
    U64 clkMeasureStart = 0;
    U64 clkMeasureStop = 0;
	U64 localNumSamplesInHalfClock = 0;

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

        // always setup the number of cycles in a half clock.  measure this
        // during the read of a CMD word.  is necessary to do multiple times
        // b/c the host can change the clock speed on us (but usually only once)
		clkMeasureStart = 0;
		clkMeasureStart = 0;
		localNumSamplesInHalfClock = 0;

        // now we're at the start bit, we start using our clock as our sample generator
        for (i = 0; i < 48; i++)
        {
            cmdBitState = mCmd->GetBitState();
            cmdData.AddBit(cmdBitState);

            
            // advance to next rising edge (i.e. full clock cycle)
            mClock->AdvanceToNextEdge();
			clkMeasureStart = mClock->GetSampleNumber();
            mClock->AdvanceToNextEdge();
			clkMeasureStop  = mClock->GetSampleNumber();
			localNumSamplesInHalfClock = clkMeasureStop - clkMeasureStart;

            // update the numSamplesInHalfClock if it is different from what we just
            // calculated
			if (localNumSamplesInHalfClock != numSamplesInHalfClock)
			{
				numSamplesInHalfClock = localNumSamplesInHalfClock;
			}

            // now get our position, based on clock
            currentSampleNo = mClock->GetSampleNumber();
            mCmd->AdvanceToAbsPosition(currentSampleNo);

        }
        Frame frame = ParseCurrentCommand(cmdValue, currentSampleNo);

            
        // advance clock 1 cycle
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
            readCmd53Data();
        }
	}
}
void SDIOAnalyzer::readCmd53Data()
{
    // for now assume we are 4 bit.  we need a setting to choose 1/4 bit
    U64 absoluteLastSample = (U64)-1;
    if (mCmd->DoMoreTransitionsExistInCurrentData())
    {
        absoluteLastSample = mCmd->GetSampleOfNextEdge();	//without moving, get the sample of the next transition. 

        // std::cout << "currentSampleNo: " << currentSampleNo << ", absoluteLastSample: "<< absoluteLastSample<< std::endl;
        // move data lines to start bit
        AdvanceDataLinesToStartBit();

        // std::cout << "CMD53 Data: " ;
        while ((currentSampleNo < absoluteLastSample) && 
                ( mData0->WouldAdvancingToAbsPositionCauseTransition(absoluteLastSample) == true))
        {
            readCmd53DataLines();
        }
        // std::cout << std::endl;
    }
    else
    {
        // this is most likely the last bit of data and there is no more cmd line activity
        AdvanceDataLinesToStartBit();

        //std::cout << "CMD53 Data: " ;
        while  ( mData0->DoMoreTransitionsExistInCurrentData() ) 
        {
            readCmd53DataLines();
        }
        // std::cout << std::endl;
    }

}

void SDIOAnalyzer::readCmd53DataLines()
{
	DataBuilder dataData;
    U64 dataValue;
    U32 i;

    // advance all the data lines to our clock
    currentSampleNo = mClock->GetSampleNumber();
    mData0->AdvanceToAbsPosition(currentSampleNo);
    mData1->AdvanceToAbsPosition(currentSampleNo);
    mData2->AdvanceToAbsPosition(currentSampleNo);
    mData3->AdvanceToAbsPosition(currentSampleNo);

    frameStartSample = currentSampleNo - (numSamplesInHalfClock/2);
    // frameStartSample = currentSampleNo;
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
    // std::cout << std::hex << dataValue << ":";
    Frame dataFrame = ParseCurrentCommand(dataValue, currentSampleNo - (1* (numSamplesInHalfClock/2)), FRAME_TYPE_DATA_DATA);
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
    //frame.mEndingSampleInclusive = currentSampleNo;
    frame.mEndingSampleInclusive = currentSample;


    mResults->AddFrame( frame );
    mResults->CommitResults();
    ReportProgress( frame.mEndingSampleInclusive );
	return frame;
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
	// if( mClock->GetBitState() == BIT_HIGH ) {
	// 	mClock->AdvanceToNextEdge();
    // }
    
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

    numSamplesInHalfClock = 0;

    return currentSampleNo;
}

U64 SDIOAnalyzer::AdvanceDataLinesToStartBit()
{
    bool dataFallIsStartBit = false;

    // make sure we're on the start bit
	if( mData0->GetBitState() == BIT_HIGH ) {
		mData0->AdvanceToNextEdge();
		currentSampleNo = mData0->GetSampleNumber();
        // printf("AdvanceDataLinesToStartBit D0 Sample number: %lld (0x%llx)\n", currentSampleNo, currentSampleNo);
    }
    // now that we have the D0 start transition, we need to see if
    while (mClock->GetSampleOfNextEdge() <= currentSampleNo)
    {
        mClock->AdvanceToNextEdge();
    }
    if (mClock->GetSampleNumber() == currentSampleNo)
    {
        if (mClock->GetBitState() == BIT_LOW)
        {
            dataFallIsStartBit = true;
            // std::cout << "AdvanceDataLinesToStartBit ------ CLOCK sample is same as D0 sample\n";
        }
    }

	if( mData1->GetBitState() == BIT_HIGH ) {
		mData1->AdvanceToNextEdge();
		if (mData1->GetSampleNumber() > currentSampleNo)
		{
			currentSampleNo = mData1->GetSampleNumber();
			mData0->AdvanceToAbsPosition(currentSampleNo);
            dataFallIsStartBit = true;
		}

    }
	if( mData2->GetBitState() == BIT_HIGH ) {
		mData2->AdvanceToNextEdge();
		if (mData2->GetSampleNumber() > currentSampleNo)
		{
			currentSampleNo = mData2->GetSampleNumber();
			mData0->AdvanceToAbsPosition(currentSampleNo);
			mData1->AdvanceToAbsPosition(currentSampleNo);
            dataFallIsStartBit = true;
		}
    }
	if( mData3->GetBitState() == BIT_HIGH ) {
		mData3->AdvanceToNextEdge();
		if (mData3->GetSampleNumber() > currentSampleNo)
		{
			currentSampleNo = mData3->GetSampleNumber();
			mData0->AdvanceToAbsPosition(currentSampleNo);
			mData1->AdvanceToAbsPosition(currentSampleNo);
			mData2->AdvanceToAbsPosition(currentSampleNo);
            dataFallIsStartBit = true;
		}
    }

    if (dataFallIsStartBit == true)
    {
        // std::cout << "AdvanceDataLinesToStartBit ------ dataFallIsStartBit == true\n";
    }
    else
    {
        if( mClock->GetBitState() == BIT_HIGH ) 
        {
            // goto falling
            mClock->AdvanceToNextEdge();
        }
    }

    if (mSettings->mSampleRead == RISING_EDGE)
    {
        // printf("----------------------------- rising edge\n");
        // rising for start bit
        mClock->AdvanceToNextEdge();
        // falling for b0
        mClock->AdvanceToNextEdge();
        // rising for b0
        mClock->AdvanceToNextEdge();
    }
    else
    {
        // printf("----------------------------- falling edge\n");
        // rising for start bit
        mClock->AdvanceToNextEdge();
        // falling for b0
        mClock->AdvanceToNextEdge();
    }

    currentSampleNo = mClock->GetSampleNumber();
    mData0->AdvanceToAbsPosition(currentSampleNo);
    mData1->AdvanceToAbsPosition(currentSampleNo);
    mData2->AdvanceToAbsPosition(currentSampleNo);
    mData3->AdvanceToAbsPosition(currentSampleNo);
    frameStartSample = currentSampleNo;

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
