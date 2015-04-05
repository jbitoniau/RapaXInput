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

#include "RXIController.h"

namespace RXI
{

class ControllerEnumerationTrigger;

/*
	ControllerManager
	
	The ControllerManager is responsible for maintaining the list of Controllers 
	connected to the machine and providing it to the client code.

	The client code must update the ControllerManager regularly in order for the 
	manager do to its job (updating the list of Controllers and the Controllers themselves).

	Besides the polling approach, the client code can register to notifications that
	inform it of newly connected or removed Controllers (the notifications are sent
	during the update). Note that the listener is not owned by the ControllerManager.
*/
class ControllerManager
{
public:
	ControllerManager();
	virtual ~ControllerManager();
	
	DWORD		getMaxNumControllers() const					{ return mNumMaxControllers; }
	Controller*	getController( DWORD controllerIndex ) const	{ return mControllers[controllerIndex]; }
	
	void		update();
	
	enum XInputVersion
	{
		XInputVersion_9_0_1,
		XInputVersion_1_3,
		XInputVersion_1_4,
		XInputVersion_Count
	};

	static XInputVersion getXInputVersion();
	static const char*	 getXInputVersionString()				{ return mXInputVersionStrings[getXInputVersion()]; }
		
	class Listener
	{
	public:
		// Called whenever a controller is being connected. The manager hasn't created the Controller object yet.
		virtual void	onControllerConnecting( ControllerManager* /*controllerManager*/ ) {}
		
		// Called whenever a controller has been connected. The Controller object has been created by the manager, 
		// is fully initialized and is passed as a parameter. 
		virtual void	onControllerConnected( ControllerManager* /*controllerManager*/, Controller* /*controller*/ ) {}

		// Called whenever a controller is being disconnected. The manager hasn't yet deleted the Controller object
		// so it's still present in the list. This gives a last chance to do things before it gets removed 
		virtual void	onControllerDisconnecting( ControllerManager* /*controllerManager*/, Controller* /*controller*/ ) {}
		
		// Called whenever a controller is disconnected. The Controller object passed as a parameter has been deleted 
		// by the manager and *should not be used*. It is passed for information purpose only.
		virtual void	onControllerDisconnected( ControllerManager* /*controllerManager*/, Controller* /*controller*/ ) {}
	};
	
	typedef			std::vector<Listener*> Listeners; 
	void			addListener( Listener* listener );
	bool			removeListener( Listener* listener );
	Listeners		getListeners() const { return mListeners; }

private:
	Controller*		addController( DWORD controllerIndex, const void* xinputState );
	void			updateController( DWORD controllerIndex );
	void			deleteController( DWORD controllerIndex );
	void			deleteAllControllers();

	static const DWORD			mNumMaxControllers;
	static unsigned int			mControllerEnumerationIntervalInMs;
	static const char*			mXInputVersionStrings[XInputVersion_Count];

	unsigned int				mNextControllerEnumerationTime;
	std::vector<Controller*>	mControllers;
	Listeners					mListeners;
};

}
