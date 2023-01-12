
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(alltypes)

find_package(VTK COMPONENTS 
  vtkCommonColor
  vtkCommonCore
  vtkCommonDataModel
  vtkFiltersCore
  vtkFiltersModeling
  vtkFiltersExtraction
  vtkFiltersGeneral
  vtkFiltersGeometry
  vtkFiltersSources
  vtkFiltersTexture
  vtkFiltersVerdict
  vtkTestingRendering
  vtkIOImage
  vtkIOImport
  vtkIOExport
  vtkImagingCore
  vtkIOGeometry
  vtkIOLegacy
  vtkIOPLY
  vtkIOXML
  vtkInteractionWidgets
  vtkInteractionStyle
  vtkRenderingContextOpenGL2
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingGL2PSOpenGL2
  vtkRenderingOpenGL2
  vtkTestingRendering
  QUIET
)

if (NOT VTK_FOUND)
  message("Skipping alltypes: ${VTK_NOT_FOUND_MESSAGE}")
  return()
endif()
message (STATUS "VTK_VERSION: ${VTK_VERSION}")
if (VTK_VERSION VERSION_LESS "8.90.0")
  # old system
  add_executable(alltypes
    alltypes.cxx)
  target_link_libraries(alltypes
    PRIVATE
      ${VTK_LIBRARIES})
else()
  # Prevent a "command line is too long" failure in Windows.
  set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")
  add_executable(alltypes MACOSX_BUNDLE alltypes.cxx )
  target_link_libraries(alltypes PRIVATE ${VTK_LIBRARIES})
  # vtk_module_autoinit is needed
  vtk_module_autoinit(
    TARGETS alltypes
    MODULES ${VTK_LIBRARIES}
    )
endif()
