#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDropEvent>
#include <QUrl>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTextEdit>
#include <QTextList>

#include "qcustomplot.h"
#include "CWavRW.h"
#include "CRtAudio.h"
#include "CRtAudio.h"
#include "CMpgRead.h"
#include "fft.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void playHandle();
    void stopHandle();
    void recHandle();
    void prevHandle();
    void nextHandle();
    void positionSliderPressHandle();
    void positionSliderReleaseHandle();
    void inPlotUpdate();
    void outPlotUpdate();
    void eofHandle();
private:
    void wavToSoundcardInit();
    void wavToSoundcardCleanup();
    void mpgToSoundcardInit();
    void mpgToSoundcardCleanup();
    void soundcardToWavInit();
    void soundcardToWavCleanup();
    void dragEnterEvent(QDragEnterEvent *dragEnterEventData);
    void dropEvent(QDropEvent *dropEventData);

    QColor backgroundColor;

    QCustomPlot *audioInPlot, *freqInPlot, *audioOutPlot, *freqOutPlot;
    QVector<unsigned int> logIdxVector;
    QVector<double> xTimeInPlot, xTimeOutPlot, yTimeInPlot, yTimeOutPlot;
    QVector<double> xFreqInPlot, xFreqOutPlot, yFreqInPlot, yFreqOutPlot, tmpFreq;
    QVector<complex_float64> complexFreqVec;
    QTimer *inPlotTimer, *outPlotTimer;

    QPushButton *playButton, *stopButton, *recButton, *prevButton, *nextButton;

    QIcon mydoobeIcon, playIcon, stopIcon, pauseIcon, recIcon, prevIcon, nextIcon;

    QTextEdit *statusTxt, *playlist;
    QList<QString> playlistFiles;

    QSlider *positionSlider;

    CWavRead *wavRead;
    CMpgRead *mpgRead;
    CWavWrite *wavWrite;
    CRtAudio *adc, *dac;

    QString outFile;

    RtAudio::Api audioApi;
    double fftNormFact;
    unsigned int blockLenIO, nfft, fsIn, fsOut, nChansIn, nChansOut, nBuffers;
    int playlistIdx, prevPlaylistIdx;
    bool bPlay, bRec, bSliderPressed, bWavReadInitialized, bWavWriteInitialized, bMpgReadInitialized;
};

#endif // MAINWINDOW_H
