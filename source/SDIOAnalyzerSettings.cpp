#include "SDIOAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


SDIOAnalyzerSettings::SDIOAnalyzerSettings()
:   mClockChannel( UNDEFINED_CHANNEL ),
 	mCmdChannel( UNDEFINED_CHANNEL ),
 	mD0Channel( UNDEFINED_CHANNEL ),
 	mD1Channel( UNDEFINED_CHANNEL ),
 	mD2Channel( UNDEFINED_CHANNEL ),
 	mD3Channel( UNDEFINED_CHANNEL )
{
	// mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	// mInputChannelInterface->SetTitleAndTooltip( "Serial", "Standard SDIO Analyzer" );
	// mInputChannelInterface->SetChannel( mInputChannel );

	// mBitRateInterface.reset( new AnalyzerSettingInterfaceInteger() );
	// mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/S)",  "Specify the bit rate in bits per second." );
	// mBitRateInterface->SetMax( 6000000 );
	// mBitRateInterface->SetMin( 1 );
	// mBitRateInterface->SetInteger( mBitRate );

	mClockChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mClockChannelInterface->SetTitleAndTooltip( "Clock", "Standard SDIO Analyzer" );
	mClockChannelInterface->SetChannel( mClockChannel );

	mCmdChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mCmdChannelInterface->SetTitleAndTooltip( "CMD", "Standard SDIO Analyzer" );
	mCmdChannelInterface->SetChannel( mCmdChannel );

	mD0ChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mD0ChannelInterface->SetTitleAndTooltip( "Data 0", "Standard SDIO Analyzer" );
	mD0ChannelInterface->SetChannel( mD0Channel );

	mD1ChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mD1ChannelInterface->SetTitleAndTooltip( "Data 1", "Standard SDIO Analyzer" );
	mD1ChannelInterface->SetChannel( mD1Channel );

	mD2ChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mD2ChannelInterface->SetTitleAndTooltip( "Data 2", "Standard SDIO Analyzer" );
	mD2ChannelInterface->SetChannel( mD2Channel );

	mD3ChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mD3ChannelInterface->SetTitleAndTooltip( "Data 3", "Standard SDIO Analyzer" );
	mD3ChannelInterface->SetChannel( mD3Channel );

	// AddInterface( mInputChannelInterface.get() );
	// AddInterface( mBitRateInterface.get() );

	AddInterface( mClockChannelInterface.get() );
	AddInterface( mCmdChannelInterface.get() );
	AddInterface( mD0ChannelInterface.get() );
	AddInterface( mD1ChannelInterface.get() );
	AddInterface( mD2ChannelInterface.get() );
	AddInterface( mD3ChannelInterface.get() );

	AddExportOption   ( SDIO_EXPORT_FULL, "Export as text/csv file" );
	AddExportExtension( SDIO_EXPORT_FULL, "text", "txt" );
	AddExportExtension( SDIO_EXPORT_FULL, "csv", "csv" );

	AddExportOption   ( SDIO_EXPORT_SHORT, "Export as text/csv file -- less annotation" );
	AddExportExtension( SDIO_EXPORT_SHORT, "text", "txt" );
	AddExportExtension( SDIO_EXPORT_SHORT, "csv", "csv" );

	AddExportOption   ( SDIO_EXPORT_DATA_ONLY, "Export as text/csv file -- data only" );
	AddExportExtension( SDIO_EXPORT_DATA_ONLY, "text", "txt" );
	AddExportExtension( SDIO_EXPORT_DATA_ONLY, "csv", "csv" );

	ClearChannels();
	// AddChannel( mInputChannel, "Serial", false );
	AddChannel( mClockChannel, "Clock", false );
	AddChannel( mCmdChannel, "CMD", false );
	AddChannel( mD0Channel, "Data 0", false );
	AddChannel( mD1Channel, "Data 1", false );
	AddChannel( mD2Channel, "Data 2", false );
	AddChannel( mD3Channel, "Data 3", false );
}

SDIOAnalyzerSettings::~SDIOAnalyzerSettings()
{
}

bool SDIOAnalyzerSettings::SetSettingsFromInterfaces()
{
	// mInputChannel = mInputChannelInterface->GetChannel();
	// mBitRate = mBitRateInterface->GetInteger();

	mClockChannel = mClockChannelInterface->GetChannel();
	mCmdChannel = mCmdChannelInterface->GetChannel();
	mD0Channel = mD0ChannelInterface->GetChannel();
	mD1Channel = mD1ChannelInterface->GetChannel();
	mD2Channel = mD2ChannelInterface->GetChannel();
	mD3Channel = mD3ChannelInterface->GetChannel();

	ClearChannels();
	// AddChannel( mInputChannel, "Original", true );
	AddChannel( mClockChannel, "Clock", true );
	AddChannel( mCmdChannel, "Cmd", true );
	AddChannel( mD0Channel, "Data 0", true );
	AddChannel( mD1Channel, "Data 1", true );
	AddChannel( mD2Channel, "Data 2", true );
	AddChannel( mD3Channel, "Data 3", true );

	return true;
}

void SDIOAnalyzerSettings::UpdateInterfacesFromSettings()
{
	// mInputChannelInterface->SetChannel( mInputChannel );
	// mBitRateInterface->SetInteger( mBitRate );

	mClockChannelInterface->SetChannel( mClockChannel );
	mCmdChannelInterface->SetChannel( mCmdChannel );
	mD0ChannelInterface->SetChannel( mD0Channel );
	mD1ChannelInterface->SetChannel( mD1Channel );
	mD2ChannelInterface->SetChannel( mD2Channel );
	mD3ChannelInterface->SetChannel( mD3Channel );
}

void SDIOAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	// text_archive >> mInputChannel;
	// text_archive >> mBitRate;

	text_archive >> mClockChannel;
	text_archive >> mCmdChannel;
	text_archive >> mD0Channel;
	text_archive >> mD1Channel;
	text_archive >> mD2Channel;
	text_archive >> mD3Channel;

	ClearChannels();
	//AddChannel( mInputChannel, "Original", true );
	AddChannel( mClockChannel, "Clock", true );
	AddChannel( mCmdChannel, "Cmd", true );
	AddChannel( mD0Channel, "Data 0", true );
	AddChannel( mD1Channel, "Data 1", true );
	AddChannel( mD2Channel, "Data 2", true );
	AddChannel( mD3Channel, "Data 3", true );

	UpdateInterfacesFromSettings();
}

const char* SDIOAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	// text_archive << mInputChannel;
	// text_archive << mBitRate;

	text_archive << mClockChannel;
	text_archive << mCmdChannel;
	text_archive << mD0Channel;
	text_archive << mD1Channel;
	text_archive << mD2Channel;
	text_archive << mD3Channel;

	return SetReturnString( text_archive.GetString() );
}
