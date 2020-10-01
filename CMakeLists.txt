cmake_minimum_required(VERSION 3.0.0)
project(semantic_parser VERSION 0.1.0)

find_package(Boost REQUIRED COMPONENTS)

add_executable(main 
main.cpp 
Ontology.cpp 
ParseNode.cpp
)

target_include_directories(main PUBLIC ${Boost_INCLUDE_DIRS}) 
target_link_libraries(main ${Boost_LIBRARIES})
