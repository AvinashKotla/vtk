#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleRubberBandZoom.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkNamedColors.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTimerLog.h>
#include <vtksys/SystemTools.hxx>

#include <algorithm>
#include <array>
#include <random>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{

  string extension;
  vtkSmartPointer<vtkPolyData> threeDfile;
  vtkNew<vtkPolyDataMapper> mapper;
  vtkNew<vtkActor> actor;
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  vtkNew<vtkProperty> backProp;
  vtkNew<vtkInteractorStyleTrackballCamera> style;
  vtkNew<vtkInteractorStyleRubberBandZoom> stylerubzoom;

  extension = vtksys::SystemTools::GetFilenameLastExtension(argv[1]);
  if ((extension == ".obj") || (extension == ".OBJ"))
  {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
    threeDfile = reader->GetOutput();    
  }
  else if ((extension == ".ply") || (extension == ".PLY"))
  {
    vtkNew<vtkPLYReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
    threeDfile = reader->GetOutput();
  }
  else if ((extension == ".stl") || (extension == ".STL"))
  {
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
    threeDfile = reader->GetOutput();
  }
  else
  {
    cout << "enter a valid file" << endl;
    return EXIT_FAILURE;
  }

  mapper->SetInputData(threeDfile);
  actor->SetMapper(mapper);

  renderWindow->SetSize(640, 480);
  renderWindow->AddRenderer(renderer);
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderer->SetBackground(colors->GetColor3d("Wheat").GetData());
  renderer->UseHiddenLineRemovalOn();
  renderer->AddActor(actor);
  
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindowInteractor->SetInteractorStyle(stylerubzoom);
  renderWindowInteractor->SetInteractorStyle(style);

  renderWindow->SetWindowName("alltypes");
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}