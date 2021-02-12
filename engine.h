#pragma once

#include <winrt/base.h>

#include <winrt/Windows.UI.Core.h>

#include "device_resources.h"

//class Engine : public winrt::implements<Engine, winrt::Windows::Foundation::IInspectable> {
class Engine{
public:
	/*
	* const reference to a shared pointer..?
	* CORE GUIDLINES ()
	* shared_ptr: 
	* 	 Use a const shared_ptr& as a parameter only if you’re not sure whether
	*	 or not you’ll take a copy and share ownership;
	*	 otherwise use widget* instead (or if not nullable, a widget&).
	*/
	Engine(std::shared_ptr<DeviceResources> const& dr) :
		device_resources_(dr),
		window_closed_(false),
		visible_ (true){}
	~Engine() {}

	void Run() {
		using namespace winrt::Windows::UI::Core;

		CoreWindow window = CoreWindow::GetForCurrentThread();
		CoreDispatcher dispatcher = window.Dispatcher();
		// Main UWP app state loop
		while (!window_closed_) {
			if (visible_) {
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			}
			else {
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
	}

	//void Suspend() {}
	//void Resume() {}
	

	void Visibility(bool visible_state){
		// Set render needs
		visible_ = visible_state;
	}
	void Close() { window_closed_ = true; }

	std::shared_ptr<DeviceResources> device_resources_;
	bool window_closed_;
	bool visible_;
};
