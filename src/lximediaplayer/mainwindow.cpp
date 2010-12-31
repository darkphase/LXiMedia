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

#include "mainwindow.h"


MainWindow::MainWindow(void)
  : QMainWindow(),
    playerGraph(NULL),
    refreshTimer(-1),
    seekPos(-1),
    captureGraph(NULL),
    secundaryView(NULL)
{
  QSettings settings;

  setAttribute(Qt::WA_DeleteOnClose);

  ui.setupUi(this);

  connect(ui.stopButton, SIGNAL(clicked()), SLOT(stop()));
  connect(ui.pauseButton, SIGNAL(toggled(bool)), SLOT(pause(bool)));
  connect(ui.fullscreenButton, SIGNAL(toggled(bool)), SLOT(fullscreen(bool)));
  connect(ui.fullscreenSecButton, SIGNAL(toggled(bool)), SLOT(fullscreenSec(bool)));
  connect(ui.timeSlider, SIGNAL(sliderMoved(int)), SLOT(seek(int)));
  connect(ui.timeSlider, SIGNAL(sliderReleased()), SLOT(applySeek()));

  connect(ui.fileList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(fileActivated(QTreeWidgetItem*)));
  connect(ui.captureDevices, SIGNAL(activated(int)), SLOT(selectCaptureDevice(int)));
  connect(ui.frequencySpin, SIGNAL(valueChanged(int)), SLOT(setFrequency(int)));
  connect(ui.captureStreams, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(streamActivated(QTreeWidgetItem*)));
  //connect(&captureStreamsTimer, SIGNAL(timeout()), SLOT(refreshCaptureStreams()));

  QStyle * const style = QApplication::style();

  setWindowIcon(style->standardIcon(QStyle::SP_MediaPlay));
  ui.stopButton->setIcon(style->standardIcon(QStyle::SP_MediaStop));
  ui.pauseButton->setIcon(style->standardIcon(QStyle::SP_MediaPause));
  ui.fullscreenSecButton->setVisible(QApplication::desktop()->screenCount() > 1);
  ui.tabWidget->setTabIcon(0, style->standardIcon(QStyle::SP_DirHomeIcon));
  ui.tunerGroup->setVisible(false);
  ui.tuneDownButton->setIcon(style->standardIcon(QStyle::SP_MediaSeekBackward));
  ui.tuneUpButton->setIcon(style->standardIcon(QStyle::SP_MediaSeekForward));

  SSystem::initialize(/*SSystem::Initialize_Default | SSystem::Initialize_AllowUntrusted*/);

  ui.captureDevices->addItem(tr("Select"));
  foreach (const QString &device, SVideoInputNode::devices())
    ui.captureDevices->addItem(device);

  if (QApplication::arguments().count() == 2)
  {
    const QFileInfo info = QApplication::arguments()[1];
    if (info.exists() && info.isReadable())
    {
      openFile(info.absoluteFilePath());
      selectDir(info.absoluteDir().absolutePath());
      return;
    }
  }

  if (QFileInfo(settings.value("LastOpenDir").toString()).exists())
    selectDir(settings.value("LastOpenDir").toString());
  else
    selectDir(QDir::homePath());

  if (settings.contains("LastGeometry"))
    setGeometry(settings.value("LastGeometry").toRect());
}

MainWindow::~MainWindow()
{
  stop();
  SSystem::shutdown();

  QSettings().setValue("LastGeometry", geometry());
}

void MainWindow::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == refreshTimer)
  {
    if (playerGraph)
    {
      lastPos = playerGraph->file.position().toSec();
      if (seekPos < 0)
      {
        ui.timeSlider->setValue(lastPos);
        ui.timeLabel->setText(QTime(0, 0).addSecs(lastPos).toString("h:mm:ss"));
      }
    }
  }
  else
    QMainWindow::timerEvent(e);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_Escape)
    ui.fullscreenButton->setChecked(false);
  else
    QMainWindow::keyPressEvent(e);
}

