# Copyright (c) 2015, Ruslan Baratov
# Copyright (c) 2015-2016, David Hirvonen
# All rights reserved.

hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)

set(app_name facefilter)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON) # for main.moc

find_package(Qt5Quick REQUIRED) # Qt5::Quick
find_package(Qt5Multimedia REQUIRED) # Qt5::QMultimediaDeclarativeModule

configure_file(
  facefilter.json
  "${CMAKE_CURRENT_BINARY_DIR}/generated/facefilter.json"
  COPYONLY
  )
configure_file(
  main.qml
  "${CMAKE_CURRENT_BINARY_DIR}/generated"
  COPYONLY
  )

## Serialization type/extension
set(model_ext "cpb")
set(facefilter_qrc "${CMAKE_CURRENT_BINARY_DIR}/generated/facefilter.qrc")
configure_file(facefilter.qrc.in "${facefilter_qrc}" @ONLY)

set(
  FACE_FILTER_SOURCES
  main.cpp

  TextureBuffer.hpp
  VideoFilter.hpp
  VideoFilter.cpp
  VideoFilterRunnable.hpp
  VideoFilterRunnable.cpp
  InfoFilter.hpp
  InfoFilter.cpp
  InfoFilterResult.hpp
  InfoFilterRunnable.hpp
  InfoFilterRunnable.cpp
  FPSCalculator.hpp
  FPSCalculator.cpp
  ${facefilter_qrc}
  main.qml
  QTRenderGL.hpp
  QTRenderGL.cpp
  QtFaceDetectorFactory.h
  QtFaceDetectorFactory.cpp
  QtFaceDetectorFactoryCereal.cpp
  QtFaceMonitor.h
  QtFaceMonitor.cpp
  QMLCameraManager.h
  QMLCameraManager.cpp
  
  QtStream.h

  ## Application specific
  FrameHandler.h
  FrameHandler.cpp

  ## Device info
  Device.h
  Device.cpp

  ## Config
  facefilter.json
  )

if(DRISHTI_USE_BEAST)
  list(APPEND FACE_FILTER_SOURCES
    ImageLogger.h
    ImageLogger.cpp
    )
endif()

if(IOS)
  list(APPEND FACE_FILTER_SOURCES
    QIOSViewController+Fix.h
    QIOSViewController+Fix.mm    
    Device.mm    
    )
endif()

if(ANDROID)
  
  add_library(${app_name} SHARED ${FACE_FILTER_SOURCES})

  include (GenerateExportHeader)
  generate_export_header(facefilter)

  target_link_libraries(${app_name} Qt5::Quick Qt5::Multimedia)
  target_link_libraries(${app_name} EGL GLESv2)

  set_target_properties(
    facefilter
    PROPERTIES
    VERSION 1.0
    )

  hunter_add_package(QtAndroidCMake)
  list(APPEND CMAKE_MODULE_PATH "${QTANDROIDCMAKE_ROOT}")
  include(AddQtAndroidApk)

  add_qt_android_apk(
    TARGET ${app_name}-apk
    BASE_TARGET facefilter
    LAUNCH_TARGET facefilter-launch
    PACKAGE_NAME facefilter.qt
    MANIFEST "${CMAKE_CURRENT_LIST_DIR}/AndroidManifest.xml"
    QML_ROOT "${CMAKE_CURRENT_LIST_DIR}"
    )

  hunter_add_package(Android-SDK)
  message("Path to `android`: ${ANDROID-SDK_ROOT}/android-sdk/tools/android")
  message("Path to `emulator`: ${ANDROID-SDK_ROOT}/android-sdk/tools/emulator")
  message("Path to `adb`: ${ANDROID-SDK_ROOT}/android-sdk/platform-tools/adb")

else()
  find_package(Qt5Quick REQUIRED) # Qt5::Quick
  find_package(Qt5Qml REQUIRED) # Qt5::QmlQtQuick2Plugin

  # We should load this manually because of cyclic dependencies in Qt
  find_package(Qt5MultimediaWidgets REQUIRED)

  if(IOS)
    set(qt_conf "${CMAKE_CURRENT_LIST_DIR}/qt.conf")
    set(qml_dir "${CMAKE_CURRENT_BINARY_DIR}/qt_qml")

    hunter_add_package(QtQmlManager)
    list(APPEND CMAKE_MODULE_PATH "${QTQMLMANAGER_ROOT}/cmake")
    include(QtCopyQmlTo)
    QtCopyQmlTo(${qml_dir})
  else()
    set(qt_conf "")
    set(qml_dir "")
  endif()

  add_executable(${app_name} MACOSX_BUNDLE ${FACE_FILTER_SOURCES}
    ${qt_conf}
    ${qml_dir}
    )

  set_target_properties(
    ${app_name}
    PROPERTIES
    RESOURCE "${qt_conf};${qml_dir}"
    )

  if(IOS)
    set_target_properties(
      ${app_name}
      PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/plist.in"
      XCODE_ATTRIBUTE_PRODUCT_NAME
      "${app_name}Example"
      XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER
      "com.github.headupinclouds.drishtisdk.${app_name}"
      XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" # iPhone/iPad
      )
  endif()
  target_link_libraries(${app_name} Qt5::Quick)
  target_link_libraries(${app_name} Qt5::Multimedia)
  target_link_libraries(${app_name} Qt5::OpenGL)
  target_link_libraries(${app_name} Qt5::Sensors)

  ### Plugins
  target_link_libraries(${app_name} Qt5::QmlQtQuick2Plugin)
  target_link_libraries(${app_name} ${Qt5Multimedia_PLUGINS})
  target_link_libraries(${app_name} Qt5::QMultimediaDeclarativeModule)
  if(IOS)
    target_link_libraries(${app_name} ${Qt5Gui_PLUGINS})
  else()
    find_package(Qt5PrintSupport REQUIRED)
    target_link_libraries(${app_name} Qt5::PrintSupport)
  endif()
endif()

target_link_libraries(${app_name} 
  drishtisdk
  ${OGLES_GPGPU_LIB}
  thread-pool-cpp::thread-pool-cpp
  nlohmann_json)

if(DRISHTI_USE_BEAST)
  target_compile_definitions(${app_name} PUBLIC DRISHTI_USE_BEAST=1)
  target_link_libraries(${app_name} Beast::Beast)
  target_include_directories(${app_name}
    PUBLIC
    "$<BUILD_INTERFACE:${DRISHTI_3RD_PARTY_DIR}/utilities>" # for websocket_async_server.hpp    
    )
endif()  

set_property(TARGET facefilter PROPERTY FOLDER "app/gui")
