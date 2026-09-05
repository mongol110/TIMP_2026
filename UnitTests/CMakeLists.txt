find_package(Qt6 COMPONENTS Core Network Sql Test REQUIRED)
add_executable(UnitTests tst_tests.cpp ../EchoServer/functionsforserver.cpp)
target_link_libraries(UnitTests PRIVATE Qt6::Core Qt6::Network Qt6::Sql Qt6::Test)
add_test(NAME UnitTests COMMAND UnitTests)
