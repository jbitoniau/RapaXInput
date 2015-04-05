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
#include "RXIQControllerManagerWidget.h"

#include <assert.h>
#include "RXIQControllerWidget.h"

namespace RXI
{

QControllerManagerWidget::QControllerManagerWidget( QWidget* parent, ControllerManager* controllerManager, Qt::WindowFlags flags )
	:	QFrame(parent, flags), 
		mControllerManager(controllerManager),
		mControllerListWidget(NULL)
{
	setWindowTitle( "XInput Controller Manager" );

	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout( mainLayout);

	QString text = "XInput version ";
	text.append( ControllerManager::getXInputVersionString() );
	QLabel* label = new QLabel( text, this );
	mainLayout->addWidget(label);

	mControllerListWidget = new QListWidget( this );
	mainLayout->addWidget( mControllerListWidget );
	bool ret = false;
	ret = connect( mControllerListWidget, SIGNAL( itemDoubleClicked(QListWidgetItem*) ), SLOT( onListItemDoubleCliked(QListWidgetItem*) ) );

	mTimer = new QTimer(this);
	ret = connect( mTimer, SIGNAL( timeout() ), SLOT( onTimerTimeOut() ) );
	assert( ret );
	mTimer->setInterval(20);
	mTimer->start();

	mControllerManager->addListener(this);
}

QControllerManagerWidget::~QControllerManagerWidget()
{
	mControllerManager->removeListener(this);
}

void QControllerManagerWidget::onTimerTimeOut()
{
	mControllerManager->update();
}

Controller* QControllerManagerWidget::getControllerFromItem( QListWidgetItem* item ) const
{
	int controllerPointer = item->data( mItemDataKey ).toInt();
	assert(controllerPointer);
	Controller* controller = reinterpret_cast<Controller*>(controllerPointer);
	return controller;
}

QListWidgetItem* QControllerManagerWidget::getItemFromController( Controller* controller ) const
{
	for ( int i=0; i<mControllerListWidget->count(); ++i )
	{
		QListWidgetItem* item = mControllerListWidget->item(i);
		int controllerPointer = item->data( mItemDataKey ).toInt();
		assert(controllerPointer);
		Controller* d = reinterpret_cast<Controller*>(controllerPointer);
		if ( d==controller )
			return item;
	}
	return NULL;
}

QControllerWidget* QControllerManagerWidget::getControllerWidgetFromItem( QListWidgetItem* item ) const
{
	Controller* controller = getControllerFromItem(item);
	int controllerPointer = reinterpret_cast<int>(controller);
	QString objectName = QString("Controller#%1").arg(controllerPointer);
	QControllerWidget* controllerWidget = findChild<QControllerWidget*>(objectName);
	return controllerWidget;
}	

void QControllerManagerWidget::onListItemDoubleCliked( QListWidgetItem* item )
{
	QControllerWidget* controllerWidget = getControllerWidgetFromItem(item);
	if ( controllerWidget )
		return;

	Controller* controller = getControllerFromItem(item);
	controllerWidget = new QControllerWidget(this, controller, Qt::Window);
	controllerWidget->setAttribute(Qt::WA_DeleteOnClose);
	int controllerPointer = reinterpret_cast<int>(controller);
	QString objectName = QString("Controller#%1").arg(controllerPointer);
	controllerWidget->setObjectName(objectName);
	QString title = QString("Controller #%1").arg( controller->getControllerIndex() );
	controllerWidget->setWindowTitle( title );
	controllerWidget->resize( 400, 400 );	
	controllerWidget->show();
}

void QControllerManagerWidget::onControllerConnected( ControllerManager* /*controllerManager*/, Controller* /*controller*/ )
{
	updateControllerListWidget();
}

void QControllerManagerWidget::onControllerDisconnecting( ControllerManager* /*controllerManager*/, Controller* controller )
{
	QListWidgetItem* item = getItemFromController(controller);
	QControllerWidget* controllerWidget = getControllerWidgetFromItem(item);
	if ( controllerWidget )
		delete controllerWidget;
}
	
void QControllerManagerWidget::onControllerDisconnected( ControllerManager* /*controllerManager*/, Controller* /*controller*/ )
{
	updateControllerListWidget();
}

void QControllerManagerWidget::updateControllerListWidget()
{
	mControllerListWidget->clear();
	for ( std::size_t i=0; i<mControllerManager->getMaxNumControllers(); ++i )
	{
		Controller* controller = mControllerManager->getController(i);
		if ( controller ) 
		{ 
			QListWidgetItem* item = new QListWidgetItem( QString("Controller #%1").arg(i) );
			mControllerListWidget->addItem( item );
			int controllerPointer = reinterpret_cast<int>(controller);
			item->setData( mItemDataKey, controllerPointer );
		}	
	}
}

}

