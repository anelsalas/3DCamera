#include "vtk_viewer.h"
#include "utilities.h"
#include <iostream>
#include <strsafe.h>
#include <sstream>


constexpr auto MAX_LOADSTRING = 100;


DisplayPlyFile::~DisplayPlyFile()
{
	// vtkSmartPointer takes care of deleting all pointers
}

DisplayPlyFile::DisplayPlyFile(HWND parentHwnd,HWND statusTextWindow,HWND displArea) : mainWindow(parentHwnd), statusText(statusTextWindow), displayArea1(displArea)
{
         
	this->status = statusTextWindow;
	struct stat buffer;
	if (stat(std::string("../clouds/MeDesk.ply").c_str(), &buffer) == 0) // fast way to check if file exists
	{
		this->filename = std::string("../clouds/MeDesk.ply");
	}
	else if (stat(std::string("MeDesk.ply").c_str(), &buffer) == 0)
	{
		this->filename = std::string("MeDesk.ply");
	}
	else
	{
		this->filename = this->GetFileName(this->filename);
		this->popup(this->filename);
	}

	// We create the basic parts of a vtk visualization pipeline and connect them

	this->plyReader = vtkPLYReader::New(); // doesn't work with a smartpointer template
	this->plyReader->SetFileName(this->filename.c_str());
	
	// Create a mapper and actor
	this->mapper = vtkPolyDataMapper::New();
	this->mapper->SetInputConnection(this->plyReader->GetOutputPort());

	this->actor = vtkSmartPointer <vtkActor>::New();
	actor->SetMapper(this->mapper);

	//this->light = vtkSmartPointer<vtkLight>::New();
	////this->light->SetFocalPoint(1.875, 0.6125, 0);
	////this->light->SetPosition(0.875, 1.6125, 1);

	//this->light->SetFocalPoint(0, 0, 30);
 //   this->light->SetPosition(0,0,200);

	//this->light->SetIntensity(1);

	// Create a renderer, render window, and interactor
	this->renderer = vtkSmartPointer <vtkRenderer>::New();
	this->renderer->AddActor(this->actor);
	this->renderer->SetBackground(0, 0, 0);// black
	//this->renderer->AddLight(light);
	//this->renderer->SetBackground(0.1804, 0.5451, 0.3412); // Sea green

	this->renWin = vtkSmartPointer <vtkRenderWindow>::New();
	this->renWin->SetParentId(parentHwnd);  // setup the parent window
	this->renWin->AddRenderer(this->renderer);

	this->iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	this->iren->SetRenderWindow(renWin);

	this->renWin->SetSize(400, 400);


	this->renWin->Render();

    this->renderer->GetActiveCamera()->GetPosition(this->initialPosition[0], this->initialPosition[2], this->initialPosition[2]);
    this->renderer->GetActiveCamera()->GetFocalPoint(this->initialFocalPoint[0], this->initialFocalPoint[1], this->initialFocalPoint[2]);
    this->renderer->GetActiveCamera()->GetViewUp(this->initialUpPosition[0], this->initialUpPosition[1], this->initialUpPosition[2]);


}
void DisplayPlyFile::ResetFocalPoint()
{
//	double focalpoint[3];
	//this->renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
	//this->renderer->GetActiveCamera()->GetFocalPoint(focalpoint);
	//this->renderer->GetActiveCamera()->GetPosition(focalpoint);

	this->renderer->GetActiveCamera()->SetPosition(-404.213118, -1792.326075, 546.431276);
	//this->renderer->GetActiveCamera()->SetViewUp(0, 1, 0);
	//this->renderer->GetActiveCamera()->ParallelProjectionOn();
	this->renderer->ResetCamera();
	//this->renderer->GetActiveCamera()->SetParallelScale(1.5);
	//std::string statusTxt = "focal Point:" + std::to_string(focalpoint[0]) + "," + std::to_string(focalpoint[1]) + "," + std::to_string(focalpoint[2]);
	//SetWindowTextA(this->status, (LPCSTR)statusTxt.c_str());
	this->renWin->Render();



}

std::string  DisplayPlyFile::GetFileName(const std::string & prompt)
{
	const int BUFSIZE = 1024;
	char buffer[BUFSIZE] = { 0 };
	OPENFILENAME ofns = { 0 };
	ofns.lpstrFilter = "All\0*.*\0PLY\0*.ply\0*.rawfans\0";
	ofns.nFilterIndex = 3;
	ofns.lStructSize = sizeof(ofns);
	ofns.lpstrFile = buffer;
	ofns.nMaxFile = BUFSIZE;
	ofns.lpstrTitle = prompt.c_str();

	GetOpenFileName(&ofns);
	return (std::string)buffer;
}

unsigned long long int get_file_size(const char *file_path) 
{
	boost::filesystem::path file{ file_path };
	auto generic_path = file.generic_path();
	return boost::filesystem::file_size(generic_path);
}
unsigned long long int get_page_size()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	return (unsigned long long int) sysInfo.dwPageSize;
}


