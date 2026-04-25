cmake_minimum_required(VERSION 3.5)
project(otomo_msgs)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)

find_package(Protobuf REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Joystick.msg"
  "msg/Diffdrive.msg"
  "msg/Pid.msg"
  DEPENDENCIES std_msgs
)

set(PROTO_DIR ${PROJECT_SOURCE_DIR}/otomo-protobuf)
file(GLOB PROTO_FILES ${PROTO_DIR}/*.proto)
message(STATUS "Proto dir: ${PROTO_DIR}")
message(STATUS "Proto files: ${PROTO_FILES}")

# set(DEVEL_PREFIX ${PROJECT_SOURCE_DIR}/../../build/${PROJECT_NAME}/devel/include/${PROJECT_NAME})
set(DEVEL_PREFIX ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME})

set(PROTO_GEN_CPP_DIR ${DEVEL_PREFIX}/generated/)
# set(PROTO_GEN_PY_DIR ${DEVEL_PREFIX}/generated_py)
file(MAKE_DIRECTORY ${PROTO_GEN_CPP_DIR})
# file(MAKE_DIRECTORY ${PROTO_GEN_PY_DIR})

set(PROTO_GEN_CPP_FILES "")
set(PROTO_GEN_PY_FILES "")
foreach(PROTO_FILE ${PROTO_FILES})
  get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
  list(APPEND PROTO_GEN_CPP_FILES ${PROTO_GEN_CPP_DIR}/${PROTO_NAME}.pb.h ${PROTO_GEN_CPP_DIR}/${PROTO_NAME}.pb.cc)
  # list(APPEND PROTO_GEN_PY_FILES ${PROTO_GEN_PY_DIR}/${PROTO_NAME}_pb2.py)
endforeach(PROTO_FILE ${PROTO_FILES})

message(STATUS " gen files: ${PROTO_GEN_CPP_FILES}")

add_custom_command(
  OUTPUT ${PROTO_GEN_CPP_FILES} ${PROTO_GEN_PY_FILES}
  # COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${PROTO_DIR} --cpp_out=${PROTO_GEN_CPP_DIR} --python_out=${PROTO_GEN_PY_DIR} ${PROTO_FILES}
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${PROTO_DIR} --cpp_out=${PROTO_GEN_CPP_DIR} ${PROTO_FILES}
  DEPENDS ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTO_FILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_source_files_properties(${PROTO_GEN_CPP_FILES} PROPERTIES GENERATED TRUE)
# set_source_files_properties(${PROTO_GEN_PY_FILES} PROPERTIES GENERATED TRUE)

add_custom_target(
  ${PROJECT_NAME}_generate_headers
  # DEPENDS ${PROTO_GEN_CPP_FILES} ${PROTO_GEN_PY_FILES}
  DEPENDS ${PROTO_GEN_CPP_FILES}
)

# Declare a C++ library
add_library(
  ${PROJECT_NAME}_proto
  ${PROTO_GEN_CPP_FILES}
)

ament_export_targets(${PROJECT_NAME}_proto HAS_LIBRARY_TARGET)

target_link_libraries(${PROJECT_NAME}_proto ${PROTOBUF_LIBRARY})
add_dependencies(${PROJECT_NAME}_proto ${PROJECT_NAME}_generate_headers)

# Mark libraries for installation
install(
  TARGETS ${PROJECT_NAME}_proto
  EXPORT ${PROJECT_NAME}_proto
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY ${PROTO_GEN_CPP_DIR}/
  DESTINATION include/${PROJECT_NAME}/${PROJECT_NAME}
  FILES_MATCHING PATTERN "*.h"
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  #set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
