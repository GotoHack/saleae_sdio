#ifndef SDIO_ANALYZER_H
#define SDIO_ANALYZER_H

#include <Analyzer.h>
#include "SDIOAnalyzerResults.h"
#include "SDIOSimulationDataGenerator.h"

class SDIOAnalyzerSettings;
class ANALYZER_EXPORT SDIOAnalyzer : public Analyzer
{
public:
	SDIOAnalyzer();
	virtual ~SDIOAnalyzer();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::auto_ptr< SDIOAnalyzerSettings > mSettings;
	std::auto_ptr< SDIOAnalyzerResults > mResults;
	// AnalyzerChannelData* mSerial;
	AnalyzerChannelData* mClock;    //a
	AnalyzerChannelData* mCmd;      //b
	AnalyzerChannelData* mData0;    //c
	AnalyzerChannelData* mData1;
	AnalyzerChannelData* mData2;
	AnalyzerChannelData* mData3;

	SDIOSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;

    // jonathan vars
    U64 currentSampleNo;
    U64 frameStartSample;

private: // internal functions
    U64 AdvanceAllLinesToNextStartBit();
    Frame ParseCurrentCommand(U64 cmdValue, U64 currentSample);
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //SDIO_ANALYZER_H
