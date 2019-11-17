// spi_master.cpp : Defines the entry point for the console application.
// Using TBB 2019.0.117
// Using DLN
//#include <stdafx.h>
using namespace std;
#include "spi_master.h"
#include <iomanip>
uint8_t g_lastMemRead;


///////////////////  DLN includes ////////////////////////////////////
// #pragma comment(lib, libname) tells the linker to add the 'libname' 
//library to the list of library dependencies, as if you had added it in the 
//project properties at Linker->Input->Additional dependencies
//#pragma comment(lib, "dln\\bin\\dln.lib")
//#pragma comment(lib, "dln/bin/dln4s_spi.lib")
//#pragma comment(lib,"C:/Users/Magdalena/Source/PCL/3dcam/trunk/dln/bin/dln4s_spi.lib")


//! Parameter ntokens controls maximum number of concurrently processed elements.
//!  It has value 4 because the project was tested on 4 - core machine and making it bigger wouldn't make an effect.
const size_t NTOKENS(4);
const size_t TOTALPIXELS(128);
const size_t MAX_CHUNKS_IN_SCAN(5000);

int16_t spi::SetupSPI(HANDLE& device4sHndl)
{
	const uint32_t frequency(8000000);
	const uint32_t spiMode(0);

	if (spi::EnableSpi4sToUsbDevice(device4sHndl, false) != true) return(1);
	if (spi::ConfigureSpi4SParameters(device4sHndl, frequency, spiMode) != true) return(2);
	return 0;
}

//! Thread safe data acquisition. It grabs CCD camera data in chunks and
//! drops the chunks into a thread safe queue that can be poped 
//! by another thread.
int16_t spi::startTriangulationAcquisition(tbb::concurrent_queue<uint8_t*>& chunk_queue_ptr, tbb::atomic<bool>& stop_flag)
{
	HANDLE device4sHndl(0);
	const uint32_t frequency(8000000);
	const uint32_t spiMode(0);
	uint8_t memorybank(0xff);
	uint8_t* inputBuffer = (uint8_t*)(tbb::tbb_allocator<uint8_t>().allocate(CHUNK_SIZE + 1));

	if (spi::SetupSPI(device4sHndl) != 0) return 1;
	chunk_queue_ptr.empty(); // start with a clean queue

	inputBuffer[5] = 0;
	while (stop_flag == false) //! run till we tell it to stop
	{
		spi::ReadSpiMemory8SPI(device4sHndl, 6, inputBuffer);
		if (memorybank != inputBuffer[5])
		{
			spi::ReadSpiMemory8SPI(device4sHndl, CHUNK_SIZE, inputBuffer);
			memorybank = inputBuffer[5];
			chunk_queue_ptr.push(inputBuffer);
		}
		//if (chunk_queue_ptr.unsafe_size() == MAX_CHUNKS_IN_SCAN) chunk_queue_ptr.empty();
	}
	if (stop_flag)
	{
		std::cout << "\nTHREAD MSG: stop flag pressed\n";
	}
	Dln4sCloseDevice(device4sHndl);
	tbb::tbb_allocator<uint8_t>().deallocate(inputBuffer, CHUNK_SIZE + 1);

	return 0;
}

void spi::AcquireAgain(HANDLE& device4sHndl, uint8_t* inputBuffer)
{
	uint8_t memorybank = inputBuffer[5];
	while (memorybank == inputBuffer[5])
	{
		spi::ReadSpiMemory8SPI(device4sHndl, 6, inputBuffer);
		memorybank = inputBuffer[5];
	}
	spi::ReadSpiMemory8SPI(device4sHndl, CHUNK_SIZE, inputBuffer);
}

void spi::AcquireAgain(HANDLE& device4sHndl, std::unique_ptr<uint8_t[]>  inputBuffer)
{
	uint8_t memorybank = inputBuffer[5];
	while (memorybank == inputBuffer[5])
	{
		spi::ReadSpiMemory8SPI(device4sHndl, 6, inputBuffer);
		memorybank = inputBuffer[5];
	}
	spi::ReadSpiMemory8SPI(device4sHndl, CHUNK_SIZE, inputBuffer);
}

