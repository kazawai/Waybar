#include "group.hpp"

#include <fmt/format.h>

#include <util/command.hpp>

namespace waybar {

Group::Group(const std::string& name, const std::string& id, const Json::Value& config,
             bool vertical)
    : AModule(config, name, id, false, false),
      box{vertical ? Gtk::ORIENTATION_VERTICAL : Gtk::ORIENTATION_HORIZONTAL, 0} {
  box.set_name(name_);
  if (!id.empty()) {
    box.get_style_context()->add_class(id);
  }
  spdlog::info("Creating group {}", name_);
  spdlog::info("  config: {}", config_);
  // Override event handling
  bool hasUserEvent =
      std::find_if(eventMap_.cbegin(), eventMap_.cend(), [&config](const auto& eventEntry) {
        // True if there is any non-release type event
        return eventEntry.first.second != GdkEventType::GDK_BUTTON_RELEASE &&
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
}

auto Group::update() -> void {
  // noop
}

bool Group::handleToggle(GdkEventButton* const& e) {
  return AModule::handleToggle(e);
}

Group::operator Gtk::Widget&() { return box; }

}  // namespace waybar
