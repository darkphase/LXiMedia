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

#include "teletextserver.h"
#include "epgdatabase.h"
#include "televisionbackend.h"
#include "televisionserver.h"

namespace LXiMediaCenter {

class TeletextServer::DataNode : public SNode
{
public:
  inline DataNode(TeletextServer *parent, const QString &channelName, QObject *parentObject)
      : SNode(Behavior_Routing, SDataBuffer::baseTypeId, SDataCodec::Format_TeletextPage, parentObject),
        parent(parent), channelName(channelName), pages(NULL)
  {
  }

public: // From SNode
  virtual bool                  prepare(const SCodecList &);
  virtual bool                  unprepare(void);
  virtual Result                processBuffer(const SBuffer &, SBufferList &);

private:
  TeletextServer        * const parent;
  const QString                 channelName;
  QMap<quint32, SDataBuffer::TeletextPage> * pages;
};


TeletextServer::TeletextServer(EpgDatabase *epgDatabase, MasterServer *server, Plugin *plugin)
               :BackendServer(QT_TR_NOOP("Teletext"), server),
                epgDatabase(epgDatabase),
                plugin(plugin),
                head()
{
  head = " <link rel=\"stylesheet\" href=\"" + httpPath().toUtf8() + "teletext.css\" "
         "type=\"text/css\" media=\"screen, handheld, projection\" />";

  // Create tables that don't exist
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
  QSqlQuery query(Database::database());

  query.exec("CREATE TEMP TABLE IF NOT EXISTS TelevisionTeletextPages ("
             "pageNumber     INTEGER NOT NULL,"
             "subpageNumber  INTEGER NOT NULL,"
             "rawChannel     TEXT NOT NULL,"
             "pageDate       DATE NOT NULL,"
             "pageData       TEXT NOT NULL,"
             "pageRawText    TEXT,"
             "PRIMARY KEY(pageNumber, subpageNumber, rawChannel))");

  query.exec("CREATE INDEX IF NOT EXISTS TelevisionTeletextPages_pageNumber "
             "ON TelevisionTeletextPages(pageNumber)");

  query.exec("CREATE INDEX IF NOT EXISTS TelevisionTeletextPages_subpageNumber "
             "ON TelevisionTeletextPages(subpageNumber)");

  query.exec("CREATE INDEX IF NOT EXISTS TelevisionTeletextPages_rawChannel "
             "ON TelevisionTeletextPages(rawChannel)");

  dl.unlock();

  connect(&cleanTimer, SIGNAL(timeout()), SLOT(cleanDatabase()));
  cleanTimer.start(3600000);
}

TeletextServer::~TeletextServer()
{
  QThreadPool::globalInstance()->waitForDone();
}

BackendServer::SearchResultList TeletextServer::search(const QStringList &queryText) const
{
  PluginSettings settings(plugin);

  SearchResultList results;
  QMap<QString, QString> channelNames;

  QString qs;
  foreach (const QString &item, queryText)
    qs += " AND pageRawText LIKE '%" + SStringParser::toRawName(item) + "%'";

  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
  QSqlQuery query(Database::database());
  query.exec("SELECT pageNumber, subpageNumber, rawChannel, pageData, pageRawText FROM TelevisionTeletextPages "
             "WHERE " + qs.mid(4));
  while (query.next())
  {
    const qreal match =
        SStringParser::computeMatch(query.value(4).toString(), queryText);

    if (match >= minSearchRelevance)
    {
      const QString rawChannel = query.value(2).toString();
      const QString channelName = epgDatabase->getChannelName(rawChannel, &channelNames);

      SearchResult result;
      result.relevance = match;
      result.headline = channelName + " P" + QString::number(query.value(0).toInt(), 16) + " (" + tr("Teletext page") + ")";
      result.location = query.value(2).toString().toAscii() + ".html"
                        "?page=" + QByteArray::number(query.value(0).toInt(), 16) +
                        "&subpage=" + QByteArray::number(query.value(1).toInt());

      const QByteArray data = QByteArray::fromBase64(query.value(3).toByteArray());
      if (data.length() >= int(sizeof(SDataBuffer::TeletextPage)))
      {
        int addLine = 0;
        QString lastLine;
        foreach (const QString &line, reinterpret_cast<const SDataBuffer::TeletextPage *>(data.data())->decodedLines())
        {
          foreach (const QString &q, queryText)
          if (SStringParser::toRawName(line).contains(SStringParser::toRawName(q)))
          {
            addLine = 2;
            break;
          }

          if (addLine > 0)
          {
            result.text += " " + lastLine + " " + line;
            lastLine = "";
            addLine--;
          }
          else
            lastLine = line;
        }

        result.text = result.text.simplified();
      }

      results += result;
    }
  }

  return results;
}

bool TeletextServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = url.path().mid(url.path().lastIndexOf('/') + 1);