void spi::AcquireAgain(HANDLE& device4sHndl)
{
	uint8_t memorybank(0);
	std::unique_ptr<uint8_t[]> inputBuffer(std::make_unique<uint8_t[]>(CHUNK_SIZE + 1));
	spi::ReadSpiMemory8SPI(device4sHndl, 6, inputBuffer);


	while (memorybank == inputBuffer[5])
	{
		spi::ReadSpiMemory8SPI(device4sHndl, 6, inputBuffer);
		memorybank = inputBuffer[5];
	}
	// new pointer for new data
	inputBuffer = std::make_unique<uint8_t[]>(CHUNK_SIZE + 1);
	spi::ReadSpiMemory8SPI(device4sHndl, CHUNK_SIZE, inputBuffer);
}





//! Filter that changes each uint8 to uint16 and changes order
void spi::SimpleAcquire()
{
	// open the spi device
	HANDLE device4sHndl(0);
	const uint32_t frequency(8000000);
	const uint32_t spiMode(0);

	uint8_t* inputBuffer;
	inputBuffer = (uint8_t*)(tbb::tbb_allocator<uint8_t>().allocate(MAX_CHUNK_SIZE));
	inputBuffer[0] = 5;

	if (spi::EnableSpi4sToUsbDevice(device4sHndl, false) != true) exit(0);
	if (spi::ConfigureSpi4SParameters(device4sHndl, frequency, spiMode) != true) exit(0);
	spi::ReadSpiMemory8SPI(device4sHndl, CHUNK_SIZE, inputBuffer);

	Dln4sCloseDevice(device4sHndl);
	tbb::tbb_allocator<uint8_t>().deallocate(inputBuffer, MAX_CHUNK_SIZE);
}

void spi::SimpleAcquire2()
{

	// open the spi device
	HANDLE device4sHndl(0);
	const uint32_t frequency(8000000);
	const uint32_t spiMode(0);

	std::unique_ptr<uint8_t[]> inputBuffer(std::make_unique<uint8_t[]>(CHUNK_SIZE + 1));

	if (spi::EnableSpi4sToUsbDevice(device4sHndl, false) != true) exit(0);
	if (spi::ConfigureSpi4SParameters(device4sHndl, frequency, spiMode) != true) exit(0);
	spi::ReadSpiMemory8SPI(device4sHndl, CHUNK_SIZE, inputBuffer);

	Dln4sCloseDevice(device4sHndl);

}











static inline uint32_t rotl32(uint32_t n, unsigned int c)
{
	const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);  // assumes width is a power of 2.

														   // assert ( (c<=mask) &&"rotate by type width or more");
	c &= mask;
	return (n << c) | (n >> ((-c)&mask));
}

static inline uint32_t rotr32(uint32_t n, unsigned int c)
{
	const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);

	// assert ( (c<=mask) &&"rotate by type width or more");
	c &= mask;
	return (n >> c) | (n << ((-c)&mask));
}


