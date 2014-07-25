#ifndef SDIO_ANALYZER_SETTINGS
#define SDIO_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

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

protected:
	// std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	// std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;

	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mClockChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mCmdChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mD0ChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mD1ChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mD2ChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mD3ChannelInterface;
};

#endif //SDIO_ANALYZER_SETTINGS
