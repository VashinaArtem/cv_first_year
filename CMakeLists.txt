cmake_minimum_required(VERSION 3.15)
project(Polevoy_sem1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenCV от vcpkg
find_package(OpenCV CONFIG REQUIRED)

add_executable(cells_pipeline cells_pipeline.cpp)
target_link_libraries(cells_pipeline PRIVATE
  opencv_core
  opencv_imgproc
  opencv_imgcodecs
  opencv_highgui
)

install(TARGETS cells_pipeline RUNTIME DESTINATION bin)
install(DIRECTORY control/ DESTINATION control)

# Doxygen (опционально)
find_package(Doxygen)
if(Doxygen_FOUND)
  add_custom_target(docs
    COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
else()
  message(STATUS "Doxygen not found in PATH. Target 'docs' will be unavailable.")
endif()




