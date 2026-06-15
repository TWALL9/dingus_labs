#ifndef DINGUS_PLUGINS__WHEEL_SENSOR_HPP_
#define DINGUS_PLUGINS__WHEEL_SENSOR_HPP_

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include <semantic_components/semantic_component_interface.hpp>
#include "otomo_msgs/msg/wheel.hpp"

namespace semantic_components
{

class WheelSensor : public SemanticComponentInterface<otomo_msgs::msg::Wheel> {
public:
  explicit WheelSensor(const std::string& name) : SemanticComponentInterface(
    name, {{
      name + "/" + "velocity",
      name + "/" + "position",
      name + "/" + "counts",
      name + "/" + "current",
    }})
  {
  }

  std::array<double, 4> get_values() const {
    update_data_from_interfaces();
    return data_;
  }

  bool get_values_as_message(otomo_msgs::msg::Wheel& message) const {
    update_data_from_interfaces();
    message.velocity = data_[0];
    message.position = data_[1];
    message.counts = data_[2];
    message.current = data_[3];

    return true;
  }

private:
  void update_data_from_interfaces() const {
    for (size_t i = 0; i < data_.size(); i++) {
      const auto data = state_interfaces_[i].get().get_optional();
      if (data.has_value())
      {
        data_[i] = data.value();
      }
    }
  }

  mutable std::array<double, 4> data_ {{ 0.0, 0.0, 0.0, 0.0 }};
};

}
#endif  // DINGUS_PLUGINS__WHEEL_SENSOR_HPP_
