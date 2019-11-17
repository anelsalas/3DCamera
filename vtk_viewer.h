#pragma once
#ifndef VTK_VIEWER_INCLUDE
#define VTK_VIEWER_INCLUDE

// added this to remove the NULL pointer returned value in vtkPolyDataMapper
#include "vtkAutoInit.h" 
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
//VTK_MODULE_INIT(vtkChartsCore);
//VTK_MODULE_INIT(vtkCommonCore);
//VTK_MODULE_INIT(vtkCommonDataModel);
//VTK_MODULE_INIT(vtkIOPLY);
//VTK_MODULE_INIT(vtkRenderingCore);
VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2);
//VTK_MODULE_INIT(vtkViewsContext2D);
//VTK_MODULE_INIT(vtkRenderingContext2D);


#include "myvtk_utilities.h"
#include <boost/thread/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>


#include "Windows.h"

// WIN32 subsystem stuff
#include <strsafe.h>
#include <WinUser.h>


#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>

class DisplayPlyFile
{
public:
	DisplayPlyFile(HWND parentHwnd, HWND statusTextWindow, HWND displArea);
	~DisplayPlyFile();
	std::string GetFileName(const std::string & prompt);
	void restartRenderWindow(HWND parent);
	void popup(const std::string message, HWND parent = NULL);
	void rotate();
	void ResetFocalPoint();
	void MapFileToMemory(boost::filesystem::path filepath, boost::iostreams::mapped_file_source file, uintmax_t& total_length);
	void DisplayRawFans(boost::filesystem::path my_path);
    void DisplayRawFansVtkImage2(boost::filesystem::path my_path);
    void DisplayTextBoundaryData(boost::filesystem::path my_path);
    void CreateColorImageFromBoundaryVector(vtkImageData* image, std::vector<unsigned short> data);

	void CreateColorImage(vtkImageData* image, uint32_t total_data_values, uintmax_t total_length, const char* dataBuffer);

private:
    HWND mainWindow;
    HWND displayArea1;
    HWND statusText;
	vtkSmartPointer <vtkRenderWindow> renWin;
	vtkSmartPointer <vtkRenderer> renderer;
	vtkSmartPointer <vtkRenderWindowInteractor>iren;
	vtkSmartPointer <vtkPolyDataMapper> mapper;
	vtkSmartPointer <vtkActor> actor;

	vtkSmartPointer <vtkRenderWindow> renWin2;
	vtkSmartPointer <vtkRenderer> renderer2;
	vtkSmartPointer <vtkRenderWindowInteractor>iren2;
	vtkSmartPointer <vtkImageActor> actor2;
    double initialPosition[3];
    double initialFocalPoint[3];
    double initialUpPosition[3];



	vtkSmartPointer <vtkPLYReader> plyReader;
	vtkSmartPointer <vtkLight> light;
	std::string filename;
	HWND status;
	//HWND mainParent;
};



class DisplayLinePlot
{
public:
	DisplayLinePlot(HWND parent, std::vector<unsigned short> data);
	void SetPosition(int x, int y);
	~DisplayLinePlot();
private:
	vtkSmartPointer<vtkTable> datatable;
	vtkSmartPointer<vtkIntArray> arrX;
	vtkSmartPointer<vtkIntArray> arrY;
	vtkSmartPointer<vtkContextView> view;
	vtkSmartPointer<vtkChartXY> chart;

};


class DisplaySphereCloud
{
public:
	DisplaySphereCloud(HWND parent);
	~DisplaySphereCloud();
private:
	vtkSmartPointer <vtkPointSource> pointSource;
	vtkSmartPointer <vtkRenderWindow> renWin;
	vtkSmartPointer <vtkRenderer> renderer;
	vtkSmartPointer <vtkRenderWindowInteractor>iren;
	vtkSmartPointer <vtkPolyDataMapper> mapper;
	vtkSmartPointer <vtkActor> actor;
	std::string filename;

};

class Display3DCircle
{
public:
	Display3DCircle(HWND parent);
	~Display3DCircle();
private:
	vtkSmartPointer<vtkRegularPolygonSource> regularPolygonSource;
	vtkSmartPointer <vtkRenderWindow> renWin;
	vtkSmartPointer <vtkRenderer> renderer;
	vtkSmartPointer <vtkRenderWindowInteractor>iren;
	vtkSmartPointer <vtkPolyDataMapper> mapper;
	vtkSmartPointer <vtkActor> actor;
	std::string filename;
};

