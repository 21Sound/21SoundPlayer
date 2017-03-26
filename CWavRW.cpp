//comments go here

#include "CWavRW.h"
#include <string.h>

CWavRead::CWavRead(const char *filePath, int blockSize)
{
	char readBuffer[5] = {0};
	string str1, str2;
	int32_t fileIdx = 12; //length of RIFF-Header
	int32_t dataLen;
    int32_t tmp, tmpLen = 0;
    currentMessage = "\nInitializing wavRead class instance:\n";
	this->blockLen = blockSize;
    noError = true;
    isInitialized = false;

	inFile = new ifstream(filePath, ios::in | ios::binary);
    if (inFile->is_open())
    {
        inFile->seekg(0, inFile->end);
        tmpLen = inFile->tellg();
        inFile->seekg(0, inFile->beg);
    }

    if (tmpLen > 0xFFFF)
    {
        inFile->read(readBuffer, 4);

        if (strcmp(readBuffer,"RIFF")!=0)
        {
            currentMessage.append("RIFF-Header not found, no .wav-File?\n");
            noError = false;
        }

        inFile->read((char*) &dataLen, 4);

        inFile->read(readBuffer, 4);

        if (strcmp(readBuffer,"WAVE")!=0)
        {
            currentMessage.append("WAVE-Header not found, no .wav-File?\n");
            noError = false;
        }

        while ((strcmp(readBuffer, " tmf")!=0) && (fileIdx < 0xFFF))
        {
            fileIdx++;
            readBuffer[3] = readBuffer[2];
            readBuffer[2] = readBuffer[1];
            readBuffer[1] = readBuffer[0];
            inFile->read(&readBuffer[0], 1);
        }

        if (fileIdx >= 0xFFF)
        {
            currentMessage.append("FMT-Header not found. Damaged .wav-File?\n");
            noError = false;
        }

        inFile->read((char*) &tmp, 4);

        if (tmp < 16)
        {
            currentMessage.append("format section too short, file may be damaged.\n");
            noError = false;
        }

        if (tmp > 16)
        {
            currentMessage.append("format section too long, invalid WAVE file format.\n");
            noError = false;
        }

        inFile->read((char*) &tmp, 2);

        if (tmp != 1)
        {
            currentMessage.append("this is no linear PCM WAVE-file.\n");
            noError = false;
        }

        inFile->read((char*) &nChans, 2);

        inFile->read((char*) &fs, 4);

        inFile->read((char*) &bytesPerSecond, 4);

        inFile->read((char*) &frameSize, 2);

        inFile->read((char*) &nBitsPerSample, 2);

        fileIdx += 20;

        while ((strcmp(readBuffer, "atad")!=0) && (fileIdx < 0xFFF0))
        {
            fileIdx++;
            readBuffer[3] = readBuffer[2];
            readBuffer[2] = readBuffer[1];
            readBuffer[1] = readBuffer[0];
            inFile->read(readBuffer, 1);
        }

        if (fileIdx >= 0xFFF0)
        {
            currentMessage.append("DATA-Header not found. Damaged .wav-File?\n");
            noError = false;
        }

        inFile->read((char*) &nAudioBytes, 4);

        if (nBitsPerSample == 16)
            {
            tmpAudio16 = (int16_t*) new int16_t[blockLen*nChans+1]();
            normFact = 1.0 / ((double) 0x8000);
            }
        else if (nBitsPerSample == 24 || nBitsPerSample == 32)
            {
            tmpAudio32 = (int32_t*) new int32_t[blockLen*nChans+1]();
            normFact = 1.0 / ((double) 0x80000000);
            }
        else
        {
            currentMessage.append("This file uses no 16, 24 or 32Bit audio stream. Can't handle this.\n");
            noError = false;
        }

        if (noError)
        {
            beginPos = fileIdx+4;

            bytesPerBlock = (uint32_t) blockLen*frameSize;
            bytesPerBlockForDouble = (uint32_t) bytesPerBlock*64/nBitsPerSample;

            nSamplesPerFrame = blockSize*nChans;

            bytesPerSample = frameSize/nChans;

            byteCnt = 0;

            isInitialized = true;
            currentMessage.append("Instance initialization finished successful.");
        }
    }
}

