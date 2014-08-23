#ifndef SDIO_SIMULATION_DATA_GENERATOR
#define SDIO_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <AnalyzerHelpers.h>
#include <string>
class SDIOAnalyzerSettings;

class SDIOSimulationDataGenerator
{
public:
	SDIOSimulationDataGenerator();
	~SDIOSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, SDIOAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	SDIOAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;
    ClockGenerator generator;

	SimulationChannelDescriptor mSerialSimulationData;

    SimulationChannelDescriptorGroup mSdioSimulationChannels;
	SimulationChannelDescriptor *mSimClk;
	SimulationChannelDescriptor *mSimCmd;
	SimulationChannelDescriptor *mSimData0;
	SimulationChannelDescriptor *mSimData1;
	SimulationChannelDescriptor *mSimData2;
	SimulationChannelDescriptor *mSimData3;

    U32 currentDataIndex;
    U32 numBitsRemaining;
    bool isDataLine;

public:
    class DataRepresentation
    {
        private: 
            U32 currentDataIndex;
            U32 numBitsRemaining;
            bool isDataLine;
            void setBits();
        public:
            DataRepresentation ();
    };

    static const U64 sampleData[598];

};
#endif //SDIO_SIMULATION_DATA_GENERATOR
