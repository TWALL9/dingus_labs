#ifndef DINGUS_PLUGINS__DINGUS_CONTROLLER_HPP_
#define DINGUS_PLUGINS__DINGUS_CONTROLLER_HPP_

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"

#include "controller_interface/controller_interface.hpp"

#include "otomo_msgs/msg/pid.hpp"

#include "dingus_plugins/pid_params.hpp"

namespace dingus_plugins::controllers {

using ci_return = controller_interface::return_type;
using interface_return = controller_interface::InterfaceConfiguration;
using cb_return = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class DingusController : public controller_interface::ControllerInterface {
public:
  DingusController();
  RCLCPP_SHARED_PTR_DEFINITIONS(DingusController)

  interface_return command_interface_configuration() const override;
  interface_return state_interface_configuration() const override;
  ci_return update(const rclcpp::Time &time, const rclcpp::Duration &period) override;

  cb_return on_init() override;
  cb_return on_configure(const rclcpp_lifecycle::State &previous_state) override;
  cb_return on_activate(const rclcpp_lifecycle::State &previous_state) override;
  cb_return on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

protected:
  void pid_update_cb(const otomo_msgs::msg::Pid::SharedPtr msg);

  std::map<std::string, PidParams> pid_controllers_;

  struct PidHandle {
    std::reference_wrapper<hardware_interface::LoanedCommandInterface> p;
    std::reference_wrapper<hardware_interface::LoanedCommandInterface> i;
    std::reference_wrapper<hardware_interface::LoanedCommandInterface> d;
  };
  std::map<std::string, PidHandle> registered_pid_handles_;

  bool is_halted_{false};

  rclcpp::Subscription<otomo_msgs::msg::Pid>::SharedPtr pid_subs_;
};

} // namespace dingus_plugins::controllers

#endif // DINGUS_PLUGINS__DINGUS_CONTROLLER_HPP_
