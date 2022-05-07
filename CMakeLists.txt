cmake_minimum_required(VERSION 3.9)
project(img_pro)
#指定lib目录
link_directories(${PROJECT_SOURCE_DIR}/lib)
#头文件的路径
include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(img_pro img_pro.c)
target_link_libraries(img_pro libbmp.a)
