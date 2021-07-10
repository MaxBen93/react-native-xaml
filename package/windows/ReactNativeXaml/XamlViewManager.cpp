#include "pch.h"
#include "XamlViewManager.h"

#include "JSValueReader.h"
#include "NativeModules.h"
#include "XamlMetadata.h"
#include <winrt/Windows.UI.Xaml.Core.Direct.h>
#include <UI.Xaml.Input.h>
#include <UI.Xaml.Documents.h>

using namespace winrt;
using namespace Microsoft::ReactNative;
using namespace xaml;
using namespace xaml::Controls;

namespace winrt::ReactNativeXaml {

  // IViewManager
  hstring XamlViewManager::Name() noexcept {
    return L"XamlControl";
  }

  xaml::FrameworkElement XamlViewManager::CreateView() noexcept {
    assert(false);
    return nullptr;
  }

  winrt::IInspectable XamlViewManager::CreateViewWithProperties(winrt::Microsoft::ReactNative::IJSValueReader const& propertyMapReader) noexcept {
    m_xamlMetadata->SetupEventDispatcher(m_reactContext);

    const JSValueObject& propertyMap = JSValue::ReadObjectFrom(propertyMapReader);
    auto typeName = propertyMap["type"].AsString();
    auto e = m_xamlMetadata->Create(typeName, m_reactContext);
    return e;
  }


  // IViewManagerWithNativeProperties
  IMapView<hstring, ViewManagerPropertyType> XamlViewManager::NativeProps() noexcept {
    auto nativeProps = winrt::single_threaded_map<hstring, ViewManagerPropertyType>();
    nativeProps.Insert(L"type", ViewManagerPropertyType::String);

    m_xamlMetadata->PopulateNativeProps(nativeProps);
    m_xamlMetadata->PopulateNativeEvents(nativeProps);
    return nativeProps.GetView();
  }

  void XamlViewManager::UpdateProperties(
    FrameworkElement const& view,
    IJSValueReader const& propertyMapReader) noexcept {

    const JSValueObject& propertyMap = JSValue::ReadObjectFrom(propertyMapReader);

    auto e = view;
    auto cn = get_class_name(e);
    if (auto control = e.try_as<DependencyObject>()) {
      for (auto const& pair : propertyMap) {
        bool handled = false;
        auto const& propertyName = pair.first;
        auto const& propertyValue = pair.second;

        if (auto prop = m_xamlMetadata->GetProp(propertyName, control)) {
          auto realObject = prop->asType(control).as<DependencyObject>();
          auto rcn = get_class_name(realObject);
          prop->SetValue(realObject, propertyValue);
          handled = true;
        }
        else if (auto eventAttacher = m_xamlMetadata->AttachEvent(m_reactContext, propertyName, control, propertyValue.AsBoolean())) {
        }
        else if (propertyName == "type") {}
        else {
          auto className = winrt::get_class_name(e);
          if (IsDebuggerPresent()) {
            assert(false && "unknown property");
          } 
        }
      }
    }
  }

  // IViewManagerWithExportedEventTypeConstants
  ConstantProviderDelegate XamlViewManager::ExportedCustomBubblingEventTypeConstants() noexcept {
    return nullptr;
  }

  ConstantProviderDelegate XamlViewManager::ExportedCustomDirectEventTypeConstants() noexcept {
    return GetEvents;
  }

  auto GetPriority(const UIElement& u) {
    return winrt::unbox_value<uint32_t>(u.GetValue(GetPriorityProperty()));
  }

  // IViewManagerWithCommands
  IVectorView<hstring> XamlViewManager::Commands() noexcept {
    auto commands = winrt::single_threaded_vector<hstring>();
    commands.Append(L"CustomCommand");
    return commands.GetView();
  }

  void XamlViewManager::DispatchCommand(
    FrameworkElement const& view,
    winrt::hstring const& commandId,
    winrt::Microsoft::ReactNative::IJSValueReader const& /*commandArgsReader*/) noexcept {
    if (auto control = view.try_as<winrt::Windows::UI::Xaml::Controls::HyperlinkButton>()) {
      if (commandId == L"CustomCommand") {
        // const JSValueArray& commandArgs = JSValue::ReadArrayFrom(commandArgsReader);
        // Execute command
      }
    }
  }

