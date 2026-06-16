#include "dingus_plugins/dingus_diffdrive.hpp"
#include "otomo_msgs/otomo.pb.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace dingus_plugins::controllers {

bool encode_message(async_serial::KissOutputStream &out_kiss, otomo::TopMsg &msg) {
  std::string out_string;
  if (!msg.SerializeToString(&out_string)) {
    return false;
  }

  const char *out_c = out_string.c_str();
  size_t len = out_string.size();

  for (uint8_t i = 0; i < len; i++) {
    out_kiss.add_byte(out_c[i]);
  }

  return true;
}

DingusDiffdrive::cb_return DingusDiffdrive::on_init(
  const hardware_interface::HardwareComponentInterfaceParams &params) {
  if (hardware_interface::SystemInterface::on_init(params) != DingusDiffdrive::cb_return::SUCCESS) {
    return DingusDiffdrive::cb_return::ERROR;
  }

  config_.left_wheel_name_ = info_.hardware_parameters["left_wheel_name"];
  config_.right_wheel_name_ = info_.hardware_parameters["right_wheel_name"];

  config_.encoder_count_ = std::stoi(info_.hardware_parameters["encoder_count"]);
  l_wheel_ = Wheel(config_.left_wheel_name_, config_.encoder_count_);
  r_wheel_ = Wheel(config_.right_wheel_name_, config_.encoder_count_);

  config_.serial_name_ = info_.hardware_parameters["serial_port"];
  config_.baud_rate_ = std::stoi(info_.hardware_parameters["serial_baud_rate"]);

  config_.timeout_ms_ = std::stoi(info_.hardware_parameters["timeout"]);
  config_.loop_rate_ = std::stof(info_.hardware_parameters["loop_rate"]);

  // init serial port
  serial_port_ =
    std::make_shared<async_serial::SerialPort>(config_.serial_name_, config_.baud_rate_);

  imu_.name = info_.hardware_parameters["imu_name"];

  return DingusDiffdrive::cb_return::SUCCESS;
}

std::vector<hardware_interface::StateInterface> DingusDiffdrive::export_state_interfaces() {
  std::vector<hardware_interface::StateInterface> state_interfaces;

  state_interfaces.emplace_back(hardware_interface::StateInterface(
    l_wheel_.name(), hardware_interface::HW_IF_VELOCITY, &l_wheel_.vel_));
  state_interfaces.emplace_back(hardware_interface::StateInterface(
    l_wheel_.name(), hardware_interface::HW_IF_POSITION, &l_wheel_.pos_));
  state_interfaces.emplace_back(hardware_interface::StateInterface(
    l_wheel_.name(), hardware_interface::HW_IF_CURRENT, &l_wheel_.current_));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(l_wheel_.name(), "counts", &l_wheel_.counts_));
  state_interfaces.emplace_back(hardware_interface::StateInterface(
    r_wheel_.name(), hardware_interface::HW_IF_VELOCITY, &r_wheel_.vel_));
  state_interfaces.emplace_back(hardware_interface::StateInterface(
    r_wheel_.name(), hardware_interface::HW_IF_POSITION, &r_wheel_.pos_));
  state_interfaces.emplace_back(hardware_interface::StateInterface(
    r_wheel_.name(), hardware_interface::HW_IF_CURRENT, &r_wheel_.current_));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(r_wheel_.name(), "counts", &r_wheel_.counts_));

  RCLCPP_INFO(get_logger(), "Interfaces: %s", imu_.name.c_str());

  state_interfaces.emplace_back(
    hardware_interface::StateInterface(imu_.name, "orientation.x", &imu_.orientation_x));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(imu_.name, "orientation.y", &imu_.orientation_y));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(imu_.name, "orientation.z", &imu_.orientation_z));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(imu_.name, "orientation.w", &imu_.orientation_w));

  state_interfaces.emplace_back(
    hardware_interface::StateInterface(imu_.name, "angular_velocity.x", &imu_.angular_velocity_x));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(imu_.name, "angular_velocity.y", &imu_.angular_velocity_y));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(imu_.name, "angular_velocity.z", &imu_.angular_velocity_z));

  state_interfaces.emplace_back(
    hardware_interface::StateInterface(imu_.name, "linear_acceleration.x", &imu_.linear_accel_x));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(imu_.name, "linear_acceleration.y", &imu_.linear_accel_y));
  state_interfaces.emplace_back(
    hardware_interface::StateInterface(imu_.name, "linear_acceleration.z", &imu_.linear_accel_z));

  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> DingusDiffdrive::export_command_interfaces() {
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  command_interfaces.emplace_back(hardware_interface::CommandInterface(
    l_wheel_.name(), hardware_interface::HW_IF_VELOCITY, &l_wheel_.cmd_));
  command_interfaces.emplace_back(hardware_interface::CommandInterface(
    r_wheel_.name(), hardware_interface::HW_IF_VELOCITY, &r_wheel_.cmd_));

  command_interfaces.emplace_back(
    hardware_interface::CommandInterface("default", HW_IF_PROPORTIONAL, &pid_.p));
  command_interfaces.emplace_back(
    hardware_interface::CommandInterface("default", HW_IF_INTEGRAL, &pid_.i));
  command_interfaces.emplace_back(
    hardware_interface::CommandInterface("default", HW_IF_DERIVATIVE, &pid_.d));

  return command_interfaces;
}

