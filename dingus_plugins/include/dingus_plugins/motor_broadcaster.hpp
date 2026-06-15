#ifndef DINGUS_PLUGINS__MOTOR_BROADCASTER_HPP_
#define DINGUS_PLUGINS__MOTOR_BROADCASTER_HPP_

#include <memory>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include "controller_interface/controller_interface.hpp"

#include "realtime_tools/realtime_publisher.hpp"
#include "dingus_plugins/wheel_sensor.hpp"
#include "otomo_msgs/msg/wheel.hpp"

namespace dingus_plugins::controllers {

using ci_return = controller_interface::return_type;
using interface_return = controller_interface::InterfaceConfiguration;
using cb_return = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class MotorBroadcaster : public controller_interface::ControllerInterface {
public:
  MotorBroadcaster() = default;
  RCLCPP_SHARED_PTR_DEFINITIONS(MotorBroadcaster)

  interface_return command_interface_configuration() const override;
  interface_return state_interface_configuration() const override;
  ci_return update(const rclcpp::Time& time, const rclcpp::Duration& period) override;

  cb_return on_init() override;
  cb_return on_configure(const rclcpp_lifecycle::State& previous_state) override;
  cb_return on_activate(const rclcpp_lifecycle::State& previous_state) override;
  cb_return on_deactivate(const rclcpp_lifecycle::State& previous_state) override;

protected:
  using RtWheelPub = realtime_tools::RealtimePublisher<otomo_msgs::msg::Wheel>;

  std::string name_;
  std::string frame_id_;

  std::unique_ptr<semantic_components::WheelSensor> wheel_sensor_;

  rclcpp::Publisher<otomo_msgs::msg::Wheel>::SharedPtr wheel_pub_;
  std::unique_ptr<RtWheelPub> rt_wheel_pub_;
};

} // namespace dingus_plugins::controllers

#endif // DINGUS_PLUGINS__MOTOR_BROADCASTER_HPP_