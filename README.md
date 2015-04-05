RapaXInput
==========
A C++ object-oriented library for accessing XBox 360 controllers on Windows

![alt text](docs/RapaXInput1.jpg?raw=true "An XBox 360 controller")

# Overview 
RapaXInput (formerly [Xiol](https://code.google.com/p/xiol/)) is a C++ library that provides access to XBox 360 controllers on Windows. 

It uses the [Microsoft XInput API](https://msdn.microsoft.com/en-us/library/windows/desktop/hh405053(v=vs.85).aspx).

RapaXInput is object-oriented: the physical controllers are represented and manipulated through objects and their methods (rather than global functions and structs).

RapaXInput offers the traditional "polling" approach. You can explicitely:
* enumerate the controllers currently connected to the system, 
* inspect the state of each controller and its components (buttons, triggers, thumbsticks, etc...)

On top of that, RapaXInput proposes an event-driven approach: the client code receives notifications whenever something happens. For example:
* a controller is being connected/disconnected, 
* the state of a components changes

RapaXInput transparently supports versions 9.0.1, 1.3 and 1.4 the XInput API. It can be compiled as a 32-bit or 64-bit library.

RapaXInput comes with a couple of examples including a GUI test application

![alt text](docs/RapaXInput2.jpg?raw=true "RapaXInput test application")
