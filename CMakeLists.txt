cmake_minimum_required(VERSION 2.8.11)
# Name of the project
set(PROJECT_NAME ShadingDemos)
project(${PROJECT_NAME})

add_subdirectory(${PROJECT_SOURCE_DIR}/Constant/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/DirectionalLightPointLight/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/PerFragADS/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/PointLightASD/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/ShaderSubroutines/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/WireFrameDiscard/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/MultipleLights/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/PerVertexASD/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/PointLightDiffuse/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/DirectionalLightASD/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/MultiplePointLights/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/PerVertexDiffuse/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/TwoSidedASD/ )

