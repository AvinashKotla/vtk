// Directives
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <typeinfo>
#include <vtksys/SystemTools.hxx>
#include <vtkVersion.h>
#include <array>
#include <algorithm>

#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkCamera.h>
#include <vtkAreaPicker.h>
#include <vtkNew.h>
#include <vtkDataSetMapper.h>
#include <vtkDataSet.h>
#include <vtkIdFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkIdTypeArray.h>
#include <vtkFeatureEdges.h>
#include <vtkExtractEdges.h>
#include <vtkTriangle.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkOutlineCornerFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkProperty.h>
#include <vtkPlanes.h>

#include <vtkAxes.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkInteractorStyleRubberBandZoom.h>

#include <vtkNamedColors.h>
#include <vtkColorSeries.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkOBBDicer.h>
#include <vtkThreshold.h>

#include <vtkDataReader.h>
#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkOBJWriter.h>
#include <vtkPLYWriter.h>
#include <vtkSTLWriter.h>

#include <vtkTexture.h>
#include <vtkTransformTextureCoords.h>
#include <vtkImageReader.h>
#include <vtkImageReader2Factory.h>



#if VTK_VERSION_NUMBER >= 89000000000ULL
#define VTK890 1
#endif

#define VTKISRBP_ORIENT 0
#define VTKISRBP_SELECT 1

using namespace std;

// For points extraction
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

// Global Declarations
  vtkStandardNewMacro(HighlightInteractorStyle);
  vtkSmartPointer<vtkPolyData> pointcloudfile;
  vtkSmartPointer<vtkPolyData> reader;
  vtkSmartPointer<vtkPolyData> inputPolyData;
  vtkNew<vtkTriangleFilter> triangles;
  vtkNew<vtkTriangleFilter> triangleFilter;  

  vtkNew<vtkPolyDataMapper> diskMapper;
  vtkNew<vtkPolyDataMapper> edgeMapper;
  vtkNew<vtkPolyDataMapper> inputMapper;
  vtkNew<vtkPolyDataMapper> mapper;
  vtkNew<vtkPolyDataMapper> outlineMapper;

  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkIdFilter> idFilter;
  vtkNew<vtkActor> actor;
  vtkNew<vtkActor> edgeActor;
  vtkNew<vtkActor> diskActor;
  vtkNew<vtkActor> inputActor;
  vtkNew<vtkActor> outlineActor;
  
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  vtkNew<HighlightInteractorStyle> style;
  vtkNew<vtkInteractorStyleRubberBandZoom> stylerubberzoom;
    
  vtkNew<vtkAreaPicker> areaPicker;
  vtkNew<vtkOBBDicer> dicer;
  vtkNew<vtkFeatureEdges> featureEdges;
  vtkNew<vtkOutlineCornerFilter> outline;
  vtkNew<vtkCamera> camera;
  
  vtkNew<vtkThreshold> selector;
  vtkNew<vtkCallbackCommand> clickCallback;
  string extension, pcf;

  vtkNew<vtkImageReader2Factory> readerFactory;
  vtkSmartPointer<vtkImageReader2> imageReader;
  vtkNew<vtkTexture> texture;
  vtkNew<vtkTransformTextureCoords> transformTexture;

  
