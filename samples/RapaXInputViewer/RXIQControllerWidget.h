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
#pragma once

#ifdef _MSC_VER
	#pragma warning( push )
	#pragma warning ( disable : 4127 )
	#pragma warning ( disable : 4231 )
	#pragma warning ( disable : 4251 )
	#pragma warning ( disable : 4800 )
#endif
#include <QFrame>
#include <QPushButton>
#include <QLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QSlider>
#include "RXIQLedWidget.h"
#include "RXIQThumbstickWidget.h"
#ifdef _MSC_VER
	#pragma warning(pop)
#endif

#include "RXIController.h"

namespace RXI
{

class QControllerWidget :	public QFrame,
							public Controller::Listener
{ 
	Q_OBJECT

public:
	QControllerWidget( QWidget* parent, Controller* controller, Qt::WindowFlags flags=0 );
	virtual ~QControllerWidget();

	Controller*		getController() const { return mController; }

protected slots:
	void			onVibrationMotorSliderValueChanged( int value );

protected:
	virtual void	onComponentChanged( Controller* controller, Controller::ComponentTypeID componentTypeID, int componentID );
	
	void			createWidgets();

	void			updateWidgets();
	void			updateButtonUI( Controller::ButtonID buttonID );
	void			updateTriggerUI( Controller::TriggerID triggerID );	
	void			updateThumbstickUI( Controller::ThumbstickID thumbstickID );
	void			updateBatteryUI( Controller::BatteryID batteryID );
	void			updateVibrationMotorUI( Controller::VibrationMotorID vibrationMotorID );

private:
	static std::string GUIDToString( const GUID& guid );

	Controller*											mController;
	std::vector<QLedWidget*>							mButtonLeds;
	std::vector<std::pair<QLabel*,QProgressBar*>>		mTriggerProgressBars;
	std::vector<std::pair<QLabel*,QThumbstickWidget*>>	mThumbstickWidgets;
	std::vector<std::pair<QLabel*,QProgressBar*>>		mBatteriesProgressBars;
	std::vector<std::pair<QLabel*,QSlider*>>			mVibrationMotorsSliders;
};

}