// Directives
#include <iostream>
#include <string>
#include <cctype>
#include <typeinfo>
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkCamera.h>
#include <vtkAreaPicker.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkColorSeries.h>
#include <vtkContourWidget.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkPolyDataCollection.h>
#include <vtkPolygonalSurfaceContourLineInterpolator.h>
#include <vtkPolygonalSurfacePointPlacer.h>
#include <vtkOBJImporter.h>

#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkLookupTable.h>
#include <vtkMeshQuality.h>
#include <vtkOBBDicer.h>
#include <vtkOutlineCornerFilter.h>

#include <vtkExtractEdges.h>
#include <vtkLine.h>
#include <vtkDataSetMapper.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkIdFilter.h>
#include <vtkIdTypeArray.h>
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkInteractorStyleRubberBandZoom.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPropPicker.h>
#include <vtkPlanes.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVersion.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkCleanPolyData.h>
#include <vtkFeatureEdges.h>
#include <vtkPoints.h>
#include <vtkTriangle.h>
#if VTK_VERSION_NUMBER >= 89000000000ULL
#define VTK890 1
#endif

#include <vtkDataReader.h>
#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>

#include <vtksys/SystemTools.hxx>

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1

using namespace std;

// Define interaction style
class HighlightInteractorStyle : public vtkInteractorStyleRubberBandPick
{
public:
  static HighlightInteractorStyle* New();
  vtkTypeMacro(HighlightInteractorStyle, vtkInteractorStyleRubberBandPick);

  HighlightInteractorStyle() : vtkInteractorStyleRubberBandPick()
  {
    this->SelectedMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    this->SelectedActor = vtkSmartPointer<vtkActor>::New();
    this->SelectedActor->SetMapper(SelectedMapper);
  }
  
  virtual void OnLeftButtonUp() override
  {
    // Forward events
    vtkInteractorStyleRubberBandPick::OnLeftButtonUp();
    if (this->CurrentMode == VTKISRBP_SELECT)
    {
      vtkNew<vtkNamedColors> colors;

      vtkPlanes* frustum = static_cast<vtkAreaPicker*>(this->GetInteractor()->GetPicker())->GetFrustum();

      vtkNew<vtkExtractPolyDataGeometry> extractPolyDataGeometry;
      extractPolyDataGeometry->SetInputData(this->pointcloudfile);
      extractPolyDataGeometry->SetImplicitFunction(frustum);
      extractPolyDataGeometry->Update();

      cout << "Extracted " << extractPolyDataGeometry->GetOutput()->GetNumberOfCells() << " cells" << endl;
      this->SelectedMapper->SetInputData(extractPolyDataGeometry->GetOutput());
      this->SelectedMapper->ScalarVisibilityOff();
      this->SelectedActor->GetProperty()->SetColor(colors->GetColor3d("Tomato").GetData());
      this->SelectedActor->GetProperty()->SetPointSize(5);
      this->SelectedActor->GetProperty()->SetRepresentationToWireframe();

      this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(SelectedActor);
      this->GetInteractor()->GetRenderWindow()->Render();
      this->HighlightProp(NULL);
    }
  }
  void SetPolyData(vtkSmartPointer<vtkPolyData> pointcloudfile)
  {
    this->pointcloudfile = pointcloudfile;
  }

private:
  vtkSmartPointer<vtkPolyData> pointcloudfile;
  vtkSmartPointer<vtkActor> SelectedActor;
  vtkSmartPointer<vtkDataSetMapper> SelectedMapper;
};

  void MakeLUT(vtkLookupTable* lut)
{
  // Make the lookup table.
  vtkNew<vtkColorSeries> colorSeries;
  // Select a color scheme.
  int colorSeriesEnum;
  colorSeriesEnum = colorSeries->BREWER_DIVERGING_BROWN_BLUE_GREEN_9;
  // colorSeriesEnum = colorSeries->BREWER_DIVERGING_SPECTRAL_10;
  // colorSeriesEnum = colorSeries->BREWER_DIVERGING_SPECTRAL_3;
  // colorSeriesEnum = colorSeries->BREWER_DIVERGING_PURPLE_ORANGE_9;
  // colorSeriesEnum = colorSeries->BREWER_SEQUENTIAL_BLUE_PURPLE_9;
  // colorSeriesEnum = colorSeries->BREWER_SEQUENTIAL_BLUE_GREEN_9;
  // colorSeriesEnum = colorSeries->BREWER_QUALITATIVE_SET3;
  // colorSeriesEnum = colorSeries->CITRUS;
  colorSeries->SetColorScheme(colorSeriesEnum);

  colorSeries->BuildLookupTable(lut);
  lut->SetNanColor(1, 0, 0, 1);
}
  
