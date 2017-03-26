//comments go here

#include <iostream>
#include <map>
#include <math.h>
#include "CRtAudio.h"
#include "CWavRW.h"

CRtAudio::CRtAudio(bool inFlag, bool outFlag, Api RtAudioApi, unsigned int blockLen, unsigned int nBuffers,
                   uint32_t fs, uint16_t nChans, uint16_t firstChan, bool minLatencyFlag)
    : RtAudio(RtAudioApi)
{
    this->blockLen = blockLen;
    this->fs = fs;
    this->nChans = nChans;
    this->firstChan = firstChan;
    this->bytesPerFrame = blockLen*nChans*sizeof(double);
    this->bufferLenInBytes = bytesPerFrame*nBuffers;
    this->nBuffers = nBuffers;
    this->bufferCnt = 0;

    if (blockLen*nChans*nBuffers > 5*fs)
    {
        nBuffers = 5*fs/blockLen/nChans;
    }

    if (outFlag && inFlag)
    {
        this->RtMessage = "\nSoundcard throughput successfully initialized";
    }
    else if (outFlag)
    {
        firstDataPtr = new double[blockLen*nChans*nBuffers]();
        currentDataPtr = firstDataPtr;
        this->RtMessage = "\nSoundcard output successfully initialized";
    }
    else if (inFlag)
    {
        firstDataPtr = new double[blockLen*nChans*nBuffers]();
        currentDataPtr = firstDataPtr;
        this->RtMessage = "\nSoundcard input successfully initialized";
    }
    else
        this->RtMessage = "\nOutput and input flag are false - Invalid Class init.";

    StreamParameters inStreamParams, outStreamParams;

    if (inFlag)
    {
        inStreamParams.deviceId = this->getDefaultInputDevice();
        inStreamParams.nChannels = nChans;
        inStreamParams.firstChannel = firstChan;
    }

    if (outFlag)
    {
        outStreamParams.deviceId = this->getDefaultOutputDevice();
        outStreamParams.nChannels = nChans;
        outStreamParams.firstChannel = firstChan;
    }

    StreamOptions streamSettings;

    if (minLatencyFlag) streamSettings.flags = RTAUDIO_MINIMIZE_LATENCY;

    if (this->getDeviceCount() < 1) this->RtMessage = "\nNo audio devices found!";

    try
    {
        if (outFlag && inFlag)
            this->openStream(&outStreamParams, &inStreamParams, RTAUDIO_FLOAT64, fs, &blockLen, &this->inOut, this, &streamSettings);
        else if (outFlag)
            this->openStream(&outStreamParams, NULL, RTAUDIO_FLOAT64, fs, &blockLen, &this->Out, this, &streamSettings);
        else if (inFlag)
            this->openStream(NULL, &inStreamParams, RTAUDIO_FLOAT64, fs, &blockLen, &this->In, this, &streamSettings);
    }
    catch (RtAudioError &err)
    {
        this->RtMessage = err.getMessage();
    }
}

int CRtAudio::In(void * /*outputBuffer*/, void *inputBuffer, unsigned int /*bufferSize*/,
           double /*streamTime*/, RtAudioStreamStatus status, void *data)
{
    CRtAudio *classPtr = (CRtAudio*) data;

    double *inBuf = (double*) inputBuffer;
    double *tmpBuf = (double*) classPtr->getCurrentDataPtr();

    memcpy(tmpBuf, inBuf, classPtr->getBytesPerFrame());

    emit classPtr->audioInDone(tmpBuf);

    if (classPtr->getBufferCnt() < classPtr->getLastBufferNumber())
    {
        classPtr->addBufferCnt();
        classPtr->setCurrentDataPtr(tmpBuf + classPtr->getFrameSize());
    }
    else
    {
        classPtr->resetBufferCnt();
        classPtr->setCurrentDataPtr(classPtr->getFirstDataPtr());
    }

    return status;
}

int CRtAudio::Out(void *outputBuffer, void */*inputBuffer*/, unsigned int /*bufferSize*/,
         double /*streamTime*/, RtAudioStreamStatus status, void *data)
{
    CRtAudio *classPtr = (CRtAudio*) data;

    double *outBuf = (double*) outputBuffer;
    double *tmpBuf = (double*) classPtr->getCurrentDataPtr();

    memcpy(outBuf, tmpBuf, classPtr->getBytesPerFrame());

    emit classPtr->audioOutDone(tmpBuf);

    if (classPtr->getBufferCnt() < classPtr->getLastBufferNumber())
    {
        classPtr->addBufferCnt();
        classPtr->setCurrentDataPtr(tmpBuf + classPtr->getFrameSize());
    }
    else
    {
        classPtr->resetBufferCnt();
        classPtr->setCurrentDataPtr(classPtr->getFirstDataPtr());
    }

    return status;
}