  if (url.hasQueryItem("bookmark"))
  {
    PluginSettings settings(plugin);

    const QString rawChannel = SStringParser::toRawName(url.queryItemValue("channel"));
    const int page = url.queryItemValue("page").toInt(NULL, 16);
    const int bookmark = url.queryItemValue("bookmark").toInt();

    if ((bookmark >= -1) && (bookmark <= 9))
    foreach (const QString &group, settings.childGroups())
    if (group.startsWith(typeName(Type_Television) + "Channel_"))
    {
      settings.beginGroup(group);

      if (SStringParser::toRawName(settings.value("Name").toString()) == rawChannel)
      {
        for (int b=0; b<=9; b++)
        {
          QString newValue = "";
          QSet<int> newList;

          foreach (const QString &ps, settings.value("EPGTeletextPage" + QString::number(b)).toString().split(' '))
          {
            const int pn = ps.toInt(NULL, 16);
            if (pn != page)
            {
              newValue += ps + " ";
              newList += pn;
            }
          }

          if (b == bookmark)
          {
            newValue += QString::number(page, 16);
            newList += page;
          }

          newValue = newValue.trimmed();
          if (newValue.isEmpty())
            settings.remove("EPGTeletextPage" + QString::number(b));
          else
            settings.setValue("EPGTeletextPage" + QString::number(b), newValue);
        }
      }

      settings.endGroup();
    }

    return handleHtmlRequest(url, file, socket);
  }
  else if (file.endsWith(".png") && (file.length() >= 9))
  {
    static const QRgb ttcolors[] =
      { 0xFF000000, 0xFFFF0000, 0xFF00FF00, 0xFFFFFF00,
        0xFF0000FF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF };

    const unsigned fg = file.mid(3, 1).toUInt(NULL, 16);
    const unsigned bg = file.mid(4, 1).toUInt(NULL, 16);
    if ((fg < 8) && (bg < 8))
    {
      QImage image(12, 15, QImage::Format_RGB32);
      QPainter p;
      p.begin(&image);
        p.fillRect(image.rect(), QColor(ttcolors[bg]));
        p.setPen(QColor(ttcolors[fg]));

        Teletext::drawGraphics(p,
                               image.rect(),
                               file.mid(2, 1).toInt(NULL, 16),
                               file.mid(0, 2).toInt(NULL, 16));
      p.end();

      QByteArray pngData;
      QBuffer buffer(&pngData);
      buffer.open(QIODevice::WriteOnly);
      image.save(&buffer, "PNG");
      buffer.close();

      return sendReply(socket, buffer.data(), "image/png", true);
    }
  }
  else if (file.endsWith(".html") || file.endsWith(".css"))
  {
    return handleHtmlRequest(url, file, socket);
  }
  else if (file.isEmpty())
  {
    QHttpResponseHeader response(301);
    response.setValue("Location", "http://" + request.value("Host") + url.path() + "1.html");
    socket->write(response.toString().toUtf8());
    return false;
  }

