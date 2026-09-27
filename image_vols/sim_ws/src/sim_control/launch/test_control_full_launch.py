import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.actions import ExecuteProcess
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():


    headless_gz = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            PathJoinSubstitution([
                FindPackageShare('usv_description'),
                'launch',
                'headless_gazebo_launch.py'
            ])
        ]),
    )
    
    localization = Node(
        package='sim_control',
        executable='localization',
    )
    
    control = Node(
        package='sim_control',
        executable='simplistic_control',
    )

    return LaunchDescription([
        headless_gz,
        localization,
        control
    ])


'''
# Lidar Frame
vtec_s4/base_link/gpu_lidar
'''