void CWavRead::readOneBlock(double* outData)
{
    if (byteCnt >= (nAudioBytes-bytesPerBlock))
    {
        emit isEOF();
    }
    else
    {
        if (nBitsPerSample == 16)
        {
            inFile->read((char*) tmpAudio16, bytesPerBlock);
            fixedToFloat64(tmpAudio16, outData, nSamplesPerFrame);
        }
        else if (nBitsPerSample == 32)
        {
            inFile->read((char*) tmpAudio32, bytesPerBlock);
            fixedToFloat64(tmpAudio32, outData, nSamplesPerFrame);
        }
        else //24 Bit Audio
        {
            for (unsigned int kk = 0; kk < blockLen*nChans; kk++)
                inFile->read(((char*) (&tmpAudio32[kk]))+1, bytesPerSample);

                fixedToFloat64(tmpAudio32, outData, nSamplesPerFrame);
        }

        byteCnt += bytesPerBlock;
    }
}

CWavRead::~CWavRead()
    {
        if (isInitialized)
        {
            inFile->close();

            if (nBitsPerSample <= 16)
                {delete[] (int16_t*) tmpAudio16;}
            else
                {delete[] (int32_t*) tmpAudio32;}
        }

        delete inFile;
    }

CWavWrite::CWavWrite(const char *filePath, int blockSize,
            int sampFreq, int nrOfChans, int bitDepth)
    {
        char tmp[44] = {0};
        this->nSamplesPerFrame = blockSize*nrOfChans;
        fs = sampFreq;
        blockLen = blockSize;
        nChans = nrOfChans;
        currentMessage = "\nInitializing wavWrite class instance:\n";
        byteCnt = 0;
        noError = true;
        isInitialized = false;

        if (bitDepth <= 16)
        {
            nBitsPerSample = 16;
            this->bytesPerBlock = blockSize*2*nChans;
            this->frameSize = sizeof(int16_t)*nChans;
            tmpAudio16 = new int16_t[blockLen*nChans+1]();
            normFact = 0x8000;
        }
        else if (bitDepth <= 32)
        {
            nBitsPerSample = 32;
            this->bytesPerBlock = blockSize*4*nChans;
            this->frameSize = sizeof(int32_t)*nChans;
            tmpAudio32 = new int32_t[blockLen*nChans+1]();
            normFact = 0x80000000;
        }
        else
        {
            currentMessage.append("More than 32Bit per sample are not implemented in this version.\n");
            noError = false;
        }

        if (noError)
        {
            normFact = (double) ((int32_t) 1 << (bitDepth-1));

            outFile = new ofstream(filePath, ios::out | ios::binary);

            if (outFile->is_open())
            {
                outFile->write(tmp, 44);
                isInitialized = true;
                currentMessage.append("Instance initialization finished successful.\n");
            }
            else
            {
                currentMessage.append("Couldn't open/create .wav-File\n");
            }
        }
}

void CWavWrite::writeOneBlock(double* inData)
{
    if (nBitsPerSample <= 16)
        {
        float64ToFixed(inData, tmpAudio16, nSamplesPerFrame);
        outFile->write((char*) tmpAudio16, bytesPerBlock);
        }
    else
        {
        float64ToFixed(inData, tmpAudio32, nSamplesPerFrame);
        outFile->write((char*) tmpAudio32, bytesPerBlock);
        }

    byteCnt+=bytesPerBlock;
}

CWavWrite::~CWavWrite()
{
	int16_t tmp16;
	int32_t tmp32;

    if (isInitialized)
    {
        outFile->seekp(0, ios::beg);

        outFile->write("RIFF", 4);
        tmp32 = byteCnt+36;
        outFile->write((char*) &tmp32, 4);
        outFile->write("WAVE", 4);

        outFile->write("fmt ", 4);
        tmp32 = 16;
        outFile->write((char*) &tmp32, 4);
        tmp16 = 1;
        outFile->write((char*) &tmp16, 2);
        tmp16 = nChans;
        outFile->write((char*) &tmp16, 2);
        outFile->write((char*) &fs, 4);
        tmp32 = fs*(uint32_t)frameSize;
        outFile->write((char*) &tmp32, 4); //bytes per second
        outFile->write((char*) &frameSize, 2);
        outFile->write((char*) &nBitsPerSample, 2);

        outFile->write("data", 4);
        tmp32 = byteCnt;
        outFile->write((char*) &tmp32, 4);

        outFile->close();
        delete outFile;

        if (nBitsPerSample <= 16)
            delete[] (int16_t*) tmpAudio16;
        else
            delete[] (int32_t*) tmpAudio32;
    }
}