  // IViewManagerWithReactContext
  IReactContext XamlViewManager::ReactContext() noexcept {
    return m_reactContext;
  }

  void XamlViewManager::ReactContext(IReactContext reactContext) noexcept {
    m_reactContext = reactContext;
    m_xamlMetadata = std::make_shared<XamlMetadata>();
  }

  void XamlViewManager::AddView(xaml::FrameworkElement parent, xaml::UIElement child, int64_t _index) {
    ConnectViews(parent, child, _index);
  }

  void XamlViewManager::ConnectViews(xaml::FrameworkElement parent, xaml::UIElement child, int64_t _index) {
    auto index = static_cast<uint32_t>(_index);

    auto e = parent;
    auto parentType = winrt::get_class_name(e);
    auto childType = winrt::get_class_name(child);

    OutputDebugStringW(L"Connecting - parent = ");
    OutputDebugStringW(parentType.c_str());
    OutputDebugStringW(L" child = ");
    OutputDebugStringW(childType.c_str());
    OutputDebugStringW(L"\n");

    if (auto childFE_ = child.try_as<FrameworkElement>()) {
      if (childFE_.Name() == L"DynamicallyAddedGrid") {
        auto x = 0;
      }
    }
    if (child.try_as<xaml::Controls::Primitives::SelectorItem>() ||
      child.try_as<NavigationView>()) { 
      // these are ContentControls too, but we shouldn't try to unwrap, so skip this 
    }
    else if (auto childAsCC = child.try_as<Wrapper>()) {
      if (auto childContent = childAsCC.Content()) {
        childType = winrt::get_class_name(childContent);
        auto tag = childAsCC.Tag();
        if (auto depObj = childContent.try_as<DependencyObject>()) {
          // tranfer the Tag from the wrapping ContentControl
          // This is used for dispatching events and TouchEventHandler
          depObj.SetValue(FrameworkElement::TagProperty(), tag);
        }

        if (auto childFlyout = childContent.try_as<Controls::Primitives::FlyoutBase>()) {
          Primitives::FlyoutBase::SetAttachedFlyout(e, childFlyout);
          childAsCC.DataContext(e);
          if (auto button = e.try_as<Button>()) {
            return button.Flyout(childFlyout);
          }
          else {
            if (auto uiParent = e.try_as<UIElement>()) {
              return uiParent.ContextFlyout(childFlyout);
            }
          }
        }
        else if (auto RTBparent = parent.try_as<RichTextBlock>()) {
          if (auto childBlock = childContent.try_as<Documents::Block>()) {
            return RTBparent.Blocks().InsertAt(index, childBlock);
          }
        }
        else if (auto TBparent = parent.try_as<TextBlock>()) {
          if (auto childInline = childContent.try_as<Documents::Inline>()) {
            return TBparent.Inlines().InsertAt(index, childInline);
          }
        }
        else if (auto paragraphParent = Unwrap<Documents::Paragraph>(parent)) {
          if (auto childInline = childContent.try_as<Documents::Inline>()) {
            return paragraphParent.Inlines().InsertAt(index, childInline);
          }
        }
        else if (auto spanParent = Unwrap<Documents::Span>(parent)) {
          if (auto childInline = childContent.try_as<Documents::Inline>()) {
            return spanParent.Inlines().InsertAt(index, childInline);
          }
        }
        else if (auto childKeyboardAccelerator = childContent.try_as<xaml::Input::KeyboardAccelerator>()) {
          if (auto uiParent = parent.try_as<UIElement>()) {
            return uiParent.KeyboardAccelerators().Append(childKeyboardAccelerator);
          }
        }
      }
    }

    if (auto panel = e.try_as<Panel>()) {
      return panel.Children().InsertAt(index, child);
    }

    if (auto navView = e.try_as<NavigationView>()) {
      auto childCN = winrt::get_class_name(child);
      if (auto childMI = child.try_as<NavigationViewItemBase>()) {
        return navView.MenuItems().InsertAt(index, child);
      }
    }

    if (auto navViewItem = e.try_as<NavigationViewItem>()) {
      if (auto childIconElement = child.try_as<IconElement>()) {
        return navViewItem.Icon(childIconElement);
      }
    }

    if (auto appBarButton = e.try_as<AppBarButton>()) {
      if (auto icon = child.try_as<IconElement>()) {
        return appBarButton.Icon(icon);
      }
    }
    if (auto commandBar = e.try_as<CommandBar>()) {
      if (auto commandBarElement = child.try_as<ICommandBarElement>()) {
        const auto priority = GetPriority(child);
        if (priority == 0) {
          return commandBar.PrimaryCommands().Append(commandBarElement);
        } else if (priority == 1) {
          return commandBar.SecondaryCommands().Append(commandBarElement);
        }
      }
    }

    if (auto contentCtrl = e.try_as<Wrapper>()) {
      auto parentContent = contentCtrl.Content();
      if (auto menuFlyout = parentContent.try_as<MenuFlyout>()) {
        if (auto mfi = child.try_as<MenuFlyoutItemBase>()) {
          return menuFlyout.Items().InsertAt(index, mfi);
        }
      }
      if (auto flyout = parentContent.try_as<Flyout>()) {
        if (index == 0) {
          return flyout.Content(child);
        }
      }
      if (index == 0 || contentCtrl.Content() == nullptr) {
        return contentCtrl.Content(child);
      }
      else {

      }
    }
    if (auto border = e.try_as<Border>()) {
      if (index == 0) {
        return border.Child(child);
      }
    }
    if (auto autoSuggestBox = e.try_as<AutoSuggestBox>()) {
      if (auto iconElement = child.try_as<IconElement>()) {
        return autoSuggestBox.QueryIcon(iconElement);
      }
    }
    if (auto itemsControl = e.try_as<ItemsControl>()) {
      return itemsControl.Items().InsertAt(index, child);
    }
    //else 
    {
      auto cn = winrt::get_class_name(e);
      assert(false && "this element cannot have children");
    }
  }

