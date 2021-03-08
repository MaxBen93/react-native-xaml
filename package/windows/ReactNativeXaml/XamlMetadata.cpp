#include "pch.h"
#include <roapi.h>

#include "XamlMetadata.h"
#include <JSValueReader.h>
#include <filesystem>
#include <unordered_map>
#include <Crc32Str.h>
#include <xutility>

#include "Codegen/TypeProperties.g.h"
#include "Codegen/TypeEvents.g.h"

#include <JSValueWriter.h>
#include "Serialize.h"
#include "PropInfo.h"

#include "CustomTypeProperties.g.h"

#if __has_include("CustomTypeProperties.g.h")
#include "CustomTypeProperties.g.h"
#else
#error Please run react-native-xaml codegen for your project before building it.
#endif

#if __has_include("CustomTypeEvents.g.h")
#include "CustomTypeEvents.g.h"
#else
#error Please run react-native-xaml codegen for your project before building it.
#endif

#if __has_include("CustomTypeCreator.g.cpp")
#else
winrt::Windows::Foundation::IInspectable XamlMetadata::CreateCustom(const std::string& typeName) const { return nullptr; }
#endif

using namespace winrt::Microsoft::ReactNative;

#define MAKE_GET_DP(type, prop) IsType<type>, []() { return type::prop(); }

XamlMetadata::XamlMetadata() {}

winrt::Windows::Foundation::IInspectable XamlMetadata::ActivateInstance(const winrt::hstring& hstr) {
  winrt::Windows::Foundation::IUnknown unknown{ nullptr };
  auto nhstr = (HSTRING)(winrt::get_abi(hstr));
  auto res = ::RoActivateInstance(nhstr, (::IInspectable**)winrt::put_abi(unknown));
  if (res != S_OK) {

  }
  auto e = unknown.as<winrt::Windows::Foundation::IInspectable>();
  return e;
}

/*
struct RoutedEventInfo {
  const char* name;

  using routedEventGetter_t = xaml::RoutedEvent(*)();
  routedEventGetter_t routedEventGetter;
};

// https://docs.microsoft.com/en-us/uwp/api/windows.ui.xaml.routedevent?view=winrt-19041#events-that-use-a-routedevent-identifier
#define ROUTED_EVENT(name)  { #name, xaml::UIElement::name##Event }
constexpr RoutedEventInfo routedEvents[] = {
  ROUTED_EVENT(DoubleTapped),
  ROUTED_EVENT(DragEnter),
  ROUTED_EVENT(DragLeave),
  ROUTED_EVENT(DragOver),
  ROUTED_EVENT(Drop),
  ROUTED_EVENT(Holding),
  ROUTED_EVENT(KeyDown),
  ROUTED_EVENT(KeyUp),
  ROUTED_EVENT(ManipulationCompleted),
  ROUTED_EVENT(ManipulationDelta),
  ROUTED_EVENT(ManipulationInertiaStarting),
  ROUTED_EVENT(ManipulationStarted),
  ROUTED_EVENT(ManipulationStarting),
  ROUTED_EVENT(PointerCanceled),
  ROUTED_EVENT(PointerCaptureLost),
  ROUTED_EVENT(PointerEntered),
  ROUTED_EVENT(PointerExited),
  ROUTED_EVENT(PointerMoved),
  ROUTED_EVENT(PointerPressed),
  ROUTED_EVENT(PointerReleased),
  ROUTED_EVENT(PointerWheelChanged),
  ROUTED_EVENT(RightTapped),
  ROUTED_EVENT(Tapped),
};
*/

FrameworkElement Wrap(const winrt::Windows::Foundation::IInspectable& d) {
  if (auto fe = d.try_as<FrameworkElement>()) {
    return fe;
  }
  else {
    ContentControl cc;
    cc.Content(d);
    return cc;
  }
}