DingusDiffdrive::cb_return DingusDiffdrive::on_activate(const rclcpp_lifecycle::State &) {
  RCLCPP_INFO(get_logger(), "Starting DingusDiffdrive controller");

  if (!serial_port_->open()) {
    RCLCPP_ERROR(get_logger(), "Cannot open serial port!");
    return DingusDiffdrive::cb_return::ERROR;
  }

  serial_port_->add_receive_callback(std::bind(
    &DingusDiffdrive::async_serial_callback, this, std::placeholders::_1, std::placeholders::_2));

  return DingusDiffdrive::cb_return::SUCCESS;
}

DingusDiffdrive::cb_return DingusDiffdrive::on_deactivate(const rclcpp_lifecycle::State &) {
  RCLCPP_INFO(get_logger(), "Stopping DingusDiffdrive controller");

  serial_port_->close();

  return DingusDiffdrive::cb_return::SUCCESS;
}

DingusDiffdrive::hwi_return DingusDiffdrive::read(const rclcpp::Time &, const rclcpp::Duration &) {
  // Most reading is done in the async_serial_callback
  if (!serial_port_->is_open()) {
    return DingusDiffdrive::hwi_return::ERROR;
  }
  return DingusDiffdrive::hwi_return::OK;
}

DingusDiffdrive::hwi_return DingusDiffdrive::write(const rclcpp::Time &, const rclcpp::Duration &) {
  if (!serial_port_->is_open()) {
    return DingusDiffdrive::hwi_return::ERROR;
  }

  auto l_cmd = l_wheel_.cmd_; // rad/s
  auto r_cmd = r_wheel_.cmd_;

  // RCLCPP_INFO_STREAM(get_logger(), "cmd: " << l_wheel_.cmd_ << ", " << r_wheel_.cmd_);

  // Create and send command to robot
  // Heads up, protobuf api handles deleting the `new` objects
  otomo::TopMsg msg;
  otomo::DiffDrive *diff_drive = new otomo::DiffDrive();
  diff_drive->set_left_motor(l_cmd);
  diff_drive->set_right_motor(r_cmd);
  msg.set_allocated_diff_drive(diff_drive);

  async_serial::KissOutputStream out_kiss;
  if (encode_message(out_kiss, msg)) {
    auto buf = out_kiss.get_buffer();
    serial_port_->send(buf);
  } else {
    RCLCPP_ERROR(get_logger(), "Cannot serialize diffdrive msg to string");
  }

  // Update PID params
  if (old_pid_.p != pid_.p || old_pid_.i != pid_.i || old_pid_.d != pid_.d) {
    old_pid_.p = pid_.p;
    old_pid_.i = pid_.i;
    old_pid_.d = pid_.d;

    RCLCPP_WARN(get_logger(), "Updating PID: %f, %f, %f", pid_.p, pid_.i, pid_.d);

    otomo::TopMsg pid_msg;
    otomo::Pid *pid = new otomo::Pid();
    pid->set_p(pid_.p);
    pid->set_i(pid_.i);
    pid->set_d(pid_.d);

    pid_msg.set_allocated_pid(pid);
    async_serial::KissOutputStream out_pid;
    if (encode_message(out_pid, pid_msg)) {
      auto buf_pid = out_pid.get_buffer();
      serial_port_->send(buf_pid);
    } else {
      RCLCPP_ERROR(get_logger(), "Cannot serialize pid msg to string");
    }
  }

  return DingusDiffdrive::hwi_return::OK;
}

