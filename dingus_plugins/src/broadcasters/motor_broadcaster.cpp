#include "dingus_plugins/motor_broadcaster.hpp"

#include "hardware_interface/types/hardware_interface_type_values.hpp"

#include <iostream>

namespace dingus_plugins::controllers {

cb_return MotorBroadcaster::on_init() {
  name_ = auto_declare<std::string>("wheel_name", name_);
  return cb_return::SUCCESS;
}

cb_return MotorBroadcaster::on_configure(const rclcpp_lifecycle::State& /* previous_state */) {
  name_ = auto_declare<std::string>("wheel_name", name_);
  frame_id_ = auto_declare<std::string>("frame_id", frame_id_);

  wheel_sensor_ = std::make_unique<semantic_components::WheelSensor>(name_);
  try {
    wheel_pub_ = get_node()->create_publisher<otomo_msgs::msg::Wheel>("~/wheel", rclcpp::SystemDefaultsQoS());
    rt_wheel_pub_ = std::make_unique<RtWheelPub>(wheel_pub_);

    rt_wheel_pub_->msg_.header.frame_id = frame_id_;
  }
  catch (const std::exception & e) {
    RCLCPP_ERROR(get_node()->get_logger(), "Exception thrown during publisher creation: %s", e.what());
  }

  return cb_return::SUCCESS;
}

interface_return MotorBroadcaster::command_interface_configuration() const {
  return interface_return{ controller_interface::interface_configuration_type::NONE, {} };
}

interface_return MotorBroadcaster::state_interface_configuration() const {
  controller_interface::InterfaceConfiguration state_interfaces_config;
  state_interfaces_config.type = controller_interface::interface_configuration_type::INDIVIDUAL;
  state_interfaces_config.names = wheel_sensor_->get_state_interface_names();
  return state_interfaces_config;
}

cb_return MotorBroadcaster::on_activate(const rclcpp_lifecycle::State& /*previous_state*/) {
  wheel_sensor_->assign_loaned_state_interfaces(state_interfaces_);
  return cb_return::SUCCESS;
}

cb_return MotorBroadcaster::on_deactivate(const rclcpp_lifecycle::State& /*previous_state*/) {
  wheel_sensor_->release_interfaces();
  return cb_return::SUCCESS;
}

ci_return MotorBroadcaster::update(const rclcpp::Time& time, const rclcpp::Duration& /*period*/) {
  if (rt_wheel_pub_ && rt_wheel_pub_->trylock()) {
    rt_wheel_pub_->msg_.header.stamp = time;
    wheel_sensor_->get_values_as_message(rt_wheel_pub_->msg_);
    rt_wheel_pub_->unlockAndPublish();
  }

  return ci_return::OK;
}

}  // namespace dingus_plugins::controllers

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(
  dingus_plugins::controllers::MotorBroadcaster, controller_interface::ControllerInterface)
