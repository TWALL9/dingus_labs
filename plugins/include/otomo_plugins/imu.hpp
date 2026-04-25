#pragma once

#include <string>

namespace otomo_plugins::controllers {
struct Imu {
    std::string name;

    double orientation_x { 0.0 };
    double orientation_y { 0.0 };
    double orientation_z { 0.0 };
    double orientation_w { 0.0 };

    double angular_velocity_x { 0.0 };
    double angular_velocity_y { 0.0 };
    double angular_velocity_z { 0.0 };

    double linear_accel_x { 0.0 };
    double linear_accel_y { 0.0 };
    double linear_accel_z { 0.0 };
};

}