//! SPI namespace only uses DLN drivers and the SPI_4s dln firmware
void spi::AcquireTriangulationDataSPIOnlyFirmware(std::unique_ptr<uint8_t[]>& inputBuffer, bool printToScr)
{

	tbb::atomic<bool> stop_flag(false);

	// Open boundary camera1 device   
	const uint8_t devicePort(0);
	uint16_t diopinNum(26);
	uint8_t diopinDir(1); // output
	uint8_t dlnLedCount(0);
	const uint32_t value(1);
	uint32_t dlnSerialNumber(0), id(0);
	std::string inputval;
	const uint16_t CameraReadyPin(2); // virtual pin 2 is real pin 19 on the chip
	uint16_t conflict(0);

	// we changed the firmware on the DLN hardware and now the maximum speed is 8MHz. 
	// mario thinks this is due to the spi isolation chips on the DLN boards
	const uint32_t frequency(8000000);
	const uint32_t spiMode(0);

	uint32_t actualFrecuency(0);
	uint16_t localResponse(0);
	const uint8_t clockPolarity(0), clockPhase(0);
	uint8_t actualPol(0), actualPhase(0);
	uint32_t actualDelayBetweenFrames(0);
	const uint32_t delayBetweenFrames(0);
	HANDLE device4sHndl(0);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);

	// get the cursor position so that I can re-write data over the previous data shunk
	// without deleting the configuration information. this helps debuging
	GetConsoleScreenBufferInfo(std_out, &csbi);
	csbi.dwCursorPosition.X = 0;
	csbi.dwCursorPosition.Y = 0;

	if (EnableSpi4sToUsbDevice(device4sHndl, printToScr) != true) exit(0);
	if (ConfigureSpi4SParameters(device4sHndl, frequency, spiMode) != true) exit(0);

	//DLN_FIRMWARE
	WriteMemoryModeSPI(device4sHndl, printToScr);
	ReadMemoryModeSPI(device4sHndl, printToScr);
	//	WriteSpiMemory8SPI(device4sHndl, 50);
	if (printToScr)std::cout << "Reading the memory.\n";

	int counter(1);
	std::string shit;
	//std::string datafilename = "datafile";
	ReadSpiMemory8SPI(device4sHndl, CHUNK_SIZE, inputBuffer, printToScr);

	//std::cout << "read what I just wrote. Press any key to continue....\n";
	if (printToScr) PrintToScreenMemoryContent(inputBuffer, csbi, std_out);

	//std::cin >> shit;

	//while (counter-- > 0) goto again;
	Dln4sCloseDevice(device4sHndl);
	//SendDataChunkToFile(inputBuffer, datafilename);
	//std::unique_ptr<uint8_t[]> inputData(std::make_unique<uint8_t[]>(CHUNK_SIZE + 1));
	//grabDataFromFile(inputData, datafilename);
	//PrintToScreenMemoryContent(inputData, csbi);
}


// SPI namespace only uses DLN drivers and the SPI_4s dln firmware
void spi::AcquireTriangulationDataThread(std::unique_ptr<uint8_t[]>& inputBuffer, tbb::concurrent_queue<uint8_t[]>& chunk_ptr)
{

	tbb::atomic<bool> stop_flag(false);

	// Open boundary camera1 device   
	const uint8_t devicePort(0);
	uint16_t diopinNum(26);
	uint8_t diopinDir(1); // output
	uint8_t dlnLedCount(0);
	const uint32_t value(1);
	uint32_t dlnSerialNumber(0), id(0);
	std::string inputval;
	const uint16_t CameraReadyPin(2); // virtual pin 2 is real pin 19 on the chip
	uint16_t conflict(0);

	// we changed the firmware on the DLN hardware and now the maximum speed is 8MHz. 
	// mario thinks this is due to the spi isolation chips on the DLN boards
	const uint32_t frequency(8000000);
	const uint32_t spiMode(0);

	uint32_t actualFrecuency(0);
	uint16_t localResponse(0);
	const uint8_t clockPolarity(0), clockPhase(0);
	uint8_t actualPol(0), actualPhase(0);
	uint32_t actualDelayBetweenFrames(0);
	const uint32_t delayBetweenFrames(0);
	HANDLE device4sHndl(0);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);

	// get the cursor position so that I can re-write data over the previous data shunk
	// without deleting the configuration information. this helps debuging
	GetConsoleScreenBufferInfo(std_out, &csbi);
	csbi.dwCursorPosition.X = 0;
	csbi.dwCursorPosition.Y = 0;

	if (EnableSpi4sToUsbDevice(device4sHndl) != true) exit(0);
	if (ConfigureSpi4SParameters(device4sHndl, frequency, spiMode) != true) exit(0);

	//DLN_FIRMWARE
	WriteMemoryModeSPI(device4sHndl);
	ReadMemoryModeSPI(device4sHndl);
	std::cout << "Starting thrad to read spi bus.\n";
	uint8_t memorybank(0);

	// grab first memory location
	ReadSpiMemory8SPI(device4sHndl, 6, inputBuffer);
	memorybank = inputBuffer[5];

	while (!stop_flag)
	{
		while (memorybank == inputBuffer[5])
		{
			ReadSpiMemory8SPI(device4sHndl, 6, inputBuffer);
		}
		ReadSpiMemory8SPI(device4sHndl, CHUNK_SIZE, inputBuffer);
		//chunk_ptr.push(inputBuffer);

	}



	//std::cin >> shit;

	//while (counter-- > 0) goto again;
	Dln4sCloseDevice(device4sHndl);
	//SendDataChunkToFile(inputBuffer, datafilename);
	//std::unique_ptr<uint8_t[]> inputData(std::make_unique<uint8_t[]>(CHUNK_SIZE + 1));
	//grabDataFromFile(inputData, datafilename);
	//PrintToScreenMemoryContent(inputData, csbi);
}

