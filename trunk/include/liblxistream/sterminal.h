/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef LXSTREAM_STERMINAL_H
#define LXSTREAM_STERMINAL_H

#include <QtCore>

namespace LXiStream {

class SNode;
class STuner;


/*! A node is responsible for managing a device, network port, etc. A terminal
    can have inputs, outputs, a tuner and provide several input and output
    streams at once. The main difference between an input and an input stream is
    that of all inputs, only one can be active at a time. However, of all
    streams, multiple or even all can be active at the same time. The same
    applies to output and output stream.<br>
    <br>
    For example a video capture device can have several inputs; television
    input, S-Video input and a composite input. It usually has no outputs. a
    video input device usually has one input stream; the video stream. A DVB
    television tuner usually has onlu one input, but several input streams; one
    for each channel on the tuned transponder.
 */
class STerminal : public QObject
{
Q_OBJECT
friend class SSystem;
public:
  /*! This structure describes an input or output stream, it is returned by
      STerminal::inputStreams() and STerminal::outputStreams().
   */
  struct Stream
  {
    inline Stream(void) : serviceID(0) { }

    /*! A user-readable name for this stream. It is not mandatory for
        this name to be unique, but is is preferable.
     */
    QString                     name;

    /*! A user-readable name for the provider of this stream. If this name is
        not applicable, it may be left empty.
     */
    QString                     provider;

    /*! A service identifier for this stream, or 0 if not applicable.
     */
    quint64                     serviceID;

    /*! A list of audio packet IDs to be expected in the stream. This is
        especially conveniant if the stream provides multiple audio substreams.
        This list may be left empty if this information is not known.
     */
    QVector<quint16>            audioPacketIDs;

    /*! A list of video packet IDs to be expected in the stream. This is
        especially conveniant if the stream provides multiple video substreams.
        This list may be left empty if this information is not known.
     */
    QVector<quint16>            videoPacketIDs;

    /*! A list of data packet IDs to be expected in the stream. This is
        especially conveniant if the stream provides multiple data substreams.
        This list may be left empty if this information is not known.
     */
    QVector<quint16>            dataPacketIDs;

    /*! An opaque value for internal use by the terminal
     */
    QVariant                    opaque;
  };

  /*! This enumeration is used to describe the type of terminal. Values can be
      combined with the | operator.
   */
  enum Type
  {
    Type_None                 = 0x0000,

    Type_Audio                = 0x0001,
    Type_AnalogAudio          = 0x0002 | Type_Audio,
    Type_DigitalAudio         = 0x0004 | Type_Audio,
    Type_Radio                = 0x0008,
    Type_AnalogRadio          = Type_Radio | Type_AnalogAudio,
    Type_DigitalRadio         = Type_Radio | Type_DigitalAudio,

    Type_Video                = 0x0010,
    Type_AnalogVideo          = 0x0020 | Type_Video,
    Type_DigitalVideo         = 0x0040 | Type_Video,
    Type_Television           = 0x0080,
    Type_AnalogTelevision     = Type_Television | Type_AnalogVideo,
    Type_DigitalTelevision    = Type_Television | Type_DigitalVideo | Type_DigitalAudio,

    Type_File                 = 0x0100,
    Type_Network              = 0x0200
  };
  Q_DECLARE_FLAGS(Types, Type)

public:
  explicit                      STerminal(QObject *);

public: // Making Qt property system virtual
  virtual bool                  setProperty(const char *name, const QVariant &);
  virtual QVariant              property(const char *name) const;

protected:
  /*! This method should actually open the underlying terminal, it is invoked
      directly after creating an instance and before returning a pointer to this
      instance to the user. During terminal creation, multiple objects may be
      created, the first one to return true from this method will be used as the
      terminal for the specified URL.
   */
  virtual bool                  open(const QUrl &) = 0;

public:
  /*! Returns a user-readable name for this terminal. It is not mandatory for
      this name to be unique, but is is preferable.

      \note For example for files this method would return the filename without
            the path.
   */
  virtual QString               friendlyName(void) const = 0;

  /*! Returns a complete name for this terminal, containing the device name,
      driver name and bus id if available. It mandatory for this name to be
      unique, therefore ensure that at either a bus id, I/O addresses or IRQ
      number is part of it.

      \note This name is not intended for the user, but to identify related
            components such as the sound device belonging to an analog video
            device.
      \note If no additional information is available; return friendlyName().
      \note For example for files this method would return the full path of the
            file.
   */
  virtual QString               longName(void) const = 0;

  /*! Should return one or more of the types described in Type.
   */
  virtual Types                 terminalType(void) const = 0;

  /*! Provides a list of available inputs for this terminal, this terminal has
      no inputs when an empty list is returned. The default implementation
      returns a list with only QString::null to indicate that this terminal has
      only one input. In this case, the user does not have to invoke
      selectInput(). The default implementation does not have to be invoked from
      an overriding implementation.
   */
  virtual QStringList           inputs(void) const;

  /*! Selects and activates one of the inputs. The default implementation
      simply returns true. The default implementation does not have to be
      invoked from an overriding implementation.
   */
  virtual bool                  selectInput(const QString &);

  /*! Provides a list of available outputs for this terminal, this terminal has
      no outputs when an empty list is returned. The default implementation
      returns a list with only QString::null to indicate that this terminal has
      only one output. In this case, the user does not have to invoke
      selectOutput(). The default implementation does not have to be invoked
      from an overriding implementation.
   */
  virtual QStringList           outputs(void) const;

  /*! Selects and activates one of the outputs. The default implementation
      simply returns true. The default implementation does not have to be
      invoked from an overriding implementation.
   */
  virtual bool                  selectOutput(const QString &);

  /*! Returns a pointer to an instance of a tuner for this terminal or NULL if
      the selected input or output of this terminal does not have a tuner. The
      default implementation returns NULL. The default implementation does not
      have to be invoked from an overriding implementation.

      \note The result of this method may depend on the selected input or
            output.
   */
  virtual STuner              * tuner(void) const;

  /*! Provides a list of available input streams for this terminal. To create
      one of the streams; provide Stream::node to SGraph::createNode() and a
      node is created that provides the data for this stream.

      \note The result of this method may depend on the selected input.
   */
  virtual QList<Stream>         inputStreams(void) const = 0;

  /*! Returns s stream with the specified serviceID. The default implementation
      searches for a stream in the list returned by inputStreams() and returns
      the first stream with the specified service ID. An empty Stream object is
      returned if a stream cannot be found.

      \note The result of this method may depend on the selected input.
   */
  virtual Stream                inputStream(quint64 serviceID) const;

  /*! Provides a list of available output streams for this terminal. To create
      one of the streams; provide Stream::node to SGraph::createNode() and a
      node is created that takes the data for this stream.

      \note The result of this method may depend on the selected output.
   */
  virtual QList<Stream>         outputStreams(void) const = 0;

  /*! Returns s stream with the specified serviceID. The default implementation
      searches for a stream in the list returned by outputStreams() and returns
      the first stream with the specified service ID. An empty Stream object is
      returned if a stream cannot be found.

      \note The result of this method may depend on the selected input.
   */
  virtual Stream                outputStream(quint64 serviceID) const;

  /*! Opens the specified stream and returns the source or sink node.
   */
  virtual SNode               * openStream(const Stream &) = 0;
};


} // End of namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::STerminal::Types)

#endif