// For keyboard interactions
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static KeyPressInteractorStyle* New();
  vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);

  virtual void OnKeyPress() override
  {
    vtkRenderWindowInteractor* rwi = this->Interactor;
    string key = rwi->GetKeySym();
    // Output the key that was pressed
    cout << "Pressed " << key << endl;
    if (key == "a")
    {
      cout << "There are " << inputPolyData->GetNumberOfPoints() << " points" << endl;
    }
    if (key == "t")
    {
      cout << "There are " << inputPolyData->GetNumberOfPolys() << " triangles" << endl;
    }
    if((key == "2")||(key == "3")||(key == "4")||(key == "5")||(key == "6")||(key == "7")||(key == "8"))
    {
      stringstream str(key);
      int pieces;
      str >> pieces;
      dicer->SetInputData(pointcloudfile);
      dicer->SetNumberOfPieces(pieces);
      dicer->SetDiceModeToSpecifiedNumberOfPieces();
      dicer->Update();
      int numberOfRegions = dicer->GetNumberOfActualPieces();
      selector->SetInputArrayToProcess(0, 0, 0, 0, "vtkOBBDicer_GroupIds");
      selector->SetInputConnection(dicer->GetOutputPort());
      selector->AllScalarsOff();

    // Use a color series to create a transfer function
      vtkNew<vtkColorSeries> colorSeries;
      colorSeries->SetColorScheme(vtkColorSeries::BREWER_DIVERGING_SPECTRAL_11);

    // Create an actor for each piece
      for (int i = 0; i < dicer->GetNumberOfActualPieces(); ++i)
      {
        selector->SetLowerThreshold(i);
        selector->SetUpperThreshold(i);
        vtkNew<vtkGeometryFilter> geometry;
        geometry->SetInputConnection(selector->GetOutputPort());
        geometry->Update();

        vtkNew<vtkDataSetMapper> mapper;
        mapper->SetInputData(geometry->GetOutput());
        mapper->ScalarVisibilityOff();

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);

        vtkColor3ub color = colorSeries->GetColor(i);
        double dColor[3];
        dColor[0] = static_cast<double>(color[0]) / 255.0;
        dColor[1] = static_cast<double>(color[1]) / 255.0;
        dColor[2] = static_cast<double>(color[2]) / 255.0;
        actor->GetProperty()->SetColor(dColor);
        renderer->AddActor(actor);
      }
      renderWindow->Render();
      renderWindowInteractor->Initialize();
      //iren->Start();
      vtkNew<vtkGeometryFilter> geometry;
      geometry->SetInputConnection(selector->GetOutputPort());

      if ((extension == ".obj") || (extension == ".OBJ"))
      {
      vtkNew<vtkOBJWriter> writer;
      writer->SetInputConnection(geometry->GetOutputPort());
        for (int i = 0; i < dicer->GetNumberOfActualPieces(); ++i)
          {
          stringstream pieceName;
          pieceName << pcf << "_" << i + 1 << extension;
          selector->SetLowerThreshold(i);
          selector->SetUpperThreshold(i);
          writer->SetFileName(pieceName.str().c_str());
          writer->Write();
        }
      }
      if ((extension == ".stl") || (extension == ".STL"))
      {
      vtkNew<vtkSTLWriter> writer;
      writer->SetInputConnection(geometry->GetOutputPort());
        for (int i = 0; i < dicer->GetNumberOfActualPieces(); ++i)
        {
          stringstream pieceName;
          pieceName << pcf << "_" << i + 1 << extension;
          selector->SetLowerThreshold(i);
          selector->SetUpperThreshold(i);
          writer->SetFileName(pieceName.str().c_str());
          writer->Write();
        }
      }
      if ((extension == ".ply") || (extension == ".PLY"))
      {
        vtkNew<vtkPLYWriter> writer;
        writer->SetInputConnection(geometry->GetOutputPort());
        for (int i = 0; i < dicer->GetNumberOfActualPieces(); ++i)
        {
          stringstream pieceName;
          pieceName << pcf << "_" << i + 1 << extension;
          selector->SetLowerThreshold(i);
          selector->SetUpperThreshold(i);
          writer->SetFileName(pieceName.str().c_str());
          writer->Write();
        }
      }    
    }
    if (key == "z")
      {
        cout << "press z for rubberbandzoom" << endl;
        renderWindowInteractor->SetInteractorStyle(stylerubberzoom);
        exit;
      }
      if (key == "r")
      {
        renderWindowInteractor->SetInteractorStyle(style);
      }
      vtkInteractorStyleTrackballCamera::OnKeyPress();
  }
};
vtkStandardNewMacro(KeyPressInteractorStyle);
vtkNew<KeyPressInteractorStyle> stylekey;

