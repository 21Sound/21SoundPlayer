#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QBrush plotBrush(QColor(150,150,150,150));
    QTextCursor tmpCursor;
    QTextBlockFormat tmpFormat;
    unsigned int lenOfLogVector;
    double logIdx = 1.99;

    bPlay = true;
    bRec = true;
    bSliderPressed = false;
    bWavReadInitialized = false;
    bWavWriteInitialized = false;
    bMpgReadInitialized = false;
    playlistIdx = 0;
    prevPlaylistIdx = 0;
    fsIn = 44100;
    fsOut = -1;
    nChansIn = 1;
    nChansOut = -1;
    audioApi = CRtAudio::WINDOWS_DS;
    blockLenIO = 8192;
    nBuffers = 5;
    nfft = 4*blockLenIO;
    fftNormFact = 0.5/((double)blockLenIO);
    QFileInfo currentPath(QCoreApplication::applicationFilePath());
    outFile = currentPath.absolutePath();
    outFile.append("/record.wav");

    /*___________________GUI initialization____________________*/

    backgroundColor = this->palette().background().color();

    QString tmpStr = currentPath.absolutePath();
    tmpStr.append("/symbols/21_logo.png");
    mydoobeIcon.addFile(tmpStr);

    tmpStr = currentPath.absolutePath();
    tmpStr.append("/symbols/play.png");
    playIcon.addFile(tmpStr);

    tmpStr = currentPath.absolutePath();
    tmpStr.append("/symbols/pause.png");
    pauseIcon.addFile(tmpStr);

    tmpStr = currentPath.absolutePath();
    tmpStr.append("/symbols/stop.png");
    stopIcon.addFile(tmpStr);

    tmpStr = currentPath.absolutePath();
    tmpStr.append("/symbols/record.png");
    recIcon.addFile(tmpStr);

    tmpStr = currentPath.absolutePath();
    tmpStr.append("/symbols/previous.png");
    prevIcon.addFile(tmpStr);

    tmpStr = currentPath.absolutePath();
    tmpStr.append("/symbols/next.png");
    nextIcon.addFile(tmpStr);

    this->setWindowIcon(mydoobeIcon);
    this->setWindowTitle("21SoundPlayer");
    this->setAcceptDrops(true);

    playButton = new QPushButton(playIcon, NULL, this);
    playButton->move(QPoint(10,15));
    playButton->resize(QSize(50,50));
    playButton->setIconSize(QSize(50,50));

    stopButton = new QPushButton(stopIcon, NULL, this);
    stopButton->move(QPoint(10,75));
    stopButton->resize(QSize(50,50));
    stopButton->setIconSize(QSize(50,50));

    recButton = new QPushButton(recIcon, NULL, this);
    recButton->move(QPoint(10,135));
    recButton->resize(QSize(50,50));
    recButton->setIconSize(QSize(25,25));
    recButton->setCheckable(true);

    prevButton = new QPushButton(prevIcon, NULL, this);
    prevButton->move(QPoint(625,15));
    prevButton->resize(QSize(50,50));
    prevButton->setIconSize(QSize(50,50));

    nextButton = new QPushButton(nextIcon, NULL, this);
    nextButton->move(QPoint(700,15));
    nextButton->resize(QSize(50,50));
    nextButton->setIconSize(QSize(50,50));

    audioOutPlot = new QCustomPlot(this);
    audioOutPlot->move(70,0);
    audioOutPlot->resize(QSize(230,100));
    audioOutPlot->addGraph();
    audioOutPlot->graph(0)->setPen(QColor(230,50,50));
    audioOutPlot->graph(0)->setBrush(plotBrush);
    audioOutPlot->xAxis->setRange(1,blockLenIO);
    audioOutPlot->yAxis->setRange(-1,1);
    audioOutPlot->setBackground(this->palette().background().color());
    xTimeOutPlot.resize(blockLenIO);
    yTimeOutPlot.resize(blockLenIO);
    for (int cnt=0; cnt<xTimeOutPlot.size(); cnt++)
    {
        xTimeOutPlot[cnt] = cnt;
        yTimeOutPlot[cnt] = 0;

    }
    audioOutPlot->graph(0)->setData(xTimeOutPlot, yTimeOutPlot, true);

    while(logIdx<(blockLenIO))
    {
        logIdxVector.push_back((unsigned int) logIdx);
        logIdx *= 1.03;
    }

    for(int kk = 0; kk<logIdxVector.size()-1; kk++)
    {
        if (logIdxVector[kk] == logIdxVector[kk+1])
        {
            logIdxVector.removeAt(kk+1);
            kk--;
        }
    }

    lenOfLogVector = logIdxVector.size();

    freqOutPlot = new QCustomPlot(this);
    freqOutPlot->move(320,0);
    freqOutPlot->resize(QSize(280,100));
    freqOutPlot->addGraph();
    freqOutPlot->graph(0)->setPen(QColor(230,50,50));
    freqOutPlot->graph(0)->setBrush(plotBrush);
    freqOutPlot->xAxis->setRange(1,lenOfLogVector);
    freqOutPlot->yAxis->setRange(0,80);
    freqOutPlot->setBackground(this->palette().background().color());
    tmpFreq.resize(nfft);
    complexFreqVec.resize(nfft);
    xFreqOutPlot.resize(lenOfLogVector);
    yFreqOutPlot.resize(lenOfLogVector);
    for (int cnt=0; cnt<xFreqOutPlot.size(); cnt++)
    {
        xFreqOutPlot[cnt] = cnt;
        yFreqOutPlot[cnt] = 0;
    }
    freqOutPlot->graph(0)->setData(xFreqOutPlot, yFreqOutPlot, true);

    outPlotTimer = new QTimer();
    outPlotTimer->setInterval(50);

    positionSlider = new QSlider(Qt::Horizontal, this);
    positionSlider->setRange(0,999);
    positionSlider->setSingleStep(1);
    positionSlider->setValue(0);
    positionSlider->setTickPosition(positionSlider->TicksBelow);
    positionSlider->setTickInterval(500);
    positionSlider->move(90,100);
    positionSlider->resize(270,20);

    audioInPlot = new QCustomPlot(this);
    audioInPlot->move(70,130);
    audioInPlot->resize(QSize(230,100));
    audioInPlot->addGraph();
    audioInPlot->graph(0)->setPen(QColor(200,50,50));
    audioInPlot->graph(0)->setBrush(plotBrush);
    audioInPlot->xAxis->setRange(1,blockLenIO);
    audioInPlot->yAxis->setRange(-1,1);
    audioInPlot->setBackground(backgroundColor);
    xTimeInPlot.resize(blockLenIO);
    yTimeInPlot.resize(blockLenIO);
    for (int cnt=0; cnt<xTimeInPlot.size(); cnt++)
    {
        xTimeInPlot[cnt] = cnt;
        yTimeInPlot[cnt] = 0;
    }
    audioInPlot->graph(0)->setData(xTimeInPlot, yTimeInPlot, true);

    freqInPlot = new QCustomPlot(this);
    freqInPlot->move(320,130);
    freqInPlot->resize(QSize(280,100));
    freqInPlot->addGraph();
    freqInPlot->graph(0)->setPen(QColor(230,50,50));
    freqInPlot->graph(0)->setBrush(plotBrush);
    freqInPlot->xAxis->setRange(1,lenOfLogVector);
    freqInPlot->yAxis->setRange(0,80);
    freqInPlot->setBackground(backgroundColor);
    xFreqInPlot.resize(lenOfLogVector);
    yFreqInPlot.resize(lenOfLogVector);
    for (int cnt=0; cnt<xFreqInPlot.size(); cnt++)
    {
        xFreqInPlot[cnt] = cnt;
        yFreqInPlot[cnt] = 0;
    }
    freqInPlot->graph(0)->setData(xFreqInPlot, yFreqInPlot, true);

    inPlotTimer = new QTimer();
    inPlotTimer->setInterval(50);

    statusTxt = new QTextEdit(this);
    statusTxt->setText(QString::fromStdString(CRtAudio::deviceInfo(audioApi)));
    statusTxt->move(QPoint(10,290));
    statusTxt->resize(QSize(580,100));
    statusTxt->setPalette(this->palette());
    statusTxt->viewport()->setAutoFillBackground(false);
    statusTxt->setReadOnly(true);

    playlist = new QTextEdit(this);
    playlist->setText("--- Playlist ---");
    playlist->move(QPoint(600,80));
    playlist->resize(QSize(180,300));
    playlist->setPalette(this->palette());
    playlist->viewport()->setAutoFillBackground(false);
    playlist->setReadOnly(true);
    playlist->append("Cooking Kung - This Girl.mp3");
    tmpStr = currentPath.absolutePath();
    tmpStr.append("/Cooking Kung - This Girl.mp3");
    playlistFiles.push_back(tmpStr);

    tmpCursor = playlist->textCursor();
    tmpCursor.movePosition(QTextCursor::Start);
    tmpCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);
    tmpFormat = tmpCursor.blockFormat();
    tmpFormat.setBackground(QColor(50,200,50));
    tmpCursor.setBlockFormat(tmpFormat);

    connect(playButton, SIGNAL (clicked()), this, SLOT (playHandle()));
    connect(stopButton, SIGNAL (clicked()), this, SLOT (stopHandle()));
    connect(recButton, SIGNAL (clicked()), this, SLOT (recHandle()));
    connect(prevButton, SIGNAL (clicked()), this, SLOT (prevHandle()));
    connect(nextButton, SIGNAL (clicked()), this, SLOT (nextHandle()));
    connect(inPlotTimer, SIGNAL (timeout()), this, SLOT (inPlotUpdate()));
    connect(outPlotTimer, SIGNAL (timeout()), this, SLOT (outPlotUpdate()));
    connect(positionSlider, SIGNAL (sliderPressed()), this, SLOT (positionSliderPressHandle()));
    connect(positionSlider, SIGNAL (sliderReleased()), this, SLOT (positionSliderReleaseHandle()));
}

