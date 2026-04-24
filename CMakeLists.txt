cmake_minimum_required(VERSION 3.16)
project(QtEx VERSION 0.1 LANGUAGES CXX C)

# ======================================================
# === Основні налаштування проекту
# ======================================================
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ENV{QTFRAMEWORK_BYPASS_LICENSE_CHECK} "1")

# ======================================================
# === Шляхи до Qt і бібліотек
# ======================================================
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/mingw_64/lib/cmake")

# ======================================================
# === Пошук бібліотек Qt
# ======================================================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

# ======================================================
# === SQLite (вбудована статична збірка)
# ======================================================
# Переміщено вище, оскільки наша Core-бібліотека залежить від БД
set(SQLITE_PATH "${CMAKE_BINARY_DIR}/sqlite")

if(NOT EXISTS "${SQLITE_PATH}/sqlite3.c")
    message(STATUS "Downloading SQLite amalgamation for main build...")
    file(MAKE_DIRECTORY "${SQLITE_PATH}")
    file(DOWNLOAD
            "https://www.sqlite.org/2024/sqlite-amalgamation-3460000.zip"
            "${SQLITE_PATH}/sqlite.zip"
            SHOW_PROGRESS
    )
    execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${SQLITE_PATH}/sqlite.zip"
            WORKING_DIRECTORY "${SQLITE_PATH}"
    )
    file(GLOB SQLITE_EXTRACTED_DIR "${SQLITE_PATH}/sqlite-amalgamation-*")
    file(COPY "${SQLITE_EXTRACTED_DIR}/sqlite3.c" DESTINATION "${SQLITE_PATH}")
    file(COPY "${SQLITE_EXTRACTED_DIR}/sqlite3.h" DESTINATION "${SQLITE_PATH}")
endif()

if(NOT TARGET sqlite3)
    add_library(sqlite3 STATIC "${SQLITE_PATH}/sqlite3.c")
    target_include_directories(sqlite3 PUBLIC "${SQLITE_PATH}")
endif()


# ======================================================
# === 1. БІБЛІОТЕКА БІЗНЕС-ЛОГІКИ (ToDoCore)
# ======================================================
# Відокремлюємо логіку від UI згідно з принципами SOLID
set(CORE_SOURCES
        # --- Accounts & Auth ---
        accounts/account.cpp
        accounts/account.h
        accounts/authManager.cpp
        accounts/authManager.h

        # --- Database ---
        databaseManager/SQLUtilities/SQLUtils.cpp
        databaseManager/SQLUtilities/SQLUtils.h
        databaseManager/accountRepository.cpp
        databaseManager/accountRepository.h
        databaseManager/databaseManager.cpp
        databaseManager/databaseManager.h
        databaseManager/TaskRepository.cpp
        databaseManager/TaskRepository.h

        # --- Tasks ---
        tasks/task.cpp
        tasks/task.h
        tasks/TaskManager.cpp
        tasks/TaskManager.h

        # --- Settings (Тільки логіка) ---
        settings/appSettings.h
        settings/appSettings.cpp

        # --- Logging ---
        logger/logger.cpp
        logger/logger.h
        logger/globalLogger.cpp
        logger/globalLogger.h
        tasks/ITaskObserver.h
)

add_library(ToDoCore STATIC ${CORE_SOURCES})

# Лінкуємо SQLite та базові модулі Qt до ядра
target_link_libraries(ToDoCore PUBLIC sqlite3 Qt${QT_VERSION_MAJOR}::Widgets)
# Робимо директорії доступними для графічного додатку
target_include_directories(ToDoCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


# ======================================================
# === 2. ВИКОНУВАНИЙ ФАЙЛ (Графічний інтерфейс QtEx)
# ======================================================
set(APP_SOURCES
        main.cpp
        resources.qrc

        # --- Main window & core UI ---
        mainWindow/mainwindow.cpp
        mainWindow/mainwindow.h
        mainWindow/mainwindow.ui
        mainWindow/taskEditorWindow.cpp
        mainWindow/taskEditorWindow.h
        mainWindow/taskEditorWindow.ui
        mainWindow/taskItemWidget.cpp
        mainWindow/taskItemWidget.h
        mainWindow/taskItemWidget.ui

        # --- Auth UI ---
        authWindow/loginWindow.cpp
        authWindow/loginWindow.h
        authWindow/loginWindow.ui
        authWindow/registerWindow.cpp
        authWindow/registerWindow.h
        authWindow/registerWindow.ui

        # --- Settings UI ---
        settings/settingsWindow.cpp
        settings/settingsWindow.h
        settings/settingsWindow.ui

        # --- Frameless Window & Snap ---
        windowEdit/framelessWindow.cpp
        windowEdit/framelessWindow.h
        windowEdit/snapPreviewWindow.cpp
        windowEdit/snapPreviewWindow.h
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(QtEx
            MANUAL_FINALIZATION
            ${APP_SOURCES}
    )
else()
    add_executable(QtEx ${APP_SOURCES})
endif()

# Лінкуємо нашу бібліотеку ядра (ToDoCore) та візуальні компоненти Qt
target_link_libraries(QtEx PRIVATE
        ToDoCore
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Multimedia
)


# ======================================================
# === Властивості додатку (Windows / macOS)
# ======================================================
set_target_properties(QtEx PROPERTIES
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.QtEx
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

# ======================================================
# === Автоматичний запуск windeployqt (Windows only)
# ======================================================
if(WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)

    add_custom_command(TARGET QtEx POST_BUILD
            COMMAND "${QT_BIN_DIR}/windeployqt.exe"
            --no-translations
            --multimedia
            "$<TARGET_FILE:QtEx>"
            COMMENT "Running windeployqt automatically after build..."
    )
endif()

# ======================================================
# === Завершення для Qt6
# ======================================================
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtEx)
endif()

# ======================================================
# === Інсталяційні шляхи (CMake install)
# ======================================================
include(GNUInstallDirs)
install(TARGETS QtEx
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# Бібліотеку ToDoCore встановлювати не обов'язково, оскільки вона статично вшивається в QtEx

# ======================================================
# === Підключення тестів
# ======================================================
enable_testing()
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
        message(STATUS "Including GoogleTests...")
        add_subdirectory(tests)
    else()
        message(WARNING "Tests directory not found, skipping tests.")
    endif()
endif()