#include "group.hpp"

#include <fmt/format.h>

#include <util/command.hpp>
#include "gdkmm/device.h"
#include "gtkmm/enums.h"
#include "gtkmm/widget.h"

namespace waybar {

Group::Group(const std::string& name, const std::string& id, const Json::Value& config,
             bool vertical, std::vector<AModule*>& modules)
    : AModule(config, name, id, false, false), box{vertical ? Gtk::ORIENTATION_VERTICAL : Gtk::ORIENTATION_HORIZONTAL, 5} {
  this->box.set_name(name_);
  if (!id.empty()) {
    this->box.get_style_context()->add_class(id);
  }

  spdlog::info("Creating group {}", name_);
  spdlog::info("  config: {}", config_);

  // Override event handling
  bool hasUserEvent =
      std::find_if(eventMap_.cbegin(), eventMap_.cend(), [&config](const auto& eventEntry) {
        // True if there is any non-release type event
        return eventEntry.first.second != GdkEventType::GDK_BUTTON_PRESS &&
               config[eventEntry.second].isString();
      }) != eventMap_.cend();

  if (hasUserEvent) {
    box.add_events(Gdk::BUTTON_PRESS_MASK);
    box.signal_button_press_event().connect(sigc::mem_fun(*this, &Group::handleToggle));
  }

  bool hasReleaseEvent =
      std::find_if(eventMap_.cbegin(), eventMap_.cend(), [&config](const auto& eventEntry) {
        // True if there is any non-release type event
        return eventEntry.first.second == GdkEventType::GDK_BUTTON_RELEASE &&
               config[eventEntry.second].isString();
      }) != eventMap_.cend();
  if (hasReleaseEvent) {
    box.add_events(Gdk::BUTTON_RELEASE_MASK);
    box.signal_button_release_event().connect(sigc::mem_fun(*this, &Group::handleRelease));
  }

  // Since hover and leave events are not supported by Gtk::Box, we will force them to the children component
  // This is done by adding the events to the children components and then forwarding them to the Group
  /*bool hasHoverEvent =
      std::find_if(eventMap_.cbegin(), eventMap_.cend(), [&config](const auto& eventEntry) {
        // True if there is any non-release type event
        return eventEntry.first.second == GdkEventType::GDK_ENTER_NOTIFY &&
               config[eventEntry.second].isString();
      }) != eventMap_.cend();
  if (hasHoverEvent) {
    for (auto& module : modules) {
      module->operator Gtk::Widget&().add_events(Gdk::ENTER_NOTIFY_MASK);
      module->operator Gtk::Widget&().signal_enter_notify_event().connect(sigc::mem_fun(*this, &Group::handleHover));
    }
  }

  bool hasLeaveEvent =
      std::find_if(eventMap_.cbegin(), eventMap_.cend(), [&config](const auto& eventEntry) {
        // True if there is any non-release type event
        return eventEntry.first.second == GdkEventType::GDK_LEAVE_NOTIFY &&
               config[eventEntry.second].isString();
      }) != eventMap_.cend();
  if (hasLeaveEvent) {
    for (auto& module : modules) {
      module->operator Gtk::Widget&().add_events(Gdk::LEAVE_NOTIFY_MASK);
      module->operator Gtk::Widget&().signal_leave_notify_event().connect(sigc::mem_fun(*this, &Group::handleLeave));
    }
  }*/


  // default orientation: orthogonal to parent
  auto orientation =
      config_["orientation"].empty() ? "orthogonal" : config_["orientation"].asString();
  if (orientation == "inherit") {
    // keep orientation passed
  } else if (orientation == "orthogonal") {
    box.set_orientation(vertical ? Gtk::ORIENTATION_HORIZONTAL : Gtk::ORIENTATION_VERTICAL);
  } else if (orientation == "vertical") {
    box.set_orientation(Gtk::ORIENTATION_VERTICAL);
  } else if (orientation == "horizontal") {
    box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
  } else {
    throw std::runtime_error("Invalid orientation value: " + orientation);
  }


  // Adds all children components to the Group
  for (auto& module : modules) {
    spdlog::info("  Adding module {}", module->getName());
    this->box.pack_start(*module, true, true, 0);
    module->dp.connect([&module] {
      try {
        module->update();
      } catch (const std::exception& e) {
        spdlog::error("{}: {}", module->getName(), e.what());
      }
    });
  }
  // Shows all children components
  this->box.show_all();
}

auto Group::update() -> void {
  // noop
}

bool Group::handleToggle(GdkEventButton* const& e) {
  return AModule::handleToggle(e);
}

bool Group::handleRelease(GdkEventButton* const& e) {
  return AModule::handleRelease(e);
}

bool Group::handleHover(GdkEventCrossing* const& e) {
  spdlog::info("Group::handleHover");
  return AModule::handleHover(e);
}

bool Group::handleLeave(GdkEventCrossing* const& e) {
  spdlog::info("Group::handleLeave");
  return AModule::handleLeave(e);
}

Group::operator Gtk::Widget&() { return box; }

}  // namespace waybar
