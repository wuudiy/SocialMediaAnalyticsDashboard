cmake_minimum_required(VERSION 3.16)

project(SocialMediaAnalyticsDashboard VERSION 0.1 LANGUAGES CXX)

# Qt 自动处理 .ui、Q_OBJECT、资源文件
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 告诉 AUTOUIC：.ui 文件在 forms 文件夹里
set(CMAKE_AUTOUIC_SEARCH_PATHS
    forms
)

# 使用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 项目用到了 Qt Widgets 和 Qt Sql
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

# 项目所有源文件、头文件、ui 文件统一放这里
set(PROJECT_SOURCES
    main.cpp

    controllers/authcontroller.h
    controllers/authcontroller.cpp

    models/user.h
    models/user.cpp

    services/databasemanager.h
    services/databasemanager.cpp
    services/userrepository.h
    services/userrepository.cpp
    services/desutil.h
    services/desutil.cpp

    views/loginview.h
    views/loginview.cpp
    views/mainwindow.h
    views/mainwindow.cpp
    views/dashboardpage.h
    views/dashboardpage.cpp
    views/usermanagementpage.h
    views/usermanagementpage.cpp

    forms/LoginForm.ui
    forms/mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SocialMediaAnalyticsDashboard
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        views/dashboardpage.h views/dashboardpage.cpp
        views/usermanagementpage.h views/usermanagementpage.cpp
    )
else()
    if(ANDROID)
        add_library(SocialMediaAnalyticsDashboard SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(SocialMediaAnalyticsDashboard
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(SocialMediaAnalyticsDashboard
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

target_include_directories(SocialMediaAnalyticsDashboard PRIVATE
    views
    models
    controllers
    services
    forms
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SocialMediaAnalyticsDashboard)
endif()

set_target_properties(SocialMediaAnalyticsDashboard PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

install(TARGETS SocialMediaAnalyticsDashboard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SocialMediaAnalyticsDashboard)
endif()
