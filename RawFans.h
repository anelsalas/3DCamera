#pragma once

#ifndef RAW_FANS_INCLUDE 
#define RAW_FANS_INCLUDE

class RawFans
{
public:
	void ConvertToLogicalFans(unsigned short *outputBuffer, int fanCount);
	int Size();
	void SampleFactor(int index, int fanCount);
	RawFans();
	virtual ~RawFans();
	unsigned short *buffer;
	int m_AdcSampleCount[16];
	double m_AdcMultiplier[16];
	double m_AdcScaling[16];
	int m_LogicalDwellToPhysicalOffset[16];

	static const int ByteCount[];
	static const int LogicalDwellSampleCount[16];
	static const long StandardDwellAcqOrder[16];
	static const int OversamplingFactor[];					// WH changed from protected to public for the CheckForStuckBit Function

protected:

	static const int nActiveDwells;
	int m_size;
	int m_bufferSize;
};

class Chunk
{
public:
	Chunk(int wordCount);
	~Chunk();
	unsigned short* buf;
};

#endif 

