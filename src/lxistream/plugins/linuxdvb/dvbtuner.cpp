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

#include "dvbtuner.h"

namespace LXiStream {
namespace LinuxDvbBackend {


DVBTuner::DVBTuner(DVBDevice *parent)
         :SDigitalTuner(parent),
          parent(parent)
{
}

quint64 DVBTuner::frequency(void) const
{
  dvb_frontend_parameters frontendParameters;
  memset(&frontendParameters, 0, sizeof(frontendParameters));

  if (ioctl(parent->frontendDesc, FE_GET_FRONTEND, &frontendParameters) >= 0)
    return frontendParameters.frequency;

  return 0;
}

bool DVBTuner::setFrequency(quint64 freq)
{
  dvb_frontend_parameters frontendParameters;
  memset(&frontendParameters, 0, sizeof(frontendParameters));

  frontendParameters.frequency = freq;
  frontendParameters.inversion = INVERSION_AUTO;
  frontendParameters.u.ofdm.bandwidth = BANDWIDTH_AUTO;
  frontendParameters.u.ofdm.code_rate_HP = FEC_AUTO;
  frontendParameters.u.ofdm.code_rate_LP = FEC_AUTO;
  frontendParameters.u.ofdm.constellation = QAM_AUTO;
  frontendParameters.u.ofdm.transmission_mode = TRANSMISSION_MODE_AUTO;
  frontendParameters.u.ofdm.guard_interval = GUARD_INTERVAL_AUTO;
  frontendParameters.u.ofdm.hierarchy_information = HIERARCHY_AUTO;

  if (ioctl(parent->frontendDesc, FE_SET_FRONTEND, &frontendParameters) >= 0)
  {
    parent->retuned = true;
    parent->lastTune.start();
    return true;
  }
  else
    return false;
}

bool DVBTuner::frequencyInfo(quint64 &low, quint64 &high, quint64 &step) const
{
  low  =  40000000;
  high = 900000000;
  step =    250000;

  return true;
}

STuner::Status DVBTuner::signalStatus(void) const
{
  Status status;
  memset(&status, 0, sizeof(status));
  uint16_t strength = 0;
  uint16_t snratio = 0;
  uint32_t berate = 0;
  fe_status_t feStatus;

  if (ioctl(parent->frontendDesc, FE_READ_STATUS, &feStatus) >= 0)
  {
    status.hasSignal = (feStatus & FE_HAS_SIGNAL) != 0;
    status.hasCarrier = (feStatus & FE_HAS_CARRIER) != 0;
    status.hasSync = (feStatus & FE_HAS_SYNC) != 0;
    status.hasLock = (feStatus & FE_HAS_LOCK) != 0;
  }

  if (ioctl(parent->frontendDesc, FE_READ_SIGNAL_STRENGTH, &strength) >= 0)
    status.signalStrength = qreal(strength) / 65535.0;

  if (ioctl(parent->frontendDesc, FE_READ_SNR, &snratio) >= 0)
    status.signalNoiseRatio = qreal(snratio) / 65535.0;

  if (ioctl(parent->frontendDesc, FE_READ_BER, &berate) >= 0)
    status.bitErrorRate = status.hasLock ? berate : 0;

  return status;
}

bool DVBTuner::hasLock(void) const
{
  fe_status_t feStatus;

  if (ioctl(parent->frontendDesc, FE_READ_STATUS, &feStatus) >= 0)
    return (feStatus & FE_HAS_LOCK) != 0;

  return false;
}


} } // End of namespaces
