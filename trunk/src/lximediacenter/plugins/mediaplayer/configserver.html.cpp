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
    " <div class=\"content\">\n"
    "  <fieldset>\n"
    "   <legend>{TR_RIGHTS}</legend>\n"
    "   {TR_RIGHTS_EXPLAIN}\n"
    "  </fieldset>\n"
    "  <fieldset style=\"float:left;\">\n"
    "   <legend>{TR_MOVIES}</legend>\n"
    "   {TR_MOVIES_EXPLAIN}<br />\n"
    "   <iframe style=\"width:30em;height:30em;\" src=\"movies-tree.html\" frameborder=\"0\">\n"
    "    <a href=\"movies-tree.html\" target=\"_blank\">{TR_MOVIES}</a>\n"
    "   </iframe>\n"
    "  </fieldset>\n"
    "  <fieldset style=\"float:left;\">\n"
    "   <legend>{TR_TVSHOWS}</legend>\n"
    "   {TR_TVSHOWS_EXPLAIN}<br />\n"
    "   <iframe style=\"width:30em;height:30em;\" src=\"tvshows-tree.html\" frameborder=\"0\">\n"
    "    <a href=\"tvshows-tree.html\" target=\"_blank\">{TR_TVSHOWS}</a>\n"
    "   </iframe>\n"
    "  </fieldset>\n"
    "  <fieldset style=\"float:left;\">\n"
    "   <legend>{TR_CLIPS}</legend>\n"
    "   {TR_CLIPS_EXPLAIN}<br />\n"
    "   <iframe style=\"width:30em;height:30em;\" src=\"clips-tree.html\" frameborder=\"0\">\n"
    "    <a href=\"clips-tree.html\" target=\"_blank\">{TR_CLIPS}</a>\n"
    "   </iframe>\n"
    "  </fieldset>\n"
    "  <fieldset style=\"float:left;\">\n"
    "   <legend>{TR_MUSIC}</legend>\n"
    "   {TR_MUSIC_EXPLAIN}<br />\n"
    "   <iframe style=\"width:30em;height:30em;\" src=\"music-tree.html\" frameborder=\"0\">\n"
    "    <a href=\"music-tree.html\" target=\"_blank\">{TR_MUSIC}</a>\n"
    "   </iframe>\n"
    "  </fieldset>\n"
    "  <fieldset style=\"float:left;\">\n"
    "   <legend>{TR_PHOTOS}</legend>\n"
    "   {TR_PHOTOS_EXPLAIN}<br />\n"
    "   <iframe style=\"width:30em;height:30em;\" src=\"photos-tree.html\" frameborder=\"0\">\n"
    "    <a href=\"photos-tree.html\" target=\"_blank\">{TR_PHOTOS}</a>\n"
    "   </iframe>\n"
    "  </fieldset>\n"
    "  <fieldset style=\"float:left;\">\n"
    "   <legend>{TR_HOMEVIDEOS}</legend>\n"
    "   {TR_HOMEVIDEOS_EXPLAIN}<br />\n"
    "   <iframe style=\"width:30em;height:30em;\" src=\"homevideos-tree.html\" frameborder=\"0\">\n"
    "    <a href=\"homevideos-tree.html\" target=\"_blank\">{TR_HOMEVIDEOS}</a>\n"
    "   </iframe>\n"
    "  </fieldset>\n"
    " </div>\n";

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
    "  <a class=\"hidden\" name=\"{DIR_FULLPATH}\" />\n"
    "  {DIR_INDENT}\n"
    "  {DIR_EXPAND}\n"
    "  {DIR_CHECK}\n"
    "  {DIR_NAME}\n"
    " </td></tr>\n";

const char * const ConfigServer::htmlDirTreeIndent =
    " <img src=\"/img/null.png\" width=\"16\" height=\"16\" />\n";

const char * const ConfigServer::htmlDirTreeExpand =
    " <a class=\"hidden\" href=\"{FILE}?open={DIR_ALLOPEN}#{DIR_FULLPATH}\">\n"
    "  <img src=\"/img/tree{DIR_OPEN}.png\" width=\"16\" height=\"16\" />\n"
    " </a>\n";