void DisplayPlyFile::MapFileToMemory(boost::filesystem::path filepath, boost::iostreams::mapped_file_source file, uintmax_t& total_length)
{
	//https://stackoverflow.com/questions/53432100/cannot-read-from-boostmapped-file-data-buffer-is-null
	total_length = boost::filesystem::file_size(filepath);

	boost::iostreams::mapped_file_params parameters;
	parameters.path = filepath.string();


	parameters.length = static_cast<size_t>(total_length);
	parameters.flags = boost::iostreams::mapped_file::mapmode::readonly;
	parameters.offset = static_cast<boost::iostreams::stream_offset>(0);
	//boost::iostreams::mapped_file_source file;
	file.open(parameters);

	//return file.data();

}
void  DisplayPlyFile::CreateColorImageFromBoundaryVector(vtkImageData* image, std::vector<unsigned short> data)
{
    uint32_t dimx(128);
    uint32_t dimy(data.size()/ dimx);// / dimx);
    uint32_t dimz(1);
    image->SetDimensions(dimx, dimy, 1);
    image->AllocateScalars(VTK_UNSIGNED_SHORT, 3);
    //std::unique_ptr <uint16_t[]> rawData(new uint16_t[data.size()]);
    int x(0), y(0);
    for (std::vector<unsigned short>::iterator it = data.begin(); it != data.end(); ++it)
    {
        unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));
        pixel[0] = 0;
        pixel[1] = *it;
        //if (x == 0) pixel[1] = 11000;
        //else   if (x == 127)pixel[1] = 11000;
        //else  if(*it>6500) pixel[1] = *it;
        //else pixel[1] = 6500;
        pixel[2] = 0;
        x++;
        if (x == 128)
        {
            x = 0;
            y++;
        }

    }

    //for (unsigned int x = 0; x < dimx; x++)
    //{
    //    for (unsigned int y = 0; y < dimy; y++)
    //    {
    //        unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));

    //        pixel[1] = data[rawdataIter++];

    //        pixel[0] = 0;
    //        pixel[2] = 0;
    //    }
    //}
    image->Modified();

}



void  DisplayPlyFile::CreateColorImage(vtkImageData* image, uint32_t total_data_values, uintmax_t total_length, const char* dataBuffer)
{
	uint32_t dimx(127);
	//uint32_t dimy(total_data_values/dimx);// / dimx);
    uint32_t dimy(199);// / dimx);

	uint32_t dimz(1),iterx(0),itery(0);

	//uint32_t dimy(20);
	//uint32_t dimx(20);// / dimx);

	//unsigned long long int offset = 0;
    unsigned long long int offset = 1028;
	unsigned long long int page_size = get_page_size();
	auto buffer_size = page_size;
	const uint32_t tdv = total_data_values;
	std::unique_ptr <uint16_t[]> rawData (new uint16_t[total_data_values]);
	uint32_t rawdataIter(0);
	image->SetDimensions(dimx, dimy, 1);
	image->AllocateScalars(VTK_UNSIGNED_SHORT, 3);

	while (offset < total_data_values)
	{
        if (offset == (dimx+1 * dimy+1)-1) break;
		auto remaining_bytes = total_length - offset;
		if (buffer_size > remaining_bytes)
		{
			buffer_size = remaining_bytes;
		}
		for (auto buffer_index = 0; buffer_index < page_size; buffer_index += 2, iterx++, itery++)
		{

			rawData[rawdataIter++] = (dataBuffer[buffer_index] << 8 | dataBuffer[buffer_index + 1]);
			//if (itery == 128) itery = 0;
			//unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(iterx, itery, 0));
			//pixel[1] =  (dataBuffer[buffer_index] << 8 | dataBuffer[buffer_index + 1]);
			//pixel[0] = 0;
			//pixel[2] = 0;
			
			//auto byte = dataBuffer[buffer_index];
			//message.append(std::to_string(byte));

		}
		offset += buffer_size;
	}
	//C:\Users\Magdalena\Source\PCL\3dcam\trunk\data\5010678\501.0678.WAQ-slice-15.rawfans
	rawdataIter = 0;


	for (unsigned int x = 0; x < dimx; x++)
	{
		for (unsigned int y = 0; y < dimy; y++)
		{
			unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));

			pixel[1] = rawData[rawdataIter++];

			pixel[0] = 0;
			pixel[2] = 0;
		}
	}






	image->Modified();
}

class myvtkHoverCallback : public vtkCommand
{
public:
	static myvtkHoverCallback *New()
	{
		return new myvtkHoverCallback;
	}

	myvtkHoverCallback() {}

	virtual void Execute(vtkObject*, unsigned long event, void *vtkNotUsed(calldata))
	{
		switch (event)
		{
			// the mouse stopped moving and the widget hovered
		case vtkCommand::TimerEvent:
			//GetMousePosition(int *x, int *y);

			break;
		//case vtkCommand::EndInteractionEvent: //EndInteractionEvent -> the mouse started to move
		//	break;
		}
	}
};


