
#include "myvtk_utilities.h"

namespace MyVtk_Utilities
{
    // Map the scalar values in the image to colors with a lookup table:
    vtkSmartPointer<vtkLookupTable> CreateBlackAndWhiteLUT()
    {
        // Map the scalar values in the image to colors with a lookup table:
        vtkSmartPointer<vtkLookupTable> lutBW = vtkSmartPointer<vtkLookupTable>::New();
        lutBW->SetHueRange(0.0, 0.0); // image intensity range 
        lutBW->SetSaturationRange(0.0, 0.0); // no color saturation 
        lutBW->SetValueRange(0, 1); // from black to white (0,1), white to black (1,0)
       //lutBW->
        //lutBW->SetRampToLinear(); // this makes no difference that i can notice
        lutBW->SetRange(0, 15000);
        lutBW->Build();
        return lutBW;
    }

    vtkSmartPointer<vtkLookupTable> CreateRainbowLUT()
    {
        // Map the scalar values in the image to colors with a lookup table:
        vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
        lut->SetHueRange(0.6667,0); // image intensity range 
        lut->SetSaturationRange(1.0, 1.0); 
        lut->SetValueRange(0, 1); 
        lut->SetAlphaRange(0, 1.0);
        lut->SetNumberOfColors(254);
        lut->SetRange(6000, 10500);
       //lutBW->
        //lutBW->SetRampToLinear(); // this makes no difference that i can notice
        lut->Build();
        return lut;
    }

    vtkSmartPointer<vtkScalarBarActor> CreateScalarBarWidget(vtkSmartPointer<vtkLookupTable> lut)
    {
        vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
        scalarBar->SetOrientationToVertical();
        //scalarBar->SetOrientationToHorizontal();
        //scalarBar->SetDisplayPosition(50, 0);
        //scalarBar->DragableOn();
        //scalarBar->SetHeight(10);
        //scalarBar->SetWidth(40);
        scalarBar->SetLookupTable(lut);
        return scalarBar;

        //# create the scalar_bar_widget
        //scalar_bar_widget = vtk.vtkScalarBarWidget()
        //scalar_bar_widget.SetInteractor(renderWindowInteractor)
        //scalar_bar_widget.SetScalarBarActor(scalar_bar)
        //scalar_bar_widget.On()
    }

    vtkSmartPointer<vtkImageReader2> ReadBinaryFile(boost::filesystem::path my_path)
    {

        readBinaryData(my_path);
        vtkSmartPointer<vtkImageReader2> reader =
                vtkSmartPointer<vtkImageReader2>::New();
        //If the data is stored in multiple files, then use SetFileNames or SetFilePrefix instead.
        reader->SetFileName(my_path.string().c_str());
        //reader->SetFilePrefix(my_path.string().c_str());
        //reader->SetDataExtent(0, 63, 0, 63, 0, 0);// produces a 64 x 64 2Dimage

        reader->SetDataExtent(0,127, 0, 0, 0, 0); // 200 boundary points each with 128 pixels
        reader->SetDataSpacing(0, 0,0);
        reader->SetDataOrigin(0, 0, 0.0);
        reader->SetDataScalarTypeToUnsignedShort();
        reader->SetDataByteOrderToLittleEndian();
       //reader->SetDataByteOrderToBigEndian();
        reader->UpdateWholeExtent();
        return reader;
    }
    void readBinaryData(boost::filesystem::path my_path)
    {

            std::ifstream input(my_path.string().c_str(), ios::in | std::ios::binary);
            short iSample;
            std::vector<unsigned short> buffer;// (std::istreambuf_iterator<char>(input), {});

            while (input.read((char *)&iSample, sizeof(short)))
            {
                //cout << iSample << " ";
                buffer.push_back(iSample);
            }
            std::string st0;




            // copies all data into buffer
            //input.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(unsigned short));


      }
    std::vector<unsigned short> ReadTextFile(boost::filesystem::path my_path)
    {
        std::vector<unsigned short> result;
        ifstream infile(my_path.c_str());
        std::string                cell;
        char *end;
        while (std::getline(infile, cell, ','))
        {
            unsigned long i = std::strtoul(cell.c_str(), &end, 10);
            //data = i & 0x0FFFF;
            result.push_back(i & 0x0FFFF);
        }
        return result;

    }

}