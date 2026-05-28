cmake_minimum_required(VERSION 3.16)

project(SocialMediaAnalyticsDashboard VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Charts)

set(PROJECT_SOURCES
    main.cpp

    controllers/authcontroller.h
    controllers/authcontroller.cpp
    controllers/postcontroller.h
    controllers/postcontroller.cpp
    controllers/usercontroller.h
    controllers/usercontroller.cpp
    controllers/exportcontroller.h
    controllers/exportcontroller.cpp
    controllers/analyticscontroller.h
    controllers/analyticscontroller.cpp
    controllers/dashboardcontroller.h
    controllers/dashboardcontroller.cpp
    controllers/logcontroller.h
    controllers/logcontroller.cpp
    controllers/settingscontroller.h
    controllers/settingscontroller.cpp
    controllers/mainwindowcontroller.h
    controllers/mainwindowcontroller.cpp

    models/user.h
    models/user.cpp
    models/post.h
    models/post.cpp
    models/operationlog.h
    models/analyticsmodels.h
    models/dashboardmodels.h
    models/ExportModels.h
    models/MainWindowModels.h

    repositories/userrepository.h
    repositories/userrepository.cpp
    repositories/postrepository.h
    repositories/postrepository.cpp
    repositories/logrepository.h
    repositories/logrepository.cpp

    infrastructure/databasemanager.h
    infrastructure/databasemanager.cpp

    utils/desutil.h
    utils/desutil.cpp
    utils/sha256util.h
    utils/sha256util.cpp
    utils/platformconstants.h

    styles/appstyle.h
    styles/appstyle.cpp

    services/analyticsservice.h
    services/analyticsservice.cpp
    services/dashboardvisualizationservice.h
    services/dashboardvisualizationservice.cpp
    services/logservice.h
    services/logservice.cpp
    services/postservice.h
    services/postservice.cpp
    services/csvimportservice.h
    services/csvimportservice.cpp
    services/userservice.h
    services/userservice.cpp
    services/settingsservice.h
    services/settingsservice.cpp
    services/exportservice.h
    services/exportservice.cpp
    services/dashboardservice.h
    services/dashboardservice.cpp
    services/authservice.h
    services/authservice.cpp
    services/reportgenerator.h
    services/reportgenerator.cpp

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
    views/exportpage.h
    views/exportpage.cpp
    views/settingspage.h
    views/settingspage.cpp

    forms/LoginForm.ui
    forms/mainwindow.ui
    forms/PostManagementPage.ui
    forms/UserManagementPage.ui
    forms/LogPage.ui
    forms/DashboardPage.ui
    forms/AnalyticsPage.ui
    forms/ExportPage.ui
    forms/SettingsPage.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
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
    ${CMAKE_CURRENT_SOURCE_DIR}/repositories
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/views
    ${CMAKE_CURRENT_SOURCE_DIR}/infrastructure
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/styles
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
