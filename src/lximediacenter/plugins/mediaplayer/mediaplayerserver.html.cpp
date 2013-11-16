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

#include "mediaplayerserver.h"
#include "mediadatabase.h"
#include "module.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

const char MediaPlayerServer::htmlFrontPageContent[] =
    "   <div class=\"list_thumbnails\" id=\"mediaplayeritems\">\n"
    "   </div>\n"
    "   <script type=\"text/javascript\">loadListContent(\"mediaplayeritems\", \"{SERVER_PATH}\", 0, 0);</script>\n";

const char MediaPlayerServer::htmlSettingsMain[] =
    "  <fieldset id=\"mediaplayer\">\n"
    "   <legend>{TR_MEDIAPLAYER}</legend>\n"
    "   <form name=\"settings\" action=\"{SERVER_PATH}#mediaplayer\" method=\"get\">\n"
    "    <input type=\"hidden\" name=\"save_settings\" value=\"settings\" />\n"
    "    <table>\n"
    "     <tr>\n"
    "      <td>\n"
    "       {TR_SELECT_MEDIA_DIRECTORIES}:<br />\n"
    "       <iframe style=\"width:30em;height:30em;\" src=\"{SERVER_PATH}?folder_tree=\">\n"
    "        <a href=\"{SERVER_PATH}?folder_tree=\" target=\"_blank\">frame</a>\n"
    "       </iframe>\n"
    "      </td>\n"
    "      <td class=\"top\">\n"
    "       {TR_RIGHTS_EXPLAIN}<br /><br />\n"
    "       {TR_SLIDEDURATION}:\n"
    "       <input type=\"text\" size=\"6\" name=\"slideduration\" value=\"{SLIDEDURATION}\" />ms.<br /><br />\n"
    "       <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "      </td>\n"
    "     </tr>\n"
    "{ADDNETWORKDRIVE}"
    "    </table>\n"
    "   </form>\n"
    "  </fieldset>\n";

const char MediaPlayerServer::htmlSettingsAddNetworkDrive[] =
    "     <tr><td colspan=\"2\">\n"
    "      {TR_ADD_NETWORK_DRIVE_EXPLAIN}<br />\n"
    "      <table>\n"
    "       <tr>\n"
    "        <td></td>\n"
    "        <td>{TR_HOSTNAME}</td>\n"
    "        <td></td>\n"
    "        <td>{TR_SHARE_PATH}</td>\n"
#if !defined(Q_OS_WIN)
    "        <td>{TR_USERNAME}</td>\n"
    "        <td>{TR_PASSWORD}</td>\n"
#endif
    "        <td></td>\n"
    "       </tr>\n"
    "       <tr>\n"
    "        <td>smb://</td>\n"
    "        <td><input type=\"text\" size=\"20\" name=\"smbhostname\" value=\"\" /></td>\n"
    "        <td>/</td>\n"
    "        <td><input type=\"text\" size=\"20\" name=\"smbpath\" value=\"\" /></td>\n"
#if !defined(Q_OS_WIN)
    "        <td><input type=\"text\" size=\"10\" name=\"smbusername\" value=\"\" /></td>\n"
    "        <td><input type=\"password\" size=\"10\" name=\"smbpassword\" value=\"\" /></td>\n"
#endif
    "        <td><input type=\"submit\" name=\"smbadd\" value=\"{TR_ADD}\" /></td>\n"
    "       </tr>\n"
    "      </table>\n"
    "     </td></tr>\n";

