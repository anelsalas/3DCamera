// Author: Anel Salas

// Feb 06, 2019: Taking PCL out, using VTK directly
// TODO in february: extract color from plane and put it into cloud.



// Nov 30 2018: finished proyecting plane into cloud
// TODO in December: extract color from plane and put it into cloud.

// Using PCL 1.8.1 using cmake 3.9.2
// migrating to PCL 1.9.1 required to upgrade CMake to 3.13
// dec. 19: Added spi_master.cpp spi_master.h 

//#include "build/BoundaryViewerConfig.h"

#include <iostream>
#include <boost/thread/thread.hpp>
#include "vtk_viewer.h"

// added this to remove the NULL pointer returned value in vtkPolyDataMapper
//#include "vtkAutoInit.h" 
//VTK_MODULE_INIT(vtkRenderingFreeType)
//VTK_MODULE_INIT(vtkRenderingContextOpenGL2);
//VTK_MODULE_INIT(vtkRenderingOpenGL2);
//VTK_MODULE_INIT(vtkInteractionStyle); 
////VTK_MODULE_INIT(vtkChartsCore);
////VTK_MODULE_INIT(vtkCommonCore);
////VTK_MODULE_INIT(vtkCommonDataModel);
////VTK_MODULE_INIT(vtkIOPLY);
////VTK_MODULE_INIT(vtkRenderingCore);
//VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2);
////VTK_MODULE_INIT(vtkViewsContext2D);
////VTK_MODULE_INIT(vtkRenderingContext2D);


//#include <vtkColorSeries.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkRenderWindow.h>
//#include <vtkPolyDataMapper.h>
//
//#include <vtkConeSource.h>
//#include <vtkRenderWindow.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkRenderer.h>
//#include <vtkPLYReader.h>
//
//// vtk chart includes
//#include <vtkChartXY.h>
//#include <vtkTable.h>
//#include <vtkPlot.h>
//#include <vtkFloatArray.h>
//#include <vtkContextView.h>
//#include <vtkContextScene.h>
//#include <vtkPen.h>

//#include "Commdlg.h"
#include "Windows.h"

// WIN32 subsystem stuff
#include <strsafe.h>
#include <WinUser.h>
#define MAX_LOADSTRING 100
// Global Variables:
HINSTANCE hInst;                                // current instance
HWND g_MainWindowHwnd;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


// spi stuff
#include "spi_master.h"

///////////////////  DLN includes ////////////////////////////////////
// #pragma comment(lib, libname) tells the linker to add the 'libname' 
//library to the list of library dependencies, as if you had added it in the 
//project properties at Linker->Input->Additional dependencies
#pragma comment(lib, "../dln/bin/dln4s_spi.lib")

tbb::atomic<bool> g_stopCCDflag(false);
tbb::concurrent_queue<uint8_t*> g_chunk_queue_ptr;

// please use this only on etreme cases or for debuging
// cuz popups are anoying
void popup(std::string message, HWND parent = NULL)
{
	//LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)message.c_str()) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("Message: %s"),
		message.c_str());
	MessageBox(parent, (LPCTSTR)lpDisplayBuf, TEXT("output"), MB_OK);
}

RECT GetLocalCoordinates(HWND hWnd) 
{
    RECT Rect;
    GetWindowRect(hWnd, &Rect);
    MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), (LPPOINT)&Rect, 2);
    return Rect;
}


void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}



//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

    g_MainWindowHwnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!g_MainWindowHwnd)
	{
		ErrorExit(TEXT("CreateWindowEx"));
		return FALSE;
	}

	ShowWindow(g_MainWindowHwnd, nCmdShow);
	UpdateWindow(g_MainWindowHwnd);

	return TRUE;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance, WNDCLASSEXW& wcex)
{
	//WNDCLASSEXW wndclass;

	wcex.cbSize = sizeof(WNDCLASSEXW);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	ATOM retval = RegisterClassExW(&wcex);
	if (retval == 0) ErrorExit(TEXT("RegisterClassExW"));
	return retval;
}

