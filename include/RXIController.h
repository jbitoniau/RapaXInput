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

#define WIN32_LEAN_AND_MEAN 
#define NOMINMAX 
#include <windows.h>

#include <vector>

namespace RXI
{

/*
	Controller
	This class represents a generic XBox 360 controller. 
	
	It's possible to identify the precise sub-type (standard gamepad, wheel, 
	arcade stick, etc...) of the actual device.

	The Controller object lets you query the presence and the state (polling) of 
	the different components on the device: the buttons, triggers, thumbsticks 
	and vibration motors. It can also be used to set the vibration motors in motion.
	
	Another way to make use of the Controller object is to register a listener to 
	the Controller's events. Whenever a component changes, the Controller fires 
	a notification to the client listener. Note that the listener is not owned by 
	the Controller.
	
	Finally, information is available concerning  the battery level and type if 
	the device is wireless. This information is also provided for an optional headset
	connected to it.
*/
class Controller
{
public:
	enum SubType
	{
		SubType_Gamepad,
		SubType_Wheel,
		SubType_ArcadeStick,
		SubType_FlightStick,		
		SubType_DancePad,
		SubType_Guitar,
		SubType_GuitarAlternate,
		SubType_DrumKit,
		SubType_GuitarBass,
		SubType_ArcadePad,
		SubType_Count
	};

	enum ComponentTypeID
	{
		ComponentType_Button,
		ComponentType_Trigger,
		ComponentType_Thumbstick,
		ComponentType_VibrationMotor,
		ComponentType_Battery,
		ComponentType_Count
	};

	enum ButtonID
	{
		Button_DPadUp,
		Button_DPadDown,
		Button_DPadLeft,
		Button_DPadRight,
		Button_Start,
		Button_Back,
		Button_LeftThumbstick,
		Button_RightThumbstick,
		Button_LeftShoulder,	
		Button_RightShoulder,
		Button_A,	
		Button_B,
		Button_X,
		Button_Y,
		Button_Count
	};

	enum TriggerID 
	{
		Trigger_Left,
		Trigger_Right,
		Trigger_Count
	};

	enum ThumbstickID
	{
		Thumbstick_Left,
		Thumbstick_Right,
		Thumbstick_Count
	};

	enum VibrationMotorID
	{
		VibrationMotor_Left,
		VibrationMotor_Right,
		VibrationMotor_Count
	};

	enum BatteryType
	{
		BatteryType_Unknown,
		BatteryType_Alkaline,
		BatteryType_NiMH,
		BatteryType_Count
	};

	enum BatteryID
	{
		Battery_Controller,
		Battery_Headset,
		Battery_Count
	};

	DWORD				getControllerIndex() const									{ return mControllerIndex; }
	bool				hasVoiceSupport() const										{ return mHasVoiceSupport; }

	static const char*	getSubTypeName( SubType subType )							{ return mSubTypeName[subType]; }
	const char*			getSubTypeName() const										{ return mSubTypeName[getSubType()]; }
	SubType				getSubType() const											{ return mSubType; }
	
	static const char*	getComponentTypeName( ComponentTypeID componentTypeID )		{ return mComponentTypeName[componentTypeID]; }
	
	static const char*	getButtonName( ButtonID buttonID )							{ return mButtonName[buttonID]; }
	bool				hasButton( ButtonID buttonID ) const						{ return mHasButton[buttonID]; }
	bool				isButtonPressed( ButtonID buttonID ) const					{ return mIsButtonPressed[buttonID]; }
	
	static const char*	getTriggerName( TriggerID triggerID )						{ return mTriggerName[triggerID]; }
	bool				hasTrigger( TriggerID triggerID ) const						{ return mHasTrigger[triggerID]; }
	BYTE				getTriggerPosition( TriggerID triggerID ) const				{ return mTriggerPosition[triggerID]; }

	static const char*	getThumbstickName( ThumbstickID thumbstickID )				{ return mThumbstickName[thumbstickID]; }
	bool				hasThumbstick( ThumbstickID thumbstickID ) const			{ return mHasThumbstick[thumbstickID]; }
	void				getThumbstickPosition( ThumbstickID thumbstickID, SHORT& positionX, SHORT& positionY ) const { positionX = mThumbstickXPosition[thumbstickID];	positionY = mThumbstickYPosition[thumbstickID]; }
	void				setThumbstickDeadZoneRadius( ThumbstickID thumbstickID, SHORT radiusX, SHORT radiusY );
	void				getThumbstickDeadZoneRadius( ThumbstickID thumbstickID, SHORT& radiusX, SHORT& radiusY ) const;