void DisplayPlyFile::DisplayRawFans(boost::filesystem::path my_path)
{
	boost::iostreams::mapped_file_source file;
	uintmax_t total_length(0);
	uint32_t total_data_values(0);


   vtkSmartPointer<vtkImageReader2> ImageReader = MyVtk_Utilities::ReadBinaryFile(my_path);

	this->MapFileToMemory(my_path, file, total_length);
	total_data_values = total_length / 2; // using 2 bytes per datum

	auto buffer = file.data();

	std::string message;

	// create color image
	vtkSmartPointer<vtkImageData> colorImage = vtkSmartPointer<vtkImageData>::New();
	this->CreateColorImage(colorImage, total_data_values, total_length, buffer);

    vtkSmartPointer<vtkLookupTable> lutBW = MyVtk_Utilities::CreateBlackAndWhiteLUT();

	vtkSmartPointer<vtkImageMapToColors> imageMapper = vtkImageMapToColors::New();
	imageMapper->SetLookupTable(lutBW);
	//imageMapper->PassAlphaToOutputOn();
	imageMapper->SetInputData(colorImage);

	// Create an image actor
	this->actor2 = vtkSmartPointer<vtkImageActor>::New();
	this->actor2->GetMapper()->SetInputConnection(imageMapper->GetOutputPort());
	this->actor2->GetProperty()->SetInterpolationTypeToNearest();

	//imageActor->SetPosition(20, 20);

	// Setup renderers
	this->renderer2 = vtkSmartPointer<vtkRenderer>::New();
	this->renderer2->AddActor(this->actor2);
	this->renderer2->ResetCamera();

	// Setup render window
	this->renWin2 = vtkSmartPointer<vtkRenderWindow>::New();

	// add observer
	//vtkSmartPointer <MyMouseMovedCallback> mouseMovedCallback = vtkSmartPointer < MyMouseMovedCallback>::New();
	//this->renWin2->AddObserver(vtkCommand::  LeftButtonPressEvent, mouseMovedCallback);


	//mouseMovedCallback->Delete();

	this->renWin2->AddRenderer(this->renderer2);

	// Setup render window interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
	renderWindowInteractor->SetInteractorStyle(style);

    // Create the mouse hover widget
	vtkSmartPointer<vtkHoverWidget> hoverWidget = vtkSmartPointer<vtkHoverWidget>::New();
	hoverWidget->SetInteractor(renderWindowInteractor);
	hoverWidget->SetTimerDuration(500);

	// Create a callback to listen to the widget's two VTK events
	vtkSmartPointer<myvtkHoverCallback> hoverCallback = vtkSmartPointer<myvtkHoverCallback>::New();
	hoverWidget->AddObserver(vtkCommand::TimerEvent, hoverCallback);
	hoverWidget->AddObserver(vtkCommand::EndInteractionEvent, hoverCallback);


    // create the scalar_bar
    

	// Render and start interaction
	renderWindowInteractor->SetRenderWindow(this->renWin2);

	//renderer->AddViewProp(imageActor);
	this->renderer2->AddActor2D(this->actor2);
    this->renderer2->AddActor2D(MyVtk_Utilities::CreateScalarBarWidget(lutBW));

    //this->renWin2->SetParentId(displayArea1);
	this->renWin2->Render();
	renderWindowInteractor->Initialize();
	this->renWin2->Render();
	hoverWidget->On();

	renderWindowInteractor->Start();

	SetWindowTextA(this->status, message.c_str());
	//MessageBox(NULL, (LPCSTR)file.data(), TEXT("Test"), MB_OK);
}

void DisplayPlyFile::DisplayTextBoundaryData(boost::filesystem::path my_path)
{
    // clear previous actors in same renderWindow
    this->renderer->RemoveAllViewProps();

    std::vector<unsigned short> data = MyVtk_Utilities::ReadTextFile(my_path);

    // display xy chart of the data also
    DisplayLinePlot* LinePlot = new DisplayLinePlot(this->mainWindow,data);

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();

   CreateColorImageFromBoundaryVector(image, data);

  // vtkSmartPointer<vtkLookupTable> lutBW = MyVtk_Utilities::CreateRainbowLUT();//CreateBlackAndWhiteLUT();
   vtkSmartPointer<vtkLookupTable> lutBW = MyVtk_Utilities::CreateBlackAndWhiteLUT();

   vtkSmartPointer<vtkImageMapToColors> imageMapper = vtkImageMapToColors::New();

   imageMapper->SetLookupTable(lutBW);
   imageMapper->SetInputData(image);

   // Create an image actor
   vtkSmartPointer<vtkImageActor>BoundaryActor = vtkSmartPointer<vtkImageActor>::New();
   BoundaryActor->GetMapper()->SetInputConnection(imageMapper->GetOutputPort());
   BoundaryActor->GetProperty()->SetInterpolationTypeToNearest();

   // Setup renderers
  // this->renderer->AddActor(BoundaryActor);


   //this->renWin->AddRenderer(this->renderer);

   // create the scalar_bar
   this->renderer->AddActor2D(BoundaryActor);
   this->renderer->AddActor2D(MyVtk_Utilities::CreateScalarBarWidget(lutBW));

   //this->renWin2->SetParentId(displayArea1);
   //renderWindowInteractor->Initialize();

   this->renderer->ResetCamera();
   //this->renderer->GetActiveCamera()->Modified();

   DisplayCameraParameters(this->status, this->renderer);

   //this->renderer->Modified();
   //this->renderer->GetActiveCamera()->SetPosition(63, 453.389,453.389);
   //this->renderer->GetActiveCamera()->SetFocalPoint(63, 99, 0);
   //this->renderer->GetActiveCamera()->SetViewUp(0, 0, 0);

   this->renderer->GetActiveCamera()->Modified();


   this->renWin->GetInteractor()->Delete();// Disable();
   this->iren->Delete();
   //this->iren = NULL;

   // Setup render window interactor
   vtkSmartPointer<vtkRenderWindowInteractor> PlotInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
   PlotInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
   vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
   PlotInteractor->SetInteractorStyle(style);

   // Render and start interaction
   PlotInteractor->SetRenderWindow(this->renWin);





   this->renWin->Render();
   PlotInteractor->Start();


}

