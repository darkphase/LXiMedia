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

#include "networkbufferreader.h"

namespace LXiStream {
namespace FFMpegBackend {

NetworkBufferReader::NetworkBufferReader(const QString &, QObject *parent)
  : SInterfaces::NetworkBufferReader(parent)
{
}

NetworkBufferReader::~NetworkBufferReader()
{
}

bool NetworkBufferReader::openProtocol(const QString &)
{
  return true;
}

bool NetworkBufferReader::start(const QUrl &url, ProduceCallback *produceCallback, quint16 /*programId*/)
{
  ::AVFormatContext * formatContext = NULL;
  if (::av_open_input_file(&formatContext, url.toEncoded(), NULL, 0, NULL) == 0)
    return BufferReaderBase::start(produceCallback, formatContext);

  return false;
}

void NetworkBufferReader::stop(void)
{
  BufferReaderBase::stop();
}

} } // End of namespaces