// Global Declarations
  vtkStandardNewMacro(HighlightInteractorStyle);
  vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName);

  vtkSmartPointer<vtkPolyData> pointcloudfile;
  vtkSmartPointer<vtkPolyData> reader;
  vtkSmartPointer<vtkPolyData> inputPolyData;
  vtkNew<vtkTriangleFilter> triangles;
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkIdFilter> idFilter;
  vtkNew<vtkActor> actor;
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkAreaPicker> areaPicker;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  //vtkNew<vtkRenderWindowInteractor> renderWindowInteractorzoom;
  //vtkNew<vtkRenderWindowInteractor> renderWindowInteractorkey;
  vtkNew<HighlightInteractorStyle> style;
  vtkNew<vtkInteractorStyleRubberBandZoom> stylerubberzoom;

  vtkNew<vtkMeshQuality> qualityFilter;
  vtkNew<vtkFeatureEdges> featureEdges;
  vtkNew<vtkPolyDataMapper> diskMapper;
  vtkNew<vtkPolyDataMapper> edgeMapper;
  vtkNew<vtkActor> edgeActor;
  vtkNew<vtkActor> diskActor;
  vtkNew<vtkPolyDataMapper> mapper;
  
  vtkNew<vtkOBBDicer> dicer;
  vtkNew<vtkActor> inputActor;
  vtkNew<vtkOutlineCornerFilter> outline;
  vtkNew<vtkPolyDataMapper> outlineMapper;
  vtkNew<vtkActor> outlineActor;
  vtkNew<vtkLookupTable> lut;
  vtkNew<vtkPolyDataMapper> inputMapper;
  vtkNew<vtkCamera> camera;

  vtkNew<vtkTriangleFilter> triangleFilter;
  vtkNew<vtkPolyDataMapper> contourmapper;
  vtkNew<vtkActor> contouractor;
  vtkNew<vtkContourWidget> contourWidget;
  vtkNew<vtkPolygonalSurfacePointPlacer> pointPlacer;
  vtkNew<vtkPolygonalSurfaceContourLineInterpolator> interpolator;

  vtkNew<vtkOBJImporter> importer;

  



// Define interaction style
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static KeyPressInteractorStyle* New();
  vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);

  virtual void OnKeyPress() override
  {
    // Get the keypress
   
    vtkRenderWindowInteractor* rwi = this->Interactor;
    string key = rwi->GetKeySym();
    // Output the key that was pressed
    cout << "Pressed " << key << endl;
    if (key == "space")
    {
      cout << "There are " << inputPolyData->GetNumberOfPoints() << " points" << endl;
    }
    if (key == "t")
    {
      cout << "There are " << inputPolyData->GetNumberOfPolys() << " triangles" << endl;
    }
    /*
    if (key == "z")
    {
      renderWindowInteractor->SetInteractorStyle(stylerubberzoom);
    }
    if (key == "r")
    {
      renderWindowInteractor->SetInteractorStyle(style);
    }
    */
    // Forward events
    vtkInteractorStyleTrackballCamera::OnKeyPress();
  }
};
vtkStandardNewMacro(KeyPressInteractorStyle);
vtkNew<KeyPressInteractorStyle> stylekey;