void DisplayPlyFile::DisplayRawFansVtkImage2(boost::filesystem::path my_path)
{
    boost::iostreams::mapped_file_source file;
    uintmax_t total_length(0);
    uint32_t total_data_values(0);


    vtkSmartPointer<vtkImageReader2> ImageReader = MyVtk_Utilities::ReadBinaryFile(my_path);


    this->MapFileToMemory(my_path, file, total_length);
    total_data_values = total_length / 2; // using 2 bytes per datum

    auto buffer = file.data();

    std::string message;

    // create color image
    //vtkSmartPointer<vtkImageData> colorImage = vtkSmartPointer<vtkImageData>::New();

    //this->CreateColorImage(colorImage, total_data_values, total_length, buffer);

    vtkSmartPointer<vtkLookupTable> lutBW = MyVtk_Utilities::CreateBlackAndWhiteLUT();

    vtkSmartPointer<vtkImageMapToColors> imageMapper = vtkImageMapToColors::New();
    imageMapper->SetLookupTable(lutBW);
    //imageMapper->PassAlphaToOutputOn();
    // imageMapper->SetInputData(colorImage);
    imageMapper->SetInputConnection(ImageReader->GetOutputPort());

    // Create an image actor
    this->actor2 = vtkSmartPointer<vtkImageActor>::New();
    this->actor2->GetMapper()->SetInputConnection(imageMapper->GetOutputPort());
    //this->actor2->GetMapper()->SetInputConnection(ImageReader->GetOutputPort());
    //this->actor2->GetProperty()->SetInterpolationTypeToNearest();

    //imageActor->SetPosition(20, 20);

    // Setup renderers
    this->renderer2 = vtkSmartPointer<vtkRenderer>::New();
    this->renderer2->AddActor(this->actor2);
    this->renderer2->ResetCamera();

    // Setup render window
    this->renWin2 = vtkSmartPointer<vtkRenderWindow>::New();

    // add observer
    //vtkSmartPointer <MyMouseMovedCallback> mouseMovedCallback = vtkSmartPointer < MyMouseMovedCallback>::New();
    //this->renWin2->AddObserver(vtkCommand::  LeftButtonPressEvent, mouseMovedCallback);


    //mouseMovedCallback->Delete();

    this->renWin2->AddRenderer(this->renderer2);

    // Setup render window interactor
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
    renderWindowInteractor->SetInteractorStyle(style);

    // Create the mouse hover widget
    vtkSmartPointer<vtkHoverWidget> hoverWidget = vtkSmartPointer<vtkHoverWidget>::New();
    hoverWidget->SetInteractor(renderWindowInteractor);
    hoverWidget->SetTimerDuration(500);

    // Create a callback to listen to the widget's two VTK events
    vtkSmartPointer<myvtkHoverCallback> hoverCallback = vtkSmartPointer<myvtkHoverCallback>::New();
    hoverWidget->AddObserver(vtkCommand::TimerEvent, hoverCallback);
    hoverWidget->AddObserver(vtkCommand::EndInteractionEvent, hoverCallback);


    // create the scalar_bar


    // Render and start interaction
    renderWindowInteractor->SetRenderWindow(this->renWin2);

    //renderer->AddViewProp(imageActor);
    this->renderer2->AddActor2D(this->actor2);
    this->renderer2->AddActor2D(MyVtk_Utilities::CreateScalarBarWidget(lutBW));

    //this->renWin2->SetParentId(displayArea1);
    this->renWin2->Render();
    renderWindowInteractor->Initialize();
    this->renWin2->Render();
    hoverWidget->On();

    renderWindowInteractor->Start();

    SetWindowTextA(this->status, message.c_str());
    //MessageBox(NULL, (LPCSTR)file.data(), TEXT("Test"), MB_OK);
}

void DisplayPlyFile::restartRenderWindow(HWND parent)
{
	//this->mapper->ReleaseGraphicsResources(this->renWin);
	//this->filename = "C:\\Users\\Magdalena\\Source\\PCL\\3dcam\\trunk\\data\\501.0682.LogicalFansRawData\\501.0682.WAQ-slice-17.rawfans";//this->GetFileName(this->filename);
    //this->filename = "C:\\Users\\Magdalena\\Source\\PCL\\3dcam\\trunk\\data\\501.0682.LogicalFansRawData\\501.0682.WAQ-slice-3.rawfans";//this->GetFileName(this->filename);
    //this->filename = "C:\\Users\\Magdalena\\Source\\PCL\\3dcam\\trunk\\data\\9501.0678.WAQ-slice-4.rawfans";
    this->filename = this->GetFileName(this->filename);
    //"501.0682.WAQ-slice-17.rawfans"
    //this->filename = "C:\\Users\\Magdalena\\Source\\PCL\\3dcam\\trunk\\data\\501.0682.LogicalFansRawData\\501.0682.WAQ-cam1-3.txt";//this->GetFileName(this->filename);

    //if(!boost::filesystem::is_directory(this->filename))
    if (!boost::filesystem::exists(this->filename))
    {
        std::ostringstream stringStream;
        stringStream << "Error Opening file: " << this->filename;

        SetWindowTextA(this->status, (LPCSTR)stringStream.str().c_str());

       // DisplayStatusMessage(this->status, stringStream);
       
        return;
    }
	boost::filesystem::path my_path(this->filename);

	if (my_path.extension().string() == ".rawfans")
	{
        this->DisplayRawFans(my_path);
        //this->DisplayRawFansVtkImage2(my_path);
		return;
		//MessageBox(NULL, (LPCSTR)my_path.extension().string().c_str(), TEXT("Test"), MB_OK);
	}
    else if (my_path.extension().string() == ".txt")
    {
        this->DisplayTextBoundaryData(my_path);

        //DisplayLinePlot::DisplayLinePlot(HWND hwnd, std::vector<int> dataVect)
        return;
    }
    else 
    {
        // raw fans show in a new renderwindow
        // so this is only necessary if replacing another ply file
        this->renderer->RemoveAllViewProps();
        this->renWin->Render();
    }

	this->plyReader->SetFileName(this->filename.c_str());

	this->mapper->SetInputConnection(this->plyReader->GetOutputPort());

	actor->SetMapper(this->mapper);

	this->renderer->AddActor(this->actor);
	//this->renderer->SetBackground(0.1804, 0.5451, 0.3412); // Sea green
    this->renWin->SetParentId(mainWindow);
	this->renWin->Render();
}
// please use this only on etreme cases or for debuging
// cuz popups are not part of this company's GUI guidelines
void DisplayPlyFile::popup(const std::string message, HWND parent)
{
//	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)message.c_str()) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("Message: %s"),
		message.c_str());
	MessageBox(parent, (LPCTSTR)lpDisplayBuf, TEXT("output"), MB_OK);
}