void MainWindow::playHandle()
{
    bool isWavFile = false, isMpgFile = false;
    int tmpPlaylistIdx = 0;

    statusTxt->setText("Play Button Toggled.");

    if  (playlistIdx < 0)
    {
        tmpPlaylistIdx = 0;
    }
    else
    {
        tmpPlaylistIdx = playlistIdx;
    }

    if (playlistFiles[tmpPlaylistIdx].contains(".wav"))
    {
        isWavFile = true;
    }
    else if (playlistFiles[tmpPlaylistIdx].contains(".mp3"))
    {
        isMpgFile = true;
    }
    else
    {
        statusTxt->append("\nNo .wav or .mp3-File dropped.");
    }

    if (bPlay)
    {
        playButton->setIcon(pauseIcon);
        statusTxt->append("\nAudio Playback started: Playing ");
        statusTxt->append(playlistFiles[tmpPlaylistIdx]);

        if (isWavFile && !bWavReadInitialized)
        {
            wavToSoundcardInit();
        }
        else if (isMpgFile && !bMpgReadInitialized)
        {
            mpgToSoundcardInit();
        }
        else if (bWavReadInitialized || bMpgReadInitialized)
        {
            dac->startStream();
            outPlotTimer->start();
        }
    }
    else
    {
        playButton->setIcon(playIcon);
        statusTxt->append("\nAudio Playback paused.");

        if (bWavReadInitialized || bMpgReadInitialized)
        {
            dac->stopStream();
            outPlotTimer->stop();
        }
    }

    bPlay = !bPlay;
}