const char * const ConfigServer::htmlDirTreeCheck =
    " <img src=\"/img/check{DIR_CHECKED}.png\" title=\"{DIR_TITLE}\" width=\"16\" height=\"16\" />";

const char * const ConfigServer::htmlDirTreeCheckLink =
    " <a class=\"hidden\" href=\"{FILE}?open={DIR_ALLOPEN}&amp;{DIR_CHECKTYPE}={DIR_FULLPATH}#{DIR_FULLPATH}\">\n"
    "  <img src=\"/img/check{DIR_CHECKED}.png\" width=\"16\" height=\"16\" />\n"
    " </a>\n";

SHttpServer::SocketOp ConfigServer::handleHtmlRequest(const SHttpServer::RequestMessage &request, QIODevice *socket, const MediaServer::File &file)
{
  SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  HtmlParser htmlParser;

  if (file.baseName().endsWith("-tree"))
  {
    PluginSettings settings(pluginName());
    settings.beginGroup(file.baseName().left(file.baseName().length() - 5));

    QStringList rootPaths = settings.value("Paths").toStringList();

    const QString open = file.url().queryItemValue("open");
    const QSet<QString> allopen = !open.isEmpty()
                                  ? QSet<QString>::fromList(QString::fromUtf8(qUncompress(QByteArray::fromHex(open.toAscii()))).split(dirSplit))
                                  : QSet<QString>();

    const QString checkon = QString::fromUtf8(QByteArray::fromHex(file.url().queryItemValue("checkon").toAscii()));
    const QString checkoff = QString::fromUtf8(QByteArray::fromHex(file.url().queryItemValue("checkoff").toAscii()));
    if (!checkon.isEmpty() || !checkoff.isEmpty())
    {
      if (!checkon.isEmpty())
        rootPaths.append(checkon);

      if (!checkoff.isEmpty())
        rootPaths.removeAll(checkoff);

      settings.setValue("Paths", rootPaths);

      mediaDatabase->rescanRoots();
    }

    htmlParser.setField("FILE", file.fullName());
    htmlParser.setField("DIRS", QByteArray(""));
    generateDirs(htmlParser, drives(), 0, allopen, rootPaths);

    socket->write(response);
    socket->write(htmlParser.parse(htmlDirTreeIndex));
    return SHttpServer::SocketOp_Close;
  }
  else
  {
    htmlParser.setField("TR_RIGHTS", tr("Access rights"));
    htmlParser.setField("TR_RIGHTS_EXPLAIN", tr(
        "By default, the LXiMediaCenter backend (lximcbackend) runs as a restricted user.\n"
#if defined(Q_OS_LINUX)
        "The user and group \"lximediacenter\" were created during installation for this purpose.\n"
#elif defined(Q_OS_WIN)
        "The  \"Local Service\" user is used for this purpose.\n"
#endif
        "This means that all files that need to be accessed by LXiMediaCenter, need to be accessible by this user.\n"
#if defined(Q_OS_LINUX)
        "This can be done by setting the read permission for \"other\" users on the files and directories that need to be accessed by the LXiMediaCenter backend.\n"
#elif defined(Q_OS_WIN)
        "This can be done by adding \"Everyone\" with the read permission set to the files and directories that need to be accessed by the LXiMediaCenter backend.\n"
#endif
        "<br /><br />\n"
        "Furthermore, certain system directories can not be selected to prevent security issues."
        ));
  
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

    drives(true);
    driveLabel(QString::null);

    return sendHtmlContent(request, socket, file.url(), response, htmlParser.parse(htmlMain));
  }
}

