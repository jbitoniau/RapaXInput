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
#include <QTimer>
#include <QFrame>
#include <QListWidget>
#include <QPushButton>
#include <QLayout>
#ifdef _MSC_VER
	#pragma warning( pop )
#endif

#include "RXIControllerManager.h"

namespace RXI
{

class QControllerWidget;

class QControllerManagerWidget : public QFrame,
								 public ControllerManager::Listener
{ 
	Q_OBJECT

public:
	QControllerManagerWidget( QWidget* parent, ControllerManager* controllerManager, Qt::WindowFlags flags=0  );
	virtual ~QControllerManagerWidget();

protected:
	virtual void		onControllerConnected( ControllerManager* controllerManager, Controller* controller );
	virtual void		onControllerDisconnecting( ControllerManager* controllerManager, Controller* controller );
	virtual void		onControllerDisconnected( ControllerManager* controllerManager, Controller* controller );

	Controller*			getControllerFromItem( QListWidgetItem* item ) const;
	QListWidgetItem*	getItemFromController( Controller* device ) const;
	QControllerWidget*	getControllerWidgetFromItem( QListWidgetItem* item ) const;
	
	void				updateControllerListWidget();

private slots:
	void				onTimerTimeOut();
	void				onListItemDoubleCliked( QListWidgetItem* item );

private:
	static const int	mItemDataKey = 0x1234;
	ControllerManager*	mControllerManager;
	QTimer*				mTimer;
	QListWidget*		mControllerListWidget;
};

}
