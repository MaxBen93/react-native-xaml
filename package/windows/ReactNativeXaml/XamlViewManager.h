#pragma once

#include "pch.h"
#include "CppWinRTIncludes.h"
#include "winrt/Microsoft.ReactNative.h"
#include "XamlMetadata.h"

// Required to avoid creating unnecessary ContentControls to hold dynamically-typed ui elements
// https://github.com/microsoft/react-native-windows/pull/7137
// #define HAS_CREATEWITHPROPERTIES

namespace winrt::ReactNativeXaml {

  struct XamlViewManager : winrt::implements<
    XamlViewManager,
    winrt::Microsoft::ReactNative::IViewManager,
    winrt::Microsoft::ReactNative::IViewManagerWithNativeProperties,
    winrt::Microsoft::ReactNative::IViewManagerWithCommands,
    winrt::Microsoft::ReactNative::IViewManagerWithExportedEventTypeConstants,
    winrt::Microsoft::ReactNative::IViewManagerWithReactContext
#ifdef HAS_CREATEWITHPROPERTIES
    ,winrt::Microsoft::ReactNative::IViewManagerCreateWithProperties
#endif
    //,winrt::Microsoft::ReactNative::IViewManagerRequiresNativeLayout
  > {
  public:
    XamlViewManager() = default;

    // IViewManager
    winrt::hstring Name() noexcept;

    xaml::FrameworkElement CreateView() noexcept;

    // IViewManagerCreateWithProperties
    winrt::IInspectable CreateViewWithProperties(winrt::Microsoft::ReactNative::IJSValueReader const& propertyMapReader) noexcept;

    // IViewManagerWithNativeProperties
    winrt::Windows::Foundation::Collections::
      IMapView<winrt::hstring, winrt::Microsoft::ReactNative::ViewManagerPropertyType>
      NativeProps() noexcept;

    void UpdateProperties(
      winrt::Windows::UI::Xaml::FrameworkElement const& view,
      winrt::Microsoft::ReactNative::IJSValueReader const& propertyMapReader) noexcept;

    // IViewManagerWithCommands
    winrt::Windows::Foundation::Collections::IVectorView<winrt::hstring> Commands() noexcept;

    void DispatchCommand(
      winrt::Windows::UI::Xaml::FrameworkElement const& view,
      winrt::hstring const& commandId,
      winrt::Microsoft::ReactNative::IJSValueReader const& commandArgsReader) noexcept;

    winrt::Microsoft::ReactNative::ConstantProviderDelegate ExportedCustomBubblingEventTypeConstants() noexcept;

    winrt::Microsoft::ReactNative::ConstantProviderDelegate ExportedCustomDirectEventTypeConstants() noexcept;

    // IViewManagerWithReactContext
    winrt::Microsoft::ReactNative::IReactContext ReactContext() noexcept;

    void ReactContext(winrt::Microsoft::ReactNative::IReactContext reactContext) noexcept;

    // IViewManagerRequiresNativeLayout
    bool RequiresNativeLayout() noexcept { return true; }
  private:
    winrt::Microsoft::ReactNative::IReactContext m_reactContext{ nullptr };
    XamlMetadata xamlMetadata;
  };

}
