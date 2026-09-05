find_package(Qt6 COMPONENTS Core Network Sql REQUIRED)
add_executable(EchoServer main.cpp mytcpserver.cpp functionsforserver.cpp)
target_link_libraries(EchoServer PRIVATE Qt6::Core Qt6::Network Qt6::Sql)