	static const char*	getVibrationMotorName( VibrationMotorID motorID )			{ return mVibrationMotorName[motorID]; }
	bool				hasVibrationMotor( VibrationMotorID motorID )				{ return mHasVibrationMotor[motorID]; }
	WORD				getVibrationMotorSpeed( VibrationMotorID motorID ) const	{ return mVibrationMotorSpeed[motorID]; }
	void				setVibrationMotorSpeed( VibrationMotorID motorID, WORD speed );

	static const char*	getBatteryTypeName( BatteryType batteryType )				{ return mBatteryTypeName[batteryType]; }
	static const char*	getBatteryName( BatteryID batteryID )						{ return mBatteryName[batteryID]; }
	bool				hasBattery( BatteryID batteryID ) const						{ return mHasBattery[batteryID]; }
	BYTE				getBatteryType( BatteryID batteryID ) const					{ return static_cast<BYTE>( mBatteryType[batteryID] ); }
	static BYTE			getBatteryLevelMax();
	// Returns a value between 0 and getBatteryLevelMax()
	BYTE				getBatteryLevel( BatteryID batteryID ) const				{ return mBatteryLevel[batteryID]; }

	// XInput 1.4 Windows 8 only. Returns empty strings otherwise.
	bool				getWindowsCoreAudioDeviceIds( std::wstring& renderDeviceId, std::wstring& captureDeviceId ) const;
	
	// XInput 1.3 non-Windows 8 only. Returns null GUIDs otherwise.
	bool				getDirectSoundAudioDeviceIds( GUID& renderGuid, GUID& captureGuid ) const;

	class Listener
	{
	public:
		virtual void onComponentChanged( Controller* /*controller*/, ComponentTypeID /*componentTypeID*/, int /*componentID*/ ) {}
	};

	typedef				std::vector<Listener*> Listeners; 
	void				addListener( Listener* listener );
	bool				removeListener( Listener* listener );
	Listeners			getListeners() const { return mListeners; }

private:
	friend class ControllerManager;
	Controller( DWORD controllerIndex, const void* xinputState );
	virtual ~Controller();

	void				clearCapabilities();
	void				clearState();
	void				updateCapabilities();
	void				update( const void* xinputState );
	
	void				setButtonPressed( ButtonID button, bool pressed );
	static BYTE			applyTriggerDeadZone( BYTE position, BYTE deadZoneRadius );
	void				setTriggerPosition( TriggerID trigger, BYTE position );
	static void			applyThumbstickDeadZone( SHORT inX, SHORT inY, SHORT& outX, SHORT& outY, SHORT deadZoneRadius );
	void				setThumbstickPosition( ThumbstickID thumbstick, SHORT positionX, SHORT positionY );
	void				getBatteryInformation( BatteryID batteryID, bool& hasBattery, BatteryType& batteryType, BYTE& batteryLevel ) const;
	void				setBatteryInformation( BatteryID batteryID, bool hasBattery, BatteryType batteryType, BYTE batteryLevel );
	
	static SubType		xinputSubTypeToSubType( int xinputSubType );
	static bool			xinputBatteryTypeToBatteryType( BYTE xinputBatteryType, Controller::BatteryType& batteryType );

	static const char*	mSubTypeName[SubType_Count];
	static const char*	mComponentTypeName[ComponentType_Count];
	static const char*	mButtonName[Button_Count];
	static const WORD	mButtonXInputID[Button_Count];
	static const char*	mTriggerName[Trigger_Count];
	static const char*	mThumbstickName[Thumbstick_Count];
	static const char*	mVibrationMotorName[VibrationMotor_Count];
	static const char*	mBatteryTypeName[BatteryType_Count];
	static const char*	mBatteryName[Battery_Count];
	
	static const unsigned int mBatteryUpdateIntervalInMs = 10000;	
	
	// Controller information
	DWORD				mControllerIndex;
	SubType				mSubType;
	
	// Capabilities
	bool				mHasVoiceSupport;
	bool				mHasButton[Button_Count];		
	bool				mHasTrigger[Trigger_Count];
	bool				mHasThumbstick[Thumbstick_Count];
	bool				mHasVibrationMotor[VibrationMotor_Count];
	bool				mHasBattery[Battery_Count];
	
	// State
	unsigned int		mNextBatteryUpdateTime;
	DWORD				mLastPacketNumber;	
	bool				mIsButtonPressed[Button_Count];		
	BYTE				mTriggerPosition[Trigger_Count];
	BYTE				mTriggerDeadZone[Trigger_Count];
	SHORT				mThumbstickXPosition[Thumbstick_Count];
	SHORT				mThumbstickYPosition[Thumbstick_Count];
	SHORT				mThumbstickDeadZoneRadius[Thumbstick_Count];
	WORD				mVibrationMotorSpeed[VibrationMotor_Count];
	BatteryType			mBatteryType[Battery_Count];
	BYTE				mBatteryLevel[Battery_Count];
	
	// Listeners
	Listeners			mListeners;
};

}
