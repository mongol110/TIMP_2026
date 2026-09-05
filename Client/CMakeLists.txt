find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)
add_executable(Client main.cpp mainwindow.cpp connectiondialog.cpp)
target_link_libraries(Client PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)
