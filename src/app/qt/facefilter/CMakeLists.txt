# Copyright (c) 2015, Ruslan Baratov
# Copyright (c) 2015-2016, David Hirvonen
# All rights reserved.

hunter_add_package(nlohmann-json)
find_package(nlohmann-json REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON) # for main.moc

find_package(Qt5Quick REQUIRED) # Qt5::Quick
find_package(Qt5Multimedia REQUIRED) # Qt5::QMultimediaDeclarativeModule

set(
  FACE_FILTER_SOURCES
  main.cpp

  Scene.hpp  
  Scene.cpp
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
  facefilter.qrc
  main.qml
  QTRenderGL.hpp
  QTRenderGL.cpp

  QtFaceDetectorFactory.h
  QtFaceDetectorFactory.cpp

  QtStream.h

  ## Application specific
  FaceFinder.h
  FaceFinder.cpp

  FrameHandler.h
  FrameHandler.cpp

  ## UI
  gpu/LineDrawing.hpp
  gpu/LineDrawing.cpp

  gpu/FacePainter.h
  gpu/FacePainter.cpp

  gpu/FlashFilter.h
  gpu/FlashFilter.cpp

  ## Config
  facefilter.json
  )

if(ANDROID)
  add_library(facefilter SHARED ${FACE_FILTER_SOURCES})

  target_link_libraries(facefilter Qt5::Quick Qt5::Multimedia)
  target_link_libraries(facefilter EGL GLESv2)

  set_target_properties(
    facefilter
    PROPERTIES
    VERSION 1.0
    DEBUG_POSTFIX "" # Fix Android load
    )

  hunter_add_package(QtAndroidCMake)
  list(APPEND CMAKE_MODULE_PATH "${QTANDROIDCMAKE_ROOT}")
  include(AddQtAndroidApk)

  add_qt_android_apk(
    TARGET facefilter-apk
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

  add_executable(facefilter MACOSX_BUNDLE ${FACE_FILTER_SOURCES}
    ${qt_conf}
    ${qml_dir}
    )

  set_target_properties(
    facefilter
    PROPERTIES
    RESOURCE "${qt_conf};${qml_dir}"
    )

  if(IOS)
    set_target_properties(
      facefilter
      PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/plist.in"
      XCODE_ATTRIBUTE_PRODUCT_NAME
      "FacefilterExample"
      XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER
      "com.github.headupinclouds.drishtisdk.facefilter"
      XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" # iPhone/iPad
      )
  endif()
  target_link_libraries(facefilter Qt5::Quick)
  target_link_libraries(facefilter Qt5::Multimedia)
  target_link_libraries(facefilter Qt5::OpenGL)

  ### Plugins
  target_link_libraries(facefilter Qt5::QmlQtQuick2Plugin)
  target_link_libraries(facefilter ${Qt5Multimedia_PLUGINS})
  target_link_libraries(facefilter Qt5::QMultimediaDeclarativeModule)
  if(IOS)
    target_link_libraries(facefilter ${Qt5Gui_PLUGINS})
  else()
    find_package(Qt5PrintSupport REQUIRED)
    target_link_libraries(facefilter Qt5::PrintSupport)
  endif()
endif()

if(APPLE)
  target_sources(facefilter PUBLIC FetchResource.h FetchResource.mm)
endif()

target_link_libraries(facefilter
  drishtisdk
  ${OGLES_GPGPU_LIB}
  ${THREAD_POOL_LIB}
  nlohmann-json::nlohmann-json)

set_property(TARGET facefilter PROPERTY FOLDER "app/gui")