void MainWindow::stopHandle()
{
    statusTxt->setText("Stop Button toggled.");

    if (!bPlay)
    {
        playHandle();
    }

    if (!bRec)
    {
        recHandle();
    }

    if (bWavReadInitialized)
    {
        wavToSoundcardCleanup();
    }
    else if (bMpgReadInitialized)
    {
        mpgToSoundcardCleanup();
    }

    if (bWavWriteInitialized)
    {
        statusTxt->append("\nWrote .wav-File: ");
        statusTxt->append(outFile);
        soundcardToWavCleanup();
    }
}

void MainWindow::recHandle()
{
    if (bRec)
    {
        recButton->setChecked(true);
        statusTxt->setText("Audio Record started.");

        if (bWavWriteInitialized)
        {
            adc->startStream();
            inPlotTimer->start();
        }
        else
        {
            soundcardToWavInit();
        }
    }
    else
    {
        recButton->setChecked(false);
        statusTxt->append("Audio Record stopped.");

        if (bWavWriteInitialized)
        {
            adc->stopStream();
            inPlotTimer->stop();
        }
        else
        {
            statusTxt->append("Something went wrong during .wav-file write initialization.");
        }
    }

    bRec = !bRec;
}

void MainWindow::prevHandle()
{
    if (playlistIdx >= 1)
    {
        playlistIdx -= 2;
        eofHandle();
    }
    else
    {
        statusTxt->setText("File Cursor is already on the first position...");
    }
}

void MainWindow::nextHandle()
{
    if (playlistIdx+1 < playlistFiles.size())
    {
        eofHandle();
    }
    else
    {
        statusTxt->setText("File cursor is on the last position...");
    }
}

void MainWindow::positionSliderPressHandle()
{
    bSliderPressed = true;
}

void MainWindow::positionSliderReleaseHandle()
{
    bSliderPressed = false;
    if (!bPlay)
    {
        if (bWavReadInitialized)
        {
            wavRead->setNormalizedPosition((double)positionSlider->value()*0.001);
        }
        else if (bMpgReadInitialized)
        {
            mpgRead->setNormalizedPosition((double)positionSlider->value()*0.001);
        }
    }
}

