cmake_minimum_required(VERSION 3.0)
project(mqttsnoop)

file (GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file (GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)
set (CMAKE_CXX_STANDARD 17)
set (THREADS_PREFER_PTHREAD_FLAG ON)
set (CMAKE_THREAD_PREFER_PTHREAD TRUE)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(qmqtt CONFIG REQUIRED)

# Tell CMake to create the helloworld executable
add_executable(mqttsnoop ${SOURCES})

# Use the Widgets module from Qt 5.
target_link_libraries(mqttsnoop Qt5::Widgets Qt5::Network qmqtt)

# Install the executable
install(TARGETS mqttsnoop DESTINATION bin)
