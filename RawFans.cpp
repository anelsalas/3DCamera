// March, 2019, brought this class from CTLM 1.0 code 
// for compatibility

//#include "stdafx.h"
//#include "winacq.h"
#include "RawFans.h"
#include <boost/assert.hpp>
#include <stdlib.h>     /* calloc, exit, free */

const int  RawFans::ByteCount[] = { 1284 * 2, 1797 * 2, 3849 * 2, 6072 * 2 };
const int  RawFans::OversamplingFactor[] = { 1, 4, 16, 29 };
const int  RawFans::LogicalDwellSampleCount[] = { 171, 171, 171, 171, 171, 171, 256 };

#ifdef CAMERADWELLATEND

#pragma message("      NOTE:  Compiling for Camera integration at end of fan!")
#pragma message("             RawFans::StandardDwellAcqOrder[] = {0,1,2,3,4,5,6);")

const long RawFans::StandardDwellAcqOrder[] = { 0,1,2,3,4,5,6 };

#else
const long RawFans::StandardDwellAcqOrder[] = { 0,1,2,6,3,4,5 };
#endif

const int RawFans::nActiveDwells = 7;

typedef unsigned short BUFFERTYPE;

RawFans::RawFans()
{
	m_size = 0;
	m_bufferSize = 0;
	buffer = 0;
	for (int i = 0; i < sizeof(m_AdcSampleCount) / sizeof(m_AdcSampleCount[0]); ++i) 
	{
		m_AdcSampleCount[i] = 1;
		m_AdcMultiplier[i] = 4.0;
		m_AdcScaling[i] = 0.25;		// Reconstruction scales fanlets by this factor
									// to achieve 14-bit ADC-equivelent signal magnitudes.
	}
}

RawFans::~RawFans()
{
	delete[] buffer;
}

void RawFans::SampleFactor(int oversamplingIndex, int fanCount)
{
	BOOST_ASSERT((oversamplingIndex >= 0) &&
		(oversamplingIndex < (sizeof(ByteCount) / sizeof(ByteCount[0]))));
	int newBufferSize = ByteCount[oversamplingIndex] / 2 * fanCount;
	if (newBufferSize > m_bufferSize) {	// The buffer only grows...
		delete[] buffer;
		buffer = new BUFFERTYPE[newBufferSize];
		m_bufferSize = newBufferSize;
	}
	m_size = ByteCount[oversamplingIndex] / sizeof(BUFFERTYPE);
	m_AdcSampleCount[5] = OversamplingFactor[oversamplingIndex];
	m_AdcMultiplier[5] = 4.0 / OversamplingFactor[oversamplingIndex];

	int offset = 2;	// Skip over the fan header.

	for (int physicalDwell = 0; physicalDwell < nActiveDwells; ++physicalDwell) {
		int logicalDwell = StandardDwellAcqOrder[physicalDwell];
		m_LogicalDwellToPhysicalOffset[logicalDwell] = offset;
		offset += LogicalDwellSampleCount[logicalDwell] * m_AdcSampleCount[logicalDwell];
	}
}

int RawFans::Size()
{
	return m_size;
}

#pragma optimize( "gt", on)

void RawFans::ConvertToLogicalFans(unsigned short *dest, int fanCount)
{
//#if 0
//#include "Util.h"
//#pragma message("     Included dump of raw fans to Dat/rawfans.data!")
//	DumpToFile("rawfans.data", (void *)buffer, fanCount * m_size * 2);
//#endif

	RawFans rawf;
	//rawf.SampleFactor(CWinacqApp::OversampleFactorIndex, 0); // Defer buffer allocation until number of fans in a chunk is known 
	int RawFanSize = rawf.Size();				// But, we need the size now.


	int outi = 0;

	for (int fan = 0; fan < fanCount; ++fan) 
	{

		int fani = m_size * fan;

		dest[outi++] = buffer[fani + 0];	// Copy the header
		dest[outi++] = buffer[fani + 1];

		for (int dwell = 0; dwell < nActiveDwells; ++dwell) 
		{
			double multiplier = m_AdcMultiplier[dwell];
			int overSampleFactor = m_AdcSampleCount[dwell];
			int sampleCount = LogicalDwellSampleCount[dwell];
			int ini = fani + m_LogicalDwellToPhysicalOffset[dwell];

			if (overSampleFactor == 1) 
			{

				for (; sampleCount--; ) 
				{
					dest[outi++] = (unsigned short)(buffer[ini++] * multiplier);
				}

			}
			else 
			{
				register long acc;
				register BUFFERTYPE *in;

				for (; sampleCount--; ) 
				{

					acc = 0;
					in = buffer + ini;
					switch (overSampleFactor) 
					{
					case 32:
						acc += *in++; acc += *in++;
						acc += *in++;
					case 29:
						acc += *in++;
						acc += *in++; acc += *in++;
						acc += *in++; acc += *in++;
						acc += *in++; acc += *in++;
						acc += *in++; acc += *in++;
						acc += *in++; acc += *in++;
						acc += *in++; acc += *in++;
					case 16:
						acc += *in++; acc += *in++;
						acc += *in++; acc += *in++;
						acc += *in++; acc += *in++;
						acc += *in++; acc += *in++;
					case 8:
						acc += *in++; acc += *in++;
						acc += *in++; acc += *in++;
					case 4:
						acc += *in++; acc += *in++;
					case 2:
						acc += *in++; acc += *in++;
						ini += overSampleFactor;
						break;
					default:
						for (int sample = overSampleFactor; sample--; ) 
						{
							acc += buffer[ini++];
						}
						break;
					}
					dest[outi++] = (unsigned short)(acc * multiplier);
				}
			}
		}
		//
		// Patch up the CCD cameras to minimize the effects of the off-by-one
		// error in RLCA079C. The first pixel of the 1st camera is missing. The
		// sample which is supposed to be the last pixel of the first camera contains
		// the first sample of the second camera, and the sample which is supposed to 
		// contain the last pixel of the second camera contains a unidentified value.
		// Here, we just eliminate the inter-camera crosstalk and the discontinuity
		// at the end, and let reconstruction's calibration take care of the 1-pixel
		// shift.
		//
		dest[outi - 1] = dest[outi - 2];  // Eliminate discontinuity.
		dest[outi - 129] = dest[outi - 130]; // Eliminate inter-camera crosstalk
	}
}


Chunk::Chunk(int wordCount)
{
	buf = (unsigned short *)calloc(wordCount, sizeof(unsigned short));
	BOOST_ASSERT(buf);
}

Chunk::~Chunk()
{
	if (buf) free(buf);
}
