find_package(Qt6 COMPONENTS Core REQUIRED)
add_executable(Singleton main.cpp)
target_link_libraries(Singleton PRIVATE Qt6::Core)
