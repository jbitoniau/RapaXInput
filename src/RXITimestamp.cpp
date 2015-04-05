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
#include "RXITimestamp.h"

#define WIN32_LEAN_AND_MEAN 
#define NOMINMAX 
#include <windows.h>

namespace RXI
{

unsigned long long int Timestamp::mTickFrequencyInHz = 0;
unsigned long long int Timestamp::mTickCountOffset = 0;
Timestamp Timestamp::mInitializer;

Timestamp::Timestamp()
{
	// Cache the tick frequency which doesn't change
	mTickFrequencyInHz = getTickFrequencyInHz();

	// Remember the initial tick count (at application startup) so we can
	// express the timestamp relative to it.
	mTickCountOffset = getCurrentTickCount();
}

unsigned int Timestamp::getTimestampInMs()
{
	unsigned long long int timestampInMs = (getCurrentTickCount() * 1000) / mTickFrequencyInHz;
	return static_cast<unsigned int>(timestampInMs);
}

unsigned long long int Timestamp::getTickFrequencyInHz()
{
	LARGE_INTEGER frequency = { 0, 0 };
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart;
}

unsigned long long int Timestamp::getCurrentTickCount()
{
	unsigned long long int tickCount;
	LARGE_INTEGER performanceCounter = { 0, 0 };
	QueryPerformanceCounter(&performanceCounter);
	tickCount = performanceCounter.QuadPart;
	return tickCount;
}

}