void ClickCallbackFunction(vtkObject* vtkNotUsed(caller), long unsigned int eventId, void* vtkNotUsed(clientData),
                           void* vtkNotUsed(callData))
{
  //cout << "Click callback" << endl;
  cout << "Event: " << vtkCommand::GetStringFromEventId(eventId) << endl;
  renderWindowInteractor->SetInteractorStyle(stylerubberzoom);
  //cout << "press z for rubberbandzoom" << endl;
  renderWindowInteractor->SetInteractorStyle(style);
  //cout << "press r to extract cells" << endl;
  renderWindowInteractor->SetInteractorStyle(stylekey);
  //cout << "Keyboard interaction is active" << endl;
  actor->GetProperty()->SetDiffuseColor(colors->GetColor3d("Lightblue").GetData());
}

// Main function
int main(int argc, char* argv[])
{
    pcf = argv[1];
    //cout << pcf << " 100" << endl;
  if (argc == 1)
  {
    cout << "Enter filename with .obj or .stl extension followed by " << argv[0] << endl;
    return EXIT_FAILURE;
  }
  int len = strlen(argv[1]);
  extension = vtksys::SystemTools::GetFilenameLastExtension(argv[1]);
  if ((extension == ".obj") || (extension == ".OBJ"))
  {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
    pointcloudfile = reader->GetOutput();
    triangles->SetInputConnection(reader->GetOutputPort());
    featureEdges->SetInputConnection(reader->GetOutputPort());
    diskMapper->SetInputConnection(reader->GetOutputPort());
    
    imageReader.TakeReference(readerFactory->CreateImageReader2(argv[3]));
    imageReader->SetFileName(argv[3]);
    texture->SetInputConnection(imageReader->GetOutputPort());
    transformTexture->SetInputConnection(reader->GetOutputPort());
    mapper->SetInputConnection(transformTexture->GetOutputPort());
    actor->SetTexture(texture);
  }
  else if ((extension == ".ply") || (extension == ".PLY"))
  {
    vtkNew<vtkPLYReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
    pointcloudfile = reader->GetOutput();
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
  else
  {
    cout << "enter a valid file" << endl;
    return EXIT_FAILURE;
  }
  
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

  vtkNew<vtkDataSetSurfaceFilter> surfaceFilter;
  surfaceFilter->SetInputConnection(idFilter->GetOutputPort());
  surfaceFilter->Update();

  vtkPolyData* input = surfaceFilter->GetOutput();
  mapper->SetInputData(pointcloudfile);
  mapper->ScalarVisibilityOff();

  actor->SetMapper(mapper);
  actor->GetProperty()->SetPointSize(5);
  actor->GetProperty()->SetDiffuseColor(colors->GetColor3d("Lightblue").GetData());
  
  renderer->UseHiddenLineRemovalOn();
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(600, 500);
  renderWindow->SetWindowName("Readfile");
  renderWindowInteractor->SetPicker(areaPicker);
  
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("Tan").GetData());
  renderWindow->Render();

  clickCallback->SetCallback(ClickCallbackFunction);
  renderWindowInteractor->AddObserver(vtkCommand::RightButtonPressEvent, clickCallback);

//  camera->SetPosition(0,0,100); // higher the z value, farther it is from the camera
//  camera->SetFocalPoint(0,0,0); // +x towards left, +y towards down
//  renderer->SetActiveCamera(camera);

  style->SetPolyData(input);
  renderWindowInteractor-> Initialize();
  renderWindowInteractor->SetInteractorStyle(style);
  
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindowInteractor-> Initialize();

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
  cout << "   11. Right mouse button to change the interaction styles" << endl;
  cout << "   11. a to know the number of triangles in the mesh" << endl;
  cout << "   12. t to know the number of triangles in the mesh" << endl;

  renderWindowInteractor->Start();
  
  return EXIT_SUCCESS;
}
