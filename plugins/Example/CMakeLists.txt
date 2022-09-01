set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(PLUGIN "Example")

add_compile_definitions(WEBRTC_POSIX QT_NO_KEYWORDS)

set(
    SRC
    plugin.cpp
    example.cpp
)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "-fno-rtti ${CMAKE_CXX_FLAGS}")

add_library(${PLUGIN} MODULE ${SRC})
set_target_properties(${PLUGIN} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PLUGIN})
qt5_use_modules(${PLUGIN} Qml Quick DBus Multimedia WebSockets)

target_link_libraries(${PLUGIN} QWebRTC)

execute_process(
    COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH
    OUTPUT_VARIABLE ARCH_TRIPLET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(QT_IMPORTS_DIR "/lib/${ARCH_TRIPLET}")

install(TARGETS ${PLUGIN} DESTINATION ${DATA_DIR}/${QT_IMPORTS_DIR}/${PLUGIN}/)
install(FILES qmldir DESTINATION ${DATA_DIR}/${QT_IMPORTS_DIR}/${PLUGIN}/)