  return BackendServer::handleConnection(request, socket);
}

bool TeletextServer::needsUpdate(const QString &channelName)
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT pageDate FROM TelevisionTeletextPages "
                "WHERE rawChannel = :rawChannel "
                "ORDER BY pageDate DESC LIMIT 1");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.exec();
  if (query.next())
    return query.value(0).toDateTime().secsTo(QDateTime::currentDateTime()) >= updateTime;

  return true;
}

SNode * TeletextServer::createTeletextNode(const QString &channelName, QObject *parent)
{
  return new DataNode(this, channelName, parent);
}

SDataBuffer::TeletextPage TeletextServer::readPage(const QString &channelName, int page, int subpage) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QString qs = "rawChannel = :rawChannel ";
  if (page >= 0)
  {
    qs += "AND pageNumber = :pageNumber ";
    if (subpage >= 0)
      qs += "AND subpageNumber = :subpageNumber ";
  }

  QSqlQuery query(Database::database());
  query.prepare("SELECT pageData FROM TelevisionTeletextPages "
                "WHERE " + qs +
                "ORDER BY pageNumber ASC, subpageNumber ASC, pageDate DESC LIMIT 1");
  query.bindValue(0, SStringParser::toRawName(channelName));
  if (page >= 0)
  {
    query.bindValue(1, page);
    if (subpage >= 0)
      query.bindValue(2, subpage);
  }
  query.exec();
  if (query.next())
  {
    const QByteArray data = QByteArray::fromBase64(query.value(0).toByteArray());
    if (data.length() >= int(sizeof(SDataBuffer::TeletextPage)))
      return *reinterpret_cast<const SDataBuffer::TeletextPage *>(data.data());
  }

  return SDataBuffer::TeletextPage();
}

QSet<int> TeletextServer::allPages(const QString &channelName) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSet<int> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT pageNumber FROM TelevisionTeletextPages "
                "WHERE rawChannel = :rawChannel");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.exec();
  while (query.next())
    result.insert(query.value(0).toInt());

  return result;
}

QSet<int> TeletextServer::allSubPages(const QString &channelName, int page) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSet<int> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT subpageNumber FROM TelevisionTeletextPages "
                "WHERE pageNumber = :pageNumber AND rawChannel = :rawChannel");
  query.bindValue(0, page);
  query.bindValue(1, SStringParser::toRawName(channelName));
  query.exec();
  while (query.next())
    result.insert(query.value(0).toInt());

  return result;
}

int TeletextServer::firstPage(const QString &channelName) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT pageNumber FROM TelevisionTeletextPages "
                "WHERE pageNumber > 0 AND rawChannel = :rawChannel "
                "ORDER BY pageNumber ASC LIMIT 1");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.exec();
  if (query.next())
    return query.value(0).toInt();

  return 0;
}

void TeletextServer::cleanDatabase(void)
{
  SDebug::Trace t("TeletextServer::cleanDatabase");

  const QDateTime oldDate = QDateTime::currentDateTime().addDays(-1);

  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
  QSqlDatabase db = Database::database();
  db.transaction();

  QSqlQuery query(db);
  query.prepare("DELETE FROM TelevisionTeletextPages WHERE pageDate < :oldDate");
  query.bindValue(0, oldDate);
  query.exec();

  db.commit();
}

