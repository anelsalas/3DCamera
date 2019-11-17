#include "utilities.h"

void DisplayStatusMessage(HWND status, std::ostringstream stringStream)
{
    SetWindowTextA(status, (LPCSTR)stringStream.str().c_str());
}

void DisplayStatusMessage(HWND status, std::string msg)
{
    std::ostringstream ss;
    
    ss << msg;
    SetWindowTextA(status, (LPCSTR)ss.str().c_str());
}
void DisplayCameraParameters(HWND status,  vtkSmartPointer<vtkRenderer>renderer)
{
    double pos[3];
    double FP[3];
    double UpView[3];


    renderer->GetActiveCamera()->GetPosition(pos[0], pos[2], pos[2]);
    renderer->GetActiveCamera()->GetFocalPoint(FP[0], FP[1], FP[2]);
    renderer->GetActiveCamera()->GetViewUp(UpView[0], UpView[1], UpView[2]);

    std::ostringstream stringStream;
    stringStream << "Pos: " << pos[0] << ", " << pos[2] << ", " << pos[2]
        << ", Focal Point: " << FP[0] << ", " << FP[1] << ", " << FP[2]
        << ", View Up: " <<UpView[0] << ", " << UpView[0] << ", " << UpView[0];


    SetWindowTextA(status, (LPCSTR)stringStream.str().c_str());
}