const char MediaPlayerServer::htmlSettingsDirTreeIndex[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <title></title>\n"
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
    " <tr align=\"middle\" id=\"{DIR_FULLPATH}\"><td align=\"left\">\n"
    "{DIR_INDENT}"
    "{DIR_EXPAND}"
    "{DIR_CHECK}"
    "  {DIR_NAME}\n"
    " </td><td align=\"right\">\n"
    "{DIR_CONTENTTYPE}"
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

const char MediaPlayerServer::htmlSettingsDirTreeContentType[] =
    "  <a class=\"hidden\" href=\"{SERVER_PATH}?folder_tree=&open={DIR_ALLOPEN}&amp;{MUSIC_CHECKTYPE}={DIR_FULLPATH}#{DIR_FULLPATH}\">\n"
    "   <img src=\"/img/check{MUSIC_CHECKED}.png\" width=\"16\" height=\"16\" />\n"
    "  </a>\n";

QByteArray MediaPlayerServer::frontPageContent(void)
{
  if (!rootPaths.isEmpty())
  {
    SStringParser htmlParser;
    htmlParser.setField("SERVER_PATH", QUrl(serverPath()));

    return htmlParser.parse(htmlFrontPageContent);
  }

  return QByteArray();
}

QByteArray MediaPlayerServer::settingsContent(void)
{
  QSettings settings;
  settings.beginGroup(Module::pluginName);

  SStringParser htmlParser;
  htmlParser.setField("SERVER_PATH", QUrl(serverPath()));
  htmlParser.setField("TR_MEDIAPLAYER", tr(Module::pluginName));
  htmlParser.setField("TR_SLIDEDURATION", tr("Photo slideshow slide duration"));
  htmlParser.setField("TR_ADD", tr("Add"));
  htmlParser.setField("TR_SAVE", tr("Save"));
  htmlParser.setField("TR_SELECT_MEDIA_DIRECTORIES", tr("Select media directories"));
  htmlParser.setField("TR_HOSTNAME", tr("Hostname"));
  htmlParser.setField("TR_SHARE_PATH", tr("Share/Path"));
#if !defined(Q_OS_WIN)
  htmlParser.setField("TR_USERNAME", tr("Username"));
  htmlParser.setField("TR_PASSWORD", tr("Password"));
#endif

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

  htmlParser.setField("TR_ADD_NETWORK_DRIVE_EXPLAIN", tr(
      "A network drive can be added here. Provide the hostname or IP address "
      "of the server and a share name. "
#if !defined(Q_OS_WIN)
      "A username and password can be optionally provided, a guest accunt is "
      "used if omitted."
#endif
      ));

  if (fileProtocols().contains("smb"))
    htmlParser.setField("ADDNETWORKDRIVE", htmlParser.parse(htmlSettingsAddNetworkDrive));
  else
    htmlParser.setField("ADDNETWORKDRIVE", "");

  htmlParser.setField("SLIDEDURATION", QByteArray::number(settings.value("SlideDuration", 7500).toInt()));

  scanDrives();

  return htmlParser.parse(htmlSettingsMain);
}

void MediaPlayerServer::generateDirs(SStringParser &htmlParser, const QList<QUrl> &dirs, int indent, const QStringList &allopen, const QList<RootPath> &rootPaths)
{
  foreach (const QUrl &dir, dirs)
  {
    QString path = dir.toString();
    if (!path.endsWith('/'))
      path += '/';

    QString fileName = path.left(path.length() - 1);
    fileName = fileName.mid(fileName.lastIndexOf('/') + 1);

    htmlParser.setField("DIR_FULLPATH", path.toUtf8().toHex());

    // Indentation
    htmlParser.setField("DIR_INDENT", "");
    for (int i=0; i<indent; i++)
      htmlParser.appendField("DIR_INDENT", htmlParser.parse(htmlSettingsDirTreeIndent));

    htmlParser.setField("DIR_CONTENTTYPE", "");

    if (dir.scheme() != "file")
    {
      htmlParser.setField("DIR_CHECKED", "full");
      htmlParser.setField("DIR_CHECKTYPE", "checkoff");
      htmlParser.setField("DIR_ALLOPEN", "");
      htmlParser.setField("DIR_CHECK", htmlParser.parse(htmlSettingsDirTreeCheckLink));
      htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlSettingsDirTreeIndent));
      htmlParser.setField("DIR_NAME", fileName + " (" + dir.toString(QUrl::RemovePassword) + ")");
      htmlParser.appendField("DIRS", htmlParser.parse(htmlSettingsDirTreeDir));
    }
    else if (!fileName.startsWith('.'))
    {
      SMediaFilesystem filesystem(dir);

      QList<QUrl> children;
      foreach (const QString &child, filesystem.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
      if (!child.isEmpty())
        children += filesystem.filePath(child);

      const bool isSingleRoot = (indent == 0) && (dirs.count() == 1);
      const bool isReadable = filesystem.readInfo(".").isReadable;

      // Expand
      htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlSettingsDirTreeIndent));
      if (!isSingleRoot && isReadable && !children.isEmpty())
      {
        QStringList all = allopen;
        if (all.contains(path))
        {
          for (QStringList::Iterator i=all.begin(); i!=all.end(); )
          if (*i == path)
            i = all.erase(i);
          else
            i++;

          htmlParser.setField("DIR_OPEN", "open");
        }
        else
        {
          all.append(path);
          htmlParser.setField("DIR_OPEN", "close");
        }

        htmlParser.setField("DIR_ALLOPEN", qCompress(all.join(QString(dirSplit)).toUtf8()).toHex());
        htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlSettingsDirTreeExpand));
      }

      // Check
      bool checkEnabled = true, checked = false, music = false;
      htmlParser.setField("DIR_CHECKED", "none");
      htmlParser.setField("DIR_CHECKTYPE", "checkon");
      foreach (const RootPath &root, rootPaths)
      {
        const QString rootPath = root.url.toString();

        if (rootPath == path)
        {
          htmlParser.setField("DIR_CHECKED", "full");
          htmlParser.setField("DIR_CHECKTYPE", "checkoff");

          checkEnabled = checked = true;
          music = root.type == RootPath::Music;

          break;
        }
        else if (rootPath.startsWith(path))
        {
          htmlParser.setField("DIR_CHECKED", "some");
          htmlParser.setField("DIR_TITLE", "A child is selected");
          checkEnabled = false;
        }
        else if (path.startsWith(rootPath))
        {
          htmlParser.setField("DIR_CHECKED", "fulldisabled");
          htmlParser.setField("DIR_TITLE", "A parent is selected");
          checkEnabled = false;
        }
      }

      if (!isReadable)
      {
        htmlParser.setField("DIR_CHECKED", "nonedisabled");
        htmlParser.setField("DIR_TITLE", tr("Access denied"));
        checkEnabled = false;
      }

      htmlParser.setField("DIR_ALLOPEN", qCompress(allopen.join(QString(dirSplit)).toUtf8()).toHex());
      htmlParser.setField("DIR_CHECK", htmlParser.parse(checkEnabled ? htmlSettingsDirTreeCheckLink : htmlSettingsDirTreeCheck));

      // Name
      QMap<QUrl, QString>::Iterator i = driveList.find(dir);
      if (i != driveList.end())
      {
        htmlParser.setField("DIR_NAME", dir.path());
        if (!i->isEmpty())
          htmlParser.appendField("DIR_NAME", " (" + *i + ")");
      }
      else
        htmlParser.setField("DIR_NAME", fileName);

      if (checkEnabled && checked)
      {
        htmlParser.setField("MUSIC_CHECKTYPE", music ? "musicoff" : "musicon");
        htmlParser.setField("MUSIC_CHECKED", music ? "note" : "none");

        htmlParser.setField("DIR_CONTENTTYPE", htmlParser.parse(htmlSettingsDirTreeContentType));
      }

      htmlParser.appendField("DIRS", htmlParser.parse(htmlSettingsDirTreeDir));

      // Recurse
      if (isSingleRoot || (allopen.contains(path)))
        generateDirs(htmlParser, children, indent + 1, allopen, rootPaths);
    }
  }
}

