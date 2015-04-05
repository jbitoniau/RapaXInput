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

#ifdef _XINPUT_9_1_0
	const DWORD ControllerManager::mNumMaxControllers = 4;
#else
	const DWORD ControllerManager::mNumMaxControllers = XUSER_MAX_COUNT;
#endif

unsigned int ControllerManager::mControllerEnumerationIntervalInMs = 1000;	

const char*	ControllerManager::mXInputVersionStrings[XInputVersion_Count] = 
		{
			"9.0.1",
			"1.3",
			"1.4",
		};

ControllerManager::ControllerManager()
	:	mNextControllerEnumerationTime(0),
		mControllers(),
		mListeners()
{
	mControllers.resize( getMaxNumControllers() );
	for ( DWORD i=0; i<getMaxNumControllers(); ++i )
		mControllers[i] = NULL;

	// Schedule a controller enumeration immediately
	mNextControllerEnumerationTime = Timestamp::getTimestampInMs();
}

ControllerManager::~ControllerManager()
{
	deleteAllControllers();
}

ControllerManager::XInputVersion ControllerManager::getXInputVersion()
{
#ifdef _XINPUT_9_1_0
	return XInputVersion_9_0_1;
#elif defined _XINPUT_1_3
	return XInputVersion_1_3;
#elif defined _XINPUT_1_4
	return XInputVersion_1_4;
#endif
}

void ControllerManager::update()
{
	bool enumerateControllers = false;
	
	// We check for new controllers only once in a while, as explained here:
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx
	// "For performance reasons, don't call XInputGetState for an 'empty' user slot every frame. 
	// We recommend that you space out checks for new controllers every few seconds instead."
	unsigned int time = Timestamp::getTimestampInMs();
	if ( time>=mNextControllerEnumerationTime )
	{
		enumerateControllers = true;
		mNextControllerEnumerationTime = time + mControllerEnumerationIntervalInMs;
	}

	// Update controllers
	for (DWORD i=0; i<getMaxNumControllers(); i++ )
	{
		if ( mControllers[i]!=NULL || enumerateControllers )
			updateController(i);		
	}
}

void ControllerManager::updateController( DWORD controllerIndex )
{
	if ( controllerIndex>=getMaxNumControllers() )
		return;		// Error: wrong controller index
	
	DWORD dwResult;    
	XINPUT_STATE state;
	ZeroMemory( &state, sizeof(XINPUT_STATE) );
	dwResult = XInputGetState( controllerIndex, &state );

	if( dwResult==ERROR_SUCCESS )
	{
		// The controller is connected
		Controller* controller = getController( controllerIndex );
		if ( !controller )
		{
			// It wasn't connected already, we create the Controller object 
			controller = addController( controllerIndex, &state );
			if ( !controller )
				return;			
		}

		// Update the Controller object with the current state
		controller->update( &state );
	}
	else
	{
		// The controller is not connected 
		Controller* controller = getController( controllerIndex );
		if ( controller )
		{
			// The controller was connected just before, we delete the object representing it
			deleteController( controllerIndex );
		}
	}
}

Controller*	ControllerManager::addController( DWORD controllerIndex, const void* xinputState )
{
	if ( controllerIndex>=getMaxNumControllers() )
		return NULL;		// Error: wrong controller index
	
	if ( mControllers[controllerIndex]!=NULL )
		return NULL;		// Error: a Controller object for this index already exists
	
	Controller*	controller = new Controller( controllerIndex, xinputState );
	mControllers[controllerIndex]=controller;

	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onControllerConnected( this, controller );

	return controller;
}

void ControllerManager::deleteController( DWORD controllerIndex )
{
	if ( controllerIndex>=getMaxNumControllers() )
		return;				// Error: wrong controller index

	if ( !mControllers[controllerIndex] )
		return;				// Error: no Controller object exists for this index

	Controller* controller = mControllers[controllerIndex];
	
	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onControllerDisconnecting( this, controller );

	delete controller;
	mControllers[controllerIndex]=NULL;

	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onControllerDisconnected( this, controller );
}

void ControllerManager::deleteAllControllers()
{
	for ( DWORD i=0; i<getMaxNumControllers(); ++i )
	{
		if ( mControllers[i] )
			deleteController(i);
	}
}

void ControllerManager::addListener( Listener* listener )
{
	if ( !listener )
		return;			// Error
	mListeners.push_back(listener);
}

bool ControllerManager::removeListener( Listener* listener )
{
	if ( !listener )
		return false;	// Error
	Listeners::iterator itr = std::find( mListeners.begin(), mListeners.end(), listener );
	if ( itr==mListeners.end() )
		return false;
	mListeners.erase( itr );
	return true;
}

}