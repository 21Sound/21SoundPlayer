#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QThread>
#include <fstream>
#include <iostream>
#include <stdint.h>

extern "C" {
#include "fmt123.h"
#include "mpg123.h"
#include "out123.h"
}

using namespace std;

class CMpgRead : public QThread
{
    Q_OBJECT

public:
    CMpgRead(const char* inFilePath, const uint32_t blockLen);

    int decodeWholeFile(const char *outFilePath);

    inline bool isInstanceInitialized(){return isInitialized;}
    inline uint32_t getSamplingRate() {return samplingRate;}
    inline uint32_t getBlockSize() {return blockLen;}
    inline uint16_t getNrOfChans() {return nChans;}
    inline std::string getCurrentMessage() {return currentMessage;}

    inline double getNormalizedPosition()
    {
        int currentPosition;
        currentPosition = mpg123_tell(mpgHandle);
        return ((double) currentPosition/nSamps);
    }

    inline int setNormalizedPosition(double normalizedPosition)
    {
        int samplePos;
        double tmpPos;
        if ((normalizedPosition < 0) || (normalizedPosition > 1)) return -1;
        tmpPos = (double) normalizedPosition*nSamps/nChans;
        samplePos = ((uint32_t)tmpPos)*nChans;
        mpg123_seek(mpgHandle, samplePos, SEEK_SET);
        return 0;
    }

    inline void fixedToFloat64(int16_t *inData, double *outData, int blockLen)
    {
        for (int kk = 0; kk < blockLen; kk++)
            outData[kk] = (double) inData[kk] * normFact;
    }

    ~CMpgRead();

public slots:
    int readOneBlock(double *outBufDouble);

signals:
    void isEOF(void);

private:

    std::string currentMessage;
    mpg123_handle *mpgHandle;
    bool noError, isInitialized;
    uint32_t blockLen, nSamps;
    long samplingRate;
    unsigned int bufferSize;
    int encodingID, nChans;
    int16_t *outBuf;
    double normFact;
};

#endif // AUDIODECODER_H