winrt::Windows::Foundation::IInspectable XamlMetadata::Create(const std::string& typeName, const winrt::Microsoft::ReactNative::IReactContext& context) const {
  auto key = COMPILE_TIME_CRC32_STR(typeName.c_str());
  auto obj = Create(typeName);
  if (!obj) {
    obj = CreateCustom(typeName);
  }
  assert(obj && "xaml type not found");
  auto e = obj.try_as<FrameworkElement>();
  bool wrapped = e == nullptr;
  if (!e) {
    e = Wrap(obj);
  }
  std::for_each(EventInfo::xamlEventMap, EventInfo::xamlEventMap + ARRAYSIZE(EventInfo::xamlEventMap), [e, context, wrapped](const EventInfo& entry) {entry.attachHandler(e, context, wrapped); });
  return e;
}


// FlyoutBase.IsOpen is read-only but we need a way to call ShowAt/Hide, so this hooks it up
void SetIsOpen_FlyoutBase(xaml::DependencyObject o, xaml::DependencyProperty prop, const winrt::Microsoft::ReactNative::JSValue& v) {
  auto flyout = Unwrap<Controls::Primitives::FlyoutBase>(o);
  if (flyout && v.Type() == JSValueType::Boolean) {
    if (v.AsBoolean()) {
      auto target = flyout.Target();

      // TODO: Need to figure out the parent element, but the content control isn't parented to anything anymore
      if (!target) target = o.as<ContentControl>().DataContext().as<FrameworkElement>();
      auto cn = winrt::get_class_name(target);
      flyout.ShowAt(target);
    }
    else {
      flyout.Hide();
    }
  }
}


const PropInfo* XamlMetadata::FindFirstMatch(const stringKey& key, const winrt::Windows::Foundation::IInspectable& obj, const PropInfo* map, size_t size) {
  auto it = std::find_if(map, map + size, [key](const PropInfo& entry) { return entry.propName == key; });
  while ((it != map + size) && it->propName == key) {
    if (it->isType(obj)) {
      return it;
    }
    it++;
  }
  return nullptr;
}

template<typename T>
bool IsWrapped(const winrt::Windows::Foundation::IInspectable& i) { return Unwrap<T>(i) != nullptr; }

const PropInfo fakeProps[] = {
    { MAKE_KEY("isOpen"), IsWrapped<Controls::Primitives::FlyoutBase>, nullptr, SetIsOpen_FlyoutBase, ViewManagerPropertyType::Boolean },
};

const struct {
  const PropInfo* map;
  size_t size;
} propertyMaps[] = {
  { xamlPropertyMap, ARRAYSIZE(xamlPropertyMap) },
  { fakeProps, ARRAYSIZE(fakeProps) },
#ifdef RNX_HAS_CUSTOM_PROPS
  { xamlCustomPropertyMap, ARRAYSIZE(xamlCustomPropertyMap) },
#endif
};

const PropInfo* XamlMetadata::GetProp(const std::string& propertyName, const winrt::Windows::Foundation::IInspectable& obj) const {
  auto key = MAKE_KEY(propertyName.c_str());
  for (const auto& mapInfo : propertyMaps) {
    auto it = FindFirstMatch(key, obj, mapInfo.map, mapInfo.size);
    if (it) return it;
  }
  return nullptr;
}

void JsEvent(winrt::Microsoft::ReactNative::IJSValueWriter const& constantWriter, std::wstring topName, std::wstring onName) {
  constantWriter.WritePropertyName(topName);
  constantWriter.WriteObjectBegin();
  WriteProperty(constantWriter, L"registrationName", onName);
  constantWriter.WriteObjectEnd();
}

ConstantProviderDelegate GetEvents =
[](winrt::Microsoft::ReactNative::IJSValueWriter const& constantWriter) {
  for (auto const& e : EventInfo::xamlEventMap) {
    auto wideName = std::wstring(winrt::to_hstring(e.name));
    JsEvent(constantWriter, L"top" + wideName, L"on" + wideName);
  }
#if RNX_HAS_CUSTOM_EVENTS
  for (auto const& e : EventInfo::xamlCustomEventMap) {
    auto wideName = std::wstring(winrt::to_hstring(e.name));
    JsEvent(constantWriter, L"top" + wideName, L"on" + wideName);
  }
#endif
};