int main(int argc, char* argv[])
{

    if (argc == 1)
  {
    cout << "Enter filename with .obj or .stl extension followed by " << argv[0] << endl;
    //cout << "Enter the number of mesh slices (say 5) required in the format: " << argv[0] << " .ext 5 " << endl;
    return EXIT_FAILURE;
  }
  
  int len = strlen(argv[1]);
 
  string extension = vtksys::SystemTools::GetFilenameLastExtension(argv[1]);
  if ((extension == ".ply") || (extension == ".PLY"))
  {
    vtkNew<vtkPLYReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
    pointcloudfile = reader->GetOutput();
    triangles->SetInputConnection(reader->GetOutputPort());
    featureEdges->SetInputConnection(reader->GetOutputPort());
    diskMapper->SetInputConnection(reader->GetOutputPort());
  }
  else if ((extension == ".obj") || (extension == ".OBJ"))
  {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
    pointcloudfile = reader->GetOutput();
    string filenameOBJ(argv[1]);
    string filenameMTL;
    if (argc >= 3)
    {
    filenameMTL = argv[2];
    }
    string texturePath = vtksys::SystemTools::GetFilenamePath(filenameOBJ);

    vtkNew<vtkOBJImporter> importer;

    importer->SetFileName(filenameOBJ.data());
    importer->SetFileNameMTL(filenameMTL.data());
    importer->SetTexturePath(texturePath.data());
    
    triangles->SetInputConnection(reader->GetOutputPort());
    featureEdges->SetInputConnection(reader->GetOutputPort());
    diskMapper->SetInputConnection(reader->GetOutputPort());
  }
  else if ((extension == ".stl") || (extension == ".STL"))
  {
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
    pointcloudfile = reader->GetOutput();
    triangles->SetInputConnection(reader->GetOutputPort());
    featureEdges->SetInputConnection(reader->GetOutputPort());
    diskMapper->SetInputConnection(reader->GetOutputPort());

}
  else if (extension == ".vtk")
  {
    vtkNew<vtkPolyDataReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
  }
  //cout << "2000" << typeid(pointcloudfile).name() << endl;
  triangles->Update();
  inputPolyData = triangles->GetOutput();
  
  idFilter->SetInputData(pointcloudfile);
#if VTK890
  idFilter->SetCellIdsArrayName("OriginalIds");
  idFilter->SetPointIdsArrayName("OriginalIds");
#else
  idFilter->SetIdsArrayName("OriginalIds");
#endif
  idFilter->Update();

  // This is needed to convert the ouput of vtkIdFilter (vtkDataSet) back to vtkPolyData
  vtkNew<vtkDataSetSurfaceFilter> surfaceFilter;
  surfaceFilter->SetInputConnection(idFilter->GetOutputPort());
  surfaceFilter->Update();

  vtkPolyData* input = surfaceFilter->GetOutput();

  // Create a mapper and actor
  mapper->SetInputData(pointcloudfile);
  mapper->ScalarVisibilityOff();

  actor->SetMapper(mapper);
  actor->GetProperty()->SetPointSize(5);
  //actor->GetProperty()->SetDiffuseColor(colors->GetColor3d("Lightblue").GetData());
  
  renderer->UseHiddenLineRemovalOn();
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(800, 600);
  renderWindow->SetWindowName("Readfile");
  renderWindowInteractor->SetPicker(areaPicker);
  
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("Tan").GetData());
  renderWindow->Render();
  importer->SetRenderWindow(renderWindow);
  importer->Update();

  //renderer->GetActiveCamera()->Zoom(0.5);
  
  style->SetPolyData(input);
  renderWindowInteractor-> Initialize();
  renderWindowInteractor->SetInteractorStyle(stylekey);
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // interact with data
  renderWindowInteractor-> Initialize();

string filenameOBJ(argv[1]);
cout << vtksys::SystemTools::GetFilenamePath(filenameOBJ) << " 100" << endl;



  cout << "PRESS: " << endl;
  cout << "   1. q or e to close the window" << endl;
  cout << "   2. w to get wireframe view" << endl;
  cout << "   3. s to get back the shaded view" << endl;
  cout << "   4. Left mouse button to rotate" << endl;
  cout << "   5. Right mouse button to zoom in and out" << endl;
  cout << "   6. Shift+left mouse button to pan" << endl;
  cout << "   7. Ctrl+left mouse button to rotate about a point" << endl;
  cout << "   8. Middle mouse buton to pan" << endl;
  cout << "   9. r+left mouse rectangle window to select mesh elements" << endl;
  cout << "   10. r to release the selection mode and rotate/pan the model" << endl;
  //cout << "   11. Spacebar to know the number of triangles in the mesh" << endl;
  //cout << "   12. t to know the number of triangles in the mesh" << endl;

