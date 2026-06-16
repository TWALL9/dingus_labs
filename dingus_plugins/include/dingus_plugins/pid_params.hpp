#ifndef DINGUS_PLUGINS__PID_PARAMS_HPP_
#define DINGUS_PLUGINS__PID_PARAMS_HPP_

#include <string_view>

namespace dingus_plugins::controllers {

constexpr char HW_IF_PROPORTIONAL[] = "proportional";
constexpr char HW_IF_INTEGRAL[] = "integral";
constexpr char HW_IF_DERIVATIVE[] = "derivative";

struct PidParams {
  double p = 0.0;
  double i = 0.0;
  double d = 0.0;
  bool update_pid = false;
};

} // namespace dingus_plugins::controllers

#endif // DINGUS_PLUGINS__PID_PARAMS_HPP_
