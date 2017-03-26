//wavReadWrite

#ifndef WAV_RW_H
#define WAV_RW_H

#include <QThread>
#include <fstream>
#include <stdio.h>
#include <stdint.h>
#include <iostream>
using namespace std;

class CWavRead : public QThread
{
    Q_OBJECT

public:

    CWavRead(const char *filePath, int blockLen);

    inline bool isInstanceInitialized() {return isInitialized;}
    inline uint32_t getSamplingRate() {return fs;}
    inline uint32_t getBlockSize() {return blockLen;}
    inline uint16_t getNrOfChans() {return nChans;}
    inline uint32_t getBytesPerSecond() {return bytesPerSecond;}
    inline uint16_t getNrBytesPerBlock() {return bytesPerBlock;}
    inline uint32_t getNrBytesPerBlockForDouble() {return bytesPerBlockForDouble;}
    inline uint32_t getSamplesPerFrame() {return nSamplesPerFrame;}
    inline uint16_t getBitDepth() {return nBitsPerSample;}
    inline uint32_t getOverallLen() {return nAudioBytes;}
    inline double getNormalizedPosition(){return ((double) byteCnt/nAudioBytes);}
    inline std::string getCurrentMessage(){return currentMessage;}
    inline int setNormalizedPosition(double normalizedPosition)
    {
        double tmpPos;
        if ((normalizedPosition < 0) || (normalizedPosition > 1)) return -1;
        tmpPos = (double) normalizedPosition*(nAudioBytes-bytesPerBlock)/frameSize;
        byteCnt = ((uint32_t)tmpPos)*frameSize+beginPos;
        inFile->seekg(byteCnt, inFile->beg);
        return 0;
    }

~CWavRead();

public slots:
    void readOneBlock(double* outData);

protected:

	inline void fixedToFloat64(int16_t *inData, double *outData, int blockLen)
	{
		for (int kk = 0; kk < blockLen; kk++)
			outData[kk] = (double) inData[kk] * normFact;
    }

	inline void fixedToFloat64(int32_t *inData, double *outData, int blockLen)
	{
		for (int kk = 0; kk < blockLen; kk++)
			outData[kk] = (double) inData[kk] * normFact;
    }

signals:
    void isEOF(void);

private:

	ifstream *inFile;
	uint32_t blockLen;
	uint32_t nSamplesPerFrame;
	uint16_t nChans;
	uint32_t fs;
	uint32_t bytesPerBlock;
    uint32_t bytesPerBlockForDouble;
	uint32_t bytesPerSecond;
	uint16_t frameSize;
	uint16_t bytesPerSample;
	uint16_t nBitsPerSample;
	uint32_t nAudioBytes;
	uint32_t byteCnt;
    uint32_t beginPos;
    int32_t *tmpAudio32;
    int16_t *tmpAudio16;
	double normFact;
    bool isInitialized, noError;
    std::string currentMessage;
};

class CWavWrite : public QThread
{
    Q_OBJECT

public:

    CWavWrite(const char *filePath, int blockSize,
              int fs, int nChans, int bitDepth);

	~CWavWrite();

    inline bool isInstanceInitialized() {return isInitialized;}
    inline std::string getCurrentMessage(){return currentMessage;}

public slots:
    void writeOneBlock(double* inData);

protected:

	inline void float64ToFixed(double *inData, int16_t *outData, int blockLen)
	{
		for (int kk = 0; kk < blockLen; kk++)
			outData[kk] = inData[kk] * normFact;
    }

	inline void float64ToFixed(double *inData, int32_t *outData, int blockLen)
	{
		for (int kk = 0; kk < blockLen; kk++)
			outData[kk] = inData[kk] * normFact;
    }

private:

	ofstream *outFile;
	uint32_t blockLen;
	uint32_t nSamplesPerFrame;
	uint16_t nChans;
	uint32_t fs;
	uint32_t bytesPerBlock;
	uint16_t frameSize;
	uint16_t nBitsPerSample;
	uint32_t byteCnt;
	int32_t *tmpAudio32;
	int16_t *tmpAudio16;
	double normFact;
    bool isInitialized, noError;
    std::string currentMessage;
};

#endif
