#pragma once
#ifndef SPI_MASTER_INCLUDE
#define SPI_MASTER_INCLUDE

#include <iostream>
#include <fstream>

#include "dln/common/dln4s_spi.h"


#include <vector>


#include "tbb\include\tbb\pipeline.h"
#include "tbb\include\tbb\tick_count.h"
#include "tbb\include\tbb\concurrent_vector.h"
#include "tbb\include\tbb\atomic.h"
#include "tbb\include\tbb\concurrent_queue.h"

// Redirects calls to "new" and "delete" to TBB thread safe allocators
#include "tbb\include\tbb\tbbmalloc_proxy.h"

// tbb redistribuitables here:
//C:\Program Files(x86)\IntelSWTools\compilers_and_libraries_2019\windows\redist\intel64_win\tbb\vc14


#define Num_Threads 4
#define pad 8  // assume 64byte L1 cache line size (to avoid false sharing)
#define DLN_FIRMWARE 1 // 0 = full version, 1 = SPI only version

//typedef tbb::concurrent_vector<int16_t[CHUNK_SIZE /2]> std_vec_packet_chunk_t;

const uint16_t  MAX_TOTAL_CHUNKS(4000);
const int MAX_CHUNK_SIZE = 128 * 512; // size of each memory 23a1024 is 128*512


const int CHUNK_SIZE = 128 * 255; // size of each memory 23a1024 is 128*512

typedef struct
{
	int8_t dataread[CHUNK_SIZE];
	//int8_t *dataread = new int8_t[CHUNK_SIZE];
} single_chunk;

typedef struct
{
	int16_t dataread[CHUNK_SIZE / 2];
	//int8_t *dataread = new int8_t[CHUNK_SIZE];
} oranized_single_chunk;


// TODO: make this a TBB pipeline class
namespace spi
{

	// DLN 4S_Spi stuff
	bool ConfigureSpi4SParameters(HANDLE &device4sHndl, uint32_t frequency, uint32_t spiMode);
	bool EnableSpi4sToUsbDevice(HANDLE &device4sHndl, bool printToScr = true);
	void WriteMemoryModeSPI(HANDLE &deviceHndl, bool printToScr = true);
	void ReadMemoryModeSPI(HANDLE &deviceHndl, bool printToScr = true);
	void WriteSpiMemory8SPI(HANDLE &device4sHndl, uint16_t count);
	void ReadSpiMemory8SPI(HANDLE &deviceHndl, uint16_t count, std::unique_ptr<uint8_t[]>& input_p, bool printToScr = true);
	void ReadSpiMemory8SPI(HANDLE &deviceHndl, uint16_t count, uint8_t                   * input_p, bool printToScr = true);


	// print to screen and to file utilities
	void PrintToScreenMemoryContent(std::unique_ptr<uint8_t[]>& input_p, CONSOLE_SCREEN_BUFFER_INFO csbi, HANDLE& std_out);
	void PrintToScreenMemoryContent(std::unique_ptr<char[]>& input_p, CONSOLE_SCREEN_BUFFER_INFO csbi);
	void SendDataChunkToFile(std::unique_ptr<uint8_t[]> &input_p, std::string datafileName);
	void grabDataFromFile(std::unique_ptr<uint8_t[]> &input_p, std::string datafileName);

	// executes the whole thing
	void AcquireTriangulationDataSPIOnlyFirmware(std::unique_ptr<uint8_t[]>& inputBuffer, bool printToScr = true);
	void AcquireTriangulationDataThread(std::unique_ptr<uint8_t[]>& inputBuffer, tbb::concurrent_queue<uint8_t[]>& chunk_ptr);
	void SimpleAcquire();
	void SimpleAcquire2();
	int16_t SetupSPI(HANDLE& device4sHndl);
	void AcquireAgain(HANDLE& device4sHndl, uint8_t* inputBuffer);
	void AcquireAgain(HANDLE& device4sHndl, std::unique_ptr<uint8_t[]>  inputBuffer);
	void AcquireAgain(HANDLE& device4sHndl);

	int16_t startTriangulationAcquisition(tbb::concurrent_queue<uint8_t*>& chunk_queue_ptr, tbb::atomic<bool>& stop_flag);




	// TBB pipeline
	class TriangulationInputFilter : public tbb::filter
	{
	public:
		TriangulationInputFilter(tbb::concurrent_queue<single_chunk>* chunk_ptr, tbb::atomic<bool>, HANDLE devHndl, uint8_t repeat);
		~TriangulationInputFilter();
	private:
		tbb::concurrent_queue<single_chunk>* local_chunk_ptr;
		tbb::atomic<bool>stp_;
		HANDLE localdevHndl;
		uint8_t repeat_;
		void* operator()(void*) /*override*/;
		uint8_t *inputa;
		uint8_t *inputb;
		uint8_t *outBuffer8a;
		uint8_t *outBuffer8b;
	};

}



#endif