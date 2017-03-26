#include "CMpgRead.h"
#include "CWavRW.h"

CMpgRead::CMpgRead(const char* inFilePath, const uint32_t blockLen)
{
    int errorHolder = MPG123_OK;

    noError = true;
    isInitialized = false;
    currentMessage = "\nAudio decoding messages:\n";
    this->blockLen = blockLen;
    mpgHandle = NULL;
    normFact = 1.0 / ((double) 0x8000);
    bufferSize = 0;
    nChans = 0;

    errorHolder = mpg123_init();

    if(errorHolder!=MPG123_OK)
    {
        currentMessage.append("Couldn't initialize mpg123 instance.\n");
        currentMessage.append(mpg123_plain_strerror(errorHolder));
        noError = false;
    }

    mpgHandle = mpg123_new(NULL, &errorHolder);

    if(errorHolder!=MPG123_OK && noError)
    {
        currentMessage.append("Couldn't create new mpg123 handle.\n");
        currentMessage.append(mpg123_plain_strerror(errorHolder));
        noError = false;
    }

    errorHolder = mpg123_open(mpgHandle, inFilePath);

    if(errorHolder!=MPG123_OK && noError)
    {
        currentMessage.append("Error when try to open the data stream.\n");
        currentMessage.append(mpg123_strerror(mpgHandle));
        noError = false;
    }

    errorHolder = mpg123_getformat(mpgHandle, &samplingRate, &nChans, &encodingID);

    if(errorHolder!=MPG123_OK && noError)
    {
        currentMessage.append("Error when try to get the standart decode format.\n");
        currentMessage.append(mpg123_strerror(mpgHandle));
        noError = false;
    }

    if (noError)
    {
        bufferSize = blockLen*nChans*sizeof(int16_t);
        outBuf = new int16_t[blockLen*nChans]();
        nSamps = mpg123_length(mpgHandle);
        isInitialized = true;

        currentMessage.append("Decoder initialized.\nCurrent sampling rate: ");
        currentMessage.append(std::to_string(samplingRate));
        currentMessage.append("\nNumber of channels: ");
        currentMessage.append(std::to_string(nChans));
    }
}

int CMpgRead::decodeWholeFile(const char *outFilePath)
{
    if (noError)
    {
        double outBufDouble[blockLen*nChans];
        int errorHolder = MPG123_OK;
        size_t nBytesDecoded;

        CWavWrite wavOutput(outFilePath, blockLen, samplingRate, nChans, 16);

        while (errorHolder==MPG123_OK)
        {
            errorHolder = mpg123_read(mpgHandle, (unsigned char*) outBuf, bufferSize, &nBytesDecoded);
            fixedToFloat64(outBuf, outBufDouble, blockLen*nChans);
            wavOutput.writeOneBlock(outBufDouble);
        }

        return 0;
    }

    return -1;
}

int CMpgRead::readOneBlock(double *outBufDouble)
{
    size_t nBytesDecoded;
    int errorHolder;

    errorHolder = mpg123_read(mpgHandle, (unsigned char*) outBuf, bufferSize, &nBytesDecoded);

    if (errorHolder == MPG123_OK)
    {
        fixedToFloat64(outBuf, outBufDouble, blockLen*nChans);
    }
    else if (errorHolder == MPG123_DONE)
    {
        emit isEOF();
    }

    return nBytesDecoded;
}

CMpgRead::~CMpgRead()
{
    if (isInitialized)
    {
        delete[] outBuf;
    }

    mpg123_close(mpgHandle);
    mpg123_delete(mpgHandle);
    mpg123_exit();
}

