#pragma once

#include <winrt/base.h>

#include <winrt/Windows.UI.Core.h>

#include "device_resources.h"
#include "renderer.h"

enum class UpdateEngineState {
	WaitingForResources,
	ResourcesLoaded,
	//WaitingForPress,
	Dynamics,
	TooSmall,
	Suspended,
	Deactivated,
};

enum class GameInfoOverlayState {
	Loading,
	GameStats,
	Pause,
};

class Engine : public /*winrt::implements<Engine, winrt::Windows::Foundation::IInspectable>,*/ IDeviceNotify 
{
//class Engine{
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
		visible_ (true){
		
		device_resources_->RegisterDeviceNotify(this);
		renderer_ = std::make_shared<Renderer>(device_resources_);

		// Asynchronously initialize the game class and load the renderer device resources.
		// By doing all this asynchronously, the game gets to its main loop more quickly
		// and in parallel all the necessary resources are loaded on other threads.
		//ConstructInBackground();
	}

//	winrt::fire_and_forget ConstructInBackground() {}


	~Engine() {}

	// IDeviceNotify
	virtual void OnDeviceLost() {
		render_needed_ = false;
		//renderer_->ReleaseDeviceDependentResources();
	}
	
	virtual void OnDeviceRestored() {
		//HandleDeviceRestored();
		render_needed_ = true;
	}

	winrt::fire_and_forget HandleDeviceRestored() {
		//renderer_->CreateDeviceDependentResources();
		//renderer_->CreateWindowSizeDependentResources();

		//update_state_= UpdateEngineState::WaitingForResources;
		render_needed_ = true;
		//co_await renderer_->CreateGameDeviceResourcesAsync();
		// The finalize code needs to run in the same thread context
		// as the m_renderer object was created because the D3D device context
		// can ONLY be accessed on a single thread. 
		// co_await of an IAsyncAction resumes in the same thread context.
		///m_renderer->FinalizeCreateGameDeviceResources();
	}
	
	void Run() {
		using namespace winrt::Windows::UI::Core;

		CoreWindow window = CoreWindow::GetForCurrentThread();
		CoreDispatcher dispatcher = window.Dispatcher();
		// Main UWP app state loop
		while (!window_closed_) {
			if (visible_) {
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
				Update();
				renderer_->Render();
				device_resources_->Present();
				render_needed_ = false;
			}
			else {
				dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
	}


	void Update() {}
	//void Suspend() {}
	//void Resume() {}
	

	void Visibility(bool visible_state){
		// Set render needs
		visible_ = visible_state;
	}
	void Close() { window_closed_ = true; }

	std::shared_ptr<DeviceResources> device_resources_;
	std::shared_ptr<Renderer> renderer_;
	
	bool window_closed_;
	bool visible_;
	bool render_needed_;

	UpdateEngineState update_state_;
};
