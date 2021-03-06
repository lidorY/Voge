

#include "pch.h"

// Standard library includes
#include <memory>

#include "device_resources.h"
#include "engine.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Input;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{

    IFrameworkView CreateView() { return *this; }

    // ----------------------------------------------------------------
    // Core Application View stuff
    // ----------------------------------------------------------------
    void Initialize(CoreApplicationView const & application_view)
    {
        application_view.Activated({ this, &App::OnActivated });
        CoreApplication::Suspending({ this, &App::OnSuspending });
        CoreApplication::Resuming({ this, &App::OnResuming });

        // Initialize device resources 
        device_resources_ = std::make_shared<DeviceResources>();
    }

    void OnActivated(CoreApplicationView const& /* applicationView */, IActivatedEventArgs const& /* args */)
    {
        CoreWindow window = CoreWindow::GetForCurrentThread();
        window.Activate();
    }

    winrt::fire_and_forget OnSuspending(IInspectable const& /* sender */, SuspendingEventArgs const& args) {
        auto lifetime = get_strong();
        // Save app state asynchronously after requesting a deferral. Holding a deferral
        // indicates that the application is busy performing suspending operations. Be
        // aware that a deferral may not be held indefinitely. After about five seconds,
        // the app will be forced to exit.
        SuspendingDeferral deferral = args.SuspendingOperation().GetDeferral();
        co_await winrt::resume_background();

        // Suspending action of the device resources
        device_resources_->Trim();
        // Suspending action of the main game loop
        //      m_main->Suspend();

        deferral.Complete();
    }

    void OnResuming(IInspectable const& /* sender */, IInspectable const& /* args */) 
    {
        // Restore any data or state that was unloaded on suspend. By default, data
        // and state are persisted when resuming from suspend. Note that this event
        // does not occur if the app was previously terminated.
        // Resuming the action of the main game loop
        //      m_main->Resume();
    }
    // -----------------------------------------------------------------
    
    // -----------------------------------------------------------------
    // Core Window stuff
    // -----------------------------------------------------------------
    void SetWindow(CoreWindow const& window)
    {
        // Define the cursor shape
        window.PointerCursor(CoreCursor(CoreCursorType::Arrow, 0));
        // TODO: Those stuff doesn't seem to be really necessary..
        //PointerVisualizationSettings visualizationSettings{ PointerVisualizationSettings::GetForCurrentView() };
        //visualizationSettings.IsContactFeedbackEnabled(false);
        //visualizationSettings.IsBarrelButtonFeedbackEnabled(false);
        // Set the device resources window
        device_resources_->SetWindow(window);

        // Register window related events callbacks
        window.Activated({ this, &App::OnWindowActivationChanged });
        window.SizeChanged({ this, &App::OnWindowSizeChanged });
        window.Closed({ this, &App::OnWindowClosed });
        window.VisibilityChanged({ this, &App::OnVisibilityChanged });

        // Register and set display information
        DisplayInformation currentDisplayInformation{ DisplayInformation::GetForCurrentView() };
        currentDisplayInformation.DpiChanged({ this, &App::OnDpiChanged });
        DisplayInformation::DisplayContentsInvalidated({ this, &App::OnDisplayContentsInvalidated });
    }

    void OnWindowActivationChanged(CoreWindow const& /* sender */, WindowActivatedEventArgs const& args)
    {
        // Set the main game loop activation state
        //m_main->WindowActivationChanged(args.WindowActivationState());
    }

    void OnWindowSizeChanged(CoreWindow const& /* window */, WindowSizeChangedEventArgs const& args)
    {
        // Update Change size in main loop and device resources
        device_resources_->SetLogicalSize(args.Size());
        //      m_main->CreateWindowSizeDependentResources();
    }

    void OnWindowClosed(CoreWindow const& /* sender */, CoreWindowEventArgs const& /* args */)
    {
        // Send shutdown signal to main loop
        //      m_main->Close();
    }

    void OnVisibilityChanged(CoreWindow const& /* sender */, VisibilityChangedEventArgs const& args)
    {
        // Set visibility properties in main game loop
        //      m_main->Visibility(args.Visible());
    }

    void OnDpiChanged(DisplayInformation const& sender, IInspectable const& /* args */)
    {
        // Add support in device resources and main game loop for dpi change event
        device_resources_->SetDpi(sender.LogicalDpi());
        //      m_main->CreateWindowSizeDependentResources();
    }

    void OnDisplayContentsInvalidated(DisplayInformation const& /* sender */, IInspectable const& /* args */)
    {   
        // Occurs when the display requires redrawing
        device_resources_->ValidateDevice();
    }

    // -----------------------------------------------------------------


    void Load(hstring const&)
    {
        if (!engine_)
        {
            //engine_ = winrt::make_self<Engine>(device_resources_);
            engine_ = std::make_unique<Engine>(device_resources_);
        }        
    }

    void Uninitialize()
    {
    }

    void Run(){ engine_->Run();}


    private:

        std::shared_ptr<DeviceResources> device_resources_;
        //winrt::com_ptr<Engine> engine_;
        std::unique_ptr<Engine> engine_;
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}




/*
* This code, and a great portion of the DirectX related code is inspired by,
* and contains code snippets from Microsoft's DirectX tutorials and samples.
* The main related repository may be found here:
* https://github.com/microsoft/Windows-universal-samples/blob/master/Samples
*
* Copyright and Legal notice:
* ----------------------------------------------------------------------------------
* Copyright (c) Microsoft Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
* ----------------------------------------------------------------------------------
*/