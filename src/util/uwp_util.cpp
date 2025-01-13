#include "uwp_util.h"

#include <cmath>
#include <Windows.h>

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

static CoreWindow^ _uwpCoreWind = nullptr;
static Windows::Graphics::Display::DisplayInformation^ _uwpDisplayInfo = nullptr;
static Windows::Graphics::Display::Core::HdmiDisplayInformation^ _uwpHdi = nullptr;
static Windows::Graphics::Display::Core::HdmiDisplayMode^ _uwpHdm = nullptr;

inline float ConvertDipsToPixels(float dips, float dpi)
{
   static const float dipsPerInch = 96.0f;
   return floorf(dips * dpi / dipsPerInch + 0.5f); // Arrotonda all'intero più vicino.
}

CoreWindow^ uwp_get_corewindow()
{
   if (!_uwpCoreWind) {
      _uwpCoreWind = CoreWindow::GetForCurrentThread();
   }

   return _uwpCoreWind;
}

void* uwp_get_window_reference()
{
   return reinterpret_cast<void*>(uwp_get_corewindow());
}

Windows::Graphics::Display::Core::HdmiDisplayInformation^ uwp_get_hdi()
{
   if (!_uwpHdi) {
      _uwpHdi = Windows::Graphics::Display::Core::HdmiDisplayInformation::GetForCurrentView();
   }

   return _uwpHdi;
}

Windows::Graphics::Display::Core::HdmiDisplayMode^ uwp_get_hdm()
{
   if (!_uwpHdm) {
      _uwpHdm = Windows::Graphics::Display::Core::HdmiDisplayInformation::GetForCurrentView()->GetCurrentDisplayMode();
   }

   return _uwpHdm;
}

Windows::Graphics::Display::DisplayInformation^ uwp_get_displayInfo()
{
   if (!_uwpDisplayInfo) {
      _uwpDisplayInfo = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
   }

   return _uwpDisplayInfo;
}

bool is_running_on_xbox(void)
{
   Platform::String^ device_family = Windows::System::Profile::AnalyticsInfo::VersionInfo->DeviceFamily;
   return (device_family == L"Windows.Xbox");
}

int current_height = -1;

int uwp_get_height(void)
{
   if (current_height != -1)
      return current_height;

   /* This function must be performed within UI thread,
    * otherwise it will cause a crash in specific cases
    * https://github.com/libretro/RetroArch/issues/13491 */
   float surface_scale    = 0;
   int ret                = -1;
   volatile bool finished = false;
   Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(
         CoreDispatcherPriority::Normal,
         ref new Windows::UI::Core::DispatchedHandler([&surface_scale, &ret, &finished]()
            {
            if (is_running_on_xbox())
            {
            const Windows::Graphics::Display::Core::HdmiDisplayInformation^ hdi = uwp_get_hdi();
            if (hdi)
            ret = uwp_get_hdm()->ResolutionHeightInRawPixels;
            }

            if (ret == -1)
            {
            const LONG32 resolution_scale = static_cast<LONG32>(uwp_get_displayInfo()->ResolutionScale);
            surface_scale                 = static_cast<float>(resolution_scale) / 100.0f;
            ret                           = static_cast<LONG32>(
                  uwp_get_corewindow()->Bounds.Height 
                  * surface_scale);
            }
            finished = true;
            }));
   Windows::UI::Core::CoreWindow^ corewindow = uwp_get_corewindow();
   while (!finished)
   {
      if (corewindow)
         corewindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
   }
   current_height = ret;
   return ret;
}

int current_width = -1;

int uwp_get_width(void)
{
   if (current_width != -1)
      return current_width;

   /* This function must be performed within UI thread,
    * otherwise it will cause a crash in specific cases
    * https://github.com/libretro/RetroArch/issues/13491 */
   float surface_scale    = 0;
   int returnValue        = -1;
   volatile bool finished = false;
   Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->RunAsync(
         CoreDispatcherPriority::Normal,
         ref new Windows::UI::Core::DispatchedHandler([&surface_scale, &returnValue, &finished]()
            {
            if (is_running_on_xbox())
            {
            const Windows::Graphics::Display::Core::HdmiDisplayInformation^ hdi = uwp_get_hdi();
            if (hdi)
            returnValue = uwp_get_hdm()->ResolutionWidthInRawPixels;
            }

            if(returnValue == -1)
            {
            const LONG32 resolution_scale = static_cast<LONG32>(uwp_get_displayInfo()->ResolutionScale);
            surface_scale = static_cast<float>(resolution_scale) / 100.0f;
            returnValue   = static_cast<LONG32>(
                  uwp_get_corewindow()->Bounds.Width 
                  * surface_scale);
            }
            finished = true;
            }));
   Windows::UI::Core::CoreWindow^ corewindow = uwp_get_corewindow();
   while (!finished)
   {
      if (corewindow)
         corewindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
   }

   current_width = returnValue;
   return returnValue;
}

