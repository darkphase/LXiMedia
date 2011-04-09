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

#include "configserver.h"
#include "mediadatabase.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

const char * const ConfigServer::htmlMain =
    " <table class=\"widgetsfull\">\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"widget\">\n"
    "    <table class=\"main\">\n"
    "     <tr class=\"main\">\n"
    "      <td class=\"center\" colspan=\"2\">\n"
    "       <p class=\"head\">{TR_MEDIA_DIRECTORIES}</p>\n"
    "       {TR_MEDIA_DIRECTORIES_EXPLAIN}\n"
    "      </td>\n"
    "     </tr>\n"
    "     <tr class=\"main\">\n"
    "      <td class=\"maincenter\" width=\"50%\">\n"
    "       <p class=\"head2\">{TR_MOVIES}</p>\n"
    "       {TR_MOVIES_EXPLAIN}\n"
    "       <iframe src=\"movies-tree.html\" width=\"100%\" height=\"300\" frameborder=\"0\"></iframe>\n"
    "      </td>\n"
    "      <td class=\"maincenter\" width=\"50%\">\n"
    "       <p class=\"head2\">{TR_TVSHOWS}</p>\n"
    "       {TR_TVSHOWS_EXPLAIN}\n"
    "       <iframe src=\"tvshows-tree.html\" width=\"100%\" height=\"300\" frameborder=\"0\"></iframe>\n"
    "      </td>\n"
    "     </tr>\n"
    "     <tr class=\"main\">\n"
    "      <td class=\"maincenter\" width=\"50%\">\n"
    "       <p class=\"head2\">{TR_CLIPS}</p>\n"
    "       {TR_CLIPS_EXPLAIN}\n"
    "       <iframe src=\"clips-tree.html\" width=\"100%\" height=\"300\" frameborder=\"0\"></iframe>\n"
    "      </td>\n"
    "      <td class=\"maincenter\" width=\"50%\">\n"
    "       <p class=\"head2\">{TR_MUSIC}</p>\n"
    "       {TR_MUSIC_EXPLAIN}\n"
    "       <iframe src=\"music-tree.html\" width=\"100%\" height=\"300\" frameborder=\"0\"></iframe>\n"
    "      </td>\n"
    "     </tr>\n"
    "     <tr class=\"main\">\n"
    "      <td class=\"maincenter\" width=\"50%\">\n"
    "       <p class=\"head2\">{TR_PHOTOS}</p>\n"
    "       {TR_PHOTOS_EXPLAIN}\n"
    "       <iframe src=\"photos-tree.html\" width=\"100%\" height=\"300\" frameborder=\"0\"></iframe>\n"
    "      </td>\n"
    "      <td class=\"maincenter\" width=\"50%\">\n"
    "       <p class=\"head2\">{TR_HOMEVIDEOS}</p>\n"
    "       {TR_HOMEVIDEOS_EXPLAIN}\n"
    "       <iframe src=\"homevideos-tree.html\" width=\"100%\" height=\"300\" frameborder=\"0\"></iframe>\n"
    "      </td>\n"
    "     </tr>\n"
    "    </table>\n"
    "   </td>\n"
    "  </tr>\n"
    " </table>\n";

const char * const ConfigServer::htmlDirTreeIndex =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "</head>\n"
    "<body>\n"
    " <table width=\"100%\" cellspacing=\"0\" cellpadding=\"3\" border=\"0\">\n"
    "{DIRS}\n"
    " </table>\n"
    "</body>\n"
    "</html>\n";

const char * const ConfigServer::htmlDirTreeDir =
    " <tr align=\"middle\"><td align=\"left\">\n"
    "  <a class=\"bookmark\" name=\"{DIR_FULLPATH}\" />\n"
    "  {DIR_INDENT}\n"
    "  {DIR_EXPAND}\n"
    "  {DIR_CHECK}\n"
    "  {DIR_NAME}\n"
    " </td></tr>\n";

const char * const ConfigServer::htmlDirTreeIndent =
    "<img src=\"/img/null.png\" width=\"16\" height=\"16\" />";

const char * const ConfigServer::htmlDirTreeExpand =
    "<a class=\"bookmark\" href=\"{FILE}?open={DIR_ALLOPEN}#{DIR_FULLPATH}\" />"
    "<img src=\"/img/tree{DIR_OPEN}.png\" width=\"16\" height=\"16\" />"
    "</a>";