bool MainWindow::openFile(const QString &fileName, int startPos)
{
  QSettings settings;

  QDir lastDir(fileName);
  lastDir.cdUp();
  settings.setValue("LastOpenDir", lastDir.absolutePath());

  stop();

  playerGraph = new PlayerGraph(fileName);
  playerGraph->audioOutput.setDelay(STime::fromMSec(250)); // Otherwise video frames may come too late.
  playerGraph->addNode(ui.videoView);
  connect(&(playerGraph->sync), SIGNAL(output(SVideoBuffer)), ui.videoView, SLOT(input(SVideoBuffer)));

  if (playerGraph->start())
  {
    this->fileName = fileName;
    ui.stopButton->setEnabled(true);
    ui.pauseButton->setEnabled(true);
    ui.timeSlider->setEnabled(true);
    ui.fullscreenButton->setEnabled(true);
    ui.fullscreenSecButton->setEnabled(true);

    const int secs = playerGraph->file.duration().toSec();
    ui.timeSlider->setMaximum(secs);
    ui.durationLabel->setText(QTime(0, 0).addSecs(secs).toString("h:mm:ss"));

    if (startPos >= 0)
      playerGraph->file.setPosition(STime::fromSec(startPos));

    display.blockScreenSaver(true);

    if (refreshTimer == -1)
      refreshTimer = startTimer(1000);

    return true;
  }

  delete playerGraph;
  playerGraph = NULL;

  ui.stopButton->setEnabled(false);
  ui.pauseButton->setEnabled(false);
  ui.timeSlider->setEnabled(false);
  ui.fullscreenButton->setEnabled(false);
  ui.fullscreenSecButton->setEnabled(false);

  return false;
}

void MainWindow::selectDir(const QString &path)
{
  QDir dir(path);

  ui.fileList->clear();

  QStyle * const style = QApplication::style();

  foreach (const QFileInfo &info, dir.entryInfoList(QDir::Dirs, QDir::Name))
  if (info.fileName() != ".")
  {
    QTreeWidgetItem * const item =
        new QTreeWidgetItem(ui.fileList, QStringList() << info.fileName()
                                                       << info.absoluteFilePath());

    item->setIcon(0, style->standardIcon(QStyle::SP_DirIcon));
  }

  foreach (const QFileInfo &info, dir.entryInfoList(QDir::Files, QDir::Name))
  {
    QTreeWidgetItem * const item =
        new QTreeWidgetItem(ui.fileList, QStringList() << info.fileName()
                                                       << info.absoluteFilePath());

    item->setIcon(0, style->standardIcon(QStyle::SP_FileIcon));
  }
}

void MainWindow::fileActivated(QTreeWidgetItem *item)
{
  const QFileInfo info(item->text(1));

  if (info.isDir())
    selectDir(info.absoluteFilePath());
  else
    openFile(info.absoluteFilePath());
}

void MainWindow::selectCaptureDevice(int id)
{
  if (id > 0)
  {
    stop();
    ui.captureDevices->setCurrentIndex(id);

    captureGraph = new CaptureGraph(ui.captureDevices->currentText());
    captureGraph->audioOutput.setDelay(STime::fromMSec(250)); // Otherwise video frames may come too late.
    captureGraph->addNode(ui.videoView);
    connect(&(captureGraph->audioVideoInput), SIGNAL(output(SVideoBuffer)), ui.videoView, SLOT(input(SVideoBuffer)));

    if (captureGraph->start())
    {
      ui.fullscreenButton->setEnabled(true);
      ui.fullscreenSecButton->setEnabled(true);

      display.blockScreenSaver(true);

      if (refreshTimer == -1)
        refreshTimer = startTimer(1000);

      return;
    }

    delete captureGraph;
    captureGraph = NULL;
  }
  else
    stop();
}

void MainWindow::setFrequency(int freq)
{
//  STuner * const tuner = captureGraph->tuner();
//
//  tuner->setFrequency(quint64(freq) * 1000);
}

