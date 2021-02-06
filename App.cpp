#include "pch.h"

// Standard library includes
#include <memory>


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
    //CompositionTarget m_target{ nullptr };
    //VisualCollection m_visuals{ nullptr };
    //Visual m_selected{ nullptr };
    //float2 m_offset{};

    //std::shared_ptr<DX::DeviceResources> device_resources_;

    IFrameworkView CreateView()
    {
        return *this;
    }

    // ----------------------------------------------------------------
    // Core Application View stuff
    // ----------------------------------------------------------------
    void Initialize(CoreApplicationView const & application_view)
    {
        application_view.Activated({ this, &App::OnActivated });
        CoreApplication::Suspending({ this, &App::OnSuspending });
        CoreApplication::Resuming({ this, &App::OnResuming });

        // Initialize device resources 
        //      device_resources_ = std::make_shared<DX::DeviceResources>();
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
        //      m_deviceResources->Trim();
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
        // TODO: Why isn't this working..?
        window.PointerCursor(CoreCursor(CoreCursorType::Arrow, 0));
        PointerVisualizationSettings visualizationSettings{ PointerVisualizationSettings::GetForCurrentView() };
        visualizationSettings.IsContactFeedbackEnabled(false);
        visualizationSettings.IsBarrelButtonFeedbackEnabled(false);
        // Set the device resources window
        //      m_deviceResources->SetWindow(window);

        // Register window related events callbacks
        window.Activated({ this, &App::OnWindowActivationChanged });
        window.SizeChanged({ this, &App::OnWindowSizeChanged });
        window.Closed({ this, &App::OnWindowClosed });
        window.VisibilityChanged({ this, &App::OnVisibilityChanged });

        // Register and set display information
        DisplayInformation currentDisplayInformation{ DisplayInformation::GetForCurrentView() };
        currentDisplayInformation.DpiChanged({ this, &App::OnDpiChanged });
        currentDisplayInformation.OrientationChanged({ this, &App::OnOrientationChanged });
        currentDisplayInformation.StereoEnabledChanged({ this, &App::OnStereoEnabledChanged });
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
        //      m_deviceResources->SetLogicalSize(args.Size());
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
        //      m_deviceResources->SetDpi(sender.LogicalDpi());
        //      m_main->CreateWindowSizeDependentResources();
    }

    void OnOrientationChanged(DisplayInformation const& sender, IInspectable const& /* args */)
    {
        // TODO: Non mobile game - no orientation support should be here..
        //      m_deviceResources->SetCurrentOrientation(sender.CurrentOrientation());
        //      m_main->CreateWindowSizeDependentResources();
    }

    void OnStereoEnabledChanged(DisplayInformation const& /* sender */, IInspectable const& /* args */)
    {
        // TODO: stereo support? maybe only relevant for VR/AR?
        //      m_deviceResources->UpdateStereoState();
        //      m_main->CreateWindowSizeDependentResources();
    }

    void OnDisplayContentsInvalidated(DisplayInformation const& /* sender */, IInspectable const& /* args */)
    {   
        // TODO: What's this for?
        //      m_deviceResources->ValidateDevice();
    }

    // -----------------------------------------------------------------


    void Load(hstring const&)
    {
        // Loading mechanism of the main game loop
        /*
        * if (!m_main)
        {
            m_main = winrt::make_self<GameMain>(m_deviceResources);
        }
        */
    }

    void Uninitialize()
    {
    }

    void Run(){
        // Calling main game loop run
        //      m_main->Run();
                // TODO: what is this dispatcher and what do we need it for?
        CoreWindow window = CoreWindow::GetForCurrentThread();
        CoreDispatcher dispatcher = window.Dispatcher();
        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    }


    private:
        //std::shared_ptr<DX::DeviceResources> m_deviceResources;
        //winrt::com_ptr<GameMain> m_main;
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
