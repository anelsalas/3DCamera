#pragma once

#ifndef MYVTK_UTILITIES_INCLUDE
#define MYVTK_VIEWER_INCLUDE


#include <vtkColorSeries.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkPolyDataMapper.h>

#include <vtkConeSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkPLYReader.h>

// vtk chart includes
#include <vtkChartXY.h>
#include <vtkTable.h>
#include <vtkPlot.h>
#include <vtkFloatArray.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkPen.h>
#include <vtkPointSource.h>
#include <vtkRegularPolygonSource.h>

#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkNamedColors.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkLight.h>

#include <vtkProperty.h>

// assembly example includes
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkConeSource.h>
#include <vtkCylinderSource.h>
#include <vtkAssembly.h>
#include <vtkStructuredGrid.h>
#include <vtkDataSetMapper.h>

// structured grid includes
#include <vtkHedgeHog.h>
#include <vtkDoubleArray.h>
#include <vtkVoxel.h>
#include <vtkUnstructuredGrid.h>

// vtkImage includes
#include <vtkTypeInt16Array.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkImageMapper3D.h>
#include <vtkImageProperty.h>
//#include <vtkActor2D.h>
#include <vtkImageActor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkCommand.h>
#include <vtkHoverWidget.h>

#include <vtkScalarBarActor.h>

#include <vtkImageReader2.h>

#include "vtkRenderTimerLog.h"

#include <boost/filesystem.hpp>

// read binary file
#include <fstream>
#include <iterator>
#include <vector>


namespace MyVtk_Utilities
{
    vtkSmartPointer<vtkLookupTable> CreateBlackAndWhiteLUT();
    vtkSmartPointer<vtkLookupTable> CreateRainbowLUT();

    vtkSmartPointer<vtkScalarBarActor> CreateScalarBarWidget(vtkSmartPointer<vtkLookupTable> lut);
    vtkSmartPointer<vtkImageReader2> ReadBinaryFile(boost::filesystem::path my_path);
    void readBinaryData(boost::filesystem::path my_path);
    std::vector<unsigned short> ReadTextFile(boost::filesystem::path my_path);


}


#endif
