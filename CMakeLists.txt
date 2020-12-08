cmake_minimum_required(VERSION 3.0)
project(mqttsnoop)

file (GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file (GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file (GLOB RC "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc")

# Find includes in corresponding build directories
set (CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_CXX_STANDARD 17)
set (THREADS_PREFER_PTHREAD_FLAG ON)
set (CMAKE_THREAD_PREFER_PTHREAD TRUE)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5Qmqtt CONFIG REQUIRED)

# Tell CMake to create the helloworld executable
add_executable(mqttsnoop ${SOURCES} ${RC})

# Use the Widgets module from Qt 5.
target_link_libraries(mqttsnoop Qt5::Widgets Qt5::Network Qt5::Qmqtt)

# Install the executable
install(TARGETS mqttsnoop DESTINATION bin)
