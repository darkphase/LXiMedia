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

#include "mediaplayerserver.h"
#include "mediadatabase.h"
#include "module.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

const char MediaPlayerServer::htmlFrontPageContent[] =
    " <div class=\"thumbnaillist\" id=\"mediaplayeritems\">\n"
    " </div>\n"
    " <script type=\"text/javascript\">loadListContent(\"mediaplayeritems\", \"{SERVER_PATH}\", 0, 0);</script>\n";

const char MediaPlayerServer::htmlSettingsMain[] =
    "  <fieldset>\n"
    "   <legend>{TR_MEDIAPLAYER}</legend>\n"
    "   <form name=\"settings\" action=\"{SERVER_PATH}\" method=\"get\">\n"
    "    <input type=\"hidden\" name=\"save_settings\" value=\"settings\" />\n"
    "    <table>\n"
    "     <tr>\n"
    "      <td>\n"
    "       <iframe style=\"width:30em;height:30em;\" src=\"{SERVER_PATH}?folder_tree=\" frameborder=\"0\">\n"
    "        <a href=\"{SERVER_PATH}?folder_tree=\" target=\"_blank\">frame</a>\n"
    "       </iframe>\n"
    "      </td>\n"
    "      <td class=\"top\">\n"
    "       {TR_RIGHTS_EXPLAIN}<br /><br />\n"
    "       {TR_SLIDEDURATION}:\n"
    "       <input type=\"text\" size=\"6\" name=\"slideduration\" value=\"{SLIDEDURATION}\" />ms.\n"
    "      </td>\n"
    "     </tr>\n"
    "     <tr><td colspan=\"2\">\n"
    "      <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "     </td></tr>\n"
    "    </table>\n"
    "   </form>\n"
    "  </fieldset>\n";

const char MediaPlayerServer::htmlSettingsDirTreeIndex[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "</head>\n"
    "<body>\n"
    " <table width=\"100%\" cellspacing=\"0\" cellpadding=\"3\" border=\"0\">\n"
    "{DIRS}\n"
    " </table>\n"
    "</body>\n"
    "</html>\n";

const char MediaPlayerServer::htmlSettingsDirTreeDir[] =
    " <tr align=\"middle\"><td align=\"left\">\n"
    "  <a class=\"hidden\" name=\"{DIR_FULLPATH}\" />\n"
    "{DIR_INDENT}"
    "{DIR_EXPAND}"
    "{DIR_CHECK}"
    "  {DIR_NAME}\n"
    " </td></tr>\n";

const char MediaPlayerServer::htmlSettingsDirTreeIndent[] =
    "  <img src=\"/img/null.png\" width=\"16\" height=\"16\" />\n";

const char MediaPlayerServer::htmlSettingsDirTreeExpand[] =
    "  <a class=\"hidden\" href=\"{SERVER_PATH}?folder_tree=&open={DIR_ALLOPEN}#{DIR_FULLPATH}\">\n"
    "   <img src=\"/img/tree{DIR_OPEN}.png\" width=\"16\" height=\"16\" />\n"
    "  </a>\n";

const char MediaPlayerServer::htmlSettingsDirTreeCheck[] =
    "  <img src=\"/img/check{DIR_CHECKED}.png\" title=\"{DIR_TITLE}\" width=\"16\" height=\"16\" />\n";

const char MediaPlayerServer::htmlSettingsDirTreeCheckLink[] =
    "  <a class=\"hidden\" href=\"{SERVER_PATH}?folder_tree=&open={DIR_ALLOPEN}&amp;{DIR_CHECKTYPE}={DIR_FULLPATH}#{DIR_FULLPATH}\">\n"
    "   <img src=\"/img/check{DIR_CHECKED}.png\" width=\"16\" height=\"16\" />\n"
    "  </a>\n";

QByteArray MediaPlayerServer::frontPageContent(void)
{
  HtmlParser htmlParser;
  htmlParser.setField("SERVER_PATH", QUrl(serverPath()).toEncoded());

  return htmlParser.parse(htmlFrontPageContent);
}

QByteArray MediaPlayerServer::settingsContent(void)
{
  HtmlParser htmlParser;
  htmlParser.setField("SERVER_PATH", QUrl(serverPath()).toEncoded());
  htmlParser.setField("TR_MEDIAPLAYER", tr(Module::pluginName));
  htmlParser.setField("TR_SLIDEDURATION", tr("Photo slideshow slide duration"));
  htmlParser.setField("TR_SAVE", tr("Save"));
  htmlParser.setField("TR_MEDIADIRS", tr("Media directories"));
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
      "Furthermore, certain system directories can not be selected to prevent security issues."
      ));

  PluginSettings settings(Module::pluginName);
  htmlParser.setField("SLIDEDURATION", QByteArray::number(settings.value("SlideDuration", 7500).toInt()));

  scanDrives();

  return htmlParser.parse(htmlSettingsMain);
}

