#pragma once
#pragma pack(push,1)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DLN4S_SPI_EXPORTS
#define DLN4S_SPI_API __declspec(dllexport)
#else
#define DLN4S_SPI_API 
#endif

#include <windows.h>

DLN4S_SPI_API int Dln4sUpdateDeviceList(unsigned int *ids);
DLN4S_SPI_API HANDLE Dln4sOpenDevice();
DLN4S_SPI_API HANDLE Dln4sOpenDeviceById(unsigned int id);
DLN4S_SPI_API void Dln4sCloseDevice(HANDLE deviceHandle);
DLN4S_SPI_API void Dln4sCloseAllDevices();

DLN4S_SPI_API BOOL Dln4sSpiSetDeviceId(HANDLE deviceHandle, unsigned int id);
DLN4S_SPI_API BOOL Dln4sSpiGetDeviceId(HANDLE deviceHandle, unsigned int *id);
DLN4S_SPI_API BOOL Dln4sSpiGetDeviceFwVer(HANDLE deviceHandle, unsigned int *ver);
DLN4S_SPI_API BOOL Dln4sSpiConfigure(HANDLE deviceHandle, unsigned int frequency, unsigned int spiMode);
DLN4S_SPI_API BOOL Dln4sSpiTransfer(HANDLE deviceHandle, unsigned char *writeBuffer, unsigned char *readBuffer, size_t length);

typedef void (*Dln4sSpiCallback)(void *pArg, BOOL status, size_t transferred, size_t totalLength);
DLN4S_SPI_API BOOL Dln4sSpiTransferAsync(HANDLE deviceHandle, unsigned char *writeBuffer, unsigned char *readBuffer, size_t length, Dln4sSpiCallback callback, void *pArgument);

#ifdef __cplusplus
}
#endif

#pragma pack(pop)
