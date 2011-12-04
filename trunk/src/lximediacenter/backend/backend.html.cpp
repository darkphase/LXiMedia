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

#include "backend.h"

const char Backend::htmlIndex[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <title>{_PRODUCT} @ {_HOSTNAME}</title>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <script type=\"text/javascript\" src=\"/js/list.js\"></script>\n" // Open and close tag due to IE bug
    "{HEAD}"
    "</head>\n"
    "<body>\n"
    " <div class=\"main_navigator\" id=\"navigator\">\n"
    "  <table>\n"
    "   <tr>\n"
    "    <td width=\"100%\">\n"
    "{NAVIGATOR_PATH}"
    "    </td>\n"
    "    <td>\n"
    "     <a href=\"/\">\n"
    "      <img src=\"/lximedia.png?scale=32\" alt=\"Home\" />\n"
    "     </a>\n"
    "    </td>\n"
    "    <td>\n"
    "     <a href=\"/settings\">\n"
    "      <img src=\"/img/control.png?scale=32\" alt=\"Settings\" />\n"
    "     </a>\n"
    "    </td>\n"
    "    <td>\n"
    "     <a href=\"/log\">\n"
    "      <img src=\"/img/journal.png?scale=32\" alt=\"Log\" />\n"
    "     </a>\n"
    "    </td>\n"
    "    <td>\n"
    "     <a href=\"/about\">\n"
    "      <img src=\"/img/glossary.png?scale=32\" alt=\"About\" />\n"
    "     </a>\n"
    "    </td>\n"
#ifndef QT_NO_DEBUG
    "    <td>\n"
    "     <a href=\"/exit\">\n"
    "      <img src=\"/img/control.png?scale=32\" alt=\"Exit\" />\n"
    "     </a>\n"
    "    </td>\n"
#endif
    "   </tr>\n"
    "  </table>\n"
    " </div>\n"
    "{CONTENT}"
    "</body>\n"
    "</html>\n";

const char Backend::htmlNavigatorRoot[] =
    "     <h1>{_HOSTNAME}</h1>\n";

const char Backend::htmlNavigatorPath[] =
    "     <ul>\n"
    "{NAVIGATOR_ITEMS}"
    "     </ul>\n";

const char Backend::htmlNavigatorItem[] =
    "      <li><a href=\"{ITEM_LINK}\">{ITEM_NAME}</a></li>\n";

const char Backend::htmlFrontPages[] =
    " <div class=\"main_frontpages\">\n"
    "{FRONTPAGES}"
    " </div>\n";

const char Backend::htmlFrontPageItem[] =
    "  <div class=\"frontpage\">\n"
    "   <h1>{ITEM_TITLE}</h1>\n"
    "{ITEM_CONTENT}"
    "  </div>\n";

const char Backend::htmlLogFile[] =
    " <div class=\"main_log\">\n"
    "  <table>\n"
    "   <tr>\n"
    "    <th class=\"nostretch\">{TR_DATE}</th>\n"
    "    <th class=\"nostretch\">{TR_TYPE}</th>\n"
    "    <th class=\"nostretch\">PID:TID</th>\n"
    "    <th class=\"stretch\">{TR_MESSAGE}</th>\n"
    "   </tr>\n"
    "{LOG_MESSAGES}"
    "  </table>\n"
    "  <a name=\"bottom\"></a>\n"
    " </div>\n";

const char Backend::htmlLogFileHeadline[] =
    "   <tr>\n"
    "    <td class=\"nostretch\" rowspan=\"{ITEM_ROWS}\">\n"
    "     {ITEM_DATE}\n"
    "    </td>\n"
    "    <td class=\"nostretch\" rowspan=\"{ITEM_ROWS}\">\n"
    "     {ITEM_TYPE}\n"
    "    </td>\n"
    "    <td class=\"nostretch\" rowspan=\"{ITEM_ROWS}\">\n"
    "     {ITEM_PID}:{ITEM_TID}\n"
    "    </td>\n"
    "    <td class=\"stretch\">\n"
    "     {ITEM_HEADLINE}\n"
    "    </td>\n"
    "   </tr>\n";

const char Backend::htmlLogFileMessage[] =
    "   <tr>\n"
    "    <td>\n"
    "{ITEM_MESSAGE}\n"
    "    </td>\n"
    "   </tr>\n";

const char Backend::htmlAbout[] =
    " <div class=\"main_about\">\n"
    "{ABOUT_LXIMEDIA}"
    " </div>\n";

const char Backend::htmlSettingsMain[] =
    " <div class=\"main_settings\">\n"
    "  <fieldset>\n"
    "   <legend>{TR_HTTP_SERVER}</legend>\n"
    "   {TR_HTTPSERVER_EXPLAIN}<br />\n"
    "   <br />\n"
    "   <form name=\"httpsettings\" action=\"/settings\" method=\"get\">\n"
    "    <input type=\"hidden\" name=\"save_settings\" value=\"http\" />\n"
    "    {TR_HTTP_PORT_NUMBER}:\n"
    "    <input type=\"text\" size=\"6\" name=\"httpport\" value=\"{HTTPPORT}\" />\n"
    "    {TR_DEVICE_NAME}:\n"
    "    <input type=\"text\" size=\"40\" name=\"devicename\" value=\"{DEVICENAME}\" /><br />\n"
    "    <br />\n"
    "    <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "   </form>\n"
    "  </fieldset>\n"
    "  <fieldset>\n"
    "   <legend>{TR_DLNA}</legend>\n"
    "   {TR_MEDIA_TRANSCODE_SETTINGS_EXPLAIN}<br />\n"
    "   <br />\n"
    "   {TR_AUDIO_TRANSCODE_SETTINGS_EXPLAIN}<br />\n"
    "   <br />\n"
    "{CLIENT_ROWS}"
    "   {TR_CLIENT_SETTINGS_EXPLAIN}<br />\n"
    "  </fieldset>\n"
    "{PLUGIN_SETTINGS}"
    " </div>\n";

const char Backend::htmlSettingsDlnaRow[] =
    "   <a name=\"{CLIENT_NAME}\"></a>\n"
    "   <fieldset>\n"
    "    <legend>{NAME}</legend>\n"
    "    <form name=\"dlnasettings\" action=\"/settings\" method=\"get\">\n"
    "     <input type=\"hidden\" name=\"save_settings\" value=\"dlna\" />\n"
    "     <input type=\"hidden\" name=\"client\" value=\"{CLIENT_NAME}\" />\n"
    "     <table>\n"
    "      <tr>\n"
    "       <td>{TR_VIDEO_SETTINGS}:</td>\n"
    "       <td>\n"
    "        <select name=\"transcodesize\">\n"
    "{FORMATS}"
    "        </select>\n"
    "        <select name=\"cropmode\">\n"
    "         <option value=\"Box\" {SELECTED_BOX}>{TR_LETTERBOX}</option>\n"
    "         <option value=\"Zoom\" {SELECTED_ZOOM}>{TR_FULLSCREEN}</option>\n"
    "        </select>\n"
    "        <select name=\"channels\">\n"
    "{CHANNELS}"
    "        </select>\n"
    "        <select name=\"encodemode\">\n"
    "         <option value=\"Fast\" {SELECTED_FAST}>{TR_FAST}</option>\n"
    "         <option value=\"Slow\" {SELECTED_SLOW}>{TR_HIGH_QUALITY}</option>\n"
    "        </select>\n"
    "       </td>\n"
    "      </tr>\n"
    "      <tr>\n"
    "       <td>{TR_MUSIC_SETTINGS}:</td>\n"
    "       <td>\n"
    "        <select name=\"musicchannels\">\n"
    "{MUSICCHANNELS}"
    "        </select>\n"
    "        <input type=\"checkbox\" name=\"musicaddvideo\" value=\"on\" {CHECKED_ADDBLACKVIDEO} />{TR_ADD_BLACK_VIDEO}\n"
    "       </td>\n"
    "      </tr>\n"
    "{PROFILES}"
    "     </table>\n"
    "     <br />\n"
    "     <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "     <input type=\"submit\" name=\"defaults\" value=\"{TR_DEFAULTS}\" />\n"
    "    </form>\n"
    "   </fieldset>\n";

const char Backend::htmlSettingsDlnaRowProfilesClosed[] =
    "      <tr>\n"
    "       <td class=\"top\">{TR_SUPPORTED_DLNA_PROFILES}:</td>\n"
    "       <td>\n"
    "        <a href=\"settings?expand={CLIENT_NAME}\">\n"
    "         {TR_SHOW_PROFILE_SETTINGS}\n"
    "        </a>\n"
    "       </td>\n"
    "      </tr>\n";

const char Backend::htmlSettingsDlnaRowProfiles[] =
    "      <tr>\n"
    "       <td class=\"top\">{TR_SUPPORTED_DLNA_PROFILES}:</td>\n"
    "       </td>\n"
    "       <td>\n"
    "        <input type=\"hidden\" name=\"expand\" value=\"{CLIENT_NAME}\" />\n"
    "        <table>\n"
    "         <tr>\n"
    "          <th>{TR_AUDIO_PROFILES}</th>\n"
    "          <th>{TR_VIDEO_PROFILES}</th>\n"
    "          <th>{TR_IMAGE_PROFILES}</th>\n"
    "         </tr>\n"
    "         <tr>\n"
    "          <td class=\"top\">\n"
    "{AUDIO_PROFILES}"
    "          </td>\n"
    "          <td class=\"top\">\n"
    "{VIDEO_PROFILES}"
    "          </td>\n"
    "          <td class=\"top\">\n"
    "{IMAGE_PROFILES}"
    "          </td>\n"
    "         </tr>"
    "         <tr>\n"
    "          <td colspan=\"3\">\n"
    "           <div style=\"width: 50em;\">\n"
    "            {TR_POST_PROFILES}<br/>\n"
    "           </div>\n"
    "           <div style=\"width: 50em; background-color: #FFFFFF; border: 1px solid #000000;\">\n"
    "            [{INI_NAME}]<br/>\n"
    "            AudioProfiles={INI_AUDIO_PROFILES}<br/>\n"
    "            VideoProfiles={INI_VIDEO_PROFILES}<br/>\n"
    "            ImageProfiles={INI_IMAGE_PROFILES}<br/>\n"
    "           </div>\n"
    "          </td>\n"
    "         </tr>\n"
    "        </table>\n"
    "       </td>\n"
    "      </tr>\n";

const char Backend::htmlSettingsDlnaRowProfilesCheck[] =
    "           <input type=\"checkbox\" name=\"profile_{PROFILE_NAME}\" value=\"on\" {CHECKED}/>{PROFILE_NAME}<br/>\n";

const char Backend::htmlSettingsOption[] =
    "         <option value=\"{VALUE}\" {SELECTED}>{TEXT}</option>\n";


SHttpServer::ResponseMessage Backend::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *)
{
  if (request.isGet())
  {
    QString dir = request.file();
    dir = dir.left(dir.lastIndexOf('/') + 1);

    if (dir == "/")
    {
      HtmlParser htmlParser(this->htmlParser);
      QByteArray content;

      if (request.fileName().isEmpty())
      {
        htmlParser.setField("FRONTPAGES", QByteArray(""));
        foreach (BackendServer *backendServer, backendServers)
        {
          htmlParser.setField("ITEM_TITLE", backendServer->serverName());
          htmlParser.setField("ITEM_CONTENT", backendServer->frontPageContent());
          htmlParser.appendField("FRONTPAGES", htmlParser.parse(htmlFrontPageItem));
        }

        content = htmlParser.parse(htmlFrontPages);
      }
      else if (request.fileName() == "settings")
      {
        if (request.url().hasQueryItem("save_settings"))
        {
          saveHtmlSettings(request);

          SHttpServer::ResponseMessage response(request, SHttpServer::Status_MovedPermanently);
          response.setField("Location", "http://" + request.host() + "/settings");
          return response;
        }

        content = handleHtmlSettings(request);
      }
      else if (request.fileName() == "log")
      {
        SApplication::LogFile logFile(sApp->activeLogFile());
        if (logFile.open(SApplication::LogFile::ReadOnly))
        {
          htmlParser.setField("TR_DATE", tr("Date"));
          htmlParser.setField("TR_TYPE", tr("Type"));
          htmlParser.setField("TR_MESSAGE", tr("Message"));

          htmlParser.setField("LOG_MESSAGES", QByteArray(""));

          for (SApplication::LogFile::Message msg=logFile.readMessage();
               msg.date.isValid();
               msg=logFile.readMessage())
          {
            const bool mr = !msg.message.isEmpty();

            htmlParser.setField("ITEM_ROWS", QByteArray::number(mr ? 2 : 1));

            htmlParser.setField("ITEM_ROWS", QByteArray::number(mr ? 2 : 1));
            htmlParser.setField("ITEM_DATE", msg.date.toString("yyyy-MM-dd/hh:mm:ss"));
            htmlParser.setField("ITEM_TYPE", msg.type);
            htmlParser.setField("ITEM_PID", QByteArray::number(msg.pid));
            htmlParser.setField("ITEM_TID", QByteArray::number(msg.tid));
            htmlParser.setField("ITEM_TYPE", msg.type);
            htmlParser.setField("ITEM_HEADLINE", msg.headline);
            htmlParser.appendField("LOG_MESSAGES", htmlParser.parse(htmlLogFileHeadline));

            if (mr)
            {
              htmlParser.setField("ITEM_MESSAGE", msg.message.replace('\n', "<br />\n"));
              htmlParser.appendField("LOG_MESSAGES", htmlParser.parse(htmlLogFileMessage));
            }
          }

          content = htmlParser.parse(htmlLogFile);
        }
      }
      else if (request.fileName() == "about")
      {
        htmlParser.setField("ABOUT_LXIMEDIA", sApp->about());
        content = htmlParser.parse(htmlAbout);
      }
#ifndef QT_NO_DEBUG
      else if (request.fileName() == "exit")
      {
        QCoreApplication::postEvent(this, new QEvent(exitEventType));
        return SHttpServer::ResponseMessage(request, SHttpServer::Status_NoContent);
      }
#endif
      else if (request.fileName() == "favicon.ico")
        return sendFile(request, ":/lximedia.ico");
      else if (request.fileName() == "lximedia.png")
        return sendFile(request, ":/lximedia.png");

      SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
      response.setContentType(SHttpEngine::mimeTextHtml);
      response.setField("Cache-Control", "no-cache");
      response.setContent(parseHtmlContent(request.url(), content, ""));
      return response;
    }
    else if ((dir == "/css/") || (dir == "/js/") || (dir == "/img/"))
      return sendFile(request, ":" + request.file());
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

QByteArray Backend::parseHtmlContent(const QUrl &url, const QByteArray &content, const QByteArray &head) const
{
  HtmlParser htmlParser(this->htmlParser);

  htmlParser.setField("HEAD", head);

  QString path = url.path();
  path = path.left(path.lastIndexOf('/'));
  if (!path.isEmpty())
  {
    htmlParser.setField("NAVIGATOR_ITEMS", QByteArray(""));

    QString fullPath = "/";
    foreach (const QString &dir, path.split('/', QString::SkipEmptyParts))
    {
      fullPath += dir + "/";
      htmlParser.setField("ITEM_LINK", QUrl(fullPath).toEncoded());
      htmlParser.setField("ITEM_NAME", dir);
      htmlParser.appendField("NAVIGATOR_ITEMS", htmlParser.parse(htmlNavigatorItem));
    }

    htmlParser.setField("NAVIGATOR_PATH", htmlParser.parse(htmlNavigatorPath));
  }
  else
    htmlParser.setField("NAVIGATOR_PATH", htmlParser.parse(htmlNavigatorRoot));

  htmlParser.setField("CONTENT", content);
  return htmlParser.parse(htmlIndex);
}

//SHttpServer::ResponseMessage Backend::handleHtmlSearch(const SHttpServer::RequestMessage &request, const MediaServer::File &)
//{
//  HtmlParser htmlParser(this->htmlParser);
//  htmlParser.setField("TR_OF", tr("of"));
//  htmlParser.setField("TR_RELEVANCE", tr("Relevance"));
//  htmlParser.setField("TR_RESULTS", tr("Results"));
//
//  SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
//  response.setContentType(SHttpEngine::mimeTextHtml);
//  response.setField("Cache-Control", "no-cache");
//
//  const MediaServer::File file(request);
//  const QString queryValue = file.url().queryItemValue("q");
//  const QString queryString = QByteArray::fromPercentEncoding(queryValue.toAscii().replace('+', ' '));
//  const SearchCacheEntry entry = search(queryString);
//  htmlParser.setField("SEARCHRESULTS", QByteArray(""));
//
//  foreach (const BackendServer::SearchResult &result, entry.results)
//  {
//    htmlParser.setField("ITEM_TITLE", result.headline);
//    htmlParser.setField("ITEM_RELEVANCE", QString::number(qBound(0, int(result.relevance * 100.0), 100)) + "%");
//    htmlParser.setField("ITEM_URL", QUrl(result.location).toEncoded());
//    htmlParser.setField("ITEM_ICONURL", QUrl(result.thumbLocation).toEncoded());
//
//    htmlParser.appendField("SEARCHRESULTS", htmlParser.parse(htmlSearchResultsItem));
//  }
//
//  htmlParser.setField("ITEM_TITLE", tr("Search") + ": " + queryString);
//
//  response.setContent(parseHtmlContent(file.url(), htmlParser.parse(htmlSearchResults), ""));
//
//  return response;
//}

QByteArray Backend::handleHtmlSettings(const SHttpServer::RequestMessage &request)
{
  GlobalSettings settings;

  HtmlParser htmlParser;
  htmlParser.setField("TR_HTTP_SERVER", tr("HTTP server"));
  htmlParser.setField("TR_HTTP_PORT_NUMBER", tr("Preferred HTTP port number"));
  htmlParser.setField("TR_DEVICE_NAME", tr("Device name"));
  htmlParser.setField("TR_HTTPSERVER_EXPLAIN",
    tr("This configures the internal HTTP server."));

  htmlParser.setField("HTTPPORT", settings.value("HttpPort", settings.defaultBackendHttpPort()).toString());
  htmlParser.setField("DEVICENAME", settings.value("DeviceName", settings.defaultDeviceName()).toString());

  settings.beginGroup("DLNA");

  const QStringList enabledAudioProfiles = MediaServer::mediaProfiles().enabledAudioProfiles();
  const QStringList enabledVideoProfiles = MediaServer::mediaProfiles().enabledVideoProfiles();
  const QStringList enabledImageProfiles = MediaServer::mediaProfiles().enabledImageProfiles();

  const QString genericTranscodeSize =
      settings.value("TranscodeSize", settings.defaultTranscodeSizeName()).toString();
  const QString genericTranscodeCrop =
      settings.value("TranscodeCrop", settings.defaultTranscodeCropName()).toString();
  const QString genericEncodeMode =
      settings.value("EncodeMode", settings.defaultEncodeModeName()).toString();
  const QString genericTranscodeChannels =
      settings.value("TranscodeChannels", settings.defaultTranscodeChannelName()).toString();
  const QString genericTranscodeMusicChannels =
      settings.value("TranscodeMusicChannels", settings.defaultTranscodeMusicChannelName()).toString();
  const bool genericMusicAddBlackVideo =
      settings.value("MusicAddBlackVideo", settings.defaultMusicAddBlackVideo()).toBool();

  htmlParser.setField("TR_DLNA", tr("DLNA"));
  htmlParser.setField("TR_VIDEO_SETTINGS", tr("Video transcode settings"));
  htmlParser.setField("TR_MUSIC_SETTINGS", tr("Music transcode settings"));
  htmlParser.setField("TR_SUPPORTED_DLNA_PROFILES", tr("Supported DLNA profiles"));
  htmlParser.setField("TR_SHOW_PROFILE_SETTINGS", tr("Show profile settings"));
  htmlParser.setField("TR_AUDIO_PROFILES", tr("Audio profiles"));
  htmlParser.setField("TR_VIDEO_PROFILES", tr("Video profiles"));
  htmlParser.setField("TR_IMAGE_PROFILES", tr("Image profiles"));
  htmlParser.setField("TR_USER_AGENT", tr("User agent"));
  htmlParser.setField("TR_SAVE", tr("Save"));
  htmlParser.setField("TR_DEFAULTS", tr("Defaults"));
  htmlParser.setField("TR_LETTERBOX", tr("Letterbox"));
  htmlParser.setField("TR_FULLSCREEN", tr("Fullscreen"));
  htmlParser.setField("TR_FAST", tr("Fast"));
  htmlParser.setField("TR_HIGH_QUALITY", tr("High quality"));
  htmlParser.setField("TR_ADD_BLACK_VIDEO", tr("Add black video"));

  htmlParser.setField("TR_MEDIA_TRANSCODE_SETTINGS_EXPLAIN",
    tr("This form will allow adjustment of the transcode settings for the DLNA "
       "clients. Note that higher settings require more CPU power and more "
       "network bandwidth. The \"Letterbox\" setting will add black bars to "
       "the image if the aspect ratio does not match, whereas the \"Fullscreen\" "
       "setting will zoom in and cut off part of the image. The \"High quality\" "
       "setting will use more CPU power but gives higher quality images than the "
       "\"Fast\" setting, which only encodes intra-frames."));

  htmlParser.setField("TR_AUDIO_TRANSCODE_SETTINGS_EXPLAIN",
    tr("There are two separate transcode settings for music. The "
       "\"4.0 Quadraphonic\" setting can be used to duplicate the front channels "
       "to the rear channels. Furthermore, the \"Add black video\" setting can "
       "be used to add a video stream with black images to simulate that a TV is "
       "switched off (audio only)."));

  htmlParser.setField("TR_CLIENT_SETTINGS_EXPLAIN",
    tr("A box will appear here for each DLNA device that connects to this "
       "server, these boxes provide device specific settings."));

  htmlParser.setField("TR_POST_PROFILES",
    tr("If your device does not work with the default enabled DLNA profiles and "
       "you were able to make it work by disabling the unsupported profiles "
       "above, please take the time to post the configuration below on the "
       "<a href=\"http://sourceforge.net/projects/lximedia/forums/forum/1178772\">"
       "Sourceforge LXIMedia forum</a>. These settings will then be added to the "
       "next release."));

  htmlParser.setField("CLIENT_ROWS", QByteArray(""));

  struct T
  {
    static void addFormat(HtmlParser &htmlParser, GlobalSettings &settings, const QString &genericTranscodeSize, const GlobalSettings::TranscodeSize &size)
    {
      if (settings.value("TranscodeSize", genericTranscodeSize).toString() == size.name)
        htmlParser.setField("SELECTED", QByteArray("selected=\"selected\""));
      else
        htmlParser.setField("SELECTED", QByteArray(""));

      htmlParser.setField("VALUE", size.name);
      htmlParser.setField("TEXT", size.name +
                          " (" + QString::number(size.size.width()) +
                          "x" + QString::number(size.size.height()) + ")");
      htmlParser.appendField("FORMATS", htmlParser.parse(htmlSettingsOption));
    }

    static void addChannel(HtmlParser &htmlParser, GlobalSettings &settings, const QString &genericTranscodeChannels, const GlobalSettings::TranscodeChannel &channel)
    {
      if (settings.value("TranscodeChannels", genericTranscodeChannels).toString() == channel.name)
        htmlParser.setField("SELECTED", QByteArray("selected=\"selected\""));
      else
        htmlParser.setField("SELECTED", QByteArray(""));

      htmlParser.setField("VALUE", channel.name);
      htmlParser.setField("TEXT", channel.name);
      htmlParser.appendField("CHANNELS", htmlParser.parse(htmlSettingsOption));
    }

    static void addMusicChannel(HtmlParser &htmlParser, GlobalSettings &settings, const QString &genericTranscodeMusicChannels, const GlobalSettings::TranscodeChannel &channel)
    {
      if (settings.value("TranscodeMusicChannels", genericTranscodeMusicChannels).toString() == channel.name)
        htmlParser.setField("SELECTED", QByteArray("selected=\"selected\""));
      else
        htmlParser.setField("SELECTED", QByteArray(""));

      htmlParser.setField("VALUE", channel.name);
      htmlParser.setField("TEXT", channel.name);
      htmlParser.appendField("MUSICCHANNELS", htmlParser.parse(htmlSettingsOption));
    }
  };

  // Default settings
  htmlParser.setField("NAME", tr("Default settings"));
  htmlParser.setField("CLIENT_NAME", QByteArray("_default"));
  htmlParser.setField("FORMATS", QByteArray(""));
  htmlParser.setField("CHANNELS", QByteArray(""));
  htmlParser.setField("MUSICCHANNELS", QByteArray(""));

  foreach (const GlobalSettings::TranscodeSize &size, settings.allTranscodeSizes())
    T::addFormat(htmlParser, settings, genericTranscodeSize,  size);

  foreach (const GlobalSettings::TranscodeChannel &channel, settings.allTranscodeChannels())
    T::addChannel(htmlParser, settings, genericTranscodeChannels, channel);

  foreach (const GlobalSettings::TranscodeChannel &channel, settings.allTranscodeChannels())
    T::addMusicChannel(htmlParser, settings, genericTranscodeMusicChannels, channel);

  if (settings.value("TranscodeCrop", genericTranscodeCrop).toString() == "Box")
  {
    htmlParser.setField("SELECTED_BOX", QByteArray("selected=\"selected\""));
    htmlParser.setField("SELECTED_ZOOM", QByteArray(""));
  }
  else
  {
    htmlParser.setField("SELECTED_BOX", QByteArray(""));
    htmlParser.setField("SELECTED_ZOOM", QByteArray("selected=\"selected\""));
  }

  if (settings.value("EncodeMode", genericEncodeMode).toString() == "Fast")
  {
    htmlParser.setField("SELECTED_FAST", QByteArray("selected=\"selected\""));
    htmlParser.setField("SELECTED_SLOW", QByteArray(""));
  }
  else
  {
    htmlParser.setField("SELECTED_FAST", QByteArray(""));
    htmlParser.setField("SELECTED_SLOW", QByteArray("selected=\"selected\""));
  }

  if (settings.value("MusicAddBlackVideo", genericMusicAddBlackVideo).toBool())
    htmlParser.setField("CHECKED_ADDBLACKVIDEO", QByteArray("checked=\"checked\""));
  else
    htmlParser.setField("CHECKED_ADDBLACKVIDEO", QByteArray(""));

  htmlParser.setField("PROFILES", QByteArray(""));

  htmlParser.appendField("CLIENT_ROWS", htmlParser.parse(htmlSettingsDlnaRow));

  // DLNA clients
  QStringList activeClients = MediaServer::activeClients().toList();
  qSort(activeClients);

  foreach (const QString &activeClient, activeClients)
  {
    const QString clientTag = SStringParser::toCleanName(activeClient).replace(' ', '_');

    settings.beginGroup("Client_" + clientTag);

    htmlParser.setField("NAME", activeClient);
    htmlParser.setField("CLIENT_NAME", clientTag);
    htmlParser.setField("FORMATS", QByteArray(""));
    htmlParser.setField("CHANNELS", QByteArray(""));
    htmlParser.setField("MUSICCHANNELS", QByteArray(""));

    foreach (const GlobalSettings::TranscodeSize &size, settings.allTranscodeSizes())
      T::addFormat(htmlParser, settings, genericTranscodeSize,  size);

    foreach (const GlobalSettings::TranscodeChannel &channel, settings.allTranscodeChannels())
      T::addChannel(htmlParser, settings, genericTranscodeChannels, channel);

    foreach (const GlobalSettings::TranscodeChannel &channel, settings.allTranscodeChannels())
      T::addMusicChannel(htmlParser, settings, genericTranscodeMusicChannels, channel);

    if (settings.value("TranscodeCrop", genericTranscodeCrop).toString() == "Box")
    {
      htmlParser.setField("SELECTED_BOX", QByteArray("selected=\"selected\""));
      htmlParser.setField("SELECTED_ZOOM", QByteArray(""));
    }
    else
    {
      htmlParser.setField("SELECTED_BOX", QByteArray(""));
      htmlParser.setField("SELECTED_ZOOM", QByteArray("selected=\"selected\""));
    }

    if (settings.value("EncodeMode", genericEncodeMode).toString() == "Fast")
    {
      htmlParser.setField("SELECTED_FAST", QByteArray("selected=\"selected\""));
      htmlParser.setField("SELECTED_SLOW", QByteArray(""));
    }
    else
    {
      htmlParser.setField("SELECTED_FAST", QByteArray(""));
      htmlParser.setField("SELECTED_SLOW", QByteArray("selected=\"selected\""));
    }

    if (settings.value("MusicAddBlackVideo", genericMusicAddBlackVideo).toBool())
      htmlParser.setField("CHECKED_ADDBLACKVIDEO", QByteArray("checked=\"checked\""));
    else
      htmlParser.setField("CHECKED_ADDBLACKVIDEO", QByteArray(""));

    if (request.url().queryItemValue("expand") == clientTag)
    {
      htmlParser.setField("INI_NAME", activeClient.split('@').first());

      QStringList checkedAudioProfiles =
          settings.value(
              "SupportedAudioProfiles",
              MediaServer::mediaProfiles().supportedAudioProfiles(activeClient)
              ).toStringList();

      if (checkedAudioProfiles.isEmpty())
        checkedAudioProfiles = enabledAudioProfiles;

      htmlParser.setField("AUDIO_PROFILES", QByteArray(""));
      foreach (const QString &profile, enabledAudioProfiles)
      {
        htmlParser.setField("PROFILE_NAME", profile);
        htmlParser.setField("CHECKED", QByteArray(checkedAudioProfiles.contains(profile) ? "checked=\"checked\"" : ""));
        htmlParser.appendField("AUDIO_PROFILES", htmlParser.parse(htmlSettingsDlnaRowProfilesCheck));
      }

      htmlParser.setField("INI_AUDIO_PROFILES", checkedAudioProfiles.join(", "));

      QStringList checkedVideoProfiles =
          settings.value(
              "SupportedVideoProfiles",
              MediaServer::mediaProfiles().supportedVideoProfiles(activeClient)
              ).toStringList();

      if (checkedVideoProfiles.isEmpty())
        checkedVideoProfiles = enabledVideoProfiles;

      htmlParser.setField("VIDEO_PROFILES", QByteArray(""));
      foreach (const QString &profile, enabledVideoProfiles)
      {
        htmlParser.setField("PROFILE_NAME", profile);
        htmlParser.setField("CHECKED", QByteArray(checkedVideoProfiles.contains(profile) ? "checked=\"checked\"" : ""));
        htmlParser.appendField("VIDEO_PROFILES", htmlParser.parse(htmlSettingsDlnaRowProfilesCheck));
      }

      htmlParser.setField("INI_VIDEO_PROFILES", checkedVideoProfiles.join(", "));

      QStringList checkedImageProfiles =
          settings.value(
              "SupportedImageProfiles",
              MediaServer::mediaProfiles().supportedImageProfiles(activeClient)
              ).toStringList();

      if (checkedImageProfiles.isEmpty())
        checkedImageProfiles = enabledImageProfiles;

      htmlParser.setField("IMAGE_PROFILES", QByteArray(""));
      foreach (const QString &profile, enabledImageProfiles)
      {
        htmlParser.setField("PROFILE_NAME", profile);
        htmlParser.setField("CHECKED", QByteArray(checkedImageProfiles.contains(profile) ? "checked=\"checked\"" : ""));
        htmlParser.appendField("IMAGE_PROFILES", htmlParser.parse(htmlSettingsDlnaRowProfilesCheck));
      }

      htmlParser.setField("INI_IMAGE_PROFILES", checkedImageProfiles.join(", "));

      htmlParser.setField("PROFILES", htmlParser.parse(htmlSettingsDlnaRowProfiles));
    }
    else
      htmlParser.setField("PROFILES", htmlParser.parse(htmlSettingsDlnaRowProfilesClosed));

    settings.endGroup();
    htmlParser.appendField("CLIENT_ROWS", htmlParser.parse(htmlSettingsDlnaRow));
  }

  htmlParser.setField("PLUGIN_SETTINGS", QByteArray(""));
  foreach (BackendServer *backendServer, backendServers)
    htmlParser.appendField("PLUGIN_SETTINGS", backendServer->settingsContent());

  return htmlParser.parse(htmlSettingsMain);
}

void Backend::saveHtmlSettings(const SHttpServer::RequestMessage &request)
{
  GlobalSettings settings;

  if ((request.url().queryItemValue("save_settings") == "http") &&
      request.url().hasQueryItem("httpport") &&
      request.url().hasQueryItem("devicename"))
  {
    const int portValue = request.url().queryItemValue("httpport").toInt();
    if ((portValue > 0) && (portValue < 65536))
      settings.setValue("HttpPort", portValue);

    const QString deviceName = request.url().queryItemValue("devicename").replace('+', ' ');
    if (!deviceName.isEmpty())
    {
      settings.setValue("DeviceName", deviceName);
      masterMediaServer.setDeviceName(deviceName);
    }

    reset();
  }

  if ((request.url().queryItemValue("save_settings") == "dlna"))
  {
    const QStringList enabledAudioProfiles = MediaServer::mediaProfiles().enabledAudioProfiles();
    const QStringList enabledVideoProfiles = MediaServer::mediaProfiles().enabledVideoProfiles();
    const QStringList enabledImageProfiles = MediaServer::mediaProfiles().enabledImageProfiles();

    settings.beginGroup("DLNA");

    if (request.url().hasQueryItem("client"))
    {
      const QString group = "Client_" + request.url().queryItemValue("client");

      bool needEndGroup = false;
      if ((group.length() > 7) && (group != "Client__default"))
      {
        settings.beginGroup(group);
        needEndGroup = true;
      }

      if (request.url().hasQueryItem("save"))
      {
        const QString sizeName =
            QByteArray::fromPercentEncoding(request.url().queryItemValue("transcodesize").toAscii().replace('+', ' '));
        if (!sizeName.isEmpty())
          settings.setValue("TranscodeSize", sizeName);
        else
          settings.remove("TranscodeSize");

        const QString cropName =
            QByteArray::fromPercentEncoding(request.url().queryItemValue("cropmode").toAscii().replace('+', ' '));
        if (!cropName.isEmpty())
          settings.setValue("TranscodeCrop", cropName);
        else
          settings.remove("TranscodeCrop");

        const QString encodeModeName =
            QByteArray::fromPercentEncoding(request.url().queryItemValue("encodemode").toAscii().replace('+', ' '));
        if (!encodeModeName.isEmpty())
          settings.setValue("EncodeMode", encodeModeName);
        else
          settings.remove("EncodeMode");

        const QString channelsName =
            QByteArray::fromPercentEncoding(request.url().queryItemValue("channels").toAscii().replace('+', ' '));
        if (!channelsName.isEmpty())
          settings.setValue("TranscodeChannels", channelsName);
        else
          settings.remove("TranscodeChannels");

        const QString musicChannelsName =
            QByteArray::fromPercentEncoding(request.url().queryItemValue("musicchannels").toAscii().replace('+', ' '));
        if (!musicChannelsName.isEmpty())
          settings.setValue("TranscodeMusicChannels", musicChannelsName);
        else
          settings.remove("TranscodeMusicChannels");

        if (request.url().queryItemValue("musicaddvideo") == "on")
          settings.setValue("MusicAddBlackVideo", true);
        else
          settings.remove("MusicAddBlackVideo");

        QStringList audioProfiles;
        foreach (const QString &profile, enabledAudioProfiles)
        if (request.url().hasQueryItem("profile_" + profile))
          audioProfiles += profile;

        if (!audioProfiles.isEmpty())
          settings.setValue("SupportedAudioProfiles", audioProfiles);
        else
          settings.remove("SupportedAudioProfiles");

        QStringList videoProfiles;
        foreach (const QString &profile, enabledVideoProfiles)
        if (request.url().hasQueryItem("profile_" + profile))
          videoProfiles += profile;

        if (!videoProfiles.isEmpty())
          settings.setValue("SupportedVideoProfiles", videoProfiles);
        else
          settings.remove("SupportedVideoProfiles");

        QStringList imageProfiles;
        foreach (const QString &profile, enabledImageProfiles)
        if (request.url().hasQueryItem("profile_" + profile))
          imageProfiles += profile;

        if (!imageProfiles.isEmpty())
          settings.setValue("SupportedImageProfiles", imageProfiles);
        else
          settings.remove("SupportedImageProfiles");

        reset();
      }
      else if (request.url().hasQueryItem("defaults"))
      {
        settings.remove("TranscodeSize");
        settings.remove("TranscodeCrop");
        settings.remove("EncodeMode");
        settings.remove("TranscodeChannels");
        settings.remove("TranscodeMusicChannels");
        settings.remove("MusicAddBlackVideo");
        settings.remove("SupportedAudioProfiles");
        settings.remove("SupportedVideoProfiles");
        settings.remove("SupportedImageProfiles");

        reset();
      }

      if (needEndGroup)
        settings.endGroup();
    }

    settings.endGroup();
  }
}
