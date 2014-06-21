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

#include "alsamixer.h"
#include "module.h"

namespace LXiStreamDevice {
namespace AlsaBackend {

AlsaMixer::AlsaMixer(const QString &dev, QObject *parent)
  : QObject(parent),
    dev(deviceName(dev)),
    mixer(NULL)
{
}

AlsaMixer::~AlsaMixer()
{
  close();
}

bool AlsaMixer::open()
{
  close();

  if ((::snd_mixer_open(&mixer, 0) == 0) && mixer)
  if (::snd_mixer_attach(mixer, dev.toLatin1().data()) == 0)
  if (::snd_mixer_selem_register(mixer, NULL, NULL) == 0)
  if (::snd_mixer_load(mixer) == 0)
    return true;

  if (mixer)
  {
    ::snd_mixer_close(mixer);
    mixer = NULL;
  }

  return false;
}

void AlsaMixer::close()
{
  if (mixer)
  {
    ::snd_mixer_close(mixer);
    mixer = NULL;
  }
}

QStringList AlsaMixer::listInputChannels()
{
  QStringList result;

  snd_mixer_elem_t * const first = ::snd_mixer_first_elem(mixer);
  snd_mixer_elem_t * const last  = ::snd_mixer_last_elem(mixer);

  for (snd_mixer_elem_t *i = first; i; i = ::snd_mixer_elem_next(i))
  {
    if (::snd_mixer_selem_has_capture_volume(i) ||
        ::snd_mixer_selem_has_capture_switch(i) ||
        ::snd_mixer_selem_has_capture_switch_exclusive(i))
    {
      result += ::snd_mixer_selem_get_name(i);
    }

    if (i == last)
      break;
  }

  return result;
}

bool AlsaMixer::activateInputChannel(const QString &channel)
{
  snd_mixer_elem_t * const first = ::snd_mixer_first_elem(mixer);
  snd_mixer_elem_t * const last  = ::snd_mixer_last_elem(mixer);

  // Find selected channel
  snd_mixer_elem_t * selected = NULL;
  for (snd_mixer_elem_t *i = first; i && (selected == NULL); i = ::snd_mixer_elem_next(i))
  {
    if (::snd_mixer_selem_get_name(i) == channel)
      selected = i;

    if (i == last)
      break;
  }

  if (selected)
  {
    // Mute all
    for (snd_mixer_elem_t *i = first; i; i = ::snd_mixer_elem_next(i))
    {
      ::snd_mixer_selem_set_capture_switch_all(i, 0);

      long min = 0, max = 0;
      if (::snd_mixer_selem_get_capture_volume_range(i, &min, &max) == 0)
        ::snd_mixer_selem_set_capture_volume_all(i, min);

      if (i == last)
        break;
    }

    // Activate selected channel
    ::snd_mixer_selem_set_capture_switch_all(selected, 1);

    long min = 0, max = 0;
    if (::snd_mixer_selem_get_capture_volume_range(selected, &min, &max) == 0)
      ::snd_mixer_selem_set_capture_volume_all(selected, max);

    return true;
  }

  return false;
}

} } // End of namespaces