void TeletextServer::storePages(const QString &channelName, QMap<quint32, SDataBuffer::TeletextPage> *pages)
{
  SDebug::Trace t("TeletextServer::storePages");

  PluginSettings settings(plugin);

  const QString rc = SStringParser::toRawName(channelName);
  const QDateTime now = QDateTime::currentDateTime();
  QVariantList pageNumber, subpageNumber, rawChannel, pageDate, pageData, pageRawText;

  QMultiMap<int, int> harvestPages;
  foreach (const QString &group, settings.childGroups())
  if (group.startsWith(typeName(Type_Television) + "Channel_"))
  {
    settings.beginGroup(group);

    if (SStringParser::toRawName(settings.value("Name").toString()) == rc)
    {
      for (unsigned b=0; b<=2; b++)
      foreach (const QString &p, settings.value("EPGTeletextPage" + QString::number(b)).toString().split(' '))
        harvestPages.insert(p.toInt(NULL, 16), b);

      for (unsigned b=3; b<=9; b++)
      foreach (const QString &p, settings.value("EPGTeletextPage" + QString::number(b)).toString().split(' '))
        harvestPages.insert(p.toInt(NULL, 16), -(b - 2));
    }

    settings.endGroup();
  }

  foreach (const SDataBuffer::TeletextPage &page, *pages)
  if ((page.pgno >= 0x100) && (page.pgno <= 0x8FF))
  {
    pageNumber += page.pgno;
    subpageNumber += page.subno;
    rawChannel += rc;
    pageDate += now;
    pageData += QByteArray(reinterpret_cast<const char *>(&page), sizeof(page)).toBase64();

    // Get the raw text
    QString text;
    foreach (const QString &line, page.decodedLines())
      text += SStringParser::toRawName(line);

    if (!text.isEmpty())
      pageRawText += text;
    else
      pageRawText += QVariant(QVariant::String);

    // Check if we can harvest EPG
    QMultiMap<int, int>::ConstIterator i = harvestPages.find(page.pgno);
    if (i != harvestPages.end())
      epgDatabase->gatherEPG(channelName, page, *i);
  }

  if (!pageNumber.isEmpty())
  {
    SDebug::MutexLocker l(&(Database::mutex()), __FILE__, __LINE__);
    QSqlDatabase db = Database::database();
    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT OR REPLACE INTO TelevisionTeletextPages VALUES (:pageNumber, :subpageNumber, :rawChannel, :pageDate, :pageData, :pageRawText)");
    query.bindValue(0, pageNumber);
    query.bindValue(1, subpageNumber);
    query.bindValue(2, rawChannel);
    query.bindValue(3, pageDate);
    query.bindValue(4, pageData);
    query.bindValue(5, pageRawText);
    query.execBatch();

    db.commit();
  }
  else // No Teletext; store dummy page to prevent re-scanning.
  {
    SDebug::MutexLocker l(&(Database::mutex()), __FILE__, __LINE__);

    QSqlQuery query(Database::database());
    query.prepare("INSERT OR REPLACE INTO TelevisionTeletextPages VALUES (:pageNumber, :subpageNumber, :rawChannel, :pageDate, :pageData, :pageRawText)");
    query.bindValue(0, 0);
    query.bindValue(1, 0);
    query.bindValue(2, rc);
    query.bindValue(3, now);
    query.bindValue(4, QByteArray(""));
    query.bindValue(5, QVariant(QVariant::String));
    query.exec();
  }

  delete pages;
}


bool TeletextServer::DataNode::prepare(const SCodecList &)
{
  pages = new QMap<quint32, SDataBuffer::TeletextPage>();

  return true;
}

bool TeletextServer::DataNode::unprepare(void)
{
  // storePages will also delete pages
  QtConcurrent::run(parent, &TeletextServer::storePages, channelName, pages);
  pages = NULL;

  return true;
}

SNode::Result TeletextServer::DataNode::processBuffer(const SBuffer &buffer, SBufferList &)
{
  SDataBuffer dataBuffer = buffer;

  if (!dataBuffer.isNull())
  if (dataBuffer.codec() == SDataCodec::Format_TeletextPage)
  {
    const SDataBuffer::TeletextPage *page = reinterpret_cast<const SDataBuffer::TeletextPage *>(dataBuffer.bits());
    if (page)
      pages->insert((quint32(page->pgno) << 16) | (quint32(page->subno) & 0xFFFF), *page);
  }

  return Result_Active;
}

} // End of namespace
