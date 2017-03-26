//comments go here

#ifndef RTFCN_H
#define RTFCN_H

#include <QThread>
#include <QTimer>
#include <vector>
#include <stdint.h>
#include "RtAudio.h"

class CRtAudio : public QThread, public RtAudio
{
    Q_OBJECT

public:
    CRtAudio(bool inFlag, bool outFlag, Api RtAudioApi, unsigned int blockLen, unsigned int nBuffers, uint32_t fs,
             uint16_t nChans, uint16_t firstChan = 0, bool minLatencyFlag = false);
    ~CRtAudio();

    inline unsigned int getBlockLen() {return this->blockLen;}
    inline unsigned int getBufferCnt() {return this->bufferCnt;}
    inline unsigned int getFrameSize() {return this->blockLen*this->nChans;}
    inline unsigned int getSamplingRate() {return this->fs;}
    inline unsigned int getNumChans() {return this->nChans;}
    inline unsigned int getfirstChan() {return this->firstChan;}
    inline unsigned int getBytesPerFrame() {return this->bytesPerFrame;}
    inline unsigned int getLastBufferNumber() {return this->nBuffers-1;}
    inline double* getCurrentDataPtr() {return this->currentDataPtr;}
    inline double* getFirstDataPtr() {return this->firstDataPtr;}
    inline void setCurrentDataPtr(double *newDataPtr) {this->currentDataPtr = newDataPtr;}
    inline void addBufferCnt() {bufferCnt++;}
    inline void resetBufferCnt() {bufferCnt = 0;}
    inline std::string getCurrentMessage() {return RtMessage;}
    static std::string deviceInfo(Api RtAudioApi);
    inline void resetBuffer()
    {
        for(unsigned int kk = 0; kk < blockLen*nChans*nBuffers; kk++)
        {
            firstDataPtr[kk] = 0;
        }
    }

protected:

    static int In( void */*outputBuffer*/, void *inputBuffer, unsigned int /*bufferSize*/,
               double /*streamTime*/, RtAudioStreamStatus /*status*/, void *data);

    static int Out(void *outputBuffer, void */*inputBuffer*/, unsigned int /*bufferSize*/,
             double streamTime, RtAudioStreamStatus status, void *data);

    static int inOut( void *outputBuffer, void *inputBuffer, unsigned int /*bufferSize*/,
               double /*streamTime*/, RtAudioStreamStatus status, void *data);

signals:
    void audioInDone(double* tmpBuf);
    void audioOutDone(double* tmpBuf);

private:
    RtAudio *dac;
    std::string RtMessage;
    double *currentDataPtr, *firstDataPtr;
    unsigned int fs, blockLen, nChans, firstChan, bytesPerFrame, bufferLenInBytes, nBuffers, bufferCnt;
};

#endif // end of include guard
