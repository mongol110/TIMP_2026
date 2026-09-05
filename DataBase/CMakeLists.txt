find_package(Qt6 COMPONENTS Core Sql REQUIRED)
add_executable(DataBase main.cpp)
target_link_libraries(DataBase PRIVATE Qt6::Core Qt6::Sql)
