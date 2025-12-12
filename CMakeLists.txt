cmake_minimum_required(VERSION 3.16)

project(MyQtProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включить автоматическую обработку Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Поиск Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Собрать все исходные файлы
file(GLOB SOURCES
    "*.cpp"                
    "src/*.cpp"           
)

file(GLOB HEADERS
    "headers/*.h"
    "headers/*.hpp"
)

file(GLOB UI_FILES
    "ui/*.ui"
)

# Создать исполняемый файл
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
)

# Установить свойства цели для UIC
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

# Настроить include директории
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/headers
    ${CMAKE_CURRENT_BINARY_DIR}  # для сгенерированных ui_*.h файлов
)

# Связать с библиотеками Qt
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
)

# Для правильной работы UIC добавить флаг компиляции
target_compile_definitions(${PROJECT_NAME} PRIVATE
    -DQT_NO_KEYWORDS
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()