  void XamlViewManager::RemoveAllChildren(xaml::FrameworkElement parent) {
    auto e = parent;
    if (auto panel = e.try_as<Panel>()) {
      return panel.Children().Clear();
    }
    else if (auto contentCtrl = e.try_as<Wrapper>()) {
      return contentCtrl.Content(nullptr);
    }
    else if (auto border = e.try_as<Border>()) {
      return border.Child(nullptr);
    }
    else if (auto itemsControl = e.try_as<ItemsControl>()) {
      return itemsControl.Items().Clear();
    }
  }

  void XamlViewManager::RemoveChildAt(xaml::FrameworkElement parent, int64_t index) {
    auto e = parent;
    if (auto panel = e.try_as<Panel>()) {
      return panel.Children().RemoveAt(static_cast<uint32_t>(index));
    }
    else if (auto itemsControl = e.try_as<ItemsControl>()) {
      return itemsControl.Items().RemoveAt(static_cast<uint32_t>(index));
    }
    else if (index == 0) {
      if (auto contentCtrl = e.try_as<Wrapper>()) {
        return contentCtrl.Content(nullptr);
      }
      else if (auto border = e.try_as<Border>()) {
        return border.Child(nullptr);
      }
    }
  }


  void XamlViewManager::ReplaceChild(xaml::FrameworkElement parent, xaml::UIElement oldChild, xaml::UIElement newChild) {
    ReplaceViews(parent, oldChild, newChild);
  }

  void XamlViewManager::ReplaceViews(xaml::FrameworkElement parent, xaml::UIElement oldChild, xaml::UIElement newChild) {
    if (auto panel = parent.try_as<Panel>()) {
      Replace(panel.Children(), oldChild, newChild);
      return;
    }
    //else if (auto itemsControl = parent.try_as<ItemsControl>()) {
    //  Replace(itemsControl.Items(), oldChild, newChild);
    //  return;
    //}
    else if (auto border = parent.try_as<Border>()) {
      border.Child(newChild);
    }
    assert(false && "nyi");
  }
}