void spi::PrintToScreenMemoryContent(std::unique_ptr<uint8_t[]> &input_p, CONSOLE_SCREEN_BUFFER_INFO csbi, HANDLE& std_out)
{
	//HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleCursorPosition(std_out, csbi.dwCursorPosition);


	for (int i(0); i < 7; i++, i++)
	{
		printf("%0.2X%0.2X,", input_p[i], input_p[i + 1]);
		//std::cout << std::hex << +input_p[i] << +input_p[i+1] << ",";
	}
	//printf("\n");
	std::cout << "\n";


	for (int i(6), m(0); i < 1029; i++, i++, m++)
	{
		if ((i - 6) % 256 == 0)
		{
			//printf("\n\n");
			std::cout << "\n\n";
		}

		//printf("%0.2X%0.2X,", input_p[i], input_p[i + 1]);
		std::cout << std::hex << +input_p[i] << +input_p[i + 1] << ",";
	}
}

void spi::PrintToScreenMemoryContent(std::unique_ptr<char[]> &input_p, CONSOLE_SCREEN_BUFFER_INFO csbi)
{
	HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(std_out, csbi.dwCursorPosition);


	for (int i(0); i < 7; i++, i++)
	{
		printf("%0.2X%0.2X,", input_p[i], input_p[i + 1]);
		//std::cout << std::hex << +input_p[i] << +input_p[i+1] << "|,";
	}
	printf("\n");

	for (int i(6), m(0); i < 1029; i++, i++, m++)
	{
		if ((i - 6) % 256 == 0)
		{
			printf("\n\n");
		}

		printf("%0.2X%0.2X,", input_p[i], input_p[i + 1]);
		//std::cout << std::hex << +input_p[i] << +input_p[i+1] << "|,";
	}
	return;

}

