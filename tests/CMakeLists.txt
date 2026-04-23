set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)

# ======================================================
# === GoogleTest Integration
# ======================================================
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# ======================================================
# === Qt Libraries
# ======================================================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Test)

# ======================================================
# === Project Sources
# ======================================================
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB_RECURSE CORE_SOURCES
        "${CMAKE_SOURCE_DIR}/databaseManager/*.cpp"
        "${CMAKE_SOURCE_DIR}/accounts/*.cpp"
        "${CMAKE_SOURCE_DIR}/tasks/*.cpp"
        "${CMAKE_SOURCE_DIR}/logger/*.cpp"
)
file(GLOB_RECURSE UI_SOURCES
        "${CMAKE_SOURCE_DIR}/authWindow/*.cpp"
        "${CMAKE_SOURCE_DIR}/mainWindow/*.cpp"
        "${CMAKE_SOURCE_DIR}/settings/*.cpp"
        "${CMAKE_SOURCE_DIR}/windowEdit/*.cpp"
)
set(RESOURCES_QRC "${CMAKE_SOURCE_DIR}/resources.qrc")

# ======================================================
# === Test Executable
# ======================================================
add_executable(QtEx_tests
        ${TEST_SOURCES}
        ${CORE_SOURCES}
        ${UI_SOURCES}
        ${RESOURCES_QRC}
)

# ======================================================
# === Link Dependencies
# ======================================================
target_link_libraries(QtEx_tests PRIVATE
        gtest
        sqlite3
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::Test
)

# ======================================================
# === Include Directories
# ======================================================
target_include_directories(QtEx_tests PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/databaseManager
        ${CMAKE_SOURCE_DIR}/databaseManager/SQLUtilities
        ${CMAKE_SOURCE_DIR}/accounts
        ${CMAKE_SOURCE_DIR}/tasks
        ${CMAKE_SOURCE_DIR}/logger
        ${CMAKE_SOURCE_DIR}/mainWindow
        ${CMAKE_SOURCE_DIR}/settings
        ${CMAKE_SOURCE_DIR}/authWindow
        ${CMAKE_SOURCE_DIR}/windowEdit
)

# ======================================================
# === Enable GTest (Метод парсингу вихідного коду)
# ======================================================
include(GoogleTest)

# 1. Отримуємо абсолютні шляхи
get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake LOCATION)
get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
get_filename_component(QT_PLUGIN_DIR "${QT_BIN_DIR}/../plugins" ABSOLUTE)
get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

set(FULL_TEST_PATH "${QT_BIN_DIR};${MINGW_BIN_DIR};$ENV{PATH}")
string(REPLACE ";" "\\;" ESCAPED_TEST_PATH "${FULL_TEST_PATH}")

# 2. Читаємо тести з коду, НЕ запускаючи програму
gtest_add_tests(
        TARGET QtEx_tests
        TEST_LIST All_Qt_Tests
)

# 3. Налаштовуємо змінні ТІЛЬКИ для етапу безпосереднього запуску тестів
if(All_Qt_Tests)
    set_tests_properties(${All_Qt_Tests} PROPERTIES
            ENVIRONMENT "TEST_MODE=1;QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${QT_PLUGIN_DIR};PATH=${ESCAPED_TEST_PATH}"
    )
endif()