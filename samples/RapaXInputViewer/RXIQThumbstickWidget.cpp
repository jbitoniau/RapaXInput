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
#include "RXIQThumbstickWidget.h"

#include <QtGui/QPainter>

namespace RXI
{

QThumbstickWidget::QThumbstickWidget( QWidget* parent, Qt::WindowFlags flags )
	: QFrame(parent, flags),
	  mPositionX(0.f),
	  mPositionY(0.f)
{
}

void QThumbstickWidget::paintEvent( QPaintEvent* /*paintEvent*/ )
{
	QPainter painter(this);
	painter.setRenderHint( QPainter::Antialiasing, true );
	
	QColor backgroundColor( 200, 200, 200 );
	painter.setBrush(backgroundColor);
	painter.drawRect( 0, 0, width()-1, height()-1 );

	painter.setPen( QColor( 150, 150, 150 ) );
	painter.drawLine( width()/2, 0, width()/2, height() );
	painter.drawLine( 0, height()/2, width(), height()/2 );

	float cx = (width()/2.f) + (mPositionX * width());
	float cy = (height()/2.f) - (mPositionY * height());
	float r = 5;
	painter.setPen( QColor( 50, 50, 50 ) );
	QColor dotColor( 0, 200, 0 );
	QBrush brush(dotColor, Qt::SolidPattern );
	painter.setBrush(brush);
	painter.drawEllipse( QPointF(cx, cy), r, r );
}
	
void QThumbstickWidget::setPosition( float x, float y )
{
	mPositionX = x;
	mPositionY = y;
	update();
}

}