void MainWindow::inPlotUpdate()
{
    const int doubleSize = sizeof(double);
    double tmpVal, *inPlotData = adc->getCurrentDataPtr();

    memcpy(yTimeInPlot.data(), inPlotData, blockLenIO*doubleSize);
    audioInPlot->graph(0)->setData(xTimeInPlot, yTimeInPlot, true);
    audioInPlot->replot();

    memcpy(tmpFreq.data(), inPlotData, blockLenIO*doubleSize);
    fft_double(tmpFreq.data(), complexFreqVec.data(), nfft);

    for (int kk=0; kk<logIdxVector.size(); kk++)
    {
        tmpVal = complex_abs(complexFreqVec[logIdxVector[kk]])*fftNormFact;
        if (tmpVal<0.00001)
        {
            tmpVal = 0.00001;
        }
        tmpVal = 20*log10(tmpVal)+100;
        yFreqInPlot[kk] = 0.2*tmpVal+0.8*yFreqInPlot[kk];
    }

    freqInPlot->graph(0)->setData(xFreqInPlot, yFreqInPlot, true);
    freqInPlot->replot();
}

void MainWindow::outPlotUpdate()
{
    const int doubleSize = sizeof(double);
    double tmpVal, *outPlotData = dac->getCurrentDataPtr();

    memcpy(yTimeOutPlot.data(), outPlotData, blockLenIO*doubleSize);
    audioOutPlot->graph(0)->setData(xTimeOutPlot, yTimeOutPlot, true);
    audioOutPlot->replot();

    memcpy(tmpFreq.data(), outPlotData, blockLenIO*doubleSize);
    fft_double(tmpFreq.data(), complexFreqVec.data(), nfft);

    for (int kk=0; kk<logIdxVector.size(); kk++)
    {
        tmpVal = complex_abs(complexFreqVec[logIdxVector[kk]])*fftNormFact;
        if (tmpVal<0.00001)
        {
            tmpVal = 0.00001;
        }
        tmpVal = 20*log10(tmpVal)+100;
        yFreqOutPlot[kk] = 0.2*tmpVal+0.8*yFreqOutPlot[kk];
    }

    freqOutPlot->graph(0)->setData(xFreqOutPlot, yFreqOutPlot, true);
    freqOutPlot->replot();

    if (!bSliderPressed)
    {
        if (bWavReadInitialized)
        {
            positionSlider->setValue(wavRead->getNormalizedPosition()*999);
        }
        else if (bMpgReadInitialized)
        {
            positionSlider->setValue(mpgRead->getNormalizedPosition()*999);
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *dragEnterEventData)
{
    bool acceptDrop = true;
    QList<QUrl> fileNames = dragEnterEventData->mimeData()->urls();

    if (fileNames.size() > 50)
    {
        acceptDrop = false;
    }

    if (acceptDrop)
    {
        dragEnterEventData->accept();
    }
}

void MainWindow::dropEvent(QDropEvent *dropEventData)
{
    QString tmpStr, combinedStr;
    QFileInfo fileNameExtractor;

    QList<QUrl> fileNames = dropEventData->mimeData()->urls();

    for (int kk = 0; kk < fileNames.size(); kk++)
    {
        tmpStr = fileNames[kk].toLocalFile();
        fileNameExtractor = QFileInfo(tmpStr);

        if (tmpStr.contains(".wav") || tmpStr.contains(".mp3"))
        {
            playlist->append(fileNameExtractor.fileName());
            playlistFiles.push_back(tmpStr);
        }
        else
        {
            combinedStr = "\nThis is no .wav or .mp3 file: ";
            combinedStr.append(fileNameExtractor.fileName());
            statusTxt->append(combinedStr);
        }
    }

    tmpStr = fileNames[0].toLocalFile();
}

void MainWindow::wavToSoundcardInit()
{
    wavRead = new CWavRead(playlistFiles[playlistIdx].toLatin1().data(), blockLenIO);
    statusTxt->append(QString::fromStdString(wavRead->getCurrentMessage()));

    if (wavRead->isInstanceInitialized())
    {
        bWavReadInitialized = true;
        fsOut = wavRead->getSamplingRate();
        nChansOut = wavRead->getNrOfChans();
        dac = new CRtAudio(false, true, audioApi, blockLenIO, 10, fsOut, nChansOut);
        statusTxt->append(QString::fromStdString(dac->getCurrentMessage()));

        if (dac->isStreamOpen())
        {
            connect(dac, SIGNAL (audioOutDone(double*)), wavRead, SLOT (readOneBlock(double*)));
            connect(wavRead, SIGNAL (isEOF()), this, SLOT (eofHandle()));
            dac->startStream();

            if (dac->isStreamRunning())
            {
                outPlotTimer->start();
            }
        }
    }
}

void MainWindow::wavToSoundcardCleanup()
{
    if (dac->isStreamRunning())
    {
        outPlotTimer->stop();
        dac->stopStream();
    }

    if (dac->isStreamOpen())
    {
        dac->closeStream();
    }

    if (wavRead->isInstanceInitialized())
    {
        delete dac;
    }

    delete wavRead;
    bWavReadInitialized = false;
}

void MainWindow::mpgToSoundcardInit()
{
    mpgRead = new CMpgRead(playlistFiles[playlistIdx].toLatin1().data(), blockLenIO);
    statusTxt->append(QString::fromStdString(mpgRead->getCurrentMessage()));

    if (mpgRead->isInstanceInitialized())
    {
        bMpgReadInitialized = true;
        fsOut = mpgRead->getSamplingRate();
        nChansOut = mpgRead->getNrOfChans();
        dac = new CRtAudio(false, true, audioApi, blockLenIO, nBuffers, fsOut, nChansOut);
        statusTxt->append(QString::fromStdString(dac->getCurrentMessage()));

        if (dac->isStreamOpen())
        {
            connect(dac, SIGNAL (audioOutDone(double*)), mpgRead, SLOT (readOneBlock(double*)));
            connect(mpgRead, SIGNAL (isEOF()), this, SLOT (eofHandle()));
            dac->startStream();

            if (dac->isStreamRunning())
            {
                outPlotTimer->start();
            }
        }
    }
}

void MainWindow::mpgToSoundcardCleanup()
{
    if (dac->isStreamRunning())
    {
        outPlotTimer->stop();
        dac->stopStream();
    }

    if (dac->isStreamOpen())
    {
        dac->closeStream();
    }

    if (mpgRead->isInstanceInitialized())
    {
        delete dac;
    }

    delete mpgRead;
    bMpgReadInitialized = false;
}

void MainWindow::soundcardToWavInit()
{
    adc = new CRtAudio(true, false, audioApi, blockLenIO, nBuffers, fsIn, nChansIn);
    statusTxt->append(QString::fromStdString(adc->getCurrentMessage()));

    if (adc->isStreamOpen())
    {
        wavWrite = new CWavWrite(outFile.toLatin1().data(), blockLenIO, fsIn, nChansIn, 16);
        statusTxt->append(QString::fromStdString(wavWrite->getCurrentMessage()));

        if (wavWrite->isInstanceInitialized())
        {
            connect(adc, SIGNAL (audioInDone(double*)), wavWrite, SLOT (writeOneBlock(double*)));
            adc->startStream();

            if (adc->isStreamRunning())
            {
                inPlotTimer->start();
                bWavWriteInitialized = true;
            }
        }
    }
    else
    {
        statusTxt->append("Audio stream to soundcard couldn't be opened!");
    }
}

void MainWindow::soundcardToWavCleanup()
{
    if (adc->isStreamRunning())
    {
        inPlotTimer->stop();
        adc->stopStream();
    }

    if (adc->isStreamOpen())
    {
        adc->closeStream();
        delete wavWrite;
    }

    delete adc;
    bWavWriteInitialized = false;
}

void MainWindow::eofHandle()
{
    QTextCursor tmpCursor = playlist->textCursor();
    QTextBlockFormat tmpFormat;

    stopHandle();

    if (playlistFiles.size()>playlistIdx+1)
    {
        playlistIdx++;

        tmpCursor.movePosition(QTextCursor::Start);
        tmpCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, playlistIdx+1);
        tmpFormat = tmpCursor.blockFormat();
        tmpFormat.setBackground(QColor(50,200,50));
        tmpCursor.setBlockFormat(tmpFormat);

        if (prevPlaylistIdx - playlistIdx > 0)
        {
            tmpCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);
        }
        else
        {
            tmpCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, 1);
        }

        tmpFormat = tmpCursor.blockFormat();
        tmpFormat.setBackground(backgroundColor);
        tmpCursor.setBlockFormat(tmpFormat);

        prevPlaylistIdx = playlistIdx;
        playlist->repaint();

        playHandle();
    }
}

MainWindow::~MainWindow()
{  
    stopHandle();
    delete statusTxt;
    delete positionSlider;
    delete inPlotTimer;
    delete outPlotTimer;
    delete playButton;
    delete stopButton;
    delete recButton;
    delete prevButton;
    delete nextButton;
}
