/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#define __FILETESTER_H

#include <QtCore>
#include <LXiStream>

class FileTester : public SGraph
{
Q_OBJECT
public:
  static void                   testFile(const QString &);

private:
  explicit                      FileTester(const QString &);

  bool                          setup(void);

private:
  SFileInputNode                file;
  SAudioDecoderNode             audioDecoder;
  SVideoDecoderNode             videoDecoder;
  SDataDecoderNode              dataDecoder;
  STimeStampResamplerNode       timeStampResampler;
  SAudioMatrixNode              audioMatrix;
  SAudioResampleNode            audioResampler;
  SVideoLetterboxDetectNode     letterboxDetectNode;
  SSubpictureRenderNode         subpictureRenderer;
  SSubtitleRenderNode           subtitleRenderer;
  STimeStampSyncNode            sync;
};
