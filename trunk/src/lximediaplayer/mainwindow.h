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
    inline PlayerGraph(const QString &path, bool isDisc)
      : SGraph(),
        source(NULL),
        audioDecoder(this),
        videoDecoder(this),
        dataDecoder(this),
        deinterlacer(this),
        subpictureRenderer(this),
        subtitleRenderer(this),
        sync(this),
        audioOutput(this)
    {
      if (isDisc)
        source = new SDiscInputNode(this, path);
      else
        source = new SFileInputNode(this, path);

      connect(source, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
      connect(source, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
      connect(source, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));
      connect(&dataDecoder, SIGNAL(output(SSubpictureBuffer)), &subpictureRenderer, SLOT(input(SSubpictureBuffer)));
      connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), &subtitleRenderer, SLOT(input(SSubtitleBuffer)));
      connect(&videoDecoder, SIGNAL(output(SVideoBuffer)), &deinterlacer, SLOT(input(SVideoBuffer)));
      connect(&deinterlacer, SIGNAL(output(SVideoBuffer)), &subpictureRenderer, SLOT(input(SVideoBuffer)));
      connect(&subpictureRenderer, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));
      connect(&subtitleRenderer, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));
      connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
      connect(&sync, SIGNAL(output(SAudioBuffer)), &audioOutput, SLOT(input(SAudioBuffer)));

      connect(source, SIGNAL(finished()), SLOT(stop()));
    }

  public:
    QObject                   * source;
    SAudioDecoderNode           audioDecoder;
    SVideoDecoderNode           videoDecoder;
    SDataDecoderNode            dataDecoder;
    SVideoDeinterlaceNode       deinterlacer;
    SSubpictureRenderNode       subpictureRenderer;
    SSubtitleRenderNode         subtitleRenderer;
    STimeStampSyncNode          sync;
    SAudioOutputNode            audioOutput;
  };

  class CaptureGraph : public SGraph
  {
  public:
    inline CaptureGraph(const QString &device)
      : SGraph(),
        audioVideoInput(this, device),
        audioOutput(this)
    {
      connect(&audioVideoInput, SIGNAL(output(SAudioBuffer)), &audioOutput, SLOT(input(SAudioBuffer)));
    }

  public:
    SAudioVideoInputNode        audioVideoInput;
    SAudioOutputNode            audioOutput;
  };

public:
                                MainWindow(void);
  virtual                       ~MainWindow();

protected:
  virtual void                  timerEvent(QTimerEvent *);
  virtual void                  keyPressEvent(QKeyEvent *);

private slots:
  bool                          openFile(const QString &, int = -1);
  bool                          openDisc(const QString &, int = -1);
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
