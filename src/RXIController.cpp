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
#include "RXIController.h"

// XInput must be included after windows.h
#include <XInput.h>

// Create a define corresponding to the version of XInput (none is officially provided!)
// Here is some information about XInput versions:
// http://msdn.microsoft.com/en-us/library/windows/desktop/hh405051(v=vs.85).aspx
#ifdef XINPUT_DEVSUBTYPE_WHEEL					// Exists only since XInput 1.3 
	#ifdef XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE	// Exists only since XInput 1.4
		#define _XINPUT_1_4
	#else
		#define _XINPUT_1_3
	#endif
#else
	#define _XINPUT_9_1_0
#endif

#include <algorithm>
#include "RXITimestamp.h"

namespace RXI
{

const char*	Controller::mSubTypeName[SubType_Count] = 
		{
			"XBox 360 Gamepad",
			"XBox 360 Wheel",
			"XBox 360 Arcade Stick",
			"XBox 360 Flight Stick",
			"XBox 360 Dance Pad",
			"XBox 360 Guitar",
			"XBox 360 Guitar Alternate",
			"XBox 360 Drum Kit",
			"XBox 360 Guitar Bass",
			"XBox 360 Arcade Pad"
		};

const char*	Controller::mComponentTypeName[ComponentType_Count] = 
		{
			"Button",
			"Trigger",
			"Thumbstick",
			"Vibration Motor",
			"Battery"
		};

const char*	Controller::mButtonName[Button_Count] = 
		{ 
			"DPad Up", 
			"DPad Down",
			"DPad Left",
			"DPad Right",
			"Start",
			"Back",
			"Left Thumbstick",
			"Right Thumbstick",
			"Left Shoulder",
			"Right Shoulder",
			"A",	
			"B",
			"X",
			"Y"
		};

// For each button of the ButtonID enum, this array gives the 
// corresponding XInput constant to be used as a bit-mask
const WORD	Controller::mButtonXInputID[Button_Count] = 
		{
			XINPUT_GAMEPAD_DPAD_UP,
			XINPUT_GAMEPAD_DPAD_DOWN,      
			XINPUT_GAMEPAD_DPAD_LEFT,      
			XINPUT_GAMEPAD_DPAD_RIGHT,     
			XINPUT_GAMEPAD_START,     
			XINPUT_GAMEPAD_BACK,         
			XINPUT_GAMEPAD_LEFT_THUMB,
			XINPUT_GAMEPAD_RIGHT_THUMB,    
			XINPUT_GAMEPAD_LEFT_SHOULDER,  
			XINPUT_GAMEPAD_RIGHT_SHOULDER, 
			XINPUT_GAMEPAD_A,
			XINPUT_GAMEPAD_B,             
			XINPUT_GAMEPAD_X,             
			XINPUT_GAMEPAD_Y
		};

const char*	Controller::mTriggerName[Trigger_Count] = 
		{
			"Left Trigger",
			"Right Trigger"
		};

const char*	Controller::mThumbstickName[Thumbstick_Count] =
		{
			"Left Thumstick",
			"Right Thumstick"
		};

const char*	Controller::mVibrationMotorName[VibrationMotor_Count] = 
		{
			"Left Motor",
			"Right Motor"
		};

const char*	Controller::mBatteryTypeName[BatteryType_Count] = 
		{
			"Unknown",
			"Alkaline",
			"NiMH"			
		};

const char*	Controller::mBatteryName[Battery_Count]  = 
		{
			"Controller",
			"Headset"
		};

Controller::Controller( DWORD controllerIndex, const void* xinputState )
	:	mControllerIndex(controllerIndex),
		mSubType(SubType_Gamepad),
		mHasVoiceSupport(false),
		//mHasButton(),		
		//mHasTrigger(),
		//mHasThumbstick(),
		//mHasVibrationMotor(),
		//mHasBattery(),
		mNextBatteryUpdateTime(0),
		mLastPacketNumber(0),
		//mIsButtonPressed(),
		//mTriggerPosition(),
		//mTriggerDeadZone(),
		//mThumbstickXPosition(),
		//mThumbstickYPosition(),
		//mThumbstickDeadZoneRadius(),
		//mVibrationMotorSpeed(),
		//mBatteryType(),
		//mBatteryLevel(),
		mListeners()
{
	// Clear members
	clearCapabilities();
	clearState();

	// Initialize capabilities (buttons, thumbsticks, etc...)
	updateCapabilities();

	// Initialize dead zones
	mThumbstickDeadZoneRadius[Thumbstick_Left] = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	mThumbstickDeadZoneRadius[Thumbstick_Right] = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	mTriggerDeadZone[Trigger_Left] = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
	mTriggerDeadZone[Trigger_Right] = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

	// Update from initial state (ensuring batter information is also updated)
	mNextBatteryUpdateTime = Timestamp::getTimestampInMs();
	update(xinputState);

	// Ensure the motors are stopped
	for ( int i=0; i<VibrationMotor_Count; ++i )
	{
		mVibrationMotorSpeed[static_cast<VibrationMotorID>(i)] = 1;		// Make sure setter call below will effectively affect device/hardware 
		setVibrationMotorSpeed( static_cast<VibrationMotorID>(i), 0 );
	}
}

Controller::~Controller()
{
	// Ensure the motors are stopped
	for ( int i=0; i<VibrationMotor_Count; ++i )
		setVibrationMotorSpeed( static_cast<VibrationMotorID>(i), 0 );
}

void Controller::clearCapabilities()
{
	for ( int i=0; i<Button_Count; ++i )
		mHasButton[i] = false;

	for ( int i=0; i<Trigger_Count; ++i )
		mHasTrigger[i] = false;

	for ( int i=0; i<Thumbstick_Count; ++i )
		mHasThumbstick[i] = false;

	for ( int i=0; i<VibrationMotor_Count; ++i )
		mHasVibrationMotor[i] = false;

	for ( int i=0; i<Battery_Count; ++i )
		mHasBattery[i] = false;
}

void Controller::clearState()
{
	for ( int i=0; i<Button_Count; ++i )
		mIsButtonPressed[i] = false;

	for ( int i=0; i<Trigger_Count; ++i )
	{
		mTriggerPosition[i] = 0;
		mTriggerDeadZone[i] = 0;
	}

	for ( int i=0; i<Thumbstick_Count; ++i )
	{
		mThumbstickXPosition[i] = 0;
		mThumbstickYPosition[i] = 0;
		mThumbstickDeadZoneRadius[i] = 0;
	}

	for ( int i=0; i<VibrationMotor_Count; ++i )
		mVibrationMotorSpeed[i] = 0;

	for ( int i=0; i<Battery_Count; ++i )
	{
		mBatteryType[i] = BatteryType_Unknown;
		mBatteryLevel[i] = 0;
	}
}

// Update the capabilities of the controller in terms of components
// The batteries capabilities are left aside as they can change dynamically.
void Controller::updateCapabilities()
{
	XINPUT_CAPABILITIES capabilities;
	DWORD dwResult;
	ZeroMemory( &capabilities, sizeof(XINPUT_CAPABILITIES) );
	dwResult = XInputGetCapabilities( getControllerIndex(), XINPUT_FLAG_GAMEPAD, &capabilities );
	if ( dwResult!=ERROR_SUCCESS )
		return;			// Error: failed to read capabilities
	
	// Sub-type
	mSubType = xinputSubTypeToSubType( capabilities.SubType );

	// Miscellaneous capability flags
	WORD flags = capabilities.Flags ;
	mHasVoiceSupport = (flags & XINPUT_CAPS_VOICE_SUPPORTED)!=0;
	// We should support these on XInput 1.4 (see http://msdn.microsoft.com/en-us/library/windows/desktop/hh405051(v=vs.85).aspx)
	// - XINPUT_CAPS_FFB_SUPPORTED 
	// - XINPUT_CAPS_WIRELESS
	// - XINPUT_CAPS_PMD_SUPPORTED 
	// - XINPUT_CAPS_NO_NAVIGATION

	// Available buttons
	const XINPUT_GAMEPAD& gamepad = capabilities.Gamepad;
	WORD buttonStates = gamepad.wButtons;
	for ( int i=0; i<Button_Count; ++i )
	{
		bool isAvailable = ( buttonStates & mButtonXInputID[i] )!=0;
		mHasButton[i] = isAvailable;
	}

	// Available triggers
	// Note: we're assuming here that if the value is non-zero then the trigger is available. 
	// This has to be tested on exotic controllers with no trigger
	mHasTrigger[Trigger_Left] = (gamepad.bLeftTrigger != 0);
	mHasTrigger[Trigger_Right] = (gamepad.bRightTrigger != 0);
	
	// Availabe Thumbsticks (same remark)
	mHasThumbstick[Thumbstick_Left] = (gamepad.sThumbLX!=0 && gamepad.sThumbLY!=0);	
	mHasThumbstick[Thumbstick_Right] = (gamepad.sThumbRX!=0 && gamepad.sThumbRY!=0);	

	// Available Motors (same remark)
	mHasVibrationMotor[VibrationMotor_Left] = ( capabilities.Vibration.wLeftMotorSpeed!=0 );
	mHasVibrationMotor[VibrationMotor_Right] = ( capabilities.Vibration.wRightMotorSpeed!=0 );
}

void Controller::setVibrationMotorSpeed( VibrationMotorID motorID, WORD speed )
{
	if ( !hasVibrationMotor(motorID) )
		return;

	if ( mVibrationMotorSpeed[motorID]==speed )
		return;
	
	XINPUT_VIBRATION vibrationStruct;
	ZeroMemory( &vibrationStruct, sizeof(XINPUT_VIBRATION) );
	if ( motorID==VibrationMotor_Left )
	{
		vibrationStruct.wLeftMotorSpeed = speed;
		vibrationStruct.wRightMotorSpeed = mVibrationMotorSpeed[VibrationMotor_Right];
	}
	else if ( motorID==VibrationMotor_Right )
	{
		vibrationStruct.wLeftMotorSpeed = mVibrationMotorSpeed[VibrationMotor_Left];
		vibrationStruct.wRightMotorSpeed = speed;
	}

	DWORD dwResult = XInputSetState( getControllerIndex(), &vibrationStruct );
	if ( dwResult!=ERROR_SUCCESS )
		return;			// Failed to change the value of the motors

	if ( motorID==VibrationMotor_Left )
		mVibrationMotorSpeed[VibrationMotor_Left] = speed;
	else if ( motorID==VibrationMotor_Right )
		mVibrationMotorSpeed[VibrationMotor_Right] = speed;

	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onComponentChanged( this, ComponentType_VibrationMotor, motorID );
}

void Controller::setButtonPressed( ButtonID buttonID, bool pressed )
{
	if ( buttonID>=Button_Count )
		return;
	if ( !hasButton(buttonID) )
		return;
	if ( mIsButtonPressed[buttonID]==pressed )
		return;

	mIsButtonPressed[buttonID] = pressed;

	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onComponentChanged( this, ComponentType_Button, buttonID );
}

BYTE Controller::applyTriggerDeadZone( BYTE position, BYTE deadZoneRadius )
{
	BYTE outPosition = 0;
	if ( position>deadZoneRadius )
	{
		float fullRange = static_cast<float>( std::numeric_limits<BYTE>::max() );
		float validRange = static_cast<float>( std::numeric_limits<BYTE>::max() - deadZoneRadius );
		float normalizedPos = static_cast<float>(position-deadZoneRadius) / validRange;
		if ( normalizedPos>1.f )
			normalizedPos=1.f;					// Error: this should never happen!
		float pos =  normalizedPos * fullRange;
		if ( pos>fullRange )
			pos = fullRange;					// Error: Again this is paranoia
		outPosition = static_cast<BYTE>( pos );
	}
	return outPosition;
}

void Controller::setTriggerPosition( TriggerID triggerID, BYTE position )
{
	if ( triggerID>=Trigger_Count )
		return;
	if ( !hasTrigger(triggerID) )
		return;
	
	BYTE pos = applyTriggerDeadZone( position, mTriggerDeadZone[triggerID] );
	if ( mTriggerPosition[triggerID]==pos)
		return;
	
	mTriggerPosition[triggerID] = pos;
		
	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onComponentChanged( this, ComponentType_Trigger, triggerID );
}

void Controller::applyThumbstickDeadZone( SHORT inX, SHORT inY, SHORT& outX, SHORT& outY, SHORT deadZoneRadius )
{
	// Inspired from http://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx#dead_zone
	outX = inX;
	outY = inY;
	if (deadZoneRadius<=0)		// Negative radius is considered same as null
		return;

	float x = static_cast<float>(inX);
	float y = static_cast<float>(inY);
	float magnitude = sqrt(x*x + y*y);
	float dirX = x / magnitude;
	float dirY = y / magnitude;
		
	float radius = static_cast<float>( deadZoneRadius );
	if ( magnitude>radius)
	{
		// The thumbstick is outside the dead zone
		
		// Clip the magnitude at its expected maximum value
		float maxMagnitude = static_cast<float>( std::numeric_limits<SHORT>::max() );
		if (magnitude>maxMagnitude) 
			magnitude=maxMagnitude;
  
		// Determine corrected and normalized magnitude
		float normalizedMagnitude = (magnitude-radius) / (maxMagnitude-radius);

		// Apply the calculated magnitude to the the full-magnitude direction vector 
		outX = static_cast<SHORT>( dirX * normalizedMagnitude * maxMagnitude );
		outY = static_cast<SHORT>( dirY * normalizedMagnitude * maxMagnitude );
	}
	else 
	{
		// The thumbstick is inside the dead zone
		outX = 0;
		outY = 0;
	}
}

void Controller::setThumbstickPosition( ThumbstickID thumbstickID, SHORT positionX, SHORT positionY )
{
	if ( thumbstickID>=Thumbstick_Count )
		return;	
	if ( !hasThumbstick(thumbstickID) )
		return;

	SHORT posX = 0;
	SHORT posY = 0;
	applyThumbstickDeadZone( positionX, positionY, posX, posY, mThumbstickDeadZoneRadius[thumbstickID] );
	
	if ( posX==mThumbstickXPosition[thumbstickID] && posY==mThumbstickYPosition[thumbstickID] )
		return;

	mThumbstickXPosition[thumbstickID] = posX;
	mThumbstickYPosition[thumbstickID] = posY;
		
	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onComponentChanged( this, ComponentType_Thumbstick, thumbstickID );
}

void Controller::update( const void* xinputState )
{
	if ( !xinputState )
		return;
	const XINPUT_STATE& state = *( static_cast<const XINPUT_STATE*>(xinputState) );
	
	// Update components state
	DWORD currentPacketNumber = state.dwPacketNumber;
	if ( currentPacketNumber!=mLastPacketNumber )
	{
		mLastPacketNumber = currentPacketNumber;
		const XINPUT_GAMEPAD& gamepad = state.Gamepad;
		
		// Buttons
		WORD buttonStates = gamepad.wButtons;
		for ( int i=0; i<Button_Count; ++i )
		{
			bool pressed = ( buttonStates & mButtonXInputID[i] )!=0;
			setButtonPressed( static_cast<ButtonID>(i), pressed );
		}

		// Triggers
		setTriggerPosition( Trigger_Left, gamepad.bLeftTrigger );
		setTriggerPosition( Trigger_Right, gamepad.bRightTrigger );
		
		// Thumbsticks
		setThumbstickPosition( Thumbstick_Left, gamepad.sThumbLX, gamepad.sThumbLY );
		setThumbstickPosition( Thumbstick_Right, gamepad.sThumbRX, gamepad.sThumbRY );
	}

	// Update battery state
	unsigned int time = Timestamp::getTimestampInMs();
	if ( time>=mNextBatteryUpdateTime )
	{
		mNextBatteryUpdateTime = time + mBatteryUpdateIntervalInMs;
		
		for ( unsigned int i=0; i<Battery_Count; ++i )
		{
			bool hasBattery = false;
			BatteryType batteryType = BatteryType_Unknown;
			BYTE batteryLevel = 0;
			getBatteryInformation( static_cast<BatteryID>(i), hasBattery, batteryType, batteryLevel );
			setBatteryInformation( static_cast<BatteryID>(i), hasBattery, batteryType, batteryLevel );
		}
	}
}

void Controller::getBatteryInformation( BatteryID batteryID, bool& hasBattery, BatteryType& batteryType, BYTE& batteryLevel ) const
{
	if ( batteryID>=Battery_Count )
		return;	
	
	hasBattery = false;
	batteryType = BatteryType_Unknown;
	batteryLevel = 0;

#ifdef _XINPUT_9_1_0
	// Battery information API not available in XInput 9.1.0
	return;
#else
	BYTE devType = 0;
	if ( batteryID==Battery_Controller ) 
		devType = BATTERY_DEVTYPE_GAMEPAD;
	else if ( batteryID==Battery_Headset ) 
		devType = BATTERY_DEVTYPE_HEADSET;
	else 
		return;				// Error: unsupported battery ID	

	DWORD dwResult;
	XINPUT_BATTERY_INFORMATION batteryInformation;
	ZeroMemory( &batteryInformation, sizeof(XINPUT_BATTERY_INFORMATION) );
	dwResult = XInputGetBatteryInformation( getControllerIndex(), devType, &batteryInformation );
	if ( dwResult!=ERROR_SUCCESS )
		return;				// Error: failed to get battery information
	
	if ( xinputBatteryTypeToBatteryType(batteryInformation.BatteryType, batteryType) )
	{
		hasBattery = true;
		batteryLevel = batteryInformation.BatteryLevel;		
		if ( batteryLevel>getBatteryLevelMax() )		
			batteryLevel = getBatteryLevelMax();	// Error: the API is returning non-sense
	}
#endif
}

void Controller::setBatteryInformation( BatteryID batteryID, bool hasBattery, BatteryType batteryType, BYTE batteryLevel )
{
	if ( batteryID>=Battery_Count )
		return;	

	bool changed = false;
	if ( hasBattery!=mHasBattery[batteryID] )		// Note: this should never happen for the Controller itself. The battery capabilities should never change over time!
	{
		mHasBattery[batteryID] = hasBattery;			
		changed = true;
	}
	
	if ( batteryType!=mBatteryType[batteryID] )		// Note: same  with battery type!
	{
		mBatteryType[batteryID] = batteryType;
		changed = true;
	}

	if ( batteryLevel!=mBatteryLevel[batteryID] )
	{
		mBatteryLevel[batteryID] = batteryLevel;
		changed = true;
	}

	// Notify
	if ( changed )
	{
		for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
			(*itr)->onComponentChanged( this, ComponentType_Battery, batteryID );
	}
}

BYTE Controller::getBatteryLevelMax()
{
#ifdef _XINPUT_9_1_0
	// Battery information API not available in XInput 9.1.0
	// We return an arbitrary number
	return 3;
#else
	return BATTERY_LEVEL_FULL;
#endif
}	

bool Controller::getWindowsCoreAudioDeviceIds( std::wstring& renderDeviceId, std::wstring& captureDeviceId ) const
{
	renderDeviceId = L"";
	captureDeviceId = L"";

#ifdef _XINPUT_1_4
	// Used hard-coded size for storing the ids as in the example:
	// http://blogs.msdn.com/b/chuckw/archive/2012/05/03/xinput-and-xaudio2.aspx
	WCHAR renderID[256] = {0};
	WCHAR captureID[256] = {0};
	UINT renderCount = 256;
	UINT captureCount = 256;
	if ( XInputGetAudioDeviceIds( getControllerIndex(), renderID, &renderCount, captureID, &captureCount  )==ERROR_SUCCESS )
	{
		renderDeviceId = renderID;
		captureDeviceId = captureID;
	}
	else
	{
		// Error: controller not connected!
	}
	return false;
#else
	return false;
#endif
}

bool Controller::getDirectSoundAudioDeviceIds( GUID& renderGuid, GUID& captureGuid ) const
{
	memset( &renderGuid, 0, sizeof(GUID) );
	memset( &captureGuid, 0, sizeof(GUID) );
	
#ifndef _XINPUT_1_4
	// DirectSound info no more available since XInput 1.4
	if ( XInputGetDSoundAudioDeviceGuids( getControllerIndex(), &renderGuid, &captureGuid )==ERROR_SUCCESS )
		return true;
	return false;
#else
	return false;  
#endif
}


Controller::SubType Controller::xinputSubTypeToSubType( int xinputSubType )
{
#ifdef _XINPUT_9_1_0
	// This is the only type supported in the 9.1.0 cut-down version
	UNREFERENCED_PARAMETER(xinputSubType);
	return SubType_Gamepad;		

#elif defined _XINPUT_1_3
	switch ( xinputSubType )
	{
		case XINPUT_DEVSUBTYPE_GAMEPAD :		return SubType_Gamepad;
		case XINPUT_DEVSUBTYPE_WHEEL :			return SubType_Wheel;
		case XINPUT_DEVSUBTYPE_ARCADE_STICK :	return SubType_ArcadeStick;
		case XINPUT_DEVSUBTYPE_FLIGHT_SICK :	return SubType_FlightStick;		// Notice the typo in "FLIGHT_SICK"!
		case XINPUT_DEVSUBTYPE_DANCE_PAD :		return SubType_DancePad;
		case XINPUT_DEVSUBTYPE_GUITAR :			return SubType_Guitar;
		case XINPUT_DEVSUBTYPE_DRUM_KIT :		return SubType_DrumKit;
	}
	return SubType_Gamepad;		// Error: should not happen

#elif defined _XINPUT_1_4
	switch ( xinputSubType )
	{
		case XINPUT_DEVSUBTYPE_GAMEPAD :			return SubType_Gamepad;
		case XINPUT_DEVSUBTYPE_WHEEL :				return SubType_Wheel;
		case XINPUT_DEVSUBTYPE_ARCADE_STICK :		return SubType_ArcadeStick;
		case XINPUT_DEVSUBTYPE_FLIGHT_STICK :		return SubType_FlightStick;		// Typo fixed in this version
		case XINPUT_DEVSUBTYPE_DANCE_PAD :			return SubType_DancePad;
		case XINPUT_DEVSUBTYPE_GUITAR :				return SubType_Guitar;
		case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE :	return SubType_GuitarAlternate;
		case XINPUT_DEVSUBTYPE_DRUM_KIT	:			return SubType_DrumKit;
		case XINPUT_DEVSUBTYPE_GUITAR_BASS : 		return SubType_GuitarBass;
		case XINPUT_DEVSUBTYPE_ARCADE_PAD : 		return SubType_ArcadePad;
	}
	return SubType_Gamepad;		// Error: should not happen

#else
	return SubType_Gamepad;		// Error: unsupported version of XInput

#endif
}

// Return false if the device is either unavailable/disconnected or of the wired kind
bool Controller::xinputBatteryTypeToBatteryType( BYTE xinputBatteryType, Controller::BatteryType& batteryType )
{
#ifdef _XINPUT_9_1_0
	// Battery information API not available in XInput 9.1.0
	UNREFERENCED_PARAMETER(xinputBatteryType);
	batteryType = BatteryType_Unknown;
	return false;
#else
	switch ( xinputBatteryType )
	{
		case BATTERY_TYPE_DISCONNECTED:
		case BATTERY_TYPE_WIRED:
			batteryType = BatteryType_Unknown;
			return false;
		case BATTERY_TYPE_ALKALINE : 
			batteryType = BatteryType_Alkaline;
			return true;
		case BATTERY_TYPE_NIMH : 
			batteryType = BatteryType_NiMH;
			return true;
		case BATTERY_TYPE_UNKNOWN : 
			batteryType = BatteryType_Unknown;
			return true;
	}
	return false;	// Error: battery type unsupported
#endif
}

void Controller::addListener( Listener* listener )
{
	if ( !listener )
		return;
	mListeners.push_back(listener);
}

bool Controller::removeListener( Listener* listener )
{
	if ( !listener )
		return false;
	Listeners::iterator itr = std::find( mListeners.begin(), mListeners.end(), listener );
	if ( itr==mListeners.end() )
		return false;
	mListeners.erase( itr );
	return true;
}

}