void ConfigServer::generateDirs(HtmlParser &htmlParser, const QFileInfoList &dirs, int indent, const QSet<QString> &allopen, const QStringList &rootPaths)
{
  foreach (const QFileInfo &info, dirs)
  if (!info.fileName().startsWith('.'))
  {
    const QString absoluteName = info.absoluteFilePath();

    const QString fileName = info.fileName();

    QFileInfoList children = QDir(absoluteName).entryInfoList(QDir::Dirs);
    for (QList<QFileInfo>::Iterator i=children.begin(); i!=children.end(); )
    if (i->fileName().startsWith('.'))
      i = children.erase(i);
    else
      i++;

    htmlParser.setField("DIR_FULLPATH", absoluteName.toUtf8().toHex());

    // Indentation
    htmlParser.setField("DIR_INDENT", QByteArray(""));
    for (int i=0; i<indent; i++)
      htmlParser.appendField("DIR_INDENT", htmlParser.parse(htmlDirTreeIndent));

    // Expand
    htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlDirTreeIndent));
    if (!isHidden(absoluteName))
    if ((indent > 0) && !children.isEmpty())
    {
      QSet<QString> all = allopen;
      if (all.contains(absoluteName))
      {
        all.remove(absoluteName);
        htmlParser.setField("DIR_OPEN", QByteArray("open"));
      }
      else
      {
        all.insert(absoluteName);
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
      if (root == absoluteName)
      {
        htmlParser.setField("DIR_CHECKED", QByteArray("full"));
        htmlParser.setField("DIR_CHECKTYPE", QByteArray("checkoff"));
        checkEnabled = true;
        break;
      }
      else if (root.startsWith(absoluteName))
      {
        htmlParser.setField("DIR_CHECKED", QByteArray("some"));
        htmlParser.setField("DIR_TITLE", QByteArray("A child is selected"));
        checkEnabled = false;
      }
      else if (absoluteName.startsWith(root))
      {
        htmlParser.setField("DIR_CHECKED", QByteArray("fulldisabled"));
        htmlParser.setField("DIR_TITLE", QByteArray("A parent is selected"));
        checkEnabled = false;
      }
    }

    if (isHidden(absoluteName))
    {
      htmlParser.appendField("DIR_CHECKED", QByteArray("disabled"));
      htmlParser.setField("DIR_TITLE", tr("This system directory can not be selected"));
      checkEnabled = false;
    }
    else if (!info.isReadable() || !info.isExecutable())
    {
      htmlParser.appendField("DIR_CHECKED", QByteArray("disabled"));
      htmlParser.setField("DIR_TITLE", tr("Access denied"));
      checkEnabled = false;
    }
    else if (indent == 0)
    {
      htmlParser.appendField("DIR_CHECKED", QByteArray("disabled"));
      htmlParser.setField("DIR_TITLE", tr("The root can not be selected"));
      checkEnabled = false;
    }

    htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(allopen.toList()).join(QString(dirSplit)).toUtf8()).toHex());
    htmlParser.setField("DIR_CHECK", htmlParser.parse(checkEnabled ? htmlDirTreeCheckLink : htmlDirTreeCheck));

    // Name
    if (fileName.isEmpty())
    {
      htmlParser.setField("DIR_NAME", info.absoluteFilePath());

      const QString label = driveLabel(info.absoluteFilePath());
      if (!label.isEmpty())
        htmlParser.appendField("DIR_NAME", " (" + label + ")");
    }
    else
      htmlParser.setField("DIR_NAME", fileName);


    if (info.isSymLink())
      htmlParser.appendField("DIR_NAME", " (" + tr("symbolic link to") + " " + info.symLinkTarget() + ")");

    htmlParser.appendField("DIRS", htmlParser.parse(htmlDirTreeDir));

    // Recurse
    if (!isHidden(absoluteName))
    if ((indent == 0) || (allopen.contains(absoluteName)))
      generateDirs(htmlParser, children, indent + 1, allopen, rootPaths);
  }
}

const QFileInfoList & ConfigServer::drives(bool rescan)
{
  static QFileInfoList driveList;
  if (rescan)
    driveList = QDir::drives();

  return driveList;
}

QString ConfigServer::driveLabel(const QString &drive)
{
#ifdef Q_OS_WIN
  static QMap<QString, QString> labels;

  if (!drive.isEmpty())
  {
    if (drive.length() <= 3)
    {
      QMap<QString, QString>::Iterator label = labels.find(drive);
      if (label != labels.end())
        return *label;

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
                                  szFileSystemName, sizeof(szFileSystemName) / sizeof(*szFileSystemName)))
      {
        label = labels.insert(drive, QString::fromUtf16((const ushort *)szVolumeName).trimmed());
      }
      else
        label = labels.insert(drive, QString::null);

      return *label;
    }
  }
  else
    labels.clear();
#else
  if (drive == "/")
    return tr("Root");
#endif

  return QString::null;
}


} } // End of namespaces