void MediaPlayerServer::scanDrives(void)
{
  driveList.clear();

  QUrl root;
  root.setScheme("file");

  SMediaFilesystem filesystem(root);
  foreach (const QString &drive, filesystem.entryList(QDir::Dirs))
  {
    const SMediaFilesystem dir(filesystem.filePath(drive));
    if (!dir.entryList().isEmpty())
    {
      const QUrl path = filesystem.filePath(drive);
      driveList.insert(path, dirLabel(path.path()));
    }
  }

  foreach (const RootPath &rootPath, rootPaths)
  if (rootPath.url.scheme() != "file")
    driveList.insert(rootPath.url, rootPath.url.toString(QUrl::RemovePassword));
}

SHttpServer::ResponseMessage MediaPlayerServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (request.isGet())
  {
    if (request.query().hasQueryItem("thumbnail"))
    {
      const RootPath filePath = realPath(request.file());
      if (!filePath.url.isEmpty())
      {
        SSize size(128, 128);
        if (request.query().hasQueryItem("resolution"))
          size = SSize::fromString(request.query().queryItemValue("resolution"));

        SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
        response.setContentType(SHttpEngine::mimeImagePng);

        if (!request.isHead())
        {
          const QByteArray result = mediaDatabase->readThumbnail(filePath.url, size.size(), Qt::black, "png");
          if (!result.isEmpty())
          {
            response.setContent(makeThumbnail(
                size.size(),
                QImage::fromData(result),
                request.query().queryItemValue("overlay")));
          }
          else
          {
            QString defaultIcon = ":/img/null.png";
            switch (mediaDatabase->readNodeFormat(filePath.url).fileType())
            {
            case SMediaInfo::ProbeInfo::FileType_None:      defaultIcon = ":/img/null.png";           break;
            case SMediaInfo::ProbeInfo::FileType_Audio:     defaultIcon = ":/img/audio-file.png";     break;
            case SMediaInfo::ProbeInfo::FileType_Video:     defaultIcon = ":/img/video-file.png";     break;
            case SMediaInfo::ProbeInfo::FileType_Image:     defaultIcon = ":/img/image-file.png";     break;
            case SMediaInfo::ProbeInfo::FileType_Subtitle:  defaultIcon = ":/img/journal.png";        break;
            case SMediaInfo::ProbeInfo::FileType_Directory: defaultIcon = ":/img/directory.png";      break;
            case SMediaInfo::ProbeInfo::FileType_Drive:     defaultIcon = ":/img/drive.png";          break;
            case SMediaInfo::ProbeInfo::FileType_Disc:      defaultIcon = ":/img/media-optical.png";  break;
            }

            response.setContent(makeThumbnail(size.size(), QImage(defaultIcon)));
          }
        }

        return response;
      }

      return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
    }
    else if (request.query().hasQueryItem("save_settings"))
    {
      if (request.query().hasQueryItem("save"))
      {
        QSettings settings;
        settings.beginGroup(Module::pluginName);
  
        settings.setValue("SlideDuration", qBound(2500, request.query().queryItemValue("slideduration").toInt(), 60000));
      }
      else if (request.query().hasQueryItem("smbadd"))
      {
        QUrl url;
        url.setScheme("smb");
        url.setHost(QString::fromUtf8(QByteArray::fromPercentEncoding(
              request.query().queryItemValue("smbhostname", QUrl::FullyEncoded).replace('+', ' ').toLatin1())));
        url.setPath(QString::fromUtf8(QByteArray::fromPercentEncoding(
              request.query().queryItemValue("smbpath", QUrl::FullyEncoded).replace('+', ' ').toLatin1())));
        url.setUserName(QString::fromUtf8(QByteArray::fromPercentEncoding(
              request.query().queryItemValue("smbusername", QUrl::FullyEncoded).replace('+', ' ').toLatin1())));
        url.setPassword(QString::fromUtf8(QByteArray::fromPercentEncoding(
              request.query().queryItemValue("smbpassword", QUrl::FullyEncoded).replace('+', ' ').toLatin1())));

        if (url.isValid())
        {
          QList<RootPath> paths = rootPaths.values();
          paths += RootPath(url, RootPath::Auto);
          setRootPaths(paths);
          scanDrives();
        }
      }
      
      SHttpServer::ResponseMessage response(request, SHttpServer::Status_TemporaryRedirect);
      response.setCacheControl(-1);
      response.setField("Location", "http://" + request.host() + "/settings");
      return response;
    }
    else if (request.query().hasQueryItem("folder_tree"))
    {
      QList<RootPath> paths = rootPaths.values();

      const QString open = request.query().queryItemValue("open");
      const QStringList allopen = !open.isEmpty()
                                  ? QString::fromUtf8(qUncompress(QByteArray::fromHex(open.toLatin1()))).split(dirSplit)
                                  : QStringList();

      const QString checkon = QString::fromUtf8(QByteArray::fromHex(request.query().queryItemValue("checkon").toLatin1()));
      const QString checkoff = QString::fromUtf8(QByteArray::fromHex(request.query().queryItemValue("checkoff").toLatin1()));
      if (!checkon.isEmpty() || !checkoff.isEmpty())
      {
        if (!checkon.isEmpty())
          paths.append(RootPath(checkon.endsWith('/') ? checkon : (checkon + '/'), RootPath::Auto));

        if (!checkoff.isEmpty())
        for (QList<RootPath>::Iterator i=paths.begin(); i!=paths.end(); )
        {
          QString path = i->url.toString();
          if (!path.endsWith('/'))
            path += '/';

          if (path == checkoff)
            i = paths.erase(i);
          else
            i++;
        }

        setRootPaths(paths);
        scanDrives();
      }

      QString musicon = QString::fromUtf8(QByteArray::fromHex(request.query().queryItemValue("musicon").toLatin1()));
      QString musicoff = QString::fromUtf8(QByteArray::fromHex(request.query().queryItemValue("musicoff").toLatin1()));
      if (!musicon.isEmpty() || !musicoff.isEmpty())
      {
        if (!musicon.isEmpty() && !musicon.endsWith('/')) musicon += '/';
        if (!musicoff.isEmpty() && !musicoff.endsWith('/')) musicoff += '/';

        for (QList<RootPath>::Iterator i=paths.begin(); i!=paths.end(); i++)
        {
          QString path = i->url.toString();
          if (!path.endsWith('/'))
            path += '/';

          if (path == musicon)
            i->type = RootPath::Music;
          else if (path == musicoff)
            i->type = RootPath::Auto;
        }

        setRootPaths(paths);
        scanDrives();
      }

      SStringParser htmlParser;
      htmlParser.setField("SERVER_PATH", QUrl(serverPath()));
      htmlParser.setField("DIRS", "");
      generateDirs(htmlParser, driveList.keys(), 0, allopen, paths);

      SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
      response.setCacheControl(-1);
      response.setContentType(SHttpEngine::mimeTextHtml);
      response.setContent(htmlParser.parse(htmlSettingsDirTreeIndex));
      return response;
    }
  }

  return MediaServer::httpRequest(request, socket);
}

} } // End of namespaces
