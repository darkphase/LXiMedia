/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
 *   code@admiraal.dds.nl                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include <LXiStream>
#include <LXiStreamGui>

#include "ui_mainwindow.h"


class MainWindow : public QMainWindow
{
Q_OBJECT
private:
  class PlayerGraph : public SGraph
  {
  public:
    inline PlayerGraph(const QString &fileName)
      : SGraph(),
        file(this, fileName),
        audioDecoder(this),
        videoDecoder(this),
        dataDecoder(this),
        deinterlacer(this),
        subtitleRenderer(this),
        sync(this),
        audioOutput(this)
    {
      connect(&file, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
      connect(&file, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
      connect(&file, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));
      connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), &subtitleRenderer, SLOT(input(SSubtitleBuffer)));
      connect(&videoDecoder, SIGNAL(output(SVideoBuffer)), &deinterlacer, SLOT(input(SVideoBuffer)));
      connect(&deinterlacer, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));
      connect(&subtitleRenderer, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));
      connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
      connect(&sync, SIGNAL(output(SAudioBuffer)), &audioOutput, SLOT(input(SAudioBuffer)));
    }

  public:
    SFileInputNode              file;
    SAudioDecoderNode           audioDecoder;
    SVideoDecoderNode           videoDecoder;
    SDataDecoderNode            dataDecoder;
    SVideoDeinterlaceNode       deinterlacer;
    SSubtitleRenderNode         subtitleRenderer;
    STimeStampSyncNode          sync;
    SAudioOutputNode            audioOutput;
  };

  class CaptureGraph : public SGraph
  {
  public:
    inline CaptureGraph(const QString &device)
      : SGraph(),
        videoInput(this, device)//,
        //audioInput(this),
        //sync(this),
        //audioOutput(this)
    {
      //connect(&videoInput, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));
      //connect(&audioInput, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
      //connect(&sync, SIGNAL(output(SAudioBuffer)), &audioOutput, SLOT(input(SAudioBuffer)));
    }

  public:
    SVideoInputNode             videoInput;
    //SAudioInputNode             audioInput;
    //STimeStampSyncNode          sync;
    //SAudioOutputNode            audioOutput;
  };

public:
                                MainWindow(void);
  virtual                       ~MainWindow();

protected:
  virtual void                  timerEvent(QTimerEvent *);
  virtual void                  keyPressEvent(QKeyEvent *);

private slots:
  bool                          openFile(const QString &, int = -1);
  void                          selectDir(const QString &);
  void                          fileActivated(QTreeWidgetItem *);

  void                          selectCaptureDevice(int);
  void                          setFrequency(int);
  void                          refreshCaptureStreams(void);
  void                          streamActivated(QTreeWidgetItem *);

  void                          stop(void);
  void                          pause(bool);
  void                          fullscreen(bool);
  void                          fullscreenSec(bool);
  void                          seek(int);
  void                          applySeek();

private:
  struct PlayerGraph;

  Ui_mainWindow                 ui;

  SDisplay                      display;

  QString                       fileName;
  PlayerGraph                 * playerGraph;
  int                           refreshTimer;
  int                           lastPos;
  int                           seekPos;

  CaptureGraph                * captureGraph;

  SVideoView                  * secundaryView;
};


#endif