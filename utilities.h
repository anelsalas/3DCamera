#pragma once
#ifndef COMMON_ANGLES_IMPL_H_
#define COMMON_ANGLES_IMPL_H_

	//inline float
	//	normAngle(float alpha)
	//{
	//	return (alpha >= 0 ?
	//		fmodf(alpha + static_cast<float>(M_PI),
	//			2.0f * static_cast<float>(M_PI))
	//		- static_cast<float>(M_PI)
	//		:
	//		-(fmodf(static_cast<float>(M_PI) - alpha,
	//			2.0f * static_cast<float>(M_PI))
	//			- static_cast<float>(M_PI)));
	//}

	inline float
		rad2deg(float alpha)
	{
		return (alpha * 57.29578f);
	}

	inline float
		deg2rad(float alpha)
	{
		return (alpha * 0.017453293f);
	}

	inline double
		rad2deg(double alpha)
	{
		return (alpha * 57.29578);
	}

	inline double
		deg2rad(double alpha)
	{
		return (alpha * 0.017453293);
	}

#endif  // COMMON_ANGLES_IMPL_H_

#ifndef COMMON_UTILITIES_H_
#define COMMON_UTILITIES_H_
#include <iostream>
#include "Windows.h"
#include <strsafe.h>
#include <sstream>
#include "myvtk_utilities.h"



    void DisplayStatusMessage(HWND status, std::ostringstream stringStream);
    void DisplayCameraParameters(HWND status,vtkSmartPointer<vtkRenderer>renderer);
    void DisplayStatusMessage(HWND status, std::string msg);




#endif