int CRtAudio::inOut( void *outputBuffer, void *inputBuffer, unsigned int /*bufferSize*/,
           double /*streamTime*/, RtAudioStreamStatus status, void *data)
{
    if (status>0) std::cout << "Stream over/underflow detected." << std::endl;

    CRtAudio *classPtr = (CRtAudio*) data;
    double *inBuf = (double*) inputBuffer;
    double *outBuf = (double*) outputBuffer;

    memcpy(outBuf, inBuf, classPtr->getBytesPerFrame());

    classPtr->setCurrentDataPtr(outBuf);

    return 0;
}

std::string CRtAudio::deviceInfo(RtAudio::Api RtAudioApi)
{
    std::string outString;

    std::map<int, std::string> apiMap;
    apiMap[RtAudio::MACOSX_CORE] = "OS-X Core Audio";
    apiMap[RtAudio::WINDOWS_ASIO] = "Windows ASIO";
    apiMap[RtAudio::WINDOWS_DS] = "Windows Direct Sound";
    apiMap[RtAudio::WINDOWS_WASAPI] = "Windows WASAPI";
    apiMap[RtAudio::UNIX_JACK] = "Jack Client";
    apiMap[RtAudio::LINUX_ALSA] = "Linux ALSA";
    apiMap[RtAudio::LINUX_PULSE] = "Linux PulseAudio";
    apiMap[RtAudio::LINUX_OSS] = "Linux OSS";
    apiMap[RtAudio::RTAUDIO_DUMMY] = "RtAudio Dummy";

    std::vector< RtAudio::Api > apis;
    RtAudio :: getCompiledApi( apis );

    outString.append("\nRtAudio Version ");
    outString.append(RtAudio::getVersion());

    outString.append("\n\nCompiled APIs:");
    for ( unsigned int i=0; i<apis.size(); i++ )
    {
        outString.append(apiMap[apis[i]]);
        outString.append("  ");
    }

    RtAudio audio(RtAudioApi);
    RtAudio::DeviceInfo info;

    outString.append("\n\nCurrent API: ");
    outString.append(apiMap[audio.getCurrentApi()]);

    unsigned int devices = audio.getDeviceCount();
    outString.append("\n\nFound ");
    outString.append(std::to_string(devices));
    outString.append(" device(s) ...\n");

    for (unsigned int i=0; i<devices; i++)
    {
        info = audio.getDeviceInfo(i);

        outString.append("\nDevice Name = ");
        outString.append(info.name);

        if ( info.probed == false )
        {
            outString.append("\nProbe Status = UNsuccessful\n");
        }
        else
        {
            outString.append("\nProbe Status = Successful");
            outString.append("\nOutput Channels = ");
            outString.append(std::to_string(info.outputChannels));
            outString.append("\nInput Channels = ");
            outString.append(std::to_string(info.inputChannels));
            outString.append("\nDuplex Channels = ");
            outString.append(std::to_string(info.duplexChannels));

            if ( info.isDefaultOutput ) outString.append("\nThis is the default output device.");
            else outString.append("\nThis is NOT the default output device.");
            if ( info.isDefaultInput ) outString.append("\nThis is the default input device.");
            else outString.append("\nThis is NOT the default input device.");

            if ( info.nativeFormats == 0 )
            {
                outString.append("No natively supported data formats(?)!");
            }
            else
            {
                outString.append("\nNatively supported data formats:");
                if ( info.nativeFormats & RTAUDIO_SINT8 )
                  outString.append(" 8Bit int ");
                if ( info.nativeFormats & RTAUDIO_SINT16 )
                  outString.append(" 16Bit int ");
                if ( info.nativeFormats & RTAUDIO_SINT24 )
                  outString.append(" 24Bit int ");
                if ( info.nativeFormats & RTAUDIO_SINT32 )
                  outString.append(" 32Bit int ");
                if ( info.nativeFormats & RTAUDIO_FLOAT32 )
                  outString.append(" 32Bit float ");
                if ( info.nativeFormats & RTAUDIO_FLOAT64 )
                  outString.append(" 64Bit float ");
            }

            if ( info.sampleRates.size() < 1 )
            {
                outString.append("No supported sample rates found!");
            }
            else
            {
                outString.append("Supported sample rates: ");
                for (unsigned int j=0; j<info.sampleRates.size(); j++)
                {
                    outString.append(std::to_string(info.sampleRates[j]));
                    outString.append("Hz  ");
                }
            }
        }
    }

    return outString;
}

CRtAudio::~CRtAudio()
{
    delete[] this->firstDataPtr;
}