void DisplayPlyFile::rotate()
{
	for (int i(0); i < 30; i++)
	{
		this->actor->SetOrigin(0, 0, 0);
		this->actor->RotateZ(1);
		this->renWin->Render();
	}
}

// class that diplays a 2d line plot. I use this to show
// the CCD camera sensor data and refresh it.
DisplayLinePlot::DisplayLinePlot(HWND parentHwnd, std::vector<unsigned short> data)
{
	this->datatable = vtkSmartPointer<vtkTable>::New();
	this->arrX = vtkSmartPointer<vtkIntArray>::New();  //vtkIntArray will automatically resize itself to hold new data.


	this->arrX->SetName("X Axis");
	this->datatable->AddColumn(this->arrX);

    vtkSmartPointer<vtkIntArray> arrXdata =
        vtkSmartPointer<vtkIntArray>::New();
    arrXdata->SetName("Pixel");
    this->datatable->AddColumn(arrXdata);

	this->arrY = vtkSmartPointer<vtkIntArray>::New();
	this->arrY->SetName("CCD Camera ADC value");
	this->datatable->AddColumn(this->arrY);

	this->datatable->SetNumberOfRows(data.size());

    int x(0);
    int inc(2);// inc(2 / (data.size() - 1));
	for (int y(0); y < data.size(); y++)
	{
        //if ((y  % 128 )==0 ) r+=2;
        {
            this->datatable->SetValue(y, 0, y);// y * inc);
            //table->SetValue(i, 1, cos(i * inc));
            //this->datatable->SetValue(y, 2, sin(y * inc));
            this->datatable->SetValue(y, 1,  data[y]);
        }
	}

	this->view = vtkSmartPointer<vtkContextView>::New();
	this->view->GetRenderWindow()->SetParentId(parentHwnd);

    this->view->GetRenderer()->SetBackground(1, 1, 1);

	this->chart = vtkSmartPointer<vtkChartXY>::New();
	this->view->GetScene()->AddItem(this->chart);

    vtkSmartPointer <vtkPlot> plot = this->chart->AddPlot(vtkChart::LINE);
    plot->SetInputData(this->datatable, 0, 1);
    plot->SetColor(0, 255, 0, 255);
    plot->SetWidth(1);
    plot->GetPen()->SetLineType(vtkPen::SOLID_LINE);//  DASH_LINE);

	this->view->GetInteractor()->Initialize();
	this->view->GetRenderWindow()->SetPosition(400, 20);
	this->view->GetRenderWindow()->SetSize(400, 400);


	this->view->GetRenderWindow()->Render();
	//	this->view->GetInteractor()->Start();
}


void DisplayLinePlot::SetPosition(int x, int y)
{
	this->view->GetRenderWindow()->SetPosition(x, y);

}
DisplayLinePlot::~DisplayLinePlot()
{
}

// generates a sphere with random number of points
DisplaySphereCloud::DisplaySphereCloud(HWND hwnd)
{
	this->pointSource = vtkSmartPointer<vtkPointSource>::New();
	this->pointSource->SetCenter(0.0, 0.0, 0.0);
	this->pointSource->SetNumberOfPoints(1000);
	this->pointSource->SetRadius(5.0);
	this->pointSource->SetDistributionToShell();
	this->pointSource->Update();

	// Create a mapper and actor
	this->mapper = vtkSmartPointer< vtkPolyDataMapper>::New();
	this->mapper->SetInputConnection(this->pointSource->GetOutputPort());
	this->actor = vtkSmartPointer<vtkActor>::New();
	this->actor->SetMapper(this->mapper);
	this->actor->GetProperty()->SetPointSize(2);


	// Create a renderer, render window, and interactor
	this->renderer = vtkSmartPointer <vtkRenderer>::New();
	this->renderer->AddActor(this->actor);
	this->renderer->SetBackground(0, 0, 0);// black
	//this->renderer->SetBackground(0.1804, 0.5451, 0.3412); // Sea green

	this->renWin = vtkSmartPointer <vtkRenderWindow>::New();
	this->renWin->SetParentId(hwnd);  // setup the parent window
	this->renWin->AddRenderer(this->renderer);

	this->iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	this->iren->SetRenderWindow(renWin);

	this->renWin->SetPosition(800,0);
	this->renWin->SetSize(400, 400);
	this->renWin->Render();
	//this->iren->Start();  // this never returns! don't start it
}
DisplaySphereCloud::~DisplaySphereCloud()
{
}

Display3DCircle::Display3DCircle(HWND hwnd)
{
	// Regular Polygon
	this->regularPolygonSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
	this->regularPolygonSource->SetCenter(4.0, 0.0, 0.0);
	this->regularPolygonSource->SetRadius(4.0);
	this->regularPolygonSource->SetNumberOfSides(10);

	// Create a mapper and actor
	this->mapper = vtkSmartPointer< vtkPolyDataMapper>::New();
	this->mapper->SetInputConnection(this->regularPolygonSource->GetOutputPort());
	this->actor = vtkSmartPointer<vtkActor>::New();
	this->actor->SetMapper(this->mapper);

	// Create a renderer, render window, and interactor
	this->renderer = vtkSmartPointer <vtkRenderer>::New();
	this->renderer->AddActor(this->actor);
	this->renderer->SetBackground(0, 0, 0);// black
	//this->renderer->SetBackground(0.1804, 0.5451, 0.3412); // Sea green

	this->renWin = vtkSmartPointer <vtkRenderWindow>::New();
	this->renWin->SetParentId(hwnd);  // setup the parent window
	this->renWin->AddRenderer(this->renderer);

	this->iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	this->iren->SetRenderWindow(renWin);

	this->renWin->SetPosition(800, 0);
	this->renWin->SetSize(400, 400);
	this->renWin->Render();

}