const char * const ConfigServer::htmlDirTreeCheck =
    "<img src=\"/img/check{DIR_CHECKED}.png\" width=\"16\" height=\"16\" />";

const char * const ConfigServer::htmlDirTreeCheckLink =
    "<a class=\"bookmark\" href=\"{FILE}?open={DIR_ALLOPEN}&amp;{DIR_CHECKTYPE}={DIR_FULLPATH}#{DIR_FULLPATH}\" />"
    "<img src=\"/img/check{DIR_CHECKED}.png\" width=\"16\" height=\"16\" />"
    "</a>";

SHttpServer::SocketOp ConfigServer::handleHtmlRequest(const SHttpServer::RequestHeader &request, QIODevice *socket, const QString &file)
{
  SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  const QUrl url(request.path());
  HtmlParser htmlParser;

  if (file.endsWith("-tree.html"))
  {
    PluginSettings settings(pluginName());
    settings.beginGroup(file.left(file.length() - 10));

    QStringList rootPaths = settings.value("Paths").toStringList();

    const QString open = url.queryItemValue("open");
    const QSet<QString> allopen = !open.isEmpty()
                                  ? QSet<QString>::fromList(QString::fromUtf8(qUncompress(QByteArray::fromHex(open.toAscii()))).split(dirSplit))
                                  : QSet<QString>();

    const QString checkon = QString::fromUtf8(QByteArray::fromHex(url.queryItemValue("checkon").toAscii()));
    const QString checkoff = QString::fromUtf8(QByteArray::fromHex(url.queryItemValue("checkoff").toAscii()));
    if (!checkon.isEmpty() || !checkoff.isEmpty())
    {
      if (!checkon.isEmpty())
        rootPaths.append(checkon);

      if (!checkoff.isEmpty())
        rootPaths.removeAll(checkoff);

      settings.setValue("Paths", rootPaths);

      mediaDatabase->rescanRoots();
    }

    htmlParser.setField("FILE", file);
    htmlParser.setField("DIRS", QByteArray(""));
    generateDirs(htmlParser, drives(), 0, allopen, rootPaths);

    socket->write(response);
    socket->write(htmlParser.parse(htmlDirTreeIndex));
    return SHttpServer::SocketOp_Close;
  }
  else
  {
    htmlParser.setField("TR_MEDIA_DIRECTORIES", tr("Media directories"));
    htmlParser.setField("TR_CLIPS", tr("Video clips"));
    htmlParser.setField("TR_CLIPS_EXPLAIN", tr("Directories containing video clips:"));
    htmlParser.setField("TR_HOMEVIDEOS", tr("Home videos"));
    htmlParser.setField("TR_HOMEVIDEOS_EXPLAIN", tr("Directories containing home video files:"));
    htmlParser.setField("TR_MOVIES", tr("Movies"));
    htmlParser.setField("TR_MOVIES_EXPLAIN", tr("Directories containing movie files:"));
    htmlParser.setField("TR_MUSIC", tr("Music"));
    htmlParser.setField("TR_MUSIC_EXPLAIN", tr("Directories containing music files, including music videos:"));
    htmlParser.setField("TR_PHOTOS", tr("Photos"));
    htmlParser.setField("TR_PHOTOS_EXPLAIN", tr("Directories containing photo albums:"));
    htmlParser.setField("TR_TVSHOWS", tr("TV Shows"));
    htmlParser.setField("TR_TVSHOWS_EXPLAIN", tr("Directories containing TV shows:"));

    htmlParser.setField("TR_MEDIA_DIRECTORIES_EXPLAIN",
      tr("Select the directories containing various media files here. Note "
         "that it may take several minutes before any changes will be visible."));

    drives(true);

    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlMain));
  }
}

