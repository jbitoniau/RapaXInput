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
#include "RXIControllerManager.h"

#include <assert.h> 
#include <sstream> 
#include <iostream> 
#include <map> 

class DebugGamepadControllerListener : public RXI::Controller::Listener
{
public:
	virtual void onComponentChanged( RXI::Controller* controller, RXI::Controller::ComponentTypeID componentTypeID, int componentID )
	{
		std::string componentTypeName = RXI::Controller::getComponentTypeName( componentTypeID );
		std::string componentName;
		std::stringstream stream;
		switch ( componentTypeID )
		{
			case RXI::Controller::ComponentType_Button : 
				componentName = RXI::Controller::getButtonName( static_cast<RXI::Controller::ButtonID>(componentID) );
				if ( controller->isButtonPressed( static_cast<RXI::Controller::ButtonID>(componentID) ) )
					stream << "on";
				else 
					stream << "off";
				break;
			case RXI::Controller::ComponentType_Trigger : 
				componentName = RXI::Controller::getTriggerName( static_cast<RXI::Controller::TriggerID>(componentID) );
				stream << (int)controller->getTriggerPosition( static_cast<RXI::Controller::TriggerID>(componentID) );
				break;
			case RXI::Controller::ComponentType_Thumbstick : 
				componentName = RXI::Controller::getThumbstickName( static_cast<RXI::Controller::ThumbstickID>(componentID) );
				SHORT x;
				SHORT y;
				controller->getThumbstickPosition( static_cast<RXI::Controller::ThumbstickID>(componentID), x, y); 
				stream << "x:" << x << " y: " << y;				
				break;
			case RXI::Controller::ComponentType_VibrationMotor : 
				componentName = RXI::Controller::getVibrationMotorName( static_cast<RXI::Controller::VibrationMotorID>(componentID) );
				WORD speed = controller->getVibrationMotorSpeed( static_cast<RXI::Controller::VibrationMotorID>(componentID) );
				stream << speed;
				break;
		}

//		std::string batteryInformation;
//		if ( controller->hasBattery(RXI::Controller::Battery_RXI::Controller) )
//			batteryInformation += "onboard bat: " + controller->getBatteryLevel(RXI::Controller::Battery_RXI::Controller)
//		bool hascb = controller->hasBattery(RXI::Controller::Battery_Controller);
//		bool haspb = controller->hasBattery(RXI::Controller::Battery_Headset);

		std::cout << "Controller " << controller->getControllerIndex() << ": ";
		std::cout << componentTypeName << ": ";
		std::cout << componentName << ": ";
		std::cout << stream.str() << std::endl;
		/*printf("RXI::Controller %d (cb:%d,%d,%d/hb:%d,%d,%d)- %s - %s - %s\n", 
			controller->getControllerIndex(),
			hascb, controller->getBatteryType(RXI::Controller::Battery_Controller), controller->getBatteryLevel(RXI::Controller::Battery_Controller),
			haspb, controller->getBatteryType(RXI::Controller::Battery_Headset), controller->getBatteryLevel(RXI::Controller::Battery_Headset),
			componentTypeName.c_str(), componentName.c_str(), stream.str().c_str()  );
	*/
		// Play with motors when button A is pressed and triggers are pushed
		if ( componentTypeID==RXI::Controller::ComponentType_Button || 
			 componentTypeID==RXI::Controller::ComponentType_Trigger )
		{
			if ( controller->isButtonPressed( RXI::Controller::Button_A ) )
			{
				WORD speedL = controller->getTriggerPosition( RXI::Controller::Trigger_Left ) * 255;
				controller->setVibrationMotorSpeed( RXI::Controller::VibrationMotor_Left, speedL );
				WORD speedR = controller->getTriggerPosition( RXI::Controller::Trigger_Right ) * 255;
				controller->setVibrationMotorSpeed( RXI::Controller::VibrationMotor_Right, speedR );
			}
			else 
			{
				controller->setVibrationMotorSpeed( RXI::Controller::VibrationMotor_Left, 0 );
				controller->setVibrationMotorSpeed( RXI::Controller::VibrationMotor_Right, 0 );
			}
		}
	}
};

class DebugControllerManagerListener : public RXI::ControllerManager::Listener
{
public:
	virtual void onControllerConnected( RXI::ControllerManager* /*controllerManager*/, RXI::Controller* controller )
	{
		printf("Controller %d - Connected (%s)\n", controller->getControllerIndex(), controller->getSubTypeName() );
		
		// Create a listener for the Controller, register it and remember it
		RXI::Controller::Listener* listener = new DebugGamepadControllerListener();
		mListeners.insert( std::make_pair( controller, listener ) );
		controller->addListener( listener );
	}

	virtual void onControllerDisconnecting( RXI::ControllerManager* /*controllerManager*/, RXI::Controller* controller )
	{
		printf("Controller %d - Disconnecting\n", controller->getControllerIndex() );
		
		// Retrieve the listener we created for the Controller, unregister it, delete it and forget it
		std::map<RXI::Controller*, RXI::Controller::Listener*>::iterator itr = mListeners.find( controller );
		assert( itr!=mListeners.end() );
		RXI::Controller::Listener* listener = (*itr).second;
		controller->removeListener( listener );
		mListeners.erase( itr );
		delete listener;
	}

private:
	std::map<RXI::Controller*, RXI::Controller::Listener*> mListeners;	
};

int main()
{
 	RXI::ControllerManager manager;

	DebugControllerManagerListener listener;
	manager.addListener( &listener );
	
	int i=0;
	while ( i<100*20)
	{
		manager.update();
		Sleep(10);
		i++;
	}
	
	manager.removeListener( &listener );
	return 0;
}