Display3DCircle::~Display3DCircle()
{
}
Display3DCube::Display3DCube(HWND hwnd)
{
	this->points = vtkSmartPointer <vtkPoints>::New();
	this->cellArray =  vtkSmartPointer <vtkCellArray>::New();
	this->structuredGrid = vtkSmartPointer <vtkStructuredGrid>::New();
	//this->points->SetDataTypeToFloat();
	double xval(0), yval(0), zval(0);
	double singlePoint[3],center[3],theta,  cosphi, sinphi, radius(0);
	vtkIdType vtkid(0);
	center[0] = 0;
	center[1] = 0;
	center[2] = 0;
	vtkIdType i(0);
	double rad(5);
	double NumberOfPoints(2000);
	this->points->SetNumberOfPoints(NumberOfPoints);
	vtkSmartPointer<vtkVoxel> voxel = vtkSmartPointer<vtkVoxel>::New();


	
	this->points->SetDataType(VTK_DOUBLE);

	for (i = 0; i < NumberOfPoints; i++)
	{
		cosphi = 1 - 2 * vtkMath::Random();
		sinphi = sqrt(1 - cosphi * cosphi);
		radius = rad * sinphi;
		theta = 2.0 * vtkMath::Pi() * vtkMath::Random();
		singlePoint[0] = center[0] + radius * cos(theta);
		singlePoint[1] = center[1] + radius * sin(theta);
		singlePoint[2] = center[2] + rad *cosphi;
		//voxel->GetPointIds()->SetId(i, i);
		//this->cellArray->InsertNextCell(1);
		//this->cellArray->InsertCellPoint(this->points->InsertNextPoint(singlePoint));
		this->points->InsertNextPoint(singlePoint);
		voxel->GetPointIds()->SetId(i, i);
	}
	//this->structuredGrid->

	//this->points->Modified();
	//this->cellArray->Modified();
	this->structuredGrid->SetPoints(this->points);
	this->structuredGrid->GetPointCells(voxel->GetCellType(),voxel->GetPointIds());
	//this->structuredGrid->
	//this->polydata = vtkSmartPointer<vtkPolyData>::New();
	//this->polydata->SetPoints(this->points);
	//this->polydata->SetPolys(voxel->);

	// Create a mapper and actor
	//this->mapper = vtkSmartPointer< vtkPolyDataMapper>::New();
	this->mapper = vtkSmartPointer<vtkDataSetMapper>::New();
	//this->mapper->SetInputData(this->polydata);
	this->mapper->SetInputData(this->structuredGrid);


	this->actor = vtkSmartPointer<vtkActor>::New();
	this->actor->SetMapper(this->mapper);
	this->actor->GetProperty()->SetPointSize(1);

	// Create a renderer, render window, and interactor
	this->renderer = vtkSmartPointer <vtkRenderer>::New();
	this->renderer->AddActor(this->actor);
	this->renderer->SetBackground(0, 0, 0);// black
	//this->renderer->SetBackground(0.1804, 0.5451, 0.3412); // Sea green

	this->renWin = vtkSmartPointer <vtkRenderWindow>::New();
	this->renWin->SetParentId(hwnd);  // setup the parent window
	this->renWin->AddRenderer(this->renderer);

	this->iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	this->iren->SetRenderWindow(renWin);

	this->renWin->SetPosition(800, 0);
	this->renWin->SetSize(400, 400);
	this->renWin->Render();
}

Display3DCube::~Display3DCube()
{
}

DisplayAssembly::DisplayAssembly(HWND hwnd)
{
	this->sphere = vtkSmartPointer <vtkSphereSource>::New();
	this->sphereMapper = vtkSmartPointer <vtkPolyDataMapper>::New();
	this->sphereMapper->SetInputConnection(this->sphere->GetOutputPort());
	this->sphereActor = vtkSmartPointer <vtkActor>::New();
	this->sphereActor->SetMapper(this->sphereMapper);
	this->sphereActor->SetOrigin(2, 1, 3);
	this->sphereActor->RotateY(6);
	this->sphereActor->SetPosition(2.25, 0, 0);
	this->sphereActor->GetProperty()->SetColor(1, 0, 0);
	this->sphereActor->GetProperty()->SetAmbient(0.5);
	this->sphereActor->GetProperty()->SetDiffuse(0.5);
	this->sphereActor->GetProperty()->SetSpecular(1.0);
	this->sphereActor->GetProperty()->SetSpecularPower(5.0);

	this->cube = vtkSmartPointer <vtkCubeSource>::New();
	this->cubeMapper = vtkSmartPointer <vtkPolyDataMapper>::New();
	this->cubeMapper->SetInputConnection(this->cube->GetOutputPort());
	this->cubeActor = vtkSmartPointer <vtkActor>::New();
	this->cubeActor->SetMapper(this->cubeMapper);
	this->cubeActor->SetPosition(0, .25, 0);
	this->cubeActor->GetProperty()->SetColor(0, 0, 1);

	this->cone = vtkSmartPointer <vtkConeSource>::New();
	this->coneMapper = vtkSmartPointer <vtkPolyDataMapper>::New();
	this->coneMapper->SetInputConnection(this->cone->GetOutputPort());
	this->coneActor = vtkSmartPointer <vtkActor>::New();
	this->coneActor->SetMapper(this->coneMapper);
	this->coneActor->SetPosition(0, 0, .25);
	this->coneActor->GetProperty()->SetColor(0, 1, 0);

	this->cylinder = vtkSmartPointer<vtkCylinderSource>::New();
	this->cylinderMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	this->cylinderActor = vtkSmartPointer<vtkActor>::New();
	this->cylinderMapper->SetInputConnection(this->cylinder->GetOutputPort());
	this->cylinderActor->SetMapper(this->cylinderMapper);
	this->cylinderActor->GetProperty()->SetColor(1, 0, 0);

	// add all the actors to one assembly
	this->assembly = vtkSmartPointer <vtkAssembly>::New();
	this->assembly->AddPart(this->sphereActor);
	this->assembly->AddPart(this->cubeActor);
	this->assembly->AddPart(this->coneActor);
	this->assembly->AddPart(this->cylinderActor);
	this->assembly->SetOrigin(5, 10, 15);
	this->assembly->AddPosition(5, 0, 0);
	this->assembly->RotateX(15);

	// Add they assembly to the render window
	//Create a renderer, render window, and interactor
	this->renderer = vtkSmartPointer <vtkRenderer>::New();
	this->renderer->AddActor(this->assembly);
	this->renderer->SetBackground(0, 0, 0);// black

	this->renWin = vtkSmartPointer <vtkRenderWindow>::New();
	this->renWin->SetParentId(hwnd);  // setup the parent window
	this->renWin->AddRenderer(this->renderer);

	this->iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	this->iren->SetRenderWindow(renWin);

	this->renWin->SetPosition(800, 0);
	this->renWin->SetSize(400, 400);
	this->renWin->Render();
}


