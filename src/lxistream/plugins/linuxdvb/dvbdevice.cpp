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

#include "dvbdevice.h"
#include <sys/poll.h>
#include "dvbinput.h"
#include "dvbtuner.h"

// AleVT functions
extern "C"
{
  // Language parsing
  void lang_init(void);
}

namespace LXiStream {
namespace LinuxDvbBackend {

const QEvent::Type  DVBDevice::retuneEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  DVBDevice::processDemuxEventType = QEvent::Type(QEvent::registerEventType());

QMap<QString, int>  DVBDevice::foundDevices;
bool                DVBDevice::initialized = false;
quint8              DVBDevice::reversedBits[256];


SSystem::DeviceEntryList DVBDevice::listDevices(void)
{
  if (!initialized)
  {
    initialized = true;

    // Initialize AleVT
    lang_init();

    // Reverse bits
    for (unsigned i=0; i<256; i++)
    {
      reversedBits[i] = 0;

      for (unsigned j=0; j<8; j++)
      if ((i & (1 << j)) != 0)
        reversedBits[i] |= (1 << (7 - j));
    }
  }

  foundDevices.clear();
  SSystem::DeviceEntryList result;

  for (unsigned i=0; i<=31; i++)
  {
    int fd = ::open(("/dev/dvb/adapter" + QString::number(i) + "/frontend0").toAscii(), O_RDONLY);
    if (fd < 0)
      break; // last device

    dvb_frontend_info frontendInfo;
    memset(&frontendInfo, 0, sizeof(frontendInfo));

    if (ioctl(fd, FE_GET_INFO, &frontendInfo) >= 0)
    {
      const QString url = "lx-linuxdvb://" + SStringParser::toRawName((const char *)frontendInfo.name).toLower();

      foundDevices[url] = i;
      result += SSystem::DeviceEntry(0, frontendInfo.name, url);
    }

    close(fd);
  }

  return result;
}

DVBDevice::DVBDevice(QObject *parent)
   : STerminals::AudioVideoDevice(parent),
     outputAudio(true),
     outputVideo(true),
     running(true),
     adapterID(0),
     mutex(QMutex::Recursive),
     thread(this),
     frontendDesc(0),
     demuxDesc(0),
     dvrDesc(0),
     retuned(true),
     lastTune(),
     tsPacketStream(this, (Common::MPEG::TSPacketStream::TSCallback)&DVBDevice::tsPacketReceived),
     tunerDev(NULL)
{
  memset(&frontendInfo, 0, sizeof(frontendInfo));

  for (unsigned i=0; i<numStreams; i++)
    inputDev[i] = NULL;

  for (unsigned i=0; i<=Common::MPEG::maxPID; i++)
    pesPacketStream[i] = NULL;

  lastTune.start();
}

DVBDevice::~DVBDevice()
{
  running = false;
  thread.wait();

  if (frontendDesc > 0) ::close(frontendDesc);
  if (demuxDesc > 0)    ::close(demuxDesc);
  if (dvrDesc > 0)      ::close(dvrDesc);
}

bool DVBDevice::open(const QUrl &url)
{
  const QString name = "lx-linuxdvb://" + url.host().toLower();

  if (foundDevices.contains(name))
  {
    adapterID = foundDevices[name];

    frontendDesc  = ::open(("/dev/dvb/adapter" + QString::number(adapterID) + "/frontend0").toAscii(), O_RDWR);
    demuxDesc     = ::open(("/dev/dvb/adapter" + QString::number(adapterID) + "/demux0").toAscii(), O_RDWR | O_NONBLOCK);
    dvrDesc       = ::open(("/dev/dvb/adapter" + QString::number(adapterID) + "/dvr0").toAscii(), O_RDONLY | O_NONBLOCK);

    if (frontendDesc > 0)
    if (ioctl(frontendDesc, FE_GET_INFO, &frontendInfo) < 0)
      memset(&frontendInfo, 0, sizeof(frontendInfo));

    if ((demuxDesc > 0) && (dvrDesc > 0) && (frontendInfo.name[0] != 0))
    {
      tunerDev = new DVBTuner(this);
      thread.start();
      return true;
    }
  }

  return false;
}

QString DVBDevice::friendlyName(void) const
{
  return (const char *)frontendInfo.name;
}

QString DVBDevice::longName(void) const
{
  return friendlyName();
}

STerminal::Types DVBDevice::terminalType(void) const
{
  return Type_DigitalTelevision | Type_DigitalRadio;
}

QStringList DVBDevice::inputs(void) const
{
  return QStringList() << "Television";
}

bool DVBDevice::selectInput(const QString &)
{
  return true;
}

STuner * DVBDevice::tuner(void) const
{
  return tunerDev;
}

QList<STerminal::Stream> DVBDevice::inputStreams(void) const
{
  QList<STerminal::Stream> result;

  foreach (const Channel &channel, channels)
  {
    STerminal::Stream stream;
    stream.name = channel.name;
    stream.provider = channel.provider;
    stream.serviceID = channel.serviceID;
    if (channel.videoPID > 0) stream.videoPacketIDs += channel.videoPID;
    if (channel.ac3PID > 0) stream.audioPacketIDs += channel.ac3PID;
    stream.audioPacketIDs += channel.audioPIDs;
    stream.dataPacketIDs += channel.teletextPID;
    stream.dataPacketIDs += channel.subtitlesPID;

    result += stream;
  }

  return result;
}

STerminal::Stream DVBDevice::inputStream(quint64 serviceID) const
{
  foreach (const STerminal::Stream &stream, inputStreams())
  if (stream.serviceID == serviceID)
    return stream;

  // Not found yet, but it may not have been detected yet, return a dummy
  // stream with the correct service ID.
  STerminal::Stream stream;
  stream.serviceID = serviceID;

  return stream;
}

QList<STerminal::Stream> DVBDevice::outputStreams(void) const
{
  return QList<STerminal::Stream>();
}

SNode * DVBDevice::openStream(const STerminal::Stream &stream)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  for (unsigned i=0; i<numStreams; i++)
  if (inputDev[i] == NULL)
    return inputDev[i] = new DVBInput(this, i, stream.serviceID);

