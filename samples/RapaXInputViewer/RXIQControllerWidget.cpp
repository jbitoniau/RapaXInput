/*
   The MIT License (MIT) (http://opensource.org/licenses/MIT)
   
   Copyright (c) 2015 Jacques Menuet
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#include "RXIQControllerWidget.h"

#include <assert.h>

#include <limits>

namespace RXI
{

QControllerWidget::QControllerWidget( QWidget* parent, Controller* controller, Qt::WindowFlags flags )
	: QFrame(parent, flags),
	  mController(controller),
	  mButtonLeds(),
	  mTriggerProgressBars(),
	  mThumbstickWidgets(),
	  mBatteriesProgressBars(),
	  mVibrationMotorsSliders()
{
	createWidgets();
	updateWidgets();
	mController->addListener( this );
}

QControllerWidget::~QControllerWidget()
{
	mController->removeListener( this );
}

void QControllerWidget::createWidgets()
{
	int textWidth = 90;

	QVBoxLayout* superMainLayout = new QVBoxLayout();
	setLayout(superMainLayout);

	QString text( mController->getSubTypeName() );
	text.append(" controller");
	
	text.append("\nVoice support: ");
	if ( mController->hasVoiceSupport() )
		text.append("yes");
	else
		text.append("no");

	GUID renderGuid;
	GUID captureGuid;
	if ( mController->getDirectSoundAudioDeviceIds(renderGuid, captureGuid) )
	{
		text.append("\nDirectSound render device: ");
		text.append( GUIDToString(renderGuid).c_str() );
		text.append("\nDirectSound capture device: ");
		text.append( GUIDToString(captureGuid).c_str() );
	}
	else
	{
		text.append("\nDirectSound render device: N/A");
		text.append("\nDirectSound capture device: N/A");
	}
	
	std::wstring renderString;
	std::wstring captureString;
	if ( mController->getWindowsCoreAudioDeviceIds(renderString, captureString) )
	{
		text.append("\nWindows Core Audio render device: ");
		text.append( QString::fromUtf16( reinterpret_cast<const ushort*>(renderString.c_str()), renderString.size()) );
		text.append("\nWindows Core Audio capture device: ");
		text.append( QString::fromUtf16( reinterpret_cast<const ushort*>(captureString.c_str()), captureString.size()) );
	}
	else
	{
		text.append("\nWindows Core Audio render device: N/A");
		text.append("\nWindows Core Audio capture device: N/A");
	}

	superMainLayout->addWidget( new QLabel(text, this ) );

	QHBoxLayout* mainLayout = new QHBoxLayout();
	superMainLayout->addLayout(mainLayout);

	QVBoxLayout* vertLayout = new QVBoxLayout();
	mainLayout->addLayout(vertLayout);

	mButtonLeds.resize(Controller::Button_Count);
	for ( std::size_t i=0; i<mButtonLeds.size(); ++i )
	{
		Controller::ButtonID buttonID = static_cast<Controller::ButtonID>(i);
		
		QHBoxLayout* layout = new QHBoxLayout();
		
		QLedWidget* led = new QLedWidget(this);
		led->setMinimumWidth(24);
		led->setMaximumWidth(24);
		led->setEnabled(false); // The button won't change state on user clicks
		led->setChecked( mController->isButtonPressed(buttonID) );
		layout->addWidget(led);
		
		QLabel* label = new QLabel(mController->getButtonName(buttonID), this);
		label->setMinimumWidth( textWidth );
		label->setMaximumWidth( textWidth );
		label->setEnabled( mController->hasButton(buttonID) );

		label->setEnabled( mController->hasButton(buttonID) );
		
		layout->addWidget(label);
		vertLayout->addLayout(layout);

		mButtonLeds[i] = led;
	}
	vertLayout->addStretch();

	vertLayout = new QVBoxLayout();
	mainLayout->addLayout(vertLayout);
	
	mTriggerProgressBars.resize(Controller::Trigger_Count);
	for ( std::size_t i=0; i<mTriggerProgressBars.size(); ++i )
	{
		Controller::TriggerID triggerID = static_cast<Controller::TriggerID>(i);

		QHBoxLayout* layout = new QHBoxLayout();
		
		QLabel* label = new QLabel(mController->getTriggerName(triggerID), this);
		label->setMinimumWidth( textWidth );
		label->setMaximumWidth( textWidth );
		label->setEnabled( mController->hasTrigger(triggerID) );
		layout->addWidget(label);

		QProgressBar* progressBar = new QProgressBar(this);
		progressBar->setMinimum( std::numeric_limits<BYTE>::min() );
		progressBar->setMaximum( std::numeric_limits<BYTE>::max() );
		progressBar->setValue( mController->getTriggerPosition(triggerID) );
		progressBar->setStyleSheet(" QProgressBar { border: 1px solid grey; text-align: center; } QProgressBar::chunk {background-color: #3add36; width: 1px;}");
		layout->addWidget(progressBar);
	
		vertLayout->addLayout(layout);
		
		mTriggerProgressBars[i].first = label;
		mTriggerProgressBars[i].second = progressBar;
	}

	mThumbstickWidgets.resize(Controller::Thumbstick_Count);
	for ( std::size_t i=0; i<mThumbstickWidgets.size(); ++i )
	{
		Controller::ThumbstickID thumbstickID = static_cast<Controller::ThumbstickID>(i);

		QHBoxLayout* layout = new QHBoxLayout();
		
		QLabel* label = new QLabel(mController->getThumbstickName(thumbstickID), this);
		label->setMinimumWidth( textWidth );
		label->setMaximumWidth( textWidth );
		label->setEnabled( mController->hasThumbstick(thumbstickID) );
		layout->addWidget(label);

		QThumbstickWidget* thumbstickWidget = new QThumbstickWidget(this);
		thumbstickWidget->setMinimumHeight( 100 );
		thumbstickWidget->setMaximumHeight( 100 );
		layout->addWidget(thumbstickWidget);
		
		vertLayout->addLayout(layout);
		
		mThumbstickWidgets[i].first = label;
		mThumbstickWidgets[i].second = thumbstickWidget;
	}
	
	mBatteriesProgressBars.resize(Controller::Battery_Count);
	for ( std::size_t i=0; i<mBatteriesProgressBars.size(); ++i )
	{
		Controller::BatteryID batteryID = static_cast<Controller::BatteryID>(i);
		
		QHBoxLayout* layout = new QHBoxLayout();
		
		QLabel* label = new QLabel(mController->getBatteryName(batteryID), this);
		label->setMinimumWidth( textWidth );
		label->setMaximumWidth( textWidth );
		// The battery capability is dynamic and is handled in the updateBatteryUI method
		layout->addWidget(label);

		QProgressBar* progressBar = new QProgressBar(this);
		progressBar->setMinimum( 0 );
		progressBar->setMaximum( Controller::getBatteryLevelMax() );
		progressBar->setStyleSheet(" QProgressBar { border: 1px solid grey; text-align: center; } QProgressBar::chunk {background-color: #3add36; width: 1px;}");
		layout->addWidget( progressBar );
		
		vertLayout->addLayout(layout);
		
		mBatteriesProgressBars[i].first = label;
		mBatteriesProgressBars[i].second = progressBar;
	}

	mVibrationMotorsSliders.resize(Controller::VibrationMotor_Count);
	for ( std::size_t i=0; i<mVibrationMotorsSliders.size(); ++i )
	{
		Controller::VibrationMotorID vibrationMotorID = static_cast<Controller::VibrationMotorID>(i);
		
		QHBoxLayout* layout = new QHBoxLayout();
		
		QLabel* label = new QLabel(mController->getVibrationMotorName(vibrationMotorID), this);
		label->setMinimumWidth( textWidth );
		label->setMaximumWidth( textWidth );
		layout->addWidget(label);

		QSlider* slider = new QSlider(Qt::Horizontal, this);
		slider->setMinimum( std::numeric_limits<WORD>::min() );
		slider->setMaximum( std::numeric_limits<WORD>::max() );
		slider->setSingleStep( std::numeric_limits<WORD>::max() / 50 );
		bool ret = connect( slider, SIGNAL( valueChanged (int) ), this, SLOT( onVibrationMotorSliderValueChanged(int) ) );
		assert(ret);
		layout->addWidget( slider );
				
		vertLayout->addLayout(layout);
		
		mVibrationMotorsSliders[i].first = label;
		mVibrationMotorsSliders[i].second = slider;
	}

	vertLayout->addStretch();
}

void QControllerWidget::updateWidgets()
{
	for ( std::size_t i=0; i<mButtonLeds.size(); ++i )
		updateButtonUI( static_cast<Controller::ButtonID>(i) );
		
	for ( std::size_t i=0; i<mTriggerProgressBars.size(); ++i )
		updateTriggerUI( static_cast<Controller::TriggerID>(i) );

	for ( std::size_t i=0; i<mThumbstickWidgets.size(); ++i )
		updateThumbstickUI(static_cast<Controller::ThumbstickID>(i) );
	
	for ( std::size_t i=0; i<mBatteriesProgressBars.size(); ++i )
		updateBatteryUI( static_cast<Controller::BatteryID>(i) );

	for ( std::size_t i=0; i<mVibrationMotorsSliders.size(); ++i )
		updateVibrationMotorUI( static_cast<Controller::VibrationMotorID>(i) );
}

void QControllerWidget::updateButtonUI( Controller::ButtonID buttonID )
{
	mButtonLeds[buttonID]->setChecked( mController->isButtonPressed(buttonID) );
}

void QControllerWidget::updateTriggerUI( Controller::TriggerID triggerID )
{
	BYTE position = mController->getTriggerPosition(triggerID);
	mTriggerProgressBars[triggerID].second->setValue(position);
	
	QString text;
	text.append( mController->getTriggerName( triggerID ) );
	text.append( QString("\n%1").arg(position));
	mTriggerProgressBars[triggerID].first->setText(text);
}

void QControllerWidget::updateThumbstickUI( Controller::ThumbstickID thumbstickID )
{
	SHORT positionX = 0;
	SHORT positionY = 0;
	mController->getThumbstickPosition( thumbstickID, positionX, positionY );
		
	float posMin = static_cast<float>( std::numeric_limits<SHORT>::min() );
	float posMax = static_cast<float>( std::numeric_limits<SHORT>::max() );
	float x = static_cast<float>(positionX) / ((posMax-posMin));
	float y = static_cast<float>(positionY) / ((posMax-posMin));
	mThumbstickWidgets[thumbstickID].second->setPosition( x, y );

	QString text;
	text.append( mController->getThumbstickName( thumbstickID ) );
	text.append( QString("\n%1, %2").arg(positionX).arg(positionY) );
	text.append( QString("\n%1, %2").arg(x, 0, 'g', 3).arg(y, 0, 'g', 3) );
	mThumbstickWidgets[thumbstickID].first->setText(text);
}

void QControllerWidget::updateBatteryUI( Controller::BatteryID batteryID )
{
	mBatteriesProgressBars[batteryID].first->setEnabled( mController->hasBattery(batteryID) );
	mBatteriesProgressBars[batteryID].second->setValue( mController->getBatteryLevel(batteryID) ); 
}

void QControllerWidget::updateVibrationMotorUI( Controller::VibrationMotorID vibrationMotorID )
{
	QString text;
	WORD speed = mController->getVibrationMotorSpeed( vibrationMotorID );
	text.append( mController->getVibrationMotorName( vibrationMotorID ) );
	text.append( QString("\n%1").arg(speed) );
	mVibrationMotorsSliders[vibrationMotorID].first->setText( text );

	mVibrationMotorsSliders[vibrationMotorID].second->setValue(speed);
}

void QControllerWidget::onVibrationMotorSliderValueChanged( int value )
{
	for ( std::size_t i=0; i<mVibrationMotorsSliders.size(); ++i )
	{
		QSlider* slider = mVibrationMotorsSliders[i].second;
		if ( slider==sender() )
		{
			Controller::VibrationMotorID motorID = static_cast<Controller::VibrationMotorID>(i);
			mController->setVibrationMotorSpeed( motorID, static_cast<WORD>(value) );
		}
	}
}

void QControllerWidget::onComponentChanged( Controller* /*controller*/, Controller::ComponentTypeID componentTypeID, int componentID )
{
	if ( componentTypeID==Controller::ComponentType_Button )
		updateButtonUI( static_cast<Controller::ButtonID>(componentID) );
	else if ( componentTypeID==Controller::ComponentType_Trigger )
		updateTriggerUI( static_cast<Controller::TriggerID>(componentID) );
	else if ( componentTypeID==Controller::ComponentType_Thumbstick )
		updateThumbstickUI( static_cast<Controller::ThumbstickID>(componentID) );
	else if ( componentTypeID==Controller::ComponentType_VibrationMotor )
		updateVibrationMotorUI( static_cast<Controller::VibrationMotorID>(componentID) );
	else if ( componentTypeID==Controller::ComponentType_Battery )
		updateBatteryUI( static_cast<Controller::BatteryID>(componentID) );
}

std::string QControllerWidget::GUIDToString( const GUID& guid )
{
	const int textSize = 37; // 32 bytes for the GUID, 4 for the dashes and 1 for the terminating 0
	char text[textSize] = { '\0' };	
	sprintf_s( text, textSize, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", 
			guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7] );
	return text;
}

}