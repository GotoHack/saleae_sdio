#ifndef SDIO_ANALYZER_SETTINGS
#define SDIO_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

#define SDIO_EXPORT_FULL         0
#define SDIO_EXPORT_SHORT        1
#define SDIO_EXPORT_DATA_ONLY    2

enum SampleEdge { FALLING_EDGE, RISING_EDGE };

class SDIOAnalyzerSettings : public AnalyzerSettings
{
public:
	SDIOAnalyzerSettings();
	virtual ~SDIOAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	// Channel mInputChannel;
	// U32 mBitRate;
	Channel mClockChannel;  //a
	Channel mCmdChannel;    //b
	Channel mD0Channel;     //c
	Channel mD1Channel;     //d
	Channel mD2Channel;     //e
	Channel mD3Channel;     //f
    enum SampleEdge mSampleRead;

protected:
	// std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	// std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;

	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mClockChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mCmdChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mD0ChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mD1ChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mD2ChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mD3ChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mSampleReadInterface;
};

#endif //SDIO_ANALYZER_SETTINGS
