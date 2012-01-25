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

#include "v4lcommon.h"

namespace LXiStream {
namespace V4lBackend {


V4lCommon::V4lCommon(void)
          :openAudioDevice(true),
           openVideoDevice(true),
           audioTerminal(NULL),
           audioNode(NULL)
{
}

V4lCommon::~V4lCommon()
{
}

bool V4lCommon::findAudioDevice(const QString &name)
{
  Q_ASSERT(audioTerminal == NULL);
  Q_ASSERT(audioNode == NULL);

  const QString myLongName = SStringParser::toRawName(name);
  QString bestMatch = QString::null;
  qreal match = 0.0f;

  foreach (const SSystem::DeviceEntry &device, SSystem::availableAudioDevices())
  {
    STerminals::AudioDevice * const terminal = SSystem::createTerminal<STerminals::AudioDevice>(object(), device.url, false);
    if (terminal)
    if (!terminal->inputs().isEmpty())
    {
      qreal m = SStringParser::computeBidirMatch(SStringParser::toRawName(terminal->longName()), myLongName);
      if (m > match)
      {
        match = m;
        bestMatch = device.url;
      }

      delete terminal;
    }
  }

  if (!bestMatch.isEmpty())
  { // Found one
    audioTerminal = SSystem::createTerminal<STerminals::AudioDevice>(object(), bestMatch, false);
    if (audioTerminal)
    {
      if (!audioTerminal->inputs().isEmpty())
      {
        QString input = audioTerminal->inputs().first();
        foreach (const QString &i, audioTerminal->inputs())
        if (SStringParser::toRawName(i) == "LINEIN")
          input = i;

        if (audioTerminal->selectInput(input))
          audioNode = audioTerminal->openStream(audioTerminal->inputStream(0));
      }

      if (audioNode == NULL)
      {
        delete audioTerminal;
        audioTerminal = NULL;
      }
      else
        audioNode->setProperty("sampleRate", 32000);
    }
  }

  return audioNode != NULL;
}


} } // End of namespaces