void ConfigServer::generateDirs(HtmlParser &htmlParser, const QFileInfoList &dirs, int indent, const QSet<QString> &allopen, const QStringList &rootPaths)
{
#ifdef Q_OS_WIN
  struct T
  {
    static QString driveLabel(const QString &drive)
    {
      WCHAR szVolumeName[MAX_PATH+1];
      WCHAR szFileSystemName[MAX_PATH+1];
      DWORD dwSerialNumber = 0;
      DWORD dwMaxFileNameLength = MAX_PATH;
      DWORD dwFileSystemFlags = 0;

      if (::GetVolumeInformationW(reinterpret_cast<const WCHAR *>(drive.utf16()),
                                  szVolumeName, sizeof(szVolumeName) / sizeof(*szVolumeName),
                                  &dwSerialNumber,
                                  &dwMaxFileNameLength,
                                  &dwFileSystemFlags,
                                  szFileSystemName, sizeof(szFileSystemName) / sizeof(*szFileSystemName)));
      {
        return QString::fromUtf16((const ushort *)szVolumeName).trimmed();
      }

      return QString::null;
    }
  };
#endif

  foreach (const QFileInfo &info, dirs)
  if (!info.fileName().startsWith('.'))
  {
    const QString canonicalName =
        (info.isReadable() ? info.canonicalFilePath() : info.absoluteFilePath())
#ifdef Q_OS_WIN
        .toLower()
#endif
        ;

    if (!hiddenDirs().contains(canonicalName))
    {
      const QString fileName = info.fileName();

      QFileInfoList children = QDir(canonicalName).entryInfoList(QDir::Dirs);
      for (QList<QFileInfo>::Iterator i=children.begin(); i!=children.end(); )
      if (i->fileName().startsWith('.'))
        i = children.erase(i);
      else
        i++;

      htmlParser.setField("DIR_FULLPATH", canonicalName.toUtf8().toHex());

      // Indentation
      htmlParser.setField("DIR_INDENT", QByteArray(""));
      for (int i=0; i<indent; i++)
        htmlParser.appendField("DIR_INDENT", htmlParser.parse(htmlDirTreeIndent));

      // Expand
      htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlDirTreeIndent));
      if ((indent > 0) && !children.isEmpty())
      {
        QSet<QString> all = allopen;
        if (all.contains(canonicalName))
        {
          all.remove(canonicalName);
          htmlParser.setField("DIR_OPEN", QByteArray("open"));
        }
        else
        {
          all.insert(canonicalName);
          htmlParser.setField("DIR_OPEN", QByteArray("close"));
        }

        htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(all.toList()).join(QString(dirSplit)).toUtf8()).toHex());
        htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlDirTreeExpand));
      }

      // Check
      bool checkEnabled = true;
      htmlParser.setField("DIR_CHECKED", QByteArray("none"));
      htmlParser.setField("DIR_CHECKTYPE", QByteArray("checkon"));
      foreach (const QString &root, rootPaths)
      {
        if (root == canonicalName)
        {
          htmlParser.setField("DIR_CHECKED", QByteArray("full"));
          htmlParser.setField("DIR_CHECKTYPE", QByteArray("checkoff"));
          checkEnabled = true;
          break;
        }
        else if (root.startsWith(canonicalName))
        {
          htmlParser.setField("DIR_CHECKED", QByteArray("some"));
          checkEnabled = false;
        }
        else if (canonicalName.startsWith(root))
        {
          htmlParser.setField("DIR_CHECKED", QByteArray("fulldisabled"));
          checkEnabled = false;
        }
      }

      if (indent == 0)
      {
        htmlParser.appendField("DIR_CHECKED", QByteArray("disabled"));
        checkEnabled = false;
      }

      htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(allopen.toList()).join(QString(dirSplit)).toUtf8()).toHex());
      htmlParser.setField("DIR_CHECK", htmlParser.parse(checkEnabled ? htmlDirTreeCheckLink : htmlDirTreeCheck));

      // Name
      if (fileName.isEmpty())
      {
        htmlParser.setField("DIR_NAME", info.absoluteFilePath());
#ifdef Q_OS_WIN
        if (info.isReadable())
        {
          const QString label = T::driveLabel(info.absoluteFilePath());
          if (!label.isEmpty())
            htmlParser.appendField("DIR_NAME", " (" + label + ")");
        }
#endif
      }
      else
        htmlParser.setField("DIR_NAME", fileName);


      if (info.isSymLink())
        htmlParser.appendField("DIR_NAME", " (" + tr("symbolic link to") + " " + info.symLinkTarget() + ")");

      htmlParser.appendField("DIRS", htmlParser.parse(htmlDirTreeDir));

      // Recurse
      if ((indent == 0) || (allopen.contains(canonicalName)))
        generateDirs(htmlParser, children, indent + 1, allopen, rootPaths);
    }
  }
}

const QFileInfoList & ConfigServer::drives(bool rescan)
{
  static QFileInfoList driveList;
  if (rescan)
    driveList = QDir::drives();

  return driveList;
}


} } // End of namespaces