void spi::SendDataChunkToFile(std::unique_ptr<uint8_t[]> &input_p, std::string datafileName)
{
	LPDWORD NumberOfCharsWritten(0);
	LPVOID  lpReserved(0);
	//	CONSOLE_SCREEN_BUFFER_INFO csbi;
	//	COORD  startingCoord; // x = col, y = line
	//	char buffer[CHUNK_SIZE + 1];
	HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);
	//	DWORD  cWritten;
	int coordIndex(0), xval(10);
	//GetConsoleScreenBufferInfo(std_out, &csbi);
	std::string filename = datafileName + ".txt";
	std::string filenameBin = datafileName + ".bin";

	std::ofstream ostrm(filename, std::ios::binary);
	std::ofstream ostrmBin(filenameBin, std::ios::binary);
	//ostrm << "jesus" << std::hex << std::setw(4) << std::setfill('0') << "\n\n" << std::to_string(input_p[10]);

	for (int i = 0; i < CHUNK_SIZE; i += 2)
	{
		//sprintf(buffer, "%0.2d%0.2d, ", input_p[i + 1], input_p[i]);
		//if (i % 128 == 0) sprintf(buffer, "\n");

		//ostrmBin << std::hex << input_p[i] << 8 | input_p[i + 1];
		ostrmBin << std::hex << input_p[i] << input_p[i + 1];
		ostrm << std::hex << std::to_string(input_p[i] << 8 | input_p[i + 1]) << ",";
	}
	ostrm << "\0";
	ostrmBin << "\0";

	ostrm.flush();
	ostrmBin.close();
	ostrm.flush();
	ostrmBin.close();


	//FILE *fp;
	//fp = fopen("test.txt", "wb");
	//fwrite(buffer, sizeof(char), CHUNK_SIZE, fp);

	//std::string filename = "Test.txt";
	//std::ofstream ostrm(filename, std::ios::binary);

	//SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), p);
	//WriteConsole();

	//for (int i = 2; i < 1024; i++,i++)
	//{
	//	if (i % 128 == 0)
	//	{
	//		printf("\n\n");
	//	}
	//	printf("%0.2X%0.2X|",input_p[i+1],input_p[i]);
	//	//std::cout << std::hex << +input_p[i] << +input_p[i+1] << "|,";
	//}
	//std::cout << "=================================================================\n";
	//std::cout << "=================================================================\n";
	//for (int i(0), m(-2); i < 1024; i++, i++, m += 2)
	//{
	//	if (i % 128 == 0)
	//	{
	//		printf("\n\n");
	//	}

	//	printf("%0.2X%0.2X|", input_p[i], input_p[i + 1]);
	////	//std::cout << std::hex << +input_p[i] << +input_p[i+1] << "|,";
	//}



}

void spi::grabDataFromFile(std::unique_ptr<uint8_t[]> &input_p, std::string datafileName)
{

	std::string filename = datafileName + ".bin";
	std::ifstream is(filename, std::ifstream::binary);
	if (!is)
	{
		std::cout << "\n Error opening file to read\n";
	}

	// get length of file:
	is.seekg(0, is.end);
	int length = is.tellg();
	is.seekg(0, is.beg);

	std::cout << "\nReading " << std::dec << length << " characters... \n";
	// read data as a block:
	is.read((char*)input_p.get(), length);

	if (is)
		std::cout << "all characters read successfully.\n";
	else
		std::cout << "error: only " << is.gcount() << " could be read \n";
	is.close();

	// ...buffer contains the entire file...


}

bool spi::ConfigureSpi4SParameters(HANDLE &device4sHndl, uint32_t frequency, uint32_t spiMode)

{
	bool response(true);
	response = Dln4sSpiConfigure(device4sHndl, frequency, spiMode);
	return response;
}

bool spi::EnableSpi4sToUsbDevice(HANDLE &device4sHndl, bool printToScr)
{
	unsigned int ver, id;
	unsigned int ids[10];
	int ideess(0);

	ver = 0;
	id = 0;
	ids[0] = 0xff;
	ids[1] = 0xff;

	bool response(true);
	ideess = Dln4sUpdateDeviceList(ids);

	device4sHndl = Dln4sOpenDeviceById(ids[0]);
	response = Dln4sSpiGetDeviceFwVer(device4sHndl, &ver);
	response = Dln4sSpiGetDeviceId(device4sHndl, &id);

	if (printToScr) std::cout << "\nfirmware version: " << (int)ver << ", device id:" << (int)id << ", handle: " << device4sHndl << "\n";

	return response;
}

void spi::WriteMemoryModeSPI(HANDLE &deviceHndl, bool printToScr)
{
	const uint8_t port(0), count(2);
	BOOL resval;


	unsigned char outBuffer[count + 1];
	unsigned char input[count + 1];

	for (uint8_t i = 0; i < count; i++)
	{
		outBuffer[i] = 0;// value;
		input[i] = 0;

	}
	outBuffer[0] = 0x01;
	outBuffer[1] = 0x40;

	//DLN4S_SPI_API BOOL Dln4sSpiTransfer(deviceHandle, unsigned char *writeBuffer, unsigned char *readBuffer, size_t length);

	resval = Dln4sSpiTransfer(deviceHndl, outBuffer, input, count);
	if (printToScr) std::cout << "Wrote this command: " << std::hex << +outBuffer[0] << std::hex << +outBuffer[1] << "\n";

	if (resval == false)
	{
		std::cout << "Error in WriteMemoryModeSPI: Dln4sSpiTransfer";
	}
}

