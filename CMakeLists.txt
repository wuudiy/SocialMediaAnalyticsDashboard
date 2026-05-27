cmake_minimum_required(VERSION 3.16)

project(SocialMediaAnalyticsDashboard VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 自动处理 ui / moc / qrc。
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# .ui 文件放在 forms 目录下，AUTOUIC 需要知道搜索路径。
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Charts)

# 项目源码统一维护在这里，后续新增文件只改这一块。
set(PROJECT_SOURCES
    main.cpp

    controllers/authcontroller.h
    controllers/authcontroller.cpp

    models/user.h
    models/user.cpp
    models/post.h
    models/post.cpp

    services/appstyle.h
    services/appstyle.cpp
    services/databasemanager.h
    services/databasemanager.cpp
    services/desutil.h
    services/desutil.cpp
    services/sha256util.h
    services/sha256util.cpp
    services/userrepository.h
    services/userrepository.cpp
    services/postrepository.h
    services/postrepository.cpp
    services/analyticsservice.h
    services/analyticsservice.cpp
    services/logservice.h
    services/logservice.cpp

    views/dashboardpage.h
    views/dashboardpage.cpp
    views/postmanagementpage.h
    views/postmanagementpage.cpp
    views/logpage.h
    views/logpage.cpp
    views/loginview.h
    views/loginview.cpp
    views/mainwindow.h
    views/mainwindow.cpp
    views/usermanagementpage.h
    views/usermanagementpage.cpp
    views/analyticspage.h
    views/analyticspage.cpp

    forms/LoginForm.ui
    forms/mainwindow.ui
    forms/PostManagementPage.ui
    forms/UserManagementPage.ui
    forms/LogPage.ui
    forms/DashboardPage.ui
    forms/AnalyticsPage.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        services/dashboardvisualizationservice.h services/dashboardvisualizationservice.cpp
    )
else()
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Charts
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/views
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
