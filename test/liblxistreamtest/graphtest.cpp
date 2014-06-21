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

#include "test.h"


/*! Tests the full audio chain from file to output using a manually created
    graph.

    \note This test depends on the FFMpegBackend being available to decode the
          file.
    \note The test sound "SoundTest.ogg" is public domain. It is a the chord of
          A major played on an acoustic guitar.
 */
void LXiStreamTest::Graph_ManualGraph(void)
{
  /*SGraph graph(SGraph::MediaTask_None);
  SFileInputNode fileSource;
  if (fileSource->open(":/SoundTest.ogg"));

  STerminals::File * const file = graph.createTerminal<STerminals::File>("file::/SoundTest.ogg");
  SNode * const fileSource = graph.openStream(file, file->inputStream(0));

  SAudioDecoderNode * const audioDecoder = new SAudioDecoderNode();
  graph.registerNode(audioDecoder);
  SNodes::Audio::Resampler * const audioResampler = graph.createNode<SNodes::Audio::Resampler>();

  TestNode *testNode = new TestNode(this);
  graph.registerNode(testNode);

  QVERIFY(fileSource->prepare());
  QVERIFY(audioDecoder->prepare());
  QVERIFY(audioResampler->prepare(SCodecList() << SAudioCodec(SAudioCodec::Format_PCM_S16LE, SAudioCodec::Channel_Stereo, 16000)));
  QVERIFY(testNode->prepare(SCodecList()));

  SBufferList list;
  while (fileSource->processBuffer(SBuffer(), list) == SNode::Result_Active)
  {
    foreach (const SBuffer &b1, list)
    {
      SBufferList decoded;
      if (audioDecoder->processBuffer(b1, decoded) == SNode::Result_Active)
      foreach (const SBuffer &b2, decoded)
      {
        SBufferList resampled, dummy;
        if (audioResampler->processBuffer(b2, resampled) == SNode::Result_Active)
        foreach (const SBuffer &b3, resampled)
          testNode->processBuffer(b3, dummy);
      }
    }

    list.clear();
  }

  QVERIFY(fileSource->unprepare());
  QVERIFY(audioDecoder->unprepare());
  QVERIFY(audioResampler->unprepare());
  QVERIFY(testNode->unprepare());

  // Sink should have processed all buffers
  Graph_NumBuffers = testNode->buffersProcessed();
  QVERIFY(Graph_NumBuffers >= 40);

  delete audioDecoder;
  delete testNode;*/
}

/*! Tests the full audio chain from file to output using a managed graph.

    \note This test depends on the FFMpegBackend being available to decode the
          file.
    \note The test sound "SoundTest.ogg" is public domain. It is a the chord of
          A major played on an acoustic guitar.
 */
void LXiStreamTest::Graph_ManagedGraph(void)
{
  /*SGraph graph(SGraph::MediaTask_None);
  STerminals::File * const file = graph.createTerminal<STerminals::File>("file::/SoundTest.ogg");
  SNode * const fileSource = graph.openStream(file, file->inputStream(0));
  SAudioDecoderNode * const audioDecoder = new SAudioDecoderNode();
  graph.registerNode(audioDecoder);
  SNodes::Audio::Resampler * const audioResampler = graph.createNode<SNodes::Audio::Resampler>();

  TestNode *testNode = new TestNode(this);
  graph.registerNode(testNode);

  audioResampler->setSampleRate(8000);

  // Now connect
  graph.connectNodes(fileSource, audioDecoder);
  graph.connectNodes(audioDecoder, audioResampler);
  graph.connectNodes(audioResampler, testNode);

  // And execute
  QVERIFY(graph.prepare());
  graph.start();
  QVERIFY(graph.wait(10000));
  QVERIFY(graph.unprepare());

  // Sink should have processed all buffers.
  QCOMPARE(testNode->buffersProcessed(), Graph_NumBuffers);

  graph.unregisterNode(audioDecoder);
  delete audioDecoder;
  graph.unregisterNode(testNode);
  delete testNode;*/
}

/*! Tests the full audio chain from file to output using a managed graph in
    single shot mode.

    \note This test depends on the FFMpegBackend being available to decode the
          file.
    \note The test sound "SoundTest.ogg" is public domain. It is a the chord of
          A major played on an acoustic guitar.
 */
void LXiStreamTest::Graph_ManagedGraphSingleShot(void)
{
  /*SGraph graph(SGraph::MediaTask_None);
  STerminals::File * const file = graph.createTerminal<STerminals::File>("file::/SoundTest.ogg");
  SNode * const fileSource = graph.openStream(file, file->inputStream(0));
  SAudioDecoderNode * const audioDecoder = new SAudioDecoderNode();
  graph.registerNode(audioDecoder);
  SNodes::Audio::Resampler * const audioResampler = graph.createNode<SNodes::Audio::Resampler>();

  TestNode *testNode = new TestNode(this);
  graph.registerNode(testNode);

  audioResampler->setSampleRate(8000);

  // Now connect
  graph.connectNodes(fileSource, audioDecoder);
  graph.connectNodes(audioDecoder, audioResampler);
  graph.connectNodes(audioResampler, testNode);

  // And execute
  QVERIFY(graph.prepare());
  // Two times since the audio decoder needs two buffers for one output.
  graph.start(true);
  graph.start(true);
  QVERIFY(graph.wait(10000));
  QVERIFY(graph.unprepare());

  // Sink should have processed exactly one buffer
  QCOMPARE(testNode->buffersProcessed(), 1u);

  graph.unregisterNode(audioDecoder);
  delete audioDecoder;
  graph.unregisterNode(testNode);
  delete testNode;*/
}
