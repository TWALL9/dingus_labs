import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():
    pkg_share_dir = get_package_share_directory("dingus_control")

    use_sim_time = LaunchConfiguration("use_sim_time")
    autostart = LaunchConfiguration("autostart")
    params_file = LaunchConfiguration("params_file")
    map = LaunchConfiguration("map")

    default_params_file = os.path.join(
        pkg_share_dir, "config", "nav2_jazzy_params.yaml"
    )

    nav_bringup_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                os.path.join(
                    get_package_share_directory("nav2_bringup"),
                    "launch",
                    "bringup_launch.py",
                )
            ]
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "map": map,
            "autostart": autostart,
            "params_file": params_file,
        }.items(),
    )

    sim_time_arg = DeclareLaunchArgument(
        "use_sim_time", default_value="false", description="Use simulation/Gazebo clock"
    )

    autostart_arg = DeclareLaunchArgument(
        "autostart", default_value="true", description="automatically start Nav2 stack"
    )

    params_file_arg = DeclareLaunchArgument(
        "params_file",
        default_value=default_params_file,
        description="Parameter file for nav2",
    )

    map_arg = DeclareLaunchArgument(
        "map", default_value="", description="Static file for nav2"
    )

    ld = LaunchDescription()

    ld.add_action(sim_time_arg)
    ld.add_action(autostart_arg)
    ld.add_action(params_file_arg)
    ld.add_action(map_arg)

    ld.add_action(nav_bringup_launch)

    return ld