void MainWindow::refreshCaptureStreams(void)
{
/*  if (captureGraph)
  {
    QStyle * const style = QApplication::style();

    captureStreams = captureGraph->streams();
    for (int i=0; i<captureStreams.count(); i++)
    {
      bool found = false;
      for (int j=0; (j<ui.captureStreams->topLevelItemCount()) && !found; j++)
        found = ui.captureStreams->topLevelItem(j)->text(0) == captureStreams[i].name;

      if (!found)
      {
        QTreeWidgetItem * const item =
            new QTreeWidgetItem(ui.captureStreams, QStringList() << captureStreams[i].name);

        item->setIcon(0, style->standardIcon(QStyle::SP_MediaPlay));
      }
    }

    STuner * const tuner = captureGraph->tuner();
    if (tuner)
    {
      const STuner::Status status = tuner->signalStatus();

      ui.signalStrength->setValue(int(status.signalStrength * 100.0));
    }
  }*/
}

void MainWindow::streamActivated(QTreeWidgetItem *item)
{
/*  if (captureGraph)
  {
    if (captureGraph->isRunning())
    {
      captureGraph->stop();
      captureGraph->unprepare();
    }

    int id = -1;
    for (int i=0; (i<captureStreams.count()) && (id < 0); i++)
    if (item->text(0) == captureStreams[i].name)
      id = i;

    if ((id >= 0) && (id < captureStreams.count()))
    {
      if (captureGraph->open(captureStreams[id]))
      if (captureGraph->prepare())
      {
        ui.stopButton->setEnabled(true);
        ui.fullscreenButton->setEnabled(true);
        ui.fullscreenSecButton->setEnabled(true);

        display.blockScreenSaver(true);
        captureGraph->start();
      }
    }
  }*/
}

void MainWindow::stop(void)
{
  display.blockScreenSaver(false);

  if (playerGraph)
  {
    if (refreshTimer != -1)
    {
      killTimer(refreshTimer);
      refreshTimer = -1;
    }

    if (playerGraph->isRunning())
      playerGraph->stop();

    delete playerGraph;
    playerGraph = NULL;
  }

  if (captureGraph)
  {
    if (captureGraph->isRunning())
      captureGraph->stop();

    delete captureGraph;
    captureGraph = NULL;

    ui.captureDevices->setCurrentIndex(0);
  }

  ui.captureStreams->clear();
  ui.tunerGroup->setVisible(false);
  ui.stopButton->setEnabled(false);
  ui.pauseButton->setEnabled(false);
  ui.timeSlider->setEnabled(false);
  ui.fullscreenButton->setEnabled(false);
  ui.fullscreenSecButton->setEnabled(false);
}

void MainWindow::pause(bool paused)
{
//  if (paused)
//    playerGraph->suspend();
//  else
//    playerGraph->resume();
}

void MainWindow::fullscreen(bool fs)
{
  if (fs)
  {
    ui.controlsFrame->setVisible(false);
    ui.tabWidget->setVisible(false);
    ui.videoView->setCursor(Qt::BlankCursor);
    showFullScreen();
  }
  else
  {
    ui.controlsFrame->setVisible(true);
    ui.tabWidget->setVisible(true);
    ui.videoView->unsetCursor();
    showNormal();
  }
}

void MainWindow::fullscreenSec(bool fs)
{
  if (fs)
  {
    QDesktopWidget * const desktop = QApplication::desktop();
    const int screen = (desktop->screenNumber(this) + 1) % desktop->screenCount();

    secundaryView = new SVideoView(NULL);
    ui.videoView->addClone(secundaryView);
    secundaryView->setCursor(Qt::BlankCursor);
    secundaryView->setGeometry(desktop->screenGeometry(screen));
    secundaryView->showFullScreen();
  }
  else
  {
    delete secundaryView;
    secundaryView = NULL;
  }
}

void MainWindow::seek(int pos)
{
  seekPos = pos;
  ui.timeLabel->setText(QTime(0, 0).addSecs(seekPos).toString("h:mm:ss"));
}

void MainWindow::applySeek(void)
{
  playerGraph->file.setPosition(STime::fromSec(seekPos));
  seekPos = -1;
}