void MediaPlayerServer::generateDirs(HtmlParser &htmlParser, const QFileInfoList &dirs, int indent, const QStringList &allopen, const QStringList &rootPaths)
{
  foreach (const QFileInfo &info, dirs)
  if (!info.fileName().startsWith('.'))
  {
    const QString fileName = info.fileName();
    const QString absoluteName = info.absoluteFilePath().endsWith('/') ? info.absoluteFilePath() : (info.absoluteFilePath() + '/');

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
      htmlParser.appendField("DIR_INDENT", htmlParser.parse(htmlSettingsDirTreeIndent));

    // Expand
    htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlSettingsDirTreeIndent));
    if (!isHidden(absoluteName))
    if ((indent > 0) && !children.isEmpty())
    {
      QStringList all = allopen;
      if (all.contains(absoluteName, caseSensitivity))
      {
        for (QStringList::Iterator i=all.begin(); i!=all.end(); )
        if (i->compare(absoluteName, caseSensitivity) == 0)
          i = all.erase(i);
        else
          i++;

        htmlParser.setField("DIR_OPEN", QByteArray("open"));
      }
      else
      {
        all.append(absoluteName);
        htmlParser.setField("DIR_OPEN", QByteArray("close"));
      }

      htmlParser.setField("DIR_ALLOPEN", qCompress(all.join(QString(dirSplit)).toUtf8()).toHex());
      htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlSettingsDirTreeExpand));
    }

    // Check
    bool checkEnabled = true;
    htmlParser.setField("DIR_CHECKED", QByteArray("none"));
    htmlParser.setField("DIR_CHECKTYPE", QByteArray("checkon"));
    foreach (const QString &root, rootPaths)
    {
      if (root.compare(absoluteName, caseSensitivity) == 0)
      {
        htmlParser.setField("DIR_CHECKED", QByteArray("full"));
        htmlParser.setField("DIR_CHECKTYPE", QByteArray("checkoff"));
        checkEnabled = true;
        break;
      }
      else if (root.startsWith(absoluteName, caseSensitivity))
      {
        htmlParser.setField("DIR_CHECKED", QByteArray("some"));
        htmlParser.setField("DIR_TITLE", QByteArray("A child is selected"));
        checkEnabled = false;
      }
      else if (absoluteName.startsWith(root, caseSensitivity))
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

    htmlParser.setField("DIR_ALLOPEN", qCompress(allopen.join(QString(dirSplit)).toUtf8()).toHex());
    htmlParser.setField("DIR_CHECK", htmlParser.parse(checkEnabled ? htmlSettingsDirTreeCheckLink : htmlSettingsDirTreeCheck));

    // Name
    if (fileName.isEmpty())
    {
      htmlParser.setField("DIR_NAME", info.absoluteFilePath());

      QMap<QString, QString>::Iterator i = driveLabelList.find(info.absoluteFilePath());
      if ((i != driveLabelList.end()) && !i->isEmpty())
        htmlParser.appendField("DIR_NAME", " (" + *i + ")");
    }
    else
      htmlParser.setField("DIR_NAME", fileName);

    if (info.isSymLink())
      htmlParser.appendField("DIR_NAME", " (" + tr("symbolic link to") + " " + info.symLinkTarget() + ")");

    htmlParser.appendField("DIRS", htmlParser.parse(htmlSettingsDirTreeDir));

    // Recurse
    if (!isHidden(absoluteName))
    if ((indent == 0) || (allopen.contains(absoluteName, caseSensitivity)))
      generateDirs(htmlParser, children, indent + 1, allopen, rootPaths);
  }
}

void MediaPlayerServer::scanDrives(void)
{
  driveInfoList.clear();
  driveLabelList.clear();

  foreach (const QFileInfo &drive, QDir::drives())
  if (QDir(drive.absoluteFilePath()).count() != 0)
  {
    driveInfoList.insert(drive.absoluteFilePath(), drive);

#ifdef Q_OS_WIN
    WCHAR szVolumeName[MAX_PATH+1];
    WCHAR szFileSystemName[MAX_PATH+1];
    DWORD dwSerialNumber = 0;
    DWORD dwMaxFileNameLength = MAX_PATH;
    DWORD dwFileSystemFlags = 0;

    if (::GetVolumeInformationW(reinterpret_cast<const WCHAR *>(drive.absoluteFilePath().utf16()),
                                szVolumeName, sizeof(szVolumeName) / sizeof(*szVolumeName),
                                &dwSerialNumber,
                                &dwMaxFileNameLength,
                                &dwFileSystemFlags,
                                szFileSystemName, sizeof(szFileSystemName) / sizeof(*szFileSystemName)))
    {
      driveLabelList.insert(
            drive.absoluteFilePath(),
            QString::fromUtf16((const ushort *)szVolumeName).trimmed());
    }
#else
    if (drive.absoluteFilePath() == "/")
      driveLabelList.insert(drive.absoluteFilePath(), tr("Root"));
#endif
  }
}

} } // End of namespaces