void DingusDiffdrive::async_serial_callback(const std::vector<uint8_t> &buf, size_t num_received) {
  for (size_t i = 0; i < num_received; i++) {
    int ret = recv_buf_.add_byte(buf[i]);
    if (ret != 0) {
      if (ret != -1) {
        // RCLCPP_WARN_STREAM(get_logger(), "receive buffer error: " << ret);
      }
      recv_buf_.init();
    } else if (recv_buf_.is_ready()) {
      uint8_t port;
      std::vector<uint8_t> in_proto = recv_buf_.get_buffer(ret, port);

      otomo::TopMsg proto_msg;
      if (!proto_msg.ParseFromArray((const void *)&in_proto[0], in_proto.size())) {
        RCLCPP_ERROR(get_logger(), "Could not deserialize proto msg from mcu!, 0x%x, %ld",
          in_proto.front(), in_proto.size());
      } else if (proto_msg.has_state()) {
        const auto state = proto_msg.state();
        l_wheel_.vel_ = state.left_motor().angular_velocity();
        l_wheel_.pos_ = state.left_motor().encoder();
        l_wheel_.counts_ = state.left_motor().counts();
        l_wheel_.current_ = state.left_motor().current();
        r_wheel_.vel_ = state.right_motor().angular_velocity();
        r_wheel_.pos_ = state.right_motor().encoder();
        r_wheel_.counts_ = state.right_motor().counts();
        r_wheel_.current_ = state.right_motor().current();

        const auto now = std::chrono::system_clock::now();
        double stamp = static_cast<double>(now.time_since_epoch().count()) / 1000000000;

        std::stringstream ss;
        ss << std::fixed << std::setprecision(5);
        ss << stamp << ", " << l_wheel_.cmd_ << ", " << l_wheel_.vel_ << ", ";
        ss << r_wheel_.cmd_ << ", " << r_wheel_.vel_;
        // RCLCPP_INFO(get_logger(), "MOTOR: %s", ss.str().c_str());
      } else if (proto_msg.has_imu()) {
        const auto imu = proto_msg.imu();
        const auto g_x = imu.gyro().x();
        const auto g_y = imu.gyro().y();
        const auto g_z = imu.gyro().z();
        const auto a_x = imu.accel().x();
        const auto a_y = imu.accel().y();
        const auto a_z = imu.accel().z();

        std::stringstream ss;
        ss << std::fixed << std::setprecision(5);
        ss << a_x << ", " << a_y << ", " << a_z;
        RCLCPP_INFO(get_logger(), "IMU: %s", ss.str().c_str());

        imu_.angular_velocity_x = g_x;
        imu_.angular_velocity_y = g_y;
        imu_.angular_velocity_z = g_z;

        imu_.linear_accel_x = a_x;
        imu_.linear_accel_y = a_y;
        imu_.linear_accel_z = a_z;
      } else if (proto_msg.has_drive_response()) {
        RCLCPP_INFO_STREAM(get_logger(), "Got robot response");
      }
      recv_buf_.init();
    }
  }
}

} // namespace dingus_plugins::controllers

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(
  dingus_plugins::controllers::DingusDiffdrive, hardware_interface::SystemInterface)
