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

#include "module.h"
#include "alsainput.h"
#include "alsaoutput.h"

namespace LXiStream {
namespace AlsaBackend {


void Module::registerClasses(void)
{
  if ((SSystem::initializeFlags() & SSystem::Initialize_Devices) == SSystem::Initialize_Devices)
  {
    int card = -1;
    if (snd_card_next(&card) >= 0)
    while (card >= 0) // Iterate through all the cards
    {
      char *name = NULL;

      if (snd_card_get_name(card, &name) >= 0)
      {
        QString dev = "hw:" + QString::number(card);
        snd_ctl_t *ctl = NULL;
        if (snd_ctl_open(&ctl, dev.toAscii().data(), 0) == 0)
        {
          snd_ctl_card_info_t *info = NULL;

          if (snd_ctl_card_info_malloc(&info) == 0)
          if (snd_ctl_card_info(ctl, info) == 0)
          {
            const QString devName = SStringParser::toRawName(snd_ctl_card_info_get_longname(info)).toLower();
            QString name = QString(snd_ctl_card_info_get_name(info));

            char **hints = NULL;
            if (snd_device_name_hint(card, "pcm", reinterpret_cast<void ***>(&hints)) == 0)
            {
              for (unsigned i=0; hints[i]!=NULL; i++)
              {
                QString name(hints[i]);
                const bool inputOnly = name.contains("IOIDInput");
                {
                  const int no = name.indexOf("NAME");
                  const int co = name.indexOf("CARD");
                  if ((no >= 0) && (co > no))
                    name = name.mid(no + 4, co - (no + 4));
                  else
                    continue;
                }

                if (name == "default:") // Analog output
                {
                  if (!inputOnly)
                    AlsaOutput::registerClass<AlsaOutput>(SFactory::Scheme(-card, "hw:" + QString::number(card)));

                  AlsaInput::registerClass<AlsaInput>(SFactory::Scheme(-card, "hw:" + QString::number(card)));
                }
                else if (name == "iec958:") // Iec958 (S/PDIF) output
                {
                  if (!inputOnly)
                    AlsaOutput::registerClass<AlsaOutput>(SFactory::Scheme(-card, "iec958:" + QString::number(card)));

                  AlsaInput::registerClass<AlsaInput>(SFactory::Scheme(-card, "iec958:" + QString::number(card)));
                }
              }
            }

            snd_device_name_free_hint(reinterpret_cast<void **>(hints));
          }

          if (info)
            snd_ctl_card_info_free(info);

          snd_ctl_close(ctl);
        }

        free(name);
      }

      // Next card
      if (snd_card_next(&card) < 0)
        break;
    }
  }
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return QByteArray();
}


} } // End of namespaces

#ifdef PLUGIN_NAME
#include <QtPlugin>
Q_EXPORT_PLUGIN2(PLUGIN_NAME, LXiStream::AlsaBackend::Module);
#endif