// -------------------------  win main program entry -------------------
// ------------------------- ----------------------- -------------------
// ------------------------- ----------------------- -------------------

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	//LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);

	char const *  shit("CCD Camera Plotter\0");
	char const * className = "CCDCameraPlotterClass\0";
	//wchar_t *  shit("windowTitle\0");
	//wchar_t * className = "windowClassName\0";
	_snwprintf(szTitle, 100, L"%Ts", (wchar_t*)shit);
	_snwprintf(szWindowClass, 100, L"%Ts", (wchar_t*)className);

	WNDCLASSEXW wcex;

	MyRegisterClass(hInstance, wcex);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	//HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //static VTKViewerApp *themyVTKViewerApp;
	//static LinePlot *myLinePlot;
	// statics here simply means that once the variables have been initialized, 
	// they remains in memory until the end of the program.
	static DisplayPlyFile *myPlyViewer;
	static DisplayLinePlot *myLinePlotViewer;
	static DisplaySphereCloud *myPointCloudViewer;
	static Display3DCircle *my3DCircleViewer;
	static Display3DCube *my3Dcube;
	static DisplayAssembly *myAssembly;
	static DisplayVoxels *myvoxels;

	//DisplayVTK mydisplaytest;
	//ShowVtkTest mydisplaytest;
	static HWND exitBtn;
	static HWND openOtherPLYFileBtn;
	static HWND show3DCircleBtn;
	static HWND rotateRightRenderWindowBtn;
	static HWND rotateLeftRenderWindowBtn;
	static HWND setFocalPointBtn;
	static HWND editBox;
    static HWND displaybox;
    static HWND displaywindow;


	switch (message)
	{
		case WM_CREATE:
		{
			//Create an exit button on the bottom of the vtk window
			exitBtn = CreateWindow((LPCSTR)"button", (LPCSTR)"Exit",
				WS_CHILD | WS_VISIBLE | SS_CENTER,
				0, 400, 50, 20,
                hWnd, (HMENU) 2,
				(HINSTANCE)GetWindowLongPtr(hWnd, -6),// GWL_HINSTANCE),
				NULL);
			setFocalPointBtn = CreateWindow((LPCSTR)"button", (LPCSTR)"SetFocalPoint",
				WS_CHILD | WS_VISIBLE | SS_CENTER,
				50, 400, 50, 20,
				hWnd, (HMENU)7,
				(HINSTANCE)GetWindowLongPtr(hWnd, -6),// GWL_HINSTANCE),
				NULL);


			openOtherPLYFileBtn = CreateWindow((LPCSTR)"button", (LPCSTR)"Open New File",
				WS_CHILD | WS_VISIBLE | SS_CENTER,
				100, 400, 100, 20,
				hWnd, (HMENU) 3,
				(HINSTANCE)GetWindowLongPtr(hWnd, -6),// GWL_HINSTANCE),
				NULL);

			show3DCircleBtn = CreateWindow((LPCSTR)"button", (LPCSTR)"Show 3D Polygon",
				WS_CHILD | WS_VISIBLE | SS_CENTER,
				200, 400, 150, 20,
				hWnd, (HMENU) 4,
				(HINSTANCE)GetWindowLongPtr(hWnd, -6),// GWL_HINSTANCE),
				NULL);
			rotateLeftRenderWindowBtn = CreateWindow((LPCSTR)"button", (LPCSTR)"Rotate",
				WS_CHILD | WS_VISIBLE | SS_CENTER,
				350, 400, 50, 20,
				hWnd, (HMENU) 5,
				(HINSTANCE)GetWindowLongPtr(hWnd, -6),// GWL_HINSTANCE),
				NULL);
			rotateRightRenderWindowBtn = CreateWindow((LPCSTR)"button", (LPCSTR)"Rotate",
				WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
				800, 400, 150, 20,
				hWnd, (HMENU) 6,
				(HINSTANCE)GetWindowLongPtr(hWnd, -6),// GWL_HINSTANCE),
				NULL);

            //https://docs.microsoft.com/en-us/windows/desktop/dlgbox/using-dialog-boxes
            RECT lpMyRect = GetLocalCoordinates(g_MainWindowHwnd);

            //GetWindowRect(g_MainWindowHwnd, &lpMyRect);



			editBox = CreateWindow((LPCSTR)"edit", (LPCSTR)" ",
				WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER ,// | WS_HSCROLL,
				0, 420, 800, 20,
				hWnd, (HMENU)8,
				(HINSTANCE)GetWindowLongPtr(hWnd, -6),// GWL_HINSTANCE),
				NULL);

            displaywindow = CreateWindow((LPCSTR)"static", (LPCSTR)" ",
                WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER | WS_HSCROLL,
                0, 0, 400, 400,
                hWnd, (HMENU)9,
                (HINSTANCE)GetWindowLongPtr(hWnd, -6),// GWL_HINSTANCE),
                NULL);



			myPlyViewer = new DisplayPlyFile(hWnd, editBox, displaywindow);
			//myLinePlotViewer = new DisplayLinePlot(hWnd);
	//		myPointCloudViewer = new DisplaySphereCloud(hWnd);
			//my3Dcube = new Display3DCube(hWnd);
			//myAssembly = new DisplayAssembly(hWnd);
			//myStructuredGrid = new DisplayStructuredGrid(hWnd, editBox);
			myvoxels = new DisplayVoxels(hWnd, editBox);

			return 0;
		}

		case WM_COMMAND:
		{
			switch (wParam)
			{
			case 2:
				PostQuitMessage(0);
				if (myPlyViewer)
				{
					delete myPlyViewer;
					myPlyViewer = NULL;

				}
				if (myLinePlotViewer)
				{
					delete myLinePlotViewer;
					myLinePlotViewer = NULL;
				}
				if (myPointCloudViewer)
				{
					delete myPointCloudViewer;
					myPointCloudViewer = NULL;
				}
				if (my3DCircleViewer)
				{
					delete my3DCircleViewer;
					my3DCircleViewer = NULL;
				}
				if (my3Dcube)
				{
					delete my3Dcube;
					my3Dcube = NULL;
				}
				if (myAssembly)
				{
					delete myAssembly;
					myAssembly = NULL;
				}
				if (myvoxels)
				{
					delete myvoxels;
					myvoxels = NULL;
				}



				break;
			case 3: //"Open New File"
				myPlyViewer->restartRenderWindow(hWnd);
				UpdateWindow(hWnd);
				break;
				//popup("shit");
			case 4:
				if (myPointCloudViewer)
				{
					delete myPointCloudViewer;
					myPointCloudViewer = NULL;
				}
				my3DCircleViewer = new Display3DCircle(hWnd);
				UpdateWindow(hWnd);
				break;
			case 5:
				if (myPlyViewer) myPlyViewer->rotate();
				UpdateWindow(hWnd);
			case 6:
				if(myAssembly) myAssembly->rotate();
				UpdateWindow(hWnd);
				break;
			case 7:
				if (myPlyViewer) myPlyViewer->ResetFocalPoint();
				UpdateWindow(hWnd);
				break;





			}
			return 0;
		}
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			if (myPlyViewer)
			{
				delete myPlyViewer;
				myPlyViewer = NULL;

			}
			if (myLinePlotViewer)
			{
				delete myLinePlotViewer;
				myLinePlotViewer = NULL;
			}
			if (myPointCloudViewer)
			{
				delete myPointCloudViewer;
				myPointCloudViewer = NULL;
			}
			if (my3DCircleViewer)
			{
				delete my3DCircleViewer;
				my3DCircleViewer = NULL;
			}
			if (my3Dcube)
			{
				delete my3Dcube;
				my3Dcube = NULL;
			}
			if (myAssembly)
			{
				delete myAssembly;
				myAssembly = NULL;
			}
			if (myvoxels)
			{
				delete myvoxels;
				myvoxels = NULL;
			}

			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



// --------------
// -----Help-----
// --------------
void
printUsage(const char* progName)
{
	std::cout << "\n\nUsage: " << progName << " [options]\n\n"
		<< "Options:\n"
		<< "-------------------------------------------\n"
		<< "-h           this help\n"
		<< "-s           Simple visualisation example\n"
		<< "-n           Normals visualisation example\n"
		<< "-t           Create sample thermal phantom\n"
		<< "-o           open file\n"
		<< "\n\n";
}


std::string  GetFileName(const std::string & prompt) 
{
	const int BUFSIZE = 1024;
	char buffer[BUFSIZE] = { 0 };
	OPENFILENAME ofns = { 0 };
	ofns.lStructSize = sizeof(ofns);
	ofns.lpstrFile = buffer;
	ofns.nMaxFile = BUFSIZE;
	ofns.lpstrTitle = prompt.c_str();

	//GetOpenFileName(&ofns);
	return buffer;
}
//void AddColorToPointXYZRGB(pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloud,
//	pcl::PointXYZRGB& prgb)
//{
//	// call the last point pushed back, and modify its g and blue components
//	if (cloud->size() < 1) return;
//	prgb.g = 100;// cloud->points[cloud->size() - 1].g - 12;
//	prgb.b = 0;// cloud->points[cloud->size() - 1].b + 12;
//
//}
//void makeSampleSingleSlicePhantom(pcl::PointCloud<pcl::PointXYZRGB>::Ptr& basic_cloud_ptr)
//{
//	std::cout << "Generating sample breast slides phantom point cloud.\n\n";
//	//The color for the XYZRGB cloud will 
//	//gradually go from red to green to blue.
//	basic_cloud_ptr->height = 1;
//	//basic_cloud_ptr->width = 1;
//	size_t element(0);
//	float sliceRadius(1);
//	uint8_t r(0), g(0), b(0);
//	//for (float z(-1.0); z <= 1.0; z += 0)
//	{
//		float z(0);
//		for (float angle(0.0), zz(0); angle <= 360.0; angle += 0.9)
//		{
//			pcl::PointXYZRGB point_rgb_thermal;
//			//point_rgb_thermal.x = (1 - z) * cosf(pcl::deg2rad(angle));
//			//point_rgb_thermal.y = (1 - z) * sinf(pcl::deg2rad(angle));
//			point_rgb_thermal.x = sliceRadius * cosf(pcl::deg2rad(angle));
//			point_rgb_thermal.y = sliceRadius * sinf(pcl::deg2rad(angle));
//
//			//std::cout << "x: " << point_rgb_thermal.x << ", y: " << point_rgb_thermal.y << ", z: " << point_rgb_thermal.z << "\n";
//
//			basic_cloud_ptr->width++;
//			point_rgb_thermal.z += z;
//			AddColorToPointXYZRGB(basic_cloud_ptr, point_rgb_thermal);
//			basic_cloud_ptr->points.push_back(point_rgb_thermal);
//			element = basic_cloud_ptr->size();
//		}
//		//std::cout << basic_cloud_ptr->size() << " total points.\n\n";
//
//		// single point of where the FOV plane is
//		/*
//		pcl::PointXYZRGB point_rgb_thermal;
//		point_rgb_thermal.x = 2;
//		point_rgb_thermal.y = 0;
//		point_rgb_thermal.z = 0;
//
//		AddColorToPointXYZRGB(basic_cloud_ptr, point_rgb_thermal);
//		basic_cloud_ptr->points.push_back(point_rgb_thermal);
//		*/
//	}
//}

//void makeSampleSlicesPhantom(pcl::PointCloud<pcl::PointXYZRGB>::Ptr& basic_cloud_ptr)
//{
//	std::cout << "Generating sample breast slides phantom point cloud.\n\n";
//	//The color for the XYZRGB cloud will 
//	//gradually go from red to green to blue.
//	basic_cloud_ptr->height = 1;
//	//basic_cloud_ptr->width = 1;
//
//	//basic_cloud_ptr->height = 800;
//	//basic_cloud_ptr->width = 800;
//	size_t element(0);
//	uint8_t r(0), g(0), b(0);
//	//for (float z(0); z <= 1.0; z += 0.01)
//	for (float z(-1.0); z <= 1.0; z += 0.01)
//	{
//		for (float angle(0.0), zz(0); angle <= 360.0; angle += 0.9)
//		{
//			pcl::PointXYZRGB point_rgb_thermal;
//			point_rgb_thermal.x = (1 - z) * cosf(pcl::deg2rad(angle));
//			point_rgb_thermal.y = (1 - z) * sinf(pcl::deg2rad(angle));
//			//basic_cloud_ptr->width++;
//			point_rgb_thermal.z += z;
//			AddColorToPointXYZRGB(basic_cloud_ptr, point_rgb_thermal);
//			basic_cloud_ptr->points.push_back(point_rgb_thermal);
//			element = basic_cloud_ptr->size();
//			//std::cout << "x=" << point_rgb_thermal.x << ", ";
//		}
//	}
//	//std::cout << basic_cloud_ptr->width << " width, "<< basic_cloud_ptr->height << " height," << basic_cloud_ptr->size() << " total points.\n\n";
//
//}


//void makeSampleHelilcalPhantom()
//{
//	pcl::PointCloud<pcl::PointXYZ>::Ptr basic_cloud_ptr(new pcl::PointCloud<pcl::PointXYZ>);
//	pcl::PointCloud<pcl::PointXYZRGB>::Ptr point_cloud_ptr(new pcl::PointCloud<pcl::PointXYZRGB>);
//	basic_cloud_ptr->height = 1;
//	std::cout << "Generating sample helical breast phantom point cloud.\n\n";
//	// We're going to make an ellipse extruded along the z-axis. The colour for
//	// the XYZRGB cloud will gradually go from red to green to blue.
//	uint8_t r(255), g(15), b(15);
//	for (float z(-1.0); z <= 1.0; )//z += 0.01)
//	{
//		for (float angle(0.0), zz(0); angle <= 360.0; angle += 0.9 * 2)
//		{
//			pcl::PointXYZ basic_point;
//			//basic_point.x = 0.5 * cosf (pcl::deg2rad(angle));
//			basic_point.x = (1 - z) * cosf(pcl::deg2rad(angle));
//
//			basic_point.y = (1 - z) * sinf(pcl::deg2rad(angle));
//			basic_cloud_ptr->width++;
//
//			basic_point.z += z;
//			z += 0.000025;
//			basic_cloud_ptr->points.push_back(basic_point);
//
//			pcl::PointXYZRGB point;
//			point.x = basic_point.x;
//			point.y = basic_point.y;
//			point.z = basic_point.z;
//			uint32_t rgb = (static_cast<uint32_t>(r) << 16 |
//				static_cast<uint32_t>(g) << 8 | static_cast<uint32_t>(b));
//			point.rgb = *reinterpret_cast<float*>(&rgb);
//			point_cloud_ptr->points.push_back(point);
//			if (z < 0.0)
//			{
//				r -= 12;
//				//g += 12;
//			}
//			else
//			{
//				//g -= 12;
//				b += 12;
//			}
//		}
//	}
//
//
//	std::cout << "width: " << basic_cloud_ptr->width << ", height:" << basic_cloud_ptr->height
//		<< ", points:" << basic_cloud_ptr->points.size() << "\n";
//	pcl::PCDWriter writer;
//	std::string namestr = "phantom_helical_sample.pcd";
//	int ret = writer.write(namestr, *basic_cloud_ptr, false);
//}
//
//boost::shared_ptr<pcl::visualization::PCLVisualizer> simpleVis(pcl::PointCloud<pcl::PointXYZ>::ConstPtr cloud)
//{
//	// --------------------------------------------
//	// -----Open 3D viewer and add point cloud-----
//	// --------------------------------------------
//	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
//	viewer->setBackgroundColor(0, 0, 0);
//	viewer->addPointCloud<pcl::PointXYZ>(cloud, "sample cloud");
//	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "sample cloud");
//	viewer->addCoordinateSystem(1.0);
//	viewer->initCameraParameters();
//	return (viewer);
//}
//
//
//boost::shared_ptr<pcl::visualization::PCLVisualizer> rgbVis(pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr cloud)
//{
//	// --------------------------------------------
//	// -----Open 3D viewer and add point cloud-----
//	// --------------------------------------------
//	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
//	viewer->setBackgroundColor(0, 0, 0);
//	pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> rgb(cloud);
//	viewer->addPointCloud<pcl::PointXYZRGB>(cloud, rgb, "sample cloud");
//	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 3, "sample cloud");
//	viewer->addCoordinateSystem(1.0);
//
//	viewer->initCameraParameters();
//	//viewer->getRenderWindow()->SetParentId(HWND);
//	return (viewer);
//}
//
//
//
//
//boost::shared_ptr<pcl::visualization::PCLVisualizer> normalsVis(
//	pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr cloud, pcl::PointCloud<pcl::Normal>::ConstPtr normals)
//{
//	// --------------------------------------------------------
//	// -----Open 3D viewer and add point cloud and normals-----
//	// --------------------------------------------------------
//	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
//	viewer->setBackgroundColor(0, 0, 0);
//	pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> rgb(cloud);
//	viewer->addPointCloud<pcl::PointXYZRGB>(cloud, rgb, "sample cloud");
//	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 3, "sample cloud");
//	viewer->addPointCloudNormals<pcl::PointXYZRGB, pcl::Normal>(cloud, normals, 10, 0.05, "normals");
//	viewer->addCoordinateSystem(1.0);
//	viewer->initCameraParameters();
//	return (viewer);
//}
//
//
//boost::shared_ptr<pcl::visualization::PCLVisualizer> shapesVis(pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr cloud)
//{
//	// --------------------------------------------
//	// -----Open 3D viewer and add point cloud-----
//	// --------------------------------------------
//	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
//	viewer->setBackgroundColor(0, 0, 0);
//	pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> rgb(cloud);
//	viewer->addPointCloud<pcl::PointXYZRGB>(cloud, rgb, "sample cloud");
//	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 3, "sample cloud");
//	viewer->addCoordinateSystem(1.0);
//	viewer->initCameraParameters();
//
//	//------------------------------------
//	//-----Add shapes at cloud points-----
//	//------------------------------------
//	viewer->addLine<pcl::PointXYZRGB>(cloud->points[0],
//		cloud->points[cloud->size() - 1], "line");
//	viewer->addSphere(cloud->points[0], 0.2, 0.5, 0.5, 0.0, "sphere");
//
//	//---------------------------------------
//	//-----Add shapes at other locations-----
//	//---------------------------------------
//	pcl::ModelCoefficients coeffs;
//	coeffs.values.push_back(0.0);
//	coeffs.values.push_back(0.0);
//	coeffs.values.push_back(1.0);
//	coeffs.values.push_back(0.0);
//	viewer->addPlane(coeffs, "plane");
//	coeffs.values.clear();
//	coeffs.values.push_back(0.3);
//	coeffs.values.push_back(0.3);
//	coeffs.values.push_back(0.0);
//	coeffs.values.push_back(0.0);
//	coeffs.values.push_back(1.0);
//	coeffs.values.push_back(0.0);
//	coeffs.values.push_back(5.0);
//	viewer->addCone(coeffs, "cone");
//
//	return (viewer);
//}
//
//
//boost::shared_ptr<pcl::visualization::PCLVisualizer> viewportsVis(
//	pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr cloud, pcl::PointCloud<pcl::Normal>::ConstPtr normals1, pcl::PointCloud<pcl::Normal>::ConstPtr normals2)
//{
//	// --------------------------------------------------------
//	// -----Open 3D viewer and add point cloud and normals-----
//	// --------------------------------------------------------
//	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
//	viewer->initCameraParameters();
//
//	int v1(0);
//	viewer->createViewPort(0.0, 0.0, 0.5, 1.0, v1);
//	viewer->setBackgroundColor(0, 0, 0, v1);
//	viewer->addText("Radius: 0.01", 10, 10, "v1 text", v1);
//	pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> rgb(cloud);
//	viewer->addPointCloud<pcl::PointXYZRGB>(cloud, rgb, "sample cloud1", v1);
//
//	int v2(0);
//	viewer->createViewPort(0.5, 0.0, 1.0, 1.0, v2);
//	viewer->setBackgroundColor(0.3, 0.3, 0.3, v2);
//	viewer->addText("Radius: 0.1", 10, 10, "v2 text", v2);
//	pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZRGB> single_color(cloud, 0, 255, 0);
//	viewer->addPointCloud<pcl::PointXYZRGB>(cloud, single_color, "sample cloud2", v2);
//
//	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 3, "sample cloud1");
//	viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 3, "sample cloud2");
//	viewer->addCoordinateSystem(1.0);
//
//	viewer->addPointCloudNormals<pcl::PointXYZRGB, pcl::Normal>(cloud, normals1, 10, 0.05, "normals1", v1);
//	viewer->addPointCloudNormals<pcl::PointXYZRGB, pcl::Normal>(cloud, normals2, 10, 0.05, "normals2", v2);
//
//	return (viewer);
//}
//void plotterKeyboardEventOccurred(const pcl::visualization::KeyboardEvent &event)
//{
//	if (event.getKeySym() == "q" && event.keyDown())
//	{
//		std::cout << "q was pressed => Stopping CCD camera threads." << std::endl;
//		g_stopCCDflag = true;
//	}
//}


////unsigned int text_id = 0;
//void keyboardEventOccurred(const pcl::visualization::KeyboardEvent &event,
//	void* viewer_void)
//{
//	pcl::visualization::PCLVisualizer *viewer = static_cast<pcl::visualization::PCLVisualizer *> (viewer_void);
//
//	if (event.getKeySym() == "r" && event.keyDown())
//	{
//		std::cout << "r was pressed => removing all text" << std::endl;
//
//		//  char str[512];
//		//  for (unsigned int i = 0; i < text_id; ++i)
//		//  {
//		//    sprintf (str, "text#%03d", i);
//		//    viewer->removeShape (str);
//		//  }
//		//  text_id = 0;
//	}
//	if (event.getKeySym() == "c" && event.keyDown())
//	{
//		std::cout << "c was pressed => clear coordinate system" << std::endl;
//		viewer->removeCoordinateSystem();// addCoordinateSystem(1.0);
//
//	}
//	if (event.getKeySym() == "s" && event.keyDown())
//	{
//		std::cout << "s was pressed => Stopping CCD camera threads." << std::endl;
//		g_stopCCDflag = true;
//	}
//	if (event.getKeySym() == "q" && event.keyDown())
//	{
//		std::cout << "s was pressed => Stopping CCD camera threads." << std::endl;
//		g_stopCCDflag = true;
//	}
//
//}
//
//void mouseEventOccurred(const pcl::visualization::MouseEvent &event,
//	void* viewer_void)
//{
//	pcl::visualization::PCLVisualizer *viewer = static_cast<pcl::visualization::PCLVisualizer *> (viewer_void);
//	if (event.getButton() == pcl::visualization::MouseEvent::LeftButton &&
//		event.getType() == pcl::visualization::MouseEvent::MouseButtonRelease)
//	{
//		std::cout << "Left mouse button released at position (" << event.getX() << ", " << event.getY() << ")" << std::endl;
//
//		//char str[512];
//		//sprintf (str, "text#%03d", text_id ++);
//		//viewer->addText ("clicked here", event.getX (), event.getY (), str);
//	}
//}
//
//boost::shared_ptr<pcl::visualization::PCLVisualizer> interactionCustomizationVis()
//{
//	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
//	viewer->setBackgroundColor(0, 0, 0);
//	viewer->addCoordinateSystem(1.0);
//
//	viewer->registerKeyboardCallback(keyboardEventOccurred, (void*)viewer.get());
//	viewer->registerMouseCallback(mouseEventOccurred, (void*)viewer.get());
//
//	return (viewer);
//}
//
//// returns a pointcloud with the points of the given cloud that are within the given plane
//void findPointsInPlane(pcl::PointCloud<pcl::PointXYZRGB>::Ptr& point_cloud_thermal_ptr, Eigen::Vector3f V_planeNormal, int32_t scalar_d)
//{
//	pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc_rgb_projection_ptr(new pcl::PointCloud<pcl::PointXYZRGB>);
//	pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc_rgb_projPlusTherm_ptr(new pcl::PointCloud<pcl::PointXYZRGB>);
//	Eigen::Vector3f V_pointInCloud, V_pointInPlane;
//	int32_t tr(0);
//
//	// given a plane defined by the normal and the scalar d
//	// a point p' on the plane closest to the given point p can be 
//	// found by:
//	// p' = p - (n . p + d) * n
//	// V_pointInPlane = V_pointInCloud - (V_planeNormal . V_pointInCloud + scalar_d) * V_planeNormal;
//
//	// iterate through the point cloud to determine if any point
//	// can be reflected on the plane
//
//	// scalar_d  // distance from center to the plane of view of the camera
//	// its invalid to multiply a scalar to a vector so we make the scalar a vector:
//	// Eigen::Vector3f V_scalar = scalar_d * Eigen::Vector3f::Ones();
//
//	for (pcl::PointCloud<pcl::PointXYZRGB>::iterator iter = point_cloud_thermal_ptr->begin();
//		iter != point_cloud_thermal_ptr->end();
//		++iter, tr++)
//	{
//
//		// std::cout << "x: " << iter->x << ", y:" << iter->y << ", iteration: " << int(tr) << endl;
//
//		V_pointInCloud << iter->x, iter->y, iter->z;
//
//		pcl::PointXYZRGB pointInPlane_rgb;
//
//		// get the distance to the plan from each point in the cloud
//		// if l_distance is negative its on the opposite side of normal if positive 
//		//float l_distance = V_pointInCloud.dot(V_planeNormal) + scalar_d;
//		float l_distance = V_planeNormal.dot(V_pointInCloud) + scalar_d;
//
//		// dunno why I can only view the graph when l_distance is less than 0. meaning negative
//		if (l_distance < 0)
//		{
//			V_pointInPlane << V_pointInCloud - l_distance * V_planeNormal;
//			// cout << "{" << V_pointInPlane[0] << "," << V_pointInPlane[1] << "," << V_pointInPlane[2] << "}, ";
//			// add the point to the cloud
//			pointInPlane_rgb.x = V_pointInPlane[0];
//			pointInPlane_rgb.y = V_pointInPlane[1];
//			pointInPlane_rgb.z = V_pointInPlane[2];
//
//			// TODO: extract color from plane and put it in cloud!
//			// Each point P's projected coordinates in the plane's own basies are given by
//			// [x',y'] = [dot(P - Center, U)], dot(P - Center,V)]
//			// to convert this to world coordinates simply do:
//			// world_coord = Center + U * X' + V * Y'
//
//			pointInPlane_rgb.r = 0;
//			pointInPlane_rgb.g = 50;
//			pointInPlane_rgb.b = 50;
//
//			// adding projection into different cloud
//			pc_rgb_projection_ptr->points.push_back(pointInPlane_rgb);
//		}
//	}
//	// now I add the 2 clouds to show only one
//	*point_cloud_thermal_ptr = *pc_rgb_projection_ptr + *point_cloud_thermal_ptr;
//	//delete pc_rgb_projection_ptr;
//}
//
//
//void ShowCameraFieldOfViewInCloud(pcl::PointCloud<pcl::PointXYZRGB>::Ptr& point_cloud_thermal_ptr,
//	pcl::ModelCoefficients *plane_coeff,
//	int32_t ax, int32_t bx, int32_t c, int32_t scalar_d,
//	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer)
//{
//	// I'm using this plane as a representation of where the thermal camera
//	// FOV would be.
//	// See SampleConsensusModelPlane for more information
//	// Eigen::Vector4f plane_parameters;
//	// the plane equation is Ax+By+Cz+d=0
//	// where d is the distance from the center of the 3D plane to the 
//	// camera FOV plane.
//	// I figured as an example a unitary normal vector in the plane as (1,0,0),
//	// then the plane equation is 1x+0y+0z+d = 0
//	// if point P within the plane is (2,0,0) then d = -2. and d is 
//	// the distance of the plane to the center of the slice
//	// I would need to establizh the width and lenght of the plane. it should be
//	// a rectangle the size of the camera FOV.
//
//	// create the above mentioned plane
//
//	//boost::shared_ptr< ::pcl::ModelCoefficients> plane_coeff;
//
//	//pcl::ModelCoefficients plane_coeff;
//	plane_coeff->values.resize(4);    // We need 4 values
//	plane_coeff->values[0] = ax; // Ax
//	plane_coeff->values[1] = bx; // By
//	plane_coeff->values[2] = c; // Cz
//	plane_coeff->values[3] = scalar_d;// d
//
//	viewer->addPlane(*plane_coeff);
//
//	// test point to view within the FOV plane
//	pcl::PointXYZRGB point_rgb_thermal;
//	pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc_rgb_projection_ptr(new pcl::PointCloud<pcl::PointXYZRGB>);
//	pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc_rgb_projPlusTherm_ptr(new pcl::PointCloud<pcl::PointXYZRGB>);
//
//	point_rgb_thermal.x = 10;
//	point_rgb_thermal.y = 0;
//	point_rgb_thermal.z = 0;
//
//
//	point_rgb_thermal.g = 0;
//	point_rgb_thermal.b = 100;
//	point_rgb_thermal.r = 255;
//	// color appears strong fucking blue
//
//	//AddColorToPointXYZRGB(point_cloud_thermal_ptr, point_rgb_thermal);
//	point_cloud_thermal_ptr->points.push_back(point_rgb_thermal);
//
//	// show the cloud with the crazy point
//	//viewer = rgbVis(point_cloud_thermal_ptr);//simpleVis(point_cloud_ptr);
//
//
//
//	// add the plane to the viewer AFTER THE DATA CLOUD HAS ALL THE POINTS	
//	// otherwise program will silently crash
//
//}

/** the data from the DLN spi adapter arrives in 8 bits.
this function converts the whole shunk into a 16 bit vector pair.
I use the pair only to plot a graph on the screen. This graph may be used
for calibration and debuging.
Pair use doble cuz thats what the plotter takes as input.
*/
void convert8bitDataTo16bit(std::unique_ptr<uint8_t[]>& inputBuffer, std::vector< std::pair<double, double>>& dataVectPair)
{
	int ii = 0;
	uint8_t pixel = 0;
	int badDataBytes(2 + 512 + 6);
	int realDataLength(CHUNK_SIZE / 2);
	for (int i = badDataBytes; i<realDataLength; i++, i++)
	{
		dataVectPair.push_back(std::make_pair(pixel++, ((inputBuffer[i] & 0x00FF) << 8) + (inputBuffer[i + 1] & 0x00FF)));
		//dataVectPair[ii].second.push( ((inputBuffer[i] & 0x00FF) << 8) + (inputBuffer[i + 1] & 0x00FF)  );
		//dataVectPair[ii].first.push( pixel++ );
		ii++;
		if (pixel > 127)
		{
			pixel = 0;
		}
	}

}
void convert8bitDataTo16bit(uint8_t* inputBuffer_ptr, std::vector< std::pair<double, double>>& dataVectPair)
{
	int ii = 0;
	uint8_t pixel = 0;
	int badDataBytes(2 + 512 + 6);
	int realDataLength(CHUNK_SIZE / 2);
	for (int i = badDataBytes; i<realDataLength; i++, i++)
	{
		dataVectPair.push_back(std::make_pair(pixel++, ((inputBuffer_ptr[i] & 0x00FF) << 8) + (inputBuffer_ptr[i + 1] & 0x00FF)));
		//dataVectPair[ii].second.push( ((inputBuffer[i] & 0x00FF) << 8) + (inputBuffer[i + 1] & 0x00FF)  );
		//dataVectPair[ii].first.push( pixel++ );
		ii++;
		if (pixel > 127)
		{
			pixel = 0;
		}
	}

}

//void flipPlotColorScheme(boost::shared_ptr<pcl::visualization::PCLPlotter> plotter)
//{
//	if (plotter->getColorScheme() == vtkColorSeries::SPECTRUM)
//	{
//		plotter->setColorScheme(vtkColorSeries::COOL);
//	}
//	else
//	{
//		plotter->setColorScheme(vtkColorSeries::SPECTRUM);
//	}
//
//}


/** Same as convert8bitDataTo16bit but using tbb::parallel_for.
*/
std::vector< std::pair<double, double>> convert8bitDataTo16bitParallel_for(std::unique_ptr<uint8_t[]>& inputBuffer)
{
	int ii = 0;
	uint8_t pixel = 0;
	int badDataBytes(2 + 512 + 6);
	int realDataLength(CHUNK_SIZE / 2);
	tbb::concurrent_vector< std::pair<double, double>> dataVectPair;
	for (int i = badDataBytes; i<realDataLength; i++, i++)
	{
		dataVectPair.push_back(std::make_pair(pixel++, ((inputBuffer[i] & 0x00FF) << 8) + (inputBuffer[i + 1] & 0x00FF)));
		ii++;
		if (pixel > 127)
			pixel = 0;
	}
	return std::vector< std::pair<double, double>>(dataVectPair.begin(), dataVectPair.end());

}
/*
void insertIntoPair(std::unique_ptr<uint8_t[]>& inputBuffer, tbb::concurrent_vector< std::pair<double, double>>& dataVectPair)
{
dataVectPair.push_back(std::make_pair(pixel++, ((inputBuffer[i] & 0x00FF) << 8) + (inputBuffer[i + 1] & 0x00FF)));
ii++;
if (pixel > 127)
pixel = 0;


}
*/

//void SetupTriangulationPlot(boost::shared_ptr<pcl::visualization::PCLPlotter> plotter)
//{
//	std::string memoryBank;
//	memoryBank = std::to_string(0);
//	std::cout << "\n" << "bank: " << memoryBank << "\n";
//	plotter->setColorScheme(vtkColorSeries::COOL);
//	std::string title = "Triangulation Data XY - mem bank: " + memoryBank;
//	plotter->setTitle(title.c_str());
//	title = "Pixel";
//	plotter->setXTitle(title.c_str());
//	title = "ADC Value";
//	plotter->setYTitle(title.c_str());
//	plotter->setYRange(0, 6000);
//
//	std::vector< std::pair<double, double>> dataVectPair;
//
//	dataVectPair.push_back(std::make_pair(1,1000));
//	dataVectPair.push_back(std::make_pair(2, 2000));
//	dataVectPair.push_back(std::make_pair(3, 3000));
//
//
//	plotter->addPlotData(dataVectPair, "ADC value", vtkChart::BAR);// POINTS);// vtkChart::LINE);
//	
//	plotter->plot();
//}
//void refreshPlotData(boost::shared_ptr<pcl::visualization::PCLPlotter> plotter)
//{
//	std::string memoryBank, AcqElapsed, PlotElapsed;
//	//std::unique_ptr<uint8_t[]> inputBuffer(std::make_unique<uint8_t[]>(CHUNK_SIZE + 1));
//	//std::unique_ptr<uint8_t*> inputBuffer;
//	uint8_t* inputBuffer;
//
//	if (g_chunk_queue_ptr.try_pop(inputBuffer) == false) return; // unlikely scenario when there is no data to plot
//
//	std::vector< std::pair<double, double>> dataVectPair;
//	convert8bitDataTo16bit(inputBuffer, dataVectPair);
//
//	memoryBank = std::to_string(inputBuffer[5]);
//
//	plotter->clearPlots();
//	plotter->addPlotData(dataVectPair, "ADC value", vtkChart::BAR);// POINTS);// vtkChart::LINE);
//
//	std::string title = "Triangulation Data XY - mem bank: " + memoryBank + ", Acq time: " + AcqElapsed;
//	plotter->setTitle(title.c_str());
//
//	flipPlotColorScheme(plotter);
//
//	//plotter->renderOnce();
//
//}
//
//
//
//void plotTriangulationData(std::unique_ptr<uint8_t[]>& inputBuffer, pcl::visualization::PCLPlotter plotter)
//{
//	tbb::tick_count mainStartTime, endTime;
//	mainStartTime = tbb::tick_count::now();
//	std::vector< std::pair<double, double>> dataVectPair, dataVectoPair2;
//	convert8bitDataTo16bit(inputBuffer, dataVectPair);
//	endTime = tbb::tick_count::now();
//	printf("\nelapsed std::vector:%g\n", (endTime - mainStartTime).seconds());
//
//	mainStartTime = tbb::tick_count::now();
//
//	tbb::concurrent_vector< std::pair<double, double>> dataConcurrentVectPair;
//	dataVectoPair2 = convert8bitDataTo16bitParallel_for(inputBuffer);
//	endTime = tbb::tick_count::now();
//	printf("elapsed tbb::concurrent_vector:%g\n", (endTime - mainStartTime).seconds());
//
//	std::string memoryBank;
//	memoryBank = std::to_string(inputBuffer[5]);
//	std::cout << "\n" << "bank: " << memoryBank << "\n";
//
//	plotter.setColorScheme(vtkColorSeries::COOL);
//	plotter.addPlotData(dataVectPair, "ADC value", vtkChart::POINTS);//vtkChart::LINE);
//	std::string title = "Triangulation Data XY - mem bank: " + memoryBank;
//	plotter.setTitle(title.c_str());
//	title = "Pixel";
//	plotter.setXTitle(title.c_str());
//	title = "ADC Value";
//	plotter.setYTitle(title.c_str());
//	plotter.setYRange(0, 6000);
//}
//void refreshPlot(pcl::visualization::PCLPlotter plotter)
//{
//	std::string memoryBank, AcqElapsed, PlotElapsed;
//	std::unique_ptr<uint8_t[]> inputBuffer(std::make_unique<uint8_t[]>(CHUNK_SIZE + 1));
//	tbb::tick_count mainStartTime, endTime;
//	mainStartTime = tbb::tick_count::now();
//
//	spi::AcquireTriangulationDataSPIOnlyFirmware(inputBuffer, false);
//	endTime = tbb::tick_count::now();
//	AcqElapsed = std::to_string((endTime - mainStartTime).seconds());
//
//	std::vector< std::pair<double, double>> dataVectPair;
//	convert8bitDataTo16bit(inputBuffer, dataVectPair);
//
//	memoryBank = std::to_string(inputBuffer[5]);
//
//	plotter.clearPlots();
//	plotter.addPlotData(dataVectPair, "ADC value", vtkChart::POINTS);//vtkChart::LINE);
//
//	std::string title = "Triangulation Data XY - mem bank: " + memoryBank + ", Acq time: " + AcqElapsed;
//	plotter.setTitle(title.c_str());
//
//	//flipPlotColorScheme(plotter);
//
//	plotter.renderOnce();
//
//}



// --------------
// -----Main-----
// --------------
//int main(int argc, char** argv)
//int mymain(int argc, char** argv)




//int mymain(int argc, char** argv)
//{
//
//
//	//--------------------
//	// -----Main loop-----
//	//--------------------
//
//
//
//
//	// --------------------------------------
//	// -----Parse Command Line Arguments-----
//	// --------------------------------------
//	bool simple(false), rgb(false), custom_c(false), normals(false),
//		shapes(false), viewports(false), interaction_customization(false),
//		openfile(false), thermal(false), ccdplot(false);
//	std::string filename("");
//
//	if (pcl::console::find_argument(argc, argv, "-h") >= 0)
//	{
//		printUsage(argv[0]);
//		return 0;
//	}
//
//	else if (pcl::console::find_argument(argc, argv, "-o") >= 0)
//	{
//		openfile = true;
//		pcl::console::parse_argument(argc, argv, "-o", filename);
//	}
//	else if (pcl::console::find_argument(argc, argv, "-t") >= 0)
//	{
//		thermal = true;
//		std::cout << "Simple visualisation example\n";
//	}
//	else if (pcl::console::find_argument(argc, argv, "-n") >= 0)
//	{
//		normals = true;
//		std::cout << "Normals visualisation example\n";
//	}
//	else if (pcl::console::find_argument(argc, argv, "-a") >= 0)
//	{
//		ccdplot = true;
//		std::cout << "CCD Acquisition Plot\n";
//	}
//
//	else
//	{
//		printUsage(argv[0]);
//		return 0;
//	}
//
//	// ------------------------------------
//	// -----Create example point cloud-----
//	// ------------------------------------
//
//
//
//	pcl::PointCloud<pcl::PointXYZ>::Ptr basic_cloud_ptr(new pcl::PointCloud<pcl::PointXYZ>);
//	// pcl::PointCloud<pcl::PointXYZRGBA>::Ptr point_cloud_ptr (new pcl::PointCloud<pcl::PointXYZRGBA>);
//	pcl::PointCloud<pcl::PointXYZRGB>::Ptr point_cloud_thermal_ptr(new pcl::PointCloud<pcl::PointXYZRGB>);
//
//	//filename = "C:\/Users\/Magdalena\/Source/\PCL/\clouds/\TitoBusto.ply";//GetFileName("type file to watch");
//	//pcl::io::loadPCDFile (filename , *basic_cloud_ptr);
//
//
//	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer;
//
//	if (simple)
//	{
//		viewer = simpleVis(basic_cloud_ptr);
//	}
//	else if (openfile)
//	{
//		std::cout << "open file \n.";
//		if (filename.find(".ply")>std::string::npos)
//		{
//			std::cout << "ply file detected\n.";
//			pcl::io::loadPLYFile(filename, *point_cloud_thermal_ptr);
//		}
//		else if (filename.find(".pcd") > std::string::npos)
//		{
//			std::cout << "pcd file detected\n.";
//			pcl::io::loadPCDFile(filename, *point_cloud_thermal_ptr);
//		}
//		viewer = rgbVis(point_cloud_thermal_ptr);//simpleVis(point_cloud_ptr);
//
//												 //viewer = simpleVis(point_cloud_ptr);
//	}
//	else if (thermal)
//	{
//		int32_t ax(0), bx(1), c(0);
//		int32_t scalar_d(-4);  // distance from center to the plane of view of the camera
//		Eigen::Vector3f V_pointInCloud, V_pointInPlane, V_planeNormal(ax, bx, c);
//		pcl::ModelCoefficients plane_coeff;
//
//		// makeSampleSlicesPhantom(point_cloud_thermal_ptr);
//		makeSampleSingleSlicePhantom(point_cloud_thermal_ptr);
//
//		// data of the position of the camera
//		// these values must be taken from the camera parameters and represent the
//		// position of the camera within the plane in the form of a plane equation Ax+By+Cz+d=0
//
//		// its invalid to multiply a scalar to a vector so we make the scalar a vector:
//		// Eigen::Vector3f V_scalar = scalar_d * Eigen::Vector3f::Ones();
//		// given a plane defined by the normal and the scalar d
//		// a point p' on the plane closest to the given point p can be 
//		// found by:
//		// p' = p - (n . p + d) * n
//		// V_pointInPlane = V_pointInCloud - (V_planeNormal . V_pointInCloud + scalar_d) * V_planeNormal;
//
//		// iterate through the point cloud to determine if any point
//		// can be reflected on the plane
//
//		findPointsInPlane(point_cloud_thermal_ptr, V_planeNormal, scalar_d);
//
//		viewer = rgbVis(point_cloud_thermal_ptr);//simpleVis(point_cloud_ptr);
//
//												 // add the plane to the viewer Just of test purposes
//												 // addition needs to be made AFTER THE DATA CLOUD HAS ALL THE POINTS	
//												 // otherwise program will silently crash
//		ShowCameraFieldOfViewInCloud(point_cloud_thermal_ptr, &plane_coeff, ax, bx, c, scalar_d, viewer);
//
//		// to grab keyboard interactions uncomment the 2 lines below
//		viewer->registerKeyboardCallback(keyboardEventOccurred, (void*)viewer.get());
//		//viewer->registerMouseCallback(mouseEventOccurred, (void*)viewer.get());
//		// viewer->spinOnce(100); this is not happening
//
//	}
//	/*
//	else if (rgb)
//	{
//	viewer = rgbVis(point_cloud_ptr);
//	}
//	else if (custom_c)
//	{
//	viewer = customColourVis(basic_cloud_ptr);
//	}
//	*/
//
//	else if (normals)
//	{
//
//		// ----------------------------------------------------------------
//		// -----Calculate surface normals with a search radius of 0.05-----
//		// ----------------------------------------------------------------
//		makeSampleSlicesPhantom(point_cloud_thermal_ptr);
//		//viewer = rgbVis(point_cloud_thermal_ptr);//simpleVis(point_cloud_ptr);
//
//		pcl::NormalEstimation<pcl::PointXYZRGB, pcl::Normal> ne;
//		ne.setInputCloud(point_cloud_thermal_ptr);
//		pcl::search::KdTree<pcl::PointXYZRGB>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGB>());
//		ne.setSearchMethod(tree);
//		pcl::PointCloud<pcl::Normal>::Ptr cloud_normals1(new pcl::PointCloud<pcl::Normal>);
//		ne.setRadiusSearch(0.05);
//		ne.compute(*cloud_normals1);
//
//		// ---------------------------------------------------------------
//		// -----Calculate surface normals with a search radius of 0.1-----
//		// ---------------------------------------------------------------
//		//pcl::PointCloud<pcl::Normal>::Ptr cloud_normals2 (new pcl::PointCloud<pcl::Normal>);
//		//ne.setRadiusSearch (0.1);
//		// ne.compute (*cloud_normals2);
//
//		viewer = normalsVis(point_cloud_thermal_ptr, cloud_normals1);
//	}
//
//	else if (ccdplot)
//	{
//		std::unique_ptr<uint8_t[]> inputBuffer(std::make_unique<uint8_t[]>(CHUNK_SIZE + 1));
//		//spi::AcquireTriangulationDataSPIOnlyFirmware(inputBuffer);
//		// boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer;
//
//		//boost::thread t1(&spi::startTriangulationAcquisition, boost::ref(g_chunk_queue_ptr), boost::ref(g_stopCCDflag));
//
//		boost::shared_ptr<pcl::visualization::PCLPlotter> plotter(new pcl::visualization::PCLPlotter("My Plotter"));
//		//plotter->registerKeyboardCallback(plotterKeyboardEventOccurred);
//
//		//pcl::visualization::PCLPlotter plotter;
//		//plotTriangulationData(inputBuffer, plotter);
//		SetupTriangulationPlot(plotter);
//		//refreshPlotData(plotter);
//		//plotter->spinOnce(100); // pop up the plot
//		while (!plotter->wasStopped())
//		{
//			plotter->spinOnce(100); // pop up the plot
//			boost::this_thread::sleep(boost::posix_time::microseconds(1));
//
//			refreshPlotData(plotter);
//			std::cout << "shit, ";
//		}
//		//g_stopCCDflag = true; // stop the triangulation acquisition thread
//	}
//
//
//	//plotter.spinOnce(100); // pop up the plot
//
//	//--------------------
//	// -----Main loop-----
//	//--------------------
//
//	// std::unique_ptr<uint8_t[]> inputBuffer(std::make_unique<uint8_t[]>(CHUNK_SIZE + 1));
//	// //spi::AcquireTriangulationDataSPIOnlyFirmware(inputBuffer);
//	// // boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer;
//
//	// boost::thread t1(&spi::startTriangulationAcquisition, boost::ref(g_chunk_queue_ptr), boost::ref(g_stopCCDflag));
//
//	// boost::shared_ptr<pcl::visualization::PCLPlotter> plotter(new pcl::visualization::PCLPlotter("My Plotter"));
//	// SetupTriangulationPlot(plotter);
//	// refreshPlotData(plotter);
//	// plotter->renderOnce();
//
//	// //plotter->spinOnce(100); // pop up the plot
//
//
//	// while (!viewer->wasStopped ())
//	// {
//	//viewer->spinOnce (100); // pop up the boundary viewer
//	//plotter->spinOnce(100); // pop up the plot
//	//boost::this_thread::sleep(boost::posix_time::microseconds(100));
//	//refreshPlotData(plotter);
//
//	// }
//	g_stopCCDflag = true; // stop the triangulation acquisition thread
//
//
//}
