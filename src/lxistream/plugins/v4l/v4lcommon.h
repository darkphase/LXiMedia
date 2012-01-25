/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
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

#ifndef V4LBACKEND_V4LCOMMON_H
#define V4LBACKEND_V4LCOMMON_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace V4lBackend {


class V4lCommon
{
public:
                                V4lCommon(void);
  virtual                       ~V4lCommon(void);

  bool                          findAudioDevice(const QString &name);

protected:
  virtual QObject             * object(void) = 0;

  bool                          openAudioDevice;
  bool                          openVideoDevice;

  STerminals::AudioDevice     * audioTerminal;
  SNode                       * audioNode;
};


} } // End of namespaces

#endif