void spi::ReadMemoryModeSPI(HANDLE &deviceHndl, bool printToScr)
{
	const uint8_t port(0), count(20);
	uint8_t outBuffer[count + 1];
	uint8_t input[count + 1];

	for (uint8_t i = 0; i < count; i++)
	{
		outBuffer[i] = 0;
		input[i] = 0;

	}
	outBuffer[0] = 0x05;

	bool resval = Dln4sSpiTransfer(deviceHndl, outBuffer, input, count);
	if (resval == FALSE)
	{

		std::cout << "ReadMemoryModeSPI: Dln4sSpiTransfer.\n";
	}
	if (printToScr)std::cout << "Command:" << +outBuffer[0] << +outBuffer[1] << ", Mode: " << +input[0] << +input[1] << "\n";


}

void spi::WriteSpiMemory8SPI(HANDLE &deviceHndl, uint16_t count)
{
	const uint8_t port(0);
	bool  resval;

	uint8_t outBuffer[CHUNK_SIZE];
	uint8_t input[CHUNK_SIZE];

	for (uint8_t i = 0; i < count; i++)
	{
		outBuffer[i] = 0xff;// value;
		input[i] = 0;

	}
	outBuffer[0] = 02;
	outBuffer[1] = 00;
	outBuffer[2] = 00;
	outBuffer[3] = 00;

	outBuffer[4] = 0XFE;
	outBuffer[5] = 00;
	outBuffer[6] = 00;

	std::cout << "WriteSpiMemory8====Wrote the following============================\n";

	for (int arrpos(3), num(0); arrpos < count; arrpos++, num++)
	{

		std::cout << std::hex << +outBuffer[arrpos] << "|,";
	}
	std::cout << "================================================================\n";

	resval = Dln4sSpiTransfer(deviceHndl, outBuffer, input, count);

	if (resval == false)
	{
		std::cout << "Dln4sSpiTransfer";
	}

}

//! Overloaded ReadSpiMemory8SPI to take the input buffer as uint8_t pointer. The 
//! pointer will be allocated and deallocated within the thread that runs the acqusition
//! of the triangulation data.
void spi::ReadSpiMemory8SPI(HANDLE &deviceHndl, uint16_t count, uint8_t* input_p, bool printToScr)
{
	const uint8_t port(0);
	BOOL resval;

	std::unique_ptr<uint8_t[]> outBuffer(std::make_unique<uint8_t[]>(count + 1));

	for (int i = 4; i < count; i++)
	{
		outBuffer[i] = 0;
		//input[i] = 0;
	}
	outBuffer[0] = 0x03;
	outBuffer[1] = 0;
	outBuffer[2] = 0;
	outBuffer[3] = 0;

	resval = Dln4sSpiTransfer(deviceHndl, outBuffer.get(), input_p, count);
	if (resval == false)
	{
		cout << "\nDln4sSpiTransfer error.\n";
	}

}


void spi::ReadSpiMemory8SPI(HANDLE &deviceHndl, uint16_t count, std::unique_ptr<uint8_t[]>& input_p, bool printToScr)
{
	const uint8_t port(0);
	BOOL resval;

	std::unique_ptr<uint8_t[]> outBuffer(std::make_unique<uint8_t[]>(count + 1));

	for (int i = 4; i < count; i++)
	{
		outBuffer[i] = 0;
		//input[i] = 0;
	}
	outBuffer[0] = 0x03;
	outBuffer[1] = 0;
	outBuffer[2] = 0;
	outBuffer[3] = 0;

	resval = Dln4sSpiTransfer(deviceHndl, outBuffer.get(), input_p.get(), count);
	if (resval == false)
	{
		cout << "\nDln4sSpiTransfer error.\n";
	}
}


