cmake_minimum_required(VERSION 3.15)
project(Polevoy_sem1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenCV от vcpkg
find_package(OpenCV CONFIG REQUIRED)

add_executable(cells_pipeline cells_pipeline.cpp)
# Минимально необходимые модули
target_link_libraries(cells_pipeline PRIVATE
  opencv_core
  opencv_imgproc
  opencv_imgcodecs
  opencv_highgui
)

# Doxygen (опционально)
find_package(Doxygen) # можно без REQUIRED
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



# cmake_minimum_required(VERSION 3.10)
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)


# project(Polevoy_sem1 CXX)

# # Найти пакет OpenCV
# find_package( OpenCV REQUIRED )
# # find_package(OpenCV CONFIG REQUIRED)
# message(STATUS "OpenCV_LIBRARIES = ${OpenCV_LIBRARIES}")
# message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")

# # Поиск исполняемого файла doxygen без использования модуля FindDoxygen
# find_program(DOXYGEN_EXECUTABLE NAMES doxygen)
# if(DOXYGEN_EXECUTABLE)
# 	add_custom_target(docs
# 		COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
# 		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
# 		COMMENT "Generating API documentation with Doxygen"
# 		VERBATIM
# 	)
# else()
# 	message(STATUS "Doxygen not found in PATH. Target 'docs' will be unavailable.")
# endif()

# include_directories(${OpenCV_INCLUDE_DIRS})

# # Включить заголовки OpenCV
# message(STATUS "OpenCV_LIBRARIES = ${OpenCV_LIBRARIES}")
# message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")

# # Добавить исполняемый файл
# add_executable(cells_pipeline cells_pipeline.cpp)

# # Линковать библиотеки OpenCV и semcv
# target_link_libraries(cells_pipeline PRIVATE ${OpenCV_LIBRARIES})