  return NULL;
}

bool DVBDevice::setFilter(int demuxDesc, const Filter &filter)
{
  // Scan DVB content
  dmx_sct_filter_params fpara;
  memset(&fpara, 0, sizeof(fpara));
  fpara.pid = filter.pid;
  fpara.timeout = filter.timeout;
  fpara.flags = DMX_IMMEDIATE_START | DMX_CHECK_CRC;

  if (filter.tid < 0x100)
  {
    fpara.filter.filter[0] = filter.tid;
    fpara.filter.mask[0] = 0xFF;
  }

  if (filter.tidExt < 0x10000)
  {
    fpara.filter.filter[1] = filter.tidExt >> 8;
    fpara.filter.filter[2] = filter.tidExt & 0xFF;
    fpara.filter.mask[1] = 0xFF;
    fpara.filter.mask[2] = 0xFF;
  }

  return ioctl(demuxDesc, DMX_SET_FILTER, &fpara) >= 0;
}

void DVBDevice::customEvent(QEvent *e)
{
  if (e->type() == processDemuxEventType)
  {
    SDebug::Trace t("DVBDevice::ProcessDemuxEvent");
    ProcessDemuxEvent * const event = static_cast<ProcessDemuxEvent *>(e);

    if (event->data)
      parseSection(event->data);

    if (filters.count() > 0)
      setFilter(demuxDesc, filters.dequeue());
    else // Finished
      ioctl(demuxDesc, DMX_STOP);

    event->sem->release();
  }
  else if (e->type() == retuneEventType)
  {
    SDebug::Trace t("DVBDevice::RetuneEvent");
    RetuneEvent * const event = static_cast<RetuneEvent *>(e);

    if (tunerDev->hasLock())
    {
      channels.clear();

      filters.clear();
      filters.enqueue(Filter(0x00, 0x00, 0xFFFFFFFF, 0));
      filters.enqueue(Filter(0x11, 0x42, 0xFFFFFFFF, 0));

      setFilter(demuxDesc, filters.dequeue());
      retuned = false;
    }

    event->sem->release();
  }
  else
    STerminals::AudioVideoDevice::customEvent(e);
}

void DVBDevice::tsPacketReceived(const Common::MPEG::TSPacket *tsPacket)
{
  const quint16 pid = tsPacket->getPID();

  if (pid <= Common::MPEG::maxPID)
  {
    SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

    Common::MPEG::PESPacketStream * const pes = pesPacketStream[pid];

    if (pes)
      pes->processPacket(tsPacket);
  }
}

dmx_pes_type_t DVBDevice::getPESType(dmx_pes_type_t pesType, unsigned number)
{
  if (number == 0)
  {
    switch (pesType)
    {
    case DMX_PES_AUDIO:     return DMX_PES_AUDIO0;
    case DMX_PES_VIDEO:     return DMX_PES_VIDEO0;
    case DMX_PES_TELETEXT:  return DMX_PES_TELETEXT0;
    case DMX_PES_SUBTITLE:  return DMX_PES_SUBTITLE0;
    case DMX_PES_PCR:       return DMX_PES_PCR0;
    default:                return DMX_PES_OTHER;
    }
  }
  else if (number == 1)
  {
    switch (pesType)
    {
    case DMX_PES_AUDIO:     return DMX_PES_AUDIO1;
    case DMX_PES_VIDEO:     return DMX_PES_VIDEO1;
    case DMX_PES_TELETEXT:  return DMX_PES_TELETEXT1;
    case DMX_PES_SUBTITLE:  return DMX_PES_SUBTITLE1;
    case DMX_PES_PCR:       return DMX_PES_PCR1;
    default:                return DMX_PES_OTHER;
    }
  }
  else if (number == 2)
  {
    switch (pesType)
    {
    case DMX_PES_AUDIO:     return DMX_PES_AUDIO2;
    case DMX_PES_VIDEO:     return DMX_PES_VIDEO2;
    case DMX_PES_TELETEXT:  return DMX_PES_TELETEXT2;
    case DMX_PES_SUBTITLE:  return DMX_PES_SUBTITLE2;
    case DMX_PES_PCR:       return DMX_PES_PCR2;
    default:                return DMX_PES_OTHER;
    }
  }
  else if (number == 3)
  {
    switch (pesType)
    {
    case DMX_PES_AUDIO:     return DMX_PES_AUDIO3;
    case DMX_PES_VIDEO:     return DMX_PES_VIDEO3;
    case DMX_PES_TELETEXT:  return DMX_PES_TELETEXT3;
    case DMX_PES_SUBTITLE:  return DMX_PES_SUBTITLE3;
    case DMX_PES_PCR:       return DMX_PES_PCR3;
    default:                return DMX_PES_OTHER;
    }
  }

  return DMX_PES_OTHER;
}

bool DVBDevice::findDescriptor(quint8 descTag, const uchar *descList, unsigned descListLen)
{
  while (descListLen > 0)
  {
    const unsigned char tag = descList[0];
    unsigned len = unsigned(descList[1]) + 2;

    if (len > descListLen)
      break;

    if (tag == descTag)
      return true;

    descList += len;
    descListLen -= len;
  }

  return false;
}

void DVBDevice::parseSection(const uchar *section)
{
  const unsigned tableID = section[0];
  const unsigned length = (unsigned(section[1] & 0x0f) << 8) | unsigned(section[2]);
  const unsigned tableIDext = (unsigned(section[3]) << 8) | unsigned(section[4]);
  //const unsigned version = (section[5] >> 1) & 0x1f;
  //const unsigned number = section[6];
  //const unsigned lastNumber = section[7];

  if (length >= 8)
  switch (tableID)
  {
  case 0x00:
    { // PAT packet
      const unsigned char *pat = section + 8; // past generic table header
      unsigned patLen = length - (5 + 4); // header + crc

      while (patLen > 0)
      {
        const unsigned serviceID = (unsigned(pat[0]) << 8) | unsigned(pat[1]);

        if (serviceID > 0)
          filters.enqueue(Filter((unsigned(pat[2] & 0x1f) << 8) | unsigned(pat[3]), 0x02, serviceID, 0));

        pat += 4;
        patLen -= 4;
      }
    }
    break;

  case 0x02:
    parsePMT(section + 8, length - (5 + 4), &channels[tableIDext]);
    break;

  case 0x42:
    parseSDT(section + 11, length - (5 + 4));
    break;
  }
}

void DVBDevice::parsePMT(const uchar *pmt, unsigned pmtLen, Channel *channel)
{
  //const unsigned pcrPID = (unsigned(pmt[0] & 0x1f) << 8) | unsigned(pmt[1]);
  const unsigned programInfoLen = (unsigned(pmt[2] & 0x0f) << 8) | unsigned(pmt[3]);

  pmt += programInfoLen + 4;
  pmtLen -= programInfoLen + 4;

  while (pmtLen >= 5)
  {
    unsigned esInfoLen = (unsigned(pmt[3] & 0x0f) << 8) | unsigned(pmt[4]);
    unsigned elmPID    = (unsigned(pmt[1] & 0x1f) << 8) | unsigned(pmt[2]);

    switch (pmt[0])
    {
    case 0x01:
    case 0x02:
      channel->videoPID = elmPID;
      break;

    case 0x03:
    case 0x81: // Audio per ATSC A/53B [2] Annex B
    case 0x04:
      channel->audioPIDs += elmPID;
      break;

    case 0x06:
      if (findDescriptor(0x56, pmt + 5, esInfoLen))
        channel->teletextPID = elmPID;

      else if (findDescriptor(0x59, pmt + 5, esInfoLen))
        channel->subtitlesPID = elmPID;

      else if (findDescriptor(0x6a, pmt + 5, esInfoLen))
        channel->ac3PID = elmPID;

      break;
    };

    pmt += esInfoLen + 5;
    pmtLen -= esInfoLen + 5;
  }
}

void DVBDevice::parseSDT(const uchar *sdt, unsigned sdtLen)
{
  while (sdtLen >= 5)
  {
    const unsigned serviceID = (unsigned(sdt[0]) << 8) | unsigned(sdt[1]);
    unsigned descLoopLen = (unsigned(sdt[3] & 0x0f) << 8) | unsigned(sdt[4]);
    Channel * const channel = &channels[serviceID];
    channel->serviceID = serviceID;

    // Parse the descriptors
    if (channel && (sdtLen >= descLoopLen))
    {
      const unsigned char *desc = sdt + 5;
      unsigned descLen = descLoopLen;

      while (descLen > 0)
      {
        unsigned char tag = desc[0];
        unsigned len = unsigned(desc[1]) + 2;

        if (len > 0)
        switch (tag) {
        case 0x0a:
          //if (t == PMT)
          //  parse_iso639_language_descriptor (buf, data);
          break;

        case 0x40:
          //if (t == NIT)
          //  parse_network_name_descriptor (buf, data);
          break;

        case 0x43:
          //if (t == NIT)
          //  parse_satellite_delivery_system_descriptor (buf, data);
          break;

        case 0x44:
          //if (t == NIT)
          //  parse_cable_delivery_system_descriptor (buf, data);
          break;

        case 0x48: // Service descriptor
          parseServiceDesc(desc, len, channel);
          break;

        case 0x53:
          //if (t == SDT)
          //  parse_ca_identifier_descriptor (buf, data);
          break;

        case 0x5a:
          //if (t == NIT)
          //  parse_terrestrial_delivery_system_descriptor (buf, data);
          break;

        case 0x62:
          //if (t == NIT)
          //  parse_frequency_list_descriptor (buf, data);
          break;
        };

        desc += len;
        descLen -= len;
      }
    }

    sdtLen -= descLoopLen + 5;
    sdt += descLoopLen + 5;
  }
}

void DVBDevice::parseServiceDesc(const uchar *desc, unsigned descLen, Channel *channel)
{
  const unsigned char * const provider = desc + 4;
  const unsigned providerLen = desc[3];

  if (providerLen <= descLen - 4)
  {
    channel->provider = QString::fromUtf8(reinterpret_cast<const char *>(provider), providerLen).trimmed();

    const unsigned char * const service = provider + providerLen + 1;
    const unsigned serviceLen = provider[providerLen];

    if (serviceLen <= descLen - (providerLen + 5))
      channel->name = QString::fromUtf8(reinterpret_cast<const char *>(service), serviceLen).trimmed();
  }
}


void DVBDevice::Thread::run(void)
{
  QSemaphore sem;
  int len = 0;
  union
  {
    quint8 buffer[64 * Common::MPEG::tsPacketSize];
    quint8 packet[1024];
  };

  while (parent->running)
  {
    if (parent->retuned)
    {
      QCoreApplication::postEvent(parent, new RetuneEvent(&sem));
      while (!sem.tryAcquire(1, 250))
      if (!parent->running)
        return;
    }

    while (!parent->retuned && parent->running)
    {
      pollfd pfd[2];
      pfd[0].fd = parent->dvrDesc;
      pfd[0].events = POLLIN;
      pfd[1].fd = parent->demuxDesc;
      pfd[1].events = POLLIN;

      if (poll(pfd, 2, 40) > 0)
      {
        if ((pfd[0].revents & POLLIN) != 0)
        if ((len = read(pfd[0].fd, buffer, sizeof(buffer))) > 0)
          parent->tsPacketStream.processData(buffer, len);

        if ((pfd[1].revents & POLLIN) != 0)
        { // A channel info packet has been received
          if (read(pfd[1].fd, packet, sizeof(packet)) > 0)
            QCoreApplication::postEvent(parent, new ProcessDemuxEvent(&sem, packet));
          else
            QCoreApplication::postEvent(parent, new ProcessDemuxEvent(&sem, NULL));

          while (!sem.tryAcquire(1, 250))
          if (!parent->running)
            return;
        }
      }
    }
  }
}


} } // End of namespaces
