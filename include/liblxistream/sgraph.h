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

#ifndef LXSTREAM_SGRAPH_H
#define LXSTREAM_SGRAPH_H

#include <QtCore>
#include <LXiCore>
#include "export.h"

namespace LXiStream {

class STimer;

class LXISTREAM_PUBLIC SGraph : public QThread
{
Q_OBJECT
public:
  /*! The Node abstract class is used for processing nodes.
   */
  class LXISTREAM_PUBLIC Node
  {
  public:
    explicit                      Node(SGraph *);
    virtual                       ~Node();

    /*! This method shall be invoked when the node is about to start processing
        data.
     */
    virtual bool                  start(void) = 0;

    /*! This method shall be invoked when the node is finished processing data.

        \note This method may be invoked multiple times.
     */
    virtual void                  stop(void) = 0;
  };

  /*! The SinkNode abstract class is used for sink nodes.
   */
  class LXISTREAM_PUBLIC SinkNode
  {
  public:
    explicit                      SinkNode(SGraph *);
    virtual                       ~SinkNode();

    /*! This method shall be invoked when the node is about to start processing
        data.
     */
    virtual bool                  start(STimer *) = 0;

    /*! This method shall be invoked when the node is finished processing data.

        \note This method may be invoked multiple times.
     */
    virtual void                  stop(void) = 0;
  };

  /*! The SourceNode abstract class is used for source nodes.
   */
  class LXISTREAM_PUBLIC SourceNode
  {
  public:
    explicit                      SourceNode(SGraph *);
    virtual                       ~SourceNode();

    /*! This method shall be invoked when the node is about to start processing
        data.
     */
    virtual bool                  start(void) = 0;

    /*! This method shall be invoked when the node is finished processing data.

        \note This method may be invoked multiple times.
     */
    virtual void                  stop(void) = 0;

    /*! This method shall be invoked to indicate the node has to produce data.
     */
    virtual void                  process(void) = 0;
  };

public:
  explicit                      SGraph(void);
  virtual                       ~SGraph();

  static bool                   connect(const QObject *, const char *, const QObject *, const char *);
  bool                          connect(const QObject *, const char *, const char *) const;

  bool                          isRunning(void) const;

  void                          addNode(Node *);
  void                          addNode(SourceNode *);
  void                          addNode(SinkNode *);

public slots:
  virtual bool                  start(void);
  virtual void                  stop(void);

protected: // From QThread and QObject
  virtual void                  run(void);
  virtual void                  customEvent(QEvent *);

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

Q_DECLARE_INTERFACE(LXiStream::SGraph::Node, "nl.dds.admiraal.www.LXiStream.SGraph.Node/1.0")
Q_DECLARE_INTERFACE(LXiStream::SGraph::SinkNode, "nl.dds.admiraal.www.LXiStream.SGraph.SinkNode/1.0")
Q_DECLARE_INTERFACE(LXiStream::SGraph::SourceNode, "nl.dds.admiraal.www.LXiStream.SGraph.SourceNode/1.0")

#endif