DisplayAssembly::~DisplayAssembly()
{
}
void DisplayAssembly::rotate()
{
	//this->sphereActor->GetProperty()->SetColor(0, 0, 1);
	//this->assembly->RotateX(6);
	
	//this->sphereActor->GetProperty()->SetAmbient(0.5);
	//this->sphereActor->GetProperty()->SetDiffuse(0.5);
	//this->sphereActor->GetProperty()->SetSpecular(1.0);
	//this->sphereActor->GetProperty()->SetSpecularPower(5.0);


	double red(1), green(0), blue(0),spec(1);
	double redA(0), greenA(0), blueA(1);


	for (int i = 0; i < 100; i++)
	{
		blue += 0.01;
		red -= 0.01;

		blueA -= 0.01;
		greenA += 0.01;

		spec += .1;

		//this->sphereActor->GetProperty()->SetDiffuseColor(red,green,blue);
		//this->sphereActor->GetProperty()->SetAmbientColor(redA, greenA, blueA);
		//this->sphereActor->GetProperty()->SetSpecular(spec);	
		this->sphereActor->GetProperty()->SetSpecularPower(spec);
		//this->assembly->RotateX(6);
		this->sphereActor->SetOrigin(0, 0,0);
		this->sphereActor->RotateY(360/100);
		//this->sphereActor->SetPosition(0, 0, 5);

		this->renWin->Render();
		//Sleep(10);
		//boost::this_thread::sleep(boost::posix_time::milliseconds(10));

	}
	
	

}
// a structured grid dataset of a semicylinder. 
// Vectors are created whose magnitude is proportional
// to radius and oriented in tangential direction
DisplayStructuredGrid::DisplayStructuredGrid(HWND parent, HWND statusTxtLabel)
{
	// a structured grid dataset of a semicylinder. 
	// Vectors are created whose magnitude is proportional
	// to radius and oriented in tangential direction
	//static int dims[3] = { 1,2,2 };

	this->status = statusTxtLabel;

	// Create the structured grid.
	this->sgrid = vtkSmartPointer<vtkStructuredGrid>::New();
	this->points = vtkSmartPointer<vtkPoints>::New();

	// We also create the points and vectors. The points 
	// form a semi-cylinder of data, or a cylinder if dim[0] == 24 and angle == 15
	this->CreateData(); // populates this->sgrid with this->points and vectors

	// hedgehog is a vtkalgorythm. a filter.
	this->hedgehog = vtkSmartPointer<vtkHedgeHog>::New();
	this->hedgehog->SetInputData(sgrid);
	this->hedgehog->SetScaleFactor(0.99);
	//this->hedgehog->SetVectorModeToUseNormal();

	this->colors = vtkSmartPointer<vtkNamedColors>::New();
	this->sgridMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	//this->sgridMapper->SetInputConnection(hedgehog->GetOutputPort());
	//this->sgridMapper->SetInputConnection(this->sgrid);
	this->sgridActor = vtkSmartPointer<vtkActor>::New();
	this->sgridActor->SetMapper(sgridMapper);
	this->sgridActor->GetProperty()->SetColor(colors->GetColor3d("Peacock").GetData());

	// Create the usual rendering stuff
	this->renderer = vtkSmartPointer<vtkRenderer>::New();
	this->renWin = vtkSmartPointer<vtkRenderWindow>::New();
	this->renWin->SetParentId(parent);  // setup the parent window
	this->renWin->AddRenderer(renderer);

	this->iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	this->iren->SetRenderWindow(renWin);


	this->renderer->AddActor(sgridActor);
	this->renderer->SetBackground(colors->GetColor3d("Black").GetData());
	this->renderer->ResetCamera();
	this->renderer->GetActiveCamera()->Elevation(60.0);
	this->renderer->GetActiveCamera()->Azimuth(30.0);
	this->renderer->GetActiveCamera()->Dolly(1.25);
	this->renWin->SetPosition(800, 0);
	this->renWin->SetSize(400, 400);

	this->renWin->Render();
	//this->iren->Start();
}