class Display3DCube
{
public:
	Display3DCube(HWND parent);
	~Display3DCube();
private:
	vtkSmartPointer <vtkFloatArray> scalars;
	vtkSmartPointer <vtkCellArray> cellArray;
	vtkSmartPointer <vtkStructuredGrid> structuredGrid;
	vtkSmartPointer<vtkDataSetMapper> mapper;
	vtkSmartPointer <vtkPoints> points;
	vtkSmartPointer <vtkPolyData> polydata;
	vtkSmartPointer <vtkNamedColors> colors;
	vtkSmartPointer <vtkRenderWindow> renWin;
	vtkSmartPointer <vtkRenderer> renderer;
	vtkSmartPointer <vtkRenderWindowInteractor>iren;
	//vtkSmartPointer <vtkPolyDataMapper> mapper;
	vtkSmartPointer <vtkActor> actor;
	std::string filename;
};

class DisplayAssembly
{
public:
	DisplayAssembly(HWND parent);
	~DisplayAssembly();
	void rotate();
private:
	vtkSmartPointer <vtkSphereSource> sphere;
	vtkSmartPointer <vtkPolyDataMapper> sphereMapper;
	vtkSmartPointer <vtkActor> sphereActor;

	vtkSmartPointer <vtkCubeSource> cube;
	vtkSmartPointer <vtkPolyDataMapper> cubeMapper;
	vtkSmartPointer <vtkActor> cubeActor;

	vtkSmartPointer <vtkConeSource> cone;
	vtkSmartPointer <vtkPolyDataMapper> coneMapper;
	vtkSmartPointer <vtkActor> coneActor;

	vtkSmartPointer <vtkCylinderSource> cylinder;
	vtkSmartPointer <vtkPolyDataMapper> cylinderMapper;
	vtkSmartPointer <vtkActor> cylinderActor;

	vtkSmartPointer <vtkAssembly> assembly;

	vtkSmartPointer <vtkRenderWindow> renWin;
	vtkSmartPointer <vtkRenderer> renderer;
	vtkSmartPointer <vtkRenderWindowInteractor>iren;
};

class DisplayStructuredGrid
{
public:
	DisplayStructuredGrid(HWND parent, HWND statusTextWindow);
	~DisplayStructuredGrid();
private:
	HWND status;

	vtkSmartPointer<vtkStructuredGrid> sgrid;
	vtkSmartPointer<vtkPoints> points;
	vtkSmartPointer<vtkNamedColors> colors;
	vtkSmartPointer<vtkHedgeHog> hedgehog; //creates oriented lines from the 
	//input data set. Line length is controlled by vector (or normal) 
	//magnitude times scale factor. If VectorMode is UseNormal, 
	//normals determine the orientation of the lines. 
	//Lines are colored by scalar data, if available.


	vtkSmartPointer <vtkPolyDataMapper> sgridMapper;
	vtkSmartPointer <vtkActor> sgridActor;

	vtkSmartPointer <vtkRenderWindow> renWin;
	vtkSmartPointer <vtkRenderer> renderer;
	vtkSmartPointer <vtkRenderWindowInteractor>iren;
	void CreateData();
};

class DisplayPointSource
{
public:
	DisplayPointSource(HWND parent, HWND statusTextWindow);
	~DisplayPointSource();
private:
	HWND status;

	vtkSmartPointer<vtkStructuredGrid> sgrid;
	vtkSmartPointer<vtkCellArray> cells;
	vtkSmartPointer<vtkPoints> points;
	vtkSmartPointer<vtkNamedColors> colors; 
	vtkSmartPointer<vtkPointSource> pointSource;

	vtkSmartPointer <vtkPolyDataMapper> sgridMapper;
	vtkSmartPointer <vtkActor> sgridActor;

	vtkSmartPointer <vtkRenderWindow> renWin;
	vtkSmartPointer <vtkRenderer> renderer;
	vtkSmartPointer <vtkRenderWindowInteractor>iren;
	void CreateData();
};

class DisplayVoxels
{
public:
	DisplayVoxels(HWND parent, HWND statusTextWindow);
	~DisplayVoxels();
private:
	HWND status;

	vtkSmartPointer<vtkUnstructuredGrid> ugrid;
	vtkSmartPointer<vtkCellArray> cells;
	vtkSmartPointer<vtkPoints> points;
	vtkSmartPointer<vtkVoxel> voxels;

	vtkSmartPointer <vtkDataSetMapper> mapper;
	vtkSmartPointer <vtkActor> sgridActor;
	vtkSmartPointer<vtkNamedColors> colors; 


	vtkSmartPointer <vtkRenderWindow> renWin;
	vtkSmartPointer <vtkRenderer> renderer;
	vtkSmartPointer <vtkRenderWindowInteractor>iren;
};

#endif
