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
#include "RXIQLedWidget.h"

#include <QtGui/QPainter>

namespace RXI
{

QLedWidget::QLedWidget( QWidget* parent )
	: QAbstractButton(parent)
{
	setMinimumSize(24,24);
	setEnabled(false);
	setCheckable(true);
}

void QLedWidget::resizeEvent(QResizeEvent* /*event*/)
{
	update();
}

void QLedWidget::paintEvent( QPaintEvent* /*paintEvent*/ )
{
	QPainter painter(this);
	painter.setRenderHint( QPainter::Antialiasing, true );
	
	painter.setPen( QColor( 50, 50, 50 ) );
	QColor dotColor;
	if ( isChecked() )
		dotColor = QColor( 0, 200, 0 );
	else
		dotColor = QColor( 20, 100, 20 );  
	
	QBrush brush(dotColor, Qt::SolidPattern );
	painter.setBrush(brush);
	int r = 8;
	painter.drawEllipse( QPoint(width()/2, height()/2), r, r );
}

}