// Create the points and vectors. The points 
// form a semi-cylinder of data.
void DisplayStructuredGrid::CreateData()
{
	int dims[3] = { 24,10,10 }; // dim[0] == 24 creates a circle
	int i, j, k, kOffset, jOffset, offset;
	double x[3], v[3], rMin = 0.5, rMax = 1.0, deltaRad, deltaZ;
	double radius, theta;

	this->sgrid->SetDimensions(dims);

	vtkSmartPointer<vtkDoubleArray> vectors =
		vtkSmartPointer<vtkDoubleArray>::New();
	vectors->SetNumberOfComponents(3);
	vectors->SetNumberOfTuples(dims[0] * dims[1] * dims[2]);
	this->points->Allocate(dims[0] * dims[1] * dims[2]);

	deltaZ = 2.0 / (dims[2] - 1);
	deltaRad = (rMax - rMin) / (dims[1] - 1);
	v[2] = 0.0;
	for (k = 0; k < dims[2]; k++)
	{
		x[2] = -1.0 + k * deltaZ;
		kOffset = k * dims[0] * dims[1];
		for (j = 0; j < dims[1]; j++)
		{
			radius = rMin + j * deltaRad;
			jOffset = j * dims[0];
			for (i = 0; i < dims[0]; i++)
			{
				theta = i * vtkMath::RadiansFromDegrees(15.0);
				x[0] = radius * cos(theta);
				x[1] = radius * sin(theta);
				v[0] = -x[1];
				v[1] = x[0];
				offset = i + jOffset + kOffset;
				this->points->InsertPoint(offset, x);
				vectors->InsertTuple(offset, v);
			}
		}
	}
	this->sgrid->SetPoints(points);
	this->sgrid->GetPointData()->SetVectors(vectors);

}


DisplayStructuredGrid::~DisplayStructuredGrid()
{

}

DisplayPointSource::DisplayPointSource(HWND parent, HWND statusTextWindow)
{

	// Create the usual rendering stuff
	this->renderer = vtkSmartPointer<vtkRenderer>::New();
	this->renWin = vtkSmartPointer<vtkRenderWindow>::New();

	// vtkCellArray is a supporting object that explicitly represents cell connectivity.
	// The cell array structure is a raw integer list of the form : (n, id1, id2, ..., 
	// idn, n, id1, id2, ..., idn, ...) where n is the number of points in the cell, 
	// and id is a zero - offset index into an associated point list.
	this->cells = vtkSmartPointer<vtkCellArray>::New();
	this->renWin->SetParentId(parent);  // setup the parent window
	this->renWin->AddRenderer(renderer);

	this->iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	this->iren->SetRenderWindow(renWin);


	this->renderer->AddActor(sgridActor);
	this->renderer->SetBackground(this->colors->GetColor3d("Black").GetData());
	this->renderer->ResetCamera();
	this->renderer->GetActiveCamera()->Elevation(60.0);
	this->renderer->GetActiveCamera()->Azimuth(30.0);
	this->renderer->GetActiveCamera()->Dolly(1.25);
	this->renWin->SetPosition(800, 0);
	this->renWin->SetSize(400, 400);

	this->renWin->Render();

}
DisplayPointSource::~DisplayPointSource()
{

}

void DisplayPointSource::CreateData()
{

}

// Unstructured grids require explicit point and cell
// representations, so every point and cell must be created, and then
// added to the vtkUnstructuredGrid instance.
DisplayVoxels::DisplayVoxels(HWND parent, HWND statusTextWindow)
{
	this->colors = vtkSmartPointer<vtkNamedColors>::New();

	this->points = vtkSmartPointer<vtkPoints>::New();
	this->points->InsertNextPoint(0, 0, 0);  //(x,y,z)
	this->points->InsertNextPoint(1, 0, 0);
	this->points->InsertNextPoint(0, 1, 0);
	this->points->InsertNextPoint(1, 1, 0);
	this->points->InsertNextPoint(0, 0, 1);
	this->points->InsertNextPoint(1, 0, 1);
	this->points->InsertNextPoint(0, 1, 1);
	this->points->InsertNextPoint(1, 1, 1);


	this->voxels = vtkSmartPointer<vtkVoxel>::New();

	for (int i = 0; i < 8; ++i)
	{
		this->voxels->GetPointIds()->SetId(i, i);
	}

	this->ugrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
	this->ugrid->SetPoints(this->points);
	this->ugrid->InsertNextCell(this->voxels->GetCellType(), this->voxels->GetPointIds());



	// Create a mapper and actor
	this->mapper = vtkSmartPointer< vtkDataSetMapper>::New();
	this->mapper->SetInputData(this->ugrid);


	this->sgridActor = vtkSmartPointer<vtkActor>::New();
	this->sgridActor->SetMapper(this->mapper);
	this->sgridActor->GetProperty()->SetColor(
		this->colors->GetColor3d("LightSteelBlue").GetData());


	// Create a renderer, render window, and interactor
	this->renderer = vtkSmartPointer <vtkRenderer>::New();
	this->renderer->AddActor(this->sgridActor);
	this->renderer->SetBackground(this->colors->GetColor3d("Black").GetData());// black
	//this->renderer->SetBackground(0.1804, 0.5451, 0.3412); // Sea green

	this->renWin = vtkSmartPointer <vtkRenderWindow>::New();
	this->renWin->SetParentId(parent);  // setup the parent window
	this->renWin->AddRenderer(this->renderer);

	this->iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	this->iren->SetRenderWindow(renWin);

	this->renWin->SetPosition(800, 0);
	this->renWin->SetSize(400, 400);
	this->renWin->Render();



}
DisplayVoxels::~DisplayVoxels()
{
}



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