// To draw contours
  /*
  //if (argv[2] == "line")
  //if (argc > 2)
  //{
  cout << argv[2] << endl;
  triangleFilter->SetInputData(pointcloudfile);
  triangleFilter->Update();
  auto pd = triangleFilter->GetOutput();
  contourmapper->SetInputConnection(triangleFilter->GetOutputPort());
  contouractor->SetMapper(contourmapper);

  contouractor->GetProperty()->SetColor(colors->GetColor3d("MistyRose").GetData());
  contouractor->GetProperty()->SetInterpolationToFlat();

  renderer->AddActor(contouractor);
  //renderer->SetBackground(colors->GetColor3d("CadetBlue").GetData());
  
  contourWidget->SetInteractor(renderWindowInteractor);
  vtkSmartPointer<vtkOrientedGlyphContourRepresentation> rep =
      dynamic_cast<vtkOrientedGlyphContourRepresentation*>(contourWidget->GetRepresentation());
  rep->GetLinesProperty()->SetColor(colors->GetColor3d("Crimson").GetData());
  rep->GetLinesProperty()->SetLineWidth(3.0);

  pointPlacer->AddProp(actor);
  pointPlacer->GetPolys()->AddItem(pd);
  rep->SetPointPlacer(pointPlacer);

  interpolator->GetPolys()->AddItem(pd);
  rep->SetLineInterpolator(interpolator);

  renderWindowInteractor->Render();
  contourWidget->EnabledOn();
//}
*/

/*
// For creating parts in the mesh: (argc == 3), refer to SplitPolyData.cxx
int pieces = 4;
  if (argc == 3)
  {
    pieces = atoi(argv[2]);
  }
int value = isdigit(atoi(argv[2]));
cout << isalpha(argv[2]) << endl;
//if(value)
if(argv[2] == "5")
{
  dicer->SetInputData(pointcloudfile);
  dicer->SetNumberOfPieces(pieces);
  dicer->SetDiceModeToSpecifiedNumberOfPieces();
  dicer->Update();
  int numberOfRegions = dicer->GetNumberOfActualPieces();

  lut->SetNumberOfTableValues(std::max(numberOfRegions, 10));
  lut->Build();
  lut->SetTableValue(0, colors->GetColor4d("Gold").GetData());
  lut->SetTableValue(1, colors->GetColor4d("Banana").GetData());
  lut->SetTableValue(2, colors->GetColor4d("Tomato").GetData());
  lut->SetTableValue(3, colors->GetColor4d("Wheat").GetData());
  lut->SetTableValue(4, colors->GetColor4d("Lavender").GetData());
  lut->SetTableValue(5, colors->GetColor4d("Flesh").GetData());
  lut->SetTableValue(6, colors->GetColor4d("Raspberry").GetData());
  lut->SetTableValue(7, colors->GetColor4d("Salmon").GetData());
  lut->SetTableValue(8, colors->GetColor4d("Mint").GetData());
  lut->SetTableValue(9, colors->GetColor4d("Peacock").GetData());
  
  inputMapper->SetInputConnection(dicer->GetOutputPort());
  inputMapper->SetScalarRange(0, dicer->GetNumberOfActualPieces());
  inputMapper->SetLookupTable(lut);
  //cout << "Asked for: " << dicer->GetNumberOfPieces()<< " pieces, got: " << dicer->GetNumberOfActualPieces() << std::endl;

  inputActor->SetMapper(inputMapper);
  inputActor->GetProperty()->SetInterpolationToFlat();
  outline->SetInputData(pointcloudfile);
  outlineMapper->SetInputConnection(outline->GetOutputPort());
  outlineActor->SetMapper(outlineMapper);
  renderer->AddActor(outlineActor);
  renderer->AddActor(inputActor);

  renderWindow->Render();
}
*/



  renderWindowInteractor->Start();
  

  return EXIT_SUCCESS;
}

