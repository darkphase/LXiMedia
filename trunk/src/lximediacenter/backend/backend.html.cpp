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

#include "backend.h"

const char Backend::htmlIndex[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <title>{_PRODUCT} @ {_HOSTNAME}</title>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "{HEAD}"
    "</head>\n"
    "<body>\n"
    " <div class=\"main_navigator\" id=\"navigator\">\n"
    "{NAVIGATOR_PATH}"
    "{NAVIGATOR_BUTTONS}"
    " </div>\n"
    "{CONTENT}"
    "</body>\n"
    "</html>\n";

const char Backend::htmlNavigatorRoot[] =
    "  <div class=\"root\">{_HOSTNAME}</div>\n";

const char Backend::htmlNavigatorPath[] =
    "  <div class=\"path\">\n"
    "   <ul>\n"
    "{NAVIGATOR_ITEMS}"
    "   </ul>\n"
    "  </div>\n";

const char Backend::htmlNavigatorItem[] =
    "    <li><a href=\"{ITEM_LINK}\">{ITEM_NAME}</a></li>\n";

const char Backend::htmlNavigatorButton[] =
    "  <div><a href=\"{ITEM_LINK}\"><img src=\"{ITEM_ICON}\" alt=\"..\" /></a></div>\n";

const char Backend::htmlFrontPagesHead[] =
    " <link rel=\"stylesheet\" href=\"/css/list.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <script type=\"text/javascript\" src=\"/js/list.js\"></script>\n"; // Open and close tag due to IE bug

const char Backend::htmlFrontPages[] =
    " <p>{TR_MAINPAGE_EXPLAIN}</p>\n"
    " <div class=\"main_buttons\">\n"
    "  <a class=\"hidden\" href=\"/settings\">\n"
    "   <div class=\"button\">\n"
    "    <div><img src=\"/img/settings.png\" alt=\"..\" /></div>\n"
    "    <div class=\"title\">\n"
    "     <p class=\"name\">{TR_BUTTON_SETTINGS}</p>\n"
    "     <p>{TR_BUTTON_SETTINGS_EXPLAIN}</p>\n"
    "    </div>\n"
    "   </div>\n"
    "  </a>\n"
    "  <a class=\"hidden\" href=\"/help/\">\n"
    "   <div class=\"button\">\n"
    "    <div><img src=\"/img/glossary.png\" alt=\"..\" /></div>\n"
    "    <div class=\"title\">\n"
    "     <p class=\"name\">{TR_BUTTON_HELP}</p>\n"
    "     <p>{TR_BUTTON_HELP_EXPLAIN}</p>\n"
    "    </div>\n"
    "   </div>\n"
    "  </a>\n"
    " </div>\n"
    " <div class=\"main_frontpages\">\n"
    "{FRONTPAGES}"
    " </div>\n";

const char Backend::htmlFrontPageItem[] =
    "  <div class=\"frontpage\">\n"
    "   <p class=\"title\">{ITEM_TITLE}</p>\n"
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
    "     {ITEM_PID}\n"
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

const char Backend::htmlSettingsMain[] =
    " <div class=\"main_settings\">\n"
    "{PLUGIN_SETTINGS}"
    "  <fieldset id=\"httpserver\">\n"
    "   <legend>{TR_SERVER}</legend>\n"
    "   {TR_SERVER_EXPLAIN}<br />\n"
    "   <br />\n"
    "   <form name=\"httpsettings\" action=\"/settings#httpserver\" method=\"get\">\n"
    "    <input type=\"hidden\" name=\"save_settings\" value=\"http\" />\n"
    "    <table>\n"
    "     <tr><td>\n"
    "      {TR_HTTP_PORT_NUMBER}:\n"
    "      <input type=\"text\" size=\"6\" name=\"httpport\" value=\"{HTTPPORT}\" />\n"
    "      <input type=\"checkbox\" name=\"bindallnetworks\" value=\"on\" {BINDALLNETWORKS} />{TR_BIND_ALL_NETWORKS}\n"
    "     </td></tr><tr><td>\n"
    "      {TR_DEVICE_NAME}:\n"
    "      <input type=\"text\" size=\"40\" name=\"devicename\" value=\"{DEVICENAME}\" />\n"
    "     </td></tr><tr><td>\n"
    "      <input type=\"checkbox\" name=\"allowshutdown\" value=\"on\" {ALLOWSHUTDOWN} />{TR_ALLOW_SHUTDOWN}\n"
    "     </td></tr>\n"
    "    </table>\n"
    "    <br />\n"
    "    <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "   </form>\n"
    "  </fieldset>\n"
    "  <fieldset id=\"localization\">\n"
    "   <legend>{TR_LOCALIZATION}</legend>\n"
    "   {TR_LOCALIZATION_EXPLAIN}<br />\n"
    "   <br />\n"
    "   <form name=\"localizationsettings\" action=\"/settings#localization\" method=\"get\">\n"
    "    <input type=\"hidden\" name=\"save_settings\" value=\"localization\" />\n"
    "    <table>\n"
    "     <tr><td>\n"
    "      {TR_DEFAULT_CODEPAGE}:\n"
    "      <select name=\"defaultcodepage\">\n"
    "{CODEPAGES}"
    "      </select>\n"
    "     </td></tr>\n"
    "    </table>\n"
    "    <br />\n"
    "    <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "   </form>\n"
    "  </fieldset>\n"
    "  <fieldset id=\"dlna\">\n"
    "   <legend>{TR_DLNA}</legend>\n"
    "   {TR_MEDIA_TRANSCODE_SETTINGS_EXPLAIN}<br />\n"
    "   <br />\n"
    "{CLIENT_ROWS}"
    "   {TR_CLIENT_SETTINGS_EXPLAIN}<br />\n"
    "  </fieldset>\n"
    " </div>\n";

const char Backend::htmlSettingsDlnaRow[] =
    "   <fieldset id=\"{CLIENT_NAME}\">\n"
    "    <legend>{NAME}</legend>\n"
    "    <form name=\"dlnasettings\" action=\"/settings#dlna\" method=\"get\">\n"
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
    "      <tr>\n"
    "       <td>{TR_SUBTITLE_SETTINGS}:</td>\n"
    "       <td>\n"
    "        <select name=\"subtitlesize\">\n"
    "{SUBTITLESIZE}"
    "        </select>\n"
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

const char Backend::htmlHelpHead[] =
    " <link rel=\"stylesheet\" href=\"/css/help.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n";

const char Backend::htmlHelpContents[] =
    " <div class=\"contents\">\n"
    "  <p>{TR_CONTENTS}</p>\n"
    "  <ul>\n"
    "   <li><a href=\"quickstart\">{TR_QUICK_START}</a></li>\n"
    "   <li><a href=\"troubleshooting\">{TR_TROUBLESHOOTING}</a></li>\n"
    "   <li><a href=\"about\">{TR_ABOUT}</a></li>\n"
    "  </ul>\n"
    " </div>\n";

HttpStatus Backend::httpRequest(const QUrl &request, const UPnP::HttpRequestInfo &, QByteArray &contentType, QIODevice *&response)
{
  const QString dir = request.path().left(request.path().lastIndexOf('/') + 1);
  const QString fileName = request.path().mid(dir.length());
  const QUrlQuery query(request);

  if (dir == "/")
  {
    SStringParser htmlParser(this->htmlParser);
    QByteArray content;

    if (fileName.isEmpty())
    {
      htmlParser.setField("TR_MAINPAGE_EXPLAIN",
          tr("Welcome to the web interface of LXiMediaCenter. Through this "
             "web interface the LXiMediaCenter server can be configured and "
             "all served items can be browsed."));

      htmlParser.setField("TR_BUTTON_SETTINGS", tr("Settings"));
      htmlParser.setField("TR_BUTTON_SETTINGS_EXPLAIN",
          tr("Configure this server."));
      htmlParser.setField("TR_BUTTON_HELP", tr("Help"));
      htmlParser.setField("TR_BUTTON_HELP_EXPLAIN",
          tr("Read the user manual."));

      htmlParser.setField("FRONTPAGES", "");
      foreach (BackendServer *backendServer, backendServers)
      {
        const QByteArray frontPageContent = backendServer->frontPageContent();
        if (!frontPageContent.isEmpty())
        {
          htmlParser.setField("ITEM_TITLE", backendServer->serverName());
          htmlParser.setField("ITEM_CONTENT", frontPageContent);
          htmlParser.appendField("FRONTPAGES", htmlParser.parse(htmlFrontPageItem));
        }
      }

      content = htmlParser.parse(htmlFrontPages);
    }
    else if (fileName == "settings")
    {
      if (query.hasQueryItem("save_settings"))
        saveHtmlSettings(request);

      content = handleHtmlSettings(request);
    }
    else if (fileName == "log")
    {
      htmlParser.setField("TR_DATE", tr("Date"));
      htmlParser.setField("TR_TYPE", tr("Type"));
      htmlParser.setField("TR_MESSAGE", tr("Message"));

      htmlParser.setField("LOG_MESSAGES", "");

      const QByteArray logFile = sApp->log();
      for (int i=0, n=-1; i>=0; i=n)
      {
        n = logFile.indexOf("\t\n", i + 2);
        if (n > i)
        {
          const QByteArray message = logFile.mid(i, n - i);
          n += 2;

          const int t1 = message.indexOf('\t');
          const int t2 = message.indexOf('\t', t1 + 1);
          const int t3 = message.indexOf('\t', t2 + 1);

          htmlParser.setField("ITEM_DATE", (t1 >= 0) ? message.left(t1) : QByteArray());
          htmlParser.setField("ITEM_TYPE", (t2 > t1) ? message.mid(t1 + 1, t2 - t1 - 1) : QByteArray());
          htmlParser.setField("ITEM_PID", (t3 > t2) ? message.mid(t2 + 1, t3 - t2 - 1) : QByteArray());

          const QByteArray msg  = (t3 > t2) ? message.mid(t3 + 1) : QByteArray();
          const int nl = msg.indexOf('\n');
          if (nl < 0)
          {
            htmlParser.setField("ITEM_ROWS", "1");
            htmlParser.setField("ITEM_HEADLINE", msg);
            htmlParser.appendField("LOG_MESSAGES", htmlParser.parse(htmlLogFileHeadline));
          }
          else
          {
            htmlParser.setField("ITEM_ROWS", "2");
            htmlParser.setField("ITEM_HEADLINE", msg.left(nl));
            htmlParser.appendField("LOG_MESSAGES", htmlParser.parse(htmlLogFileHeadline));
            htmlParser.setField("ITEM_MESSAGE", msg.mid(nl + 1));
            htmlParser.appendField("LOG_MESSAGES", htmlParser.parse(htmlLogFileMessage));
          }
        }
      }

      content = htmlParser.parse(htmlLogFile);
    }
#if !defined(QT_NO_DEBUG) || defined(Q_OS_MACX)
    else if (fileName == "exit")
    {
      // Wait a bit to allow any pending HTTP requests to be handled.
      QTimer::singleShot(500, this, SLOT(performExit()));
    }
#endif
    else if (fileName == "robots.txt")
    {
      // Prevent search engines from crawling this site.
      content = "User-agent: *\r\nDisallow: /\r\n";
    }
    else if (fileName == "favicon.ico")
      return sendFile(request, ":/lximedia.ico", contentType, response);
    else if (fileName == "lximedia.png")
      return sendFile(request, ":/lximedia.png", contentType, response);

    return parseHtmlContent(request, content, htmlFrontPagesHead, contentType, response);
  }
  else if (dir == "/help/")
  {
    if (fileName.endsWith(".png"))
    {
      return sendFile(request, ":/help/" + fileName, contentType, response);
    }
    else
    {
      SStringParser htmlParser(this->htmlParser);
      htmlParser.setField("TR_ABOUT", tr("About"));
      htmlParser.setField("TR_CONTENTS", tr("Contents"));
      htmlParser.setField("TR_QUICK_START", tr("Quick Start"));
      htmlParser.setField("TR_TROUBLESHOOTING", tr("Troubleshooting"));

      QByteArray content = htmlParser.parse(htmlHelpContents);

      if (fileName == "about")
      {
        content +=
            " <h1>" + tr("About") + ' ' + qApp->applicationName() + "</h1>\n" +
            sApp->about();
      }
      else
      {
        const QString file = "/help/" + (fileName.isEmpty() ? "quickstart" : fileName);

        QFile htmlFile(':' + file + ".en.html");
        if (htmlFile.open(QFile::ReadOnly))
        {
          bool body = false;
          for (QByteArray line=htmlFile.readLine(); !line.isEmpty(); line=htmlFile.readLine())
          {
            const QByteArray cline = line.simplified().toLower();

            if (cline.startsWith("<body "))
              body = true;
            else if (cline.startsWith("</body>"))
              body = false;
            else if (body)
              content += ' ' + line.replace('\t', ' ');
          }
        }
      }

      return parseHtmlContent(request, content, htmlHelpHead, contentType, response);
    }
  }
  else if ((dir == "/css/") || (dir == "/js/") || (dir == "/img/"))
    return sendFile(request, ':' + request.path(), contentType, response);

  return HttpStatus_NotFound;
}

HttpStatus Backend::parseHtmlContent(const QUrl &request, const QByteArray &content, const QByteArray &head, QByteArray &contentType, QIODevice *&response) const
{
  SStringParser htmlParser(this->htmlParser);

  htmlParser.setField("HEAD", head);

  QString path = request.path();
  path = path.left(path.lastIndexOf('/'));
  if (!path.isEmpty())
  {
    htmlParser.setField("NAVIGATOR_ITEMS", "");

    QString fullPath = "/";
    foreach (const QString &dir, path.split('/', QString::SkipEmptyParts))
    {
      fullPath += dir + "/";
      htmlParser.setField("ITEM_LINK", QUrl(fullPath));
      htmlParser.setField("ITEM_NAME", dir);
      htmlParser.appendField("NAVIGATOR_ITEMS", htmlParser.parse(htmlNavigatorItem));
    }

    htmlParser.setField("NAVIGATOR_PATH", htmlParser.parse(htmlNavigatorPath));
  }
  else
    htmlParser.setField("NAVIGATOR_PATH", htmlParser.parse(htmlNavigatorRoot));

  htmlParser.setField("NAVIGATOR_BUTTONS", "");

  htmlParser.setField("ITEM_LINK", "/");
  htmlParser.setField("ITEM_ICON", "/img/home.png");
  htmlParser.appendField("NAVIGATOR_BUTTONS", htmlParser.parse(htmlNavigatorButton));

  htmlParser.setField("ITEM_LINK", "/settings");
  htmlParser.setField("ITEM_ICON", "/img/settings.png");
  htmlParser.appendField("NAVIGATOR_BUTTONS", htmlParser.parse(htmlNavigatorButton));

  htmlParser.setField("ITEM_LINK", "/log");
  htmlParser.setField("ITEM_ICON", "/img/journal.png");
  htmlParser.appendField("NAVIGATOR_BUTTONS", htmlParser.parse(htmlNavigatorButton));

  htmlParser.setField("ITEM_LINK", "/help/");
  htmlParser.setField("ITEM_ICON", "/img/glossary.png");
  htmlParser.appendField("NAVIGATOR_BUTTONS", htmlParser.parse(htmlNavigatorButton));

#ifndef QT_NO_DEBUG
  htmlParser.setField("ITEM_LINK", "/exit");
  htmlParser.setField("ITEM_ICON", "/img/close.png");
  htmlParser.appendField("NAVIGATOR_BUTTONS", htmlParser.parse(htmlNavigatorButton));
#endif

  htmlParser.setField("CONTENT", content);

  QBuffer * const buffer = new QBuffer();
  buffer->setData(htmlParser.parse(htmlIndex));
  contentType = UPnP::mimeTextHtml;
  response = buffer;
  return HttpStatus_Ok;
}

QByteArray Backend::handleHtmlSettings(const QUrl &request)
{
  const QUrlQuery query(request);

  QSettings settings;

  SStringParser htmlParser;

  htmlParser.setField("PLUGIN_SETTINGS", "");
  foreach (BackendServer *backendServer, backendServers)
    htmlParser.appendField("PLUGIN_SETTINGS", backendServer->settingsContent());

  htmlParser.setField("TR_SERVER", tr("Server"));
  htmlParser.setField("TR_HTTP_PORT_NUMBER", tr("Preferred HTTP port number"));
  htmlParser.setField("TR_DEVICE_NAME", tr("Device name"));
  htmlParser.setField("TR_BIND_ALL_NETWORKS", tr("Bind all networks"));
  htmlParser.setField("TR_ALLOW_SHUTDOWN", tr("Allow shutting down the computer remotely"));

  htmlParser.setField("TR_SERVER_EXPLAIN",
    tr("This configures the internal server. By default, the server only binds "
       "local/private networks (i.e. 10.0.0.0/8, 127.0.0.0/8, 169.254.0.0/16, "
       "172.16.0.0/12, and 192.168.0.0/16), all other networks are not bound. "
       "This can be overridden by checking the \"Bind all networks\" option, "
       "but note that this might expose this server to the internet; so before "
       "enabling \"Bind all networks\" make sure the local router/firewall is "
       "properly configured.")
#ifdef Q_OS_WIN
    + " (" + tr("the Windows firewall will not be sufficient") + ")"
#endif
    );
  
  htmlParser.setField("HTTPPORT", settings.value("HttpPort", defaultPort).toString());
  htmlParser.setField("BINDALLNETWORKS", settings.value("BindAllNetworks", false).toBool() ? "checked=\"checked\"" : "");
  htmlParser.setField("DEVICENAME", settings.value("DeviceName", defaultDeviceName()).toString());
  htmlParser.setField("ALLOWSHUTDOWN", settings.value("AllowShutdown", true).toBool() ? "checked=\"checked\"" : "");

  htmlParser.setField("TR_LOCALIZATION", tr("Localization"));
  htmlParser.setField("TR_DEFAULT_CODEPAGE", tr("Default codepage"));
  htmlParser.setField("TR_LOCALIZATION_EXPLAIN",
    tr("This configures the localization settings. The codepage is used to "
       "decode non-utf8 text (e.g. subtitles), when left at \"System\" the "
       "codepage is automatically determined based on the system settings and "
       "the language of the text."));

  const QByteArray defaultCodepage =
      settings.value("DefaultCodepage", "System").toByteArray();

  htmlParser.setField("CODEPAGES", "");
  foreach (const QByteArray &name, QTextCodec::availableCodecs())
  {
    if (defaultCodepage == name)
      htmlParser.setField("SELECTED", "selected=\"selected\"");
    else
      htmlParser.setField("SELECTED", "");

    htmlParser.setField("VALUE", name);
    htmlParser.setField("TEXT", name);
    htmlParser.appendField("CODEPAGES", htmlParser.parse(htmlSettingsOption));
  }

  settings.beginGroup("DLNA");

  const QStringList enabledAudioProfiles = MediaServer::mediaProfiles().enabledAudioProfiles();
  const QStringList enabledVideoProfiles = MediaServer::mediaProfiles().enabledVideoProfiles();
  const QStringList enabledImageProfiles = MediaServer::mediaProfiles().enabledImageProfiles();

  const QString genericTranscodeSize =
      settings.value("TranscodeSize", MediaServer::defaultTranscodeSizeName()).toString();
  const QString genericTranscodeCrop =
      settings.value("TranscodeCrop", MediaServer::defaultTranscodeCropName()).toString();
  const QString genericEncodeMode =
      settings.value("EncodeMode", MediaServer::defaultEncodeModeName()).toString();
  const QString genericTranscodeChannels =
      settings.value("TranscodeChannels", MediaServer::defaultTranscodeChannelName()).toString();
  const QString genericTranscodeMusicChannels =
      settings.value("TranscodeMusicChannels", MediaServer::defaultTranscodeMusicChannelName()).toString();
  const bool genericMusicAddBlackVideo =
      settings.value("MusicAddBlackVideo", MediaServer::defaultMusicAddBlackVideo()).toBool();
  const QString genericSubtitleSize =
      settings.value("SubtitleSize", MediaServer::defaultSubtitleSizeName()).toString();

  htmlParser.setField("TR_DLNA", tr("DLNA"));
  htmlParser.setField("TR_VIDEO_SETTINGS", tr("Video transcode settings"));
  htmlParser.setField("TR_MUSIC_SETTINGS", tr("Music transcode settings"));
  htmlParser.setField("TR_SUBTITLE_SETTINGS", tr("Subtitle settings"));
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
       "network bandwidth."));

  htmlParser.setField("TR_CLIENT_SETTINGS_EXPLAIN",
    tr("A box will appear here for each DLNA device that connects to this "
       "server, these boxes provide device specific settings."));

  htmlParser.setField("TR_POST_PROFILES",
    tr("If your device does not work with the default enabled DLNA profiles and "
       "you were able to make it work by disabling the unsupported profiles "
       "above, please take the time to post the configuration below on the "
       "Sourceforge LXIMedia forum. These settings will then be added to the "
       "next release."));

  htmlParser.setField("CLIENT_ROWS", QByteArray(""));

  struct T
  {
    static void addFormat(SStringParser &htmlParser, QSettings &settings, const QString &genericTranscodeSize, const MediaServer::TranscodeSize &size)
    {
      if (settings.value("TranscodeSize", genericTranscodeSize).toString() == size.name)
        htmlParser.setField("SELECTED", "selected=\"selected\"");
      else
        htmlParser.setField("SELECTED", "");

      htmlParser.setField("VALUE", size.name);
      htmlParser.setField("TEXT", size.name +
                          " (" + QString::number(size.size.width()) +
                          "x" + QString::number(size.size.height()) + ")");
      htmlParser.appendField("FORMATS", htmlParser.parse(htmlSettingsOption));
    }

    static void addChannel(SStringParser &htmlParser, QSettings &settings, const QString &genericTranscodeChannels, const MediaServer::TranscodeChannel &channel)
    {
      if (settings.value("TranscodeChannels", genericTranscodeChannels).toString() == channel.name)
        htmlParser.setField("SELECTED", "selected=\"selected\"");
      else
        htmlParser.setField("SELECTED", "");

      htmlParser.setField("VALUE", channel.name);
      htmlParser.setField("TEXT", channel.name);
      htmlParser.appendField("CHANNELS", htmlParser.parse(htmlSettingsOption));
    }

    static void addMusicChannel(SStringParser &htmlParser, QSettings &settings, const QString &genericTranscodeMusicChannels, const MediaServer::TranscodeChannel &channel)
    {
      if (settings.value("TranscodeMusicChannels", genericTranscodeMusicChannels).toString() == channel.name)
        htmlParser.setField("SELECTED", "selected=\"selected\"");
      else
        htmlParser.setField("SELECTED", "");

      htmlParser.setField("VALUE", channel.name);
      htmlParser.setField("TEXT", channel.name);
      htmlParser.appendField("MUSICCHANNELS", htmlParser.parse(htmlSettingsOption));
    }

    static void addSubtitleSizel(SStringParser &htmlParser, QSettings &settings, const QString &genericSubtitleSize, const MediaServer::SubtitleSize &size)
    {
      if (settings.value("SubtitleSize", genericSubtitleSize).toString() == size.name)
        htmlParser.setField("SELECTED", "selected=\"selected\"");
      else
        htmlParser.setField("SELECTED", "");

      htmlParser.setField("VALUE", size.name);
      htmlParser.setField("TEXT", size.name);
      htmlParser.appendField("SUBTITLESIZE", htmlParser.parse(htmlSettingsOption));
    }
  };

  // Default settings
  htmlParser.setField("NAME", tr("Default settings"));
  htmlParser.setField("CLIENT_NAME", "_default");
  htmlParser.setField("FORMATS", "");
  htmlParser.setField("CHANNELS", "");
  htmlParser.setField("MUSICCHANNELS", "");
  htmlParser.setField("SUBTITLESIZE", "");

  foreach (const MediaServer::TranscodeSize &size, MediaServer::allTranscodeSizes())
    T::addFormat(htmlParser, settings, genericTranscodeSize,  size);

  foreach (const MediaServer::TranscodeChannel &channel, MediaServer::allTranscodeChannels())
    T::addChannel(htmlParser, settings, genericTranscodeChannels, channel);

  foreach (const MediaServer::TranscodeChannel &channel, MediaServer::allTranscodeChannels())
    T::addMusicChannel(htmlParser, settings, genericTranscodeMusicChannels, channel);

  foreach (const MediaServer::SubtitleSize &size, MediaServer::allSubtitleSizes())
    T::addSubtitleSizel(htmlParser, settings, genericSubtitleSize, size);

  if (settings.value("TranscodeCrop", genericTranscodeCrop).toString() == "Box")
  {
    htmlParser.setField("SELECTED_BOX", "selected=\"selected\"");
    htmlParser.setField("SELECTED_ZOOM", "");
  }
  else
  {
    htmlParser.setField("SELECTED_BOX", "");
    htmlParser.setField("SELECTED_ZOOM", "selected=\"selected\"");
  }

  if (settings.value("EncodeMode", genericEncodeMode).toString() == "Fast")
  {
    htmlParser.setField("SELECTED_FAST", "selected=\"selected\"");
    htmlParser.setField("SELECTED_SLOW", "");
  }
  else
  {
    htmlParser.setField("SELECTED_FAST", "");
    htmlParser.setField("SELECTED_SLOW", "selected=\"selected\"");
  }

  if (settings.value("MusicAddBlackVideo", genericMusicAddBlackVideo).toBool())
    htmlParser.setField("CHECKED_ADDBLACKVIDEO", "checked=\"checked\"");
  else
    htmlParser.setField("CHECKED_ADDBLACKVIDEO", "");

  htmlParser.setField("PROFILES", "");

  htmlParser.appendField("CLIENT_ROWS", htmlParser.parse(htmlSettingsDlnaRow));

  // DLNA clients
  QList<QByteArray> activeClients = MediaServer::activeClients().toList();
  foreach (const QString &group, settings.childGroups())
  if (group.startsWith("Client_"))
  {
    QByteArray clientTag = group.mid(7).toLatin1();

    bool found = false;
    for (int i=0; (i<activeClients.count()) && !found; i++)
    if (SStringParser::toCleanName(activeClients[i]).replace(' ', '_') == clientTag)
      found = true;

    if (!found)
      activeClients += clientTag.replace('_', ' ');
  }

  qSort(activeClients);

  foreach (const QByteArray &activeClient, activeClients)
  {
    const QString clientTag = SStringParser::toCleanName(activeClient).replace(' ', '_');

    settings.beginGroup("Client_" + clientTag);

    htmlParser.setField("NAME", activeClient);
    htmlParser.setField("CLIENT_NAME", clientTag);
    htmlParser.setField("SUBTITLESIZE", "");

    QStringList checkedAudioProfiles =
        settings.value(
            "SupportedAudioProfiles",
            MediaServer::mediaProfiles().supportedAudioProfiles(activeClient)
            ).toStringList();

    if (checkedAudioProfiles.isEmpty())
      checkedAudioProfiles = enabledAudioProfiles;
    else for (QStringList::Iterator i=checkedAudioProfiles.begin(); i!=checkedAudioProfiles.end(); )
    if (!enabledAudioProfiles.contains(*i))
      i = checkedAudioProfiles.erase(i);
    else
      i++;

    QStringList checkedVideoProfiles =
        settings.value(
            "SupportedVideoProfiles",
            MediaServer::mediaProfiles().supportedVideoProfiles(activeClient)
            ).toStringList();

    if (checkedVideoProfiles.isEmpty())
      checkedVideoProfiles = enabledVideoProfiles;
    else for (QStringList::Iterator i=checkedVideoProfiles.begin(); i!=checkedVideoProfiles.end(); )
    if (!enabledVideoProfiles.contains(*i))
      i = checkedVideoProfiles.erase(i);
    else
      i++;

    QStringList checkedImageProfiles =
        settings.value(
            "SupportedImageProfiles",
            MediaServer::mediaProfiles().supportedImageProfiles(activeClient)
            ).toStringList();

    if (checkedImageProfiles.isEmpty())
      checkedImageProfiles = enabledImageProfiles;
    else for (QStringList::Iterator i=checkedImageProfiles.begin(); i!=checkedImageProfiles.end(); )
    if (!enabledImageProfiles.contains(*i))
      i = checkedImageProfiles.erase(i);
    else
      i++;

    const SSize maximumResolution =
        MediaServer::mediaProfiles().maximumResolution(checkedVideoProfiles);

    const SAudioFormat::Channels maximumChannels =
        MediaServer::mediaProfiles().maximumChannels(checkedAudioProfiles + checkedVideoProfiles);

    htmlParser.setField("SELECTED", "");
    htmlParser.setField("VALUE", "");
    htmlParser.setField("TEXT", tr("Optimal") +
                        " (" + QString::number(maximumResolution.width()) +
                        "x" + QString::number(maximumResolution.height()) + ")");
    htmlParser.setField("FORMATS", htmlParser.parse(htmlSettingsOption));

    foreach (const MediaServer::TranscodeSize &size, MediaServer::allTranscodeSizes())
    if (size.size <= maximumResolution)
      T::addFormat(htmlParser, settings, genericTranscodeSize,  size);

    htmlParser.setField("SELECTED", "");
    htmlParser.setField("VALUE", "");
    htmlParser.setField("TEXT", tr("Optimal") +
                        " (" + SAudioFormat::channelSetupName(maximumChannels) + ")");
    htmlParser.setField("CHANNELS", htmlParser.parse(htmlSettingsOption));

    foreach (const MediaServer::TranscodeChannel &channel, MediaServer::allTranscodeChannels())
    if (SAudioFormat::numChannels(channel.channels) <= SAudioFormat::numChannels(maximumChannels))
      T::addChannel(htmlParser, settings, genericTranscodeChannels, channel);

    htmlParser.setField("SELECTED", "");
    htmlParser.setField("VALUE", "");
    htmlParser.setField("TEXT", tr("Optimal") +
                        " (" + SAudioFormat::channelSetupName(maximumChannels) + ")");
    htmlParser.setField("MUSICCHANNELS", htmlParser.parse(htmlSettingsOption));

    foreach (const MediaServer::TranscodeChannel &channel, MediaServer::allTranscodeChannels())
    if (SAudioFormat::numChannels(channel.channels) <= SAudioFormat::numChannels(maximumChannels))
      T::addMusicChannel(htmlParser, settings, genericTranscodeMusicChannels, channel);

    foreach (const MediaServer::SubtitleSize &size, MediaServer::allSubtitleSizes())
      T::addSubtitleSizel(htmlParser, settings, genericSubtitleSize, size);

    if (settings.value("TranscodeCrop", genericTranscodeCrop).toString() == "Box")
    {
      htmlParser.setField("SELECTED_BOX", "selected=\"selected\"");
      htmlParser.setField("SELECTED_ZOOM", "");
    }
    else
    {
      htmlParser.setField("SELECTED_BOX", "");
      htmlParser.setField("SELECTED_ZOOM", "selected=\"selected\"");
    }

    if (settings.value("EncodeMode", genericEncodeMode).toString() == "Fast")
    {
      htmlParser.setField("SELECTED_FAST", "selected=\"selected\"");
      htmlParser.setField("SELECTED_SLOW", "");
    }
    else
    {
      htmlParser.setField("SELECTED_FAST", "");
      htmlParser.setField("SELECTED_SLOW", "selected=\"selected\"");
    }

    if (settings.value("MusicAddBlackVideo", genericMusicAddBlackVideo).toBool())
      htmlParser.setField("CHECKED_ADDBLACKVIDEO", "checked=\"checked\"");
    else
      htmlParser.setField("CHECKED_ADDBLACKVIDEO", "");

    if (query.queryItemValue("expand") == clientTag)
    {
      htmlParser.setField("INI_NAME", activeClient.split('@').first());

      htmlParser.setField("AUDIO_PROFILES", "");
      foreach (const QString &profile, enabledAudioProfiles)
      {
        htmlParser.setField("PROFILE_NAME", profile);
        htmlParser.setField("CHECKED", checkedAudioProfiles.contains(profile) ? "checked=\"checked\"" : "");
        htmlParser.appendField("AUDIO_PROFILES", htmlParser.parse(htmlSettingsDlnaRowProfilesCheck));
      }

      htmlParser.setField("INI_AUDIO_PROFILES", checkedAudioProfiles.join(", "));

      htmlParser.setField("VIDEO_PROFILES", "");
      foreach (const QString &profile, enabledVideoProfiles)
      {
        htmlParser.setField("PROFILE_NAME", profile);
        htmlParser.setField("CHECKED", checkedVideoProfiles.contains(profile) ? "checked=\"checked\"" : "");
        htmlParser.appendField("VIDEO_PROFILES", htmlParser.parse(htmlSettingsDlnaRowProfilesCheck));
      }

      htmlParser.setField("INI_VIDEO_PROFILES", checkedVideoProfiles.join(", "));

      htmlParser.setField("IMAGE_PROFILES", "");
      foreach (const QString &profile, enabledImageProfiles)
      {
        htmlParser.setField("PROFILE_NAME", profile);
        htmlParser.setField("CHECKED", checkedImageProfiles.contains(profile) ? "checked=\"checked\"" : "");
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

  return htmlParser.parse(htmlSettingsMain);
}

void Backend::saveHtmlSettings(const QUrl &request)
{
  const QUrlQuery query(request);

  QSettings settings;

  if ((query.queryItemValue("save_settings") == "http") &&
      query.hasQueryItem("httpport") &&
      query.hasQueryItem("devicename"))
  {
    const int portValue = query.queryItemValue("httpport").toInt();
    if ((portValue > 0) && (portValue < 65536))
      settings.setValue("HttpPort", portValue);

    if (query.queryItemValue("bindallnetworks") == "on")
      settings.setValue("BindAllNetworks", true);
    else
      settings.remove("BindAllNetworks");

    const QString deviceName =
        QString::fromUtf8(QByteArray::fromPercentEncoding(
          query.queryItemValue("devicename", QUrl::FullyEncoded).replace('+', ' ').toLatin1()));

    if (!deviceName.isEmpty())
      settings.setValue("DeviceName", deviceName);

    if (query.queryItemValue("allowshutdown") == "on")
      settings.setValue("AllowShutdown", true);
    else
      settings.remove("AllowShutdown");

    reset();
  }

  if ((query.queryItemValue("save_settings") == "localization") &&
      query.hasQueryItem("defaultcodepage"))
  {
    const QByteArray codepage = QByteArray::fromPercentEncoding(
          query.queryItemValue("defaultcodepage", QUrl::FullyEncoded).replace('+', ' ').toLatin1());

    if (!codepage.isEmpty())
    {
      settings.setValue("DefaultCodepage", codepage);

      if (codepage == "System")
        QTextCodec::setCodecForLocale(NULL);
      else
        QTextCodec::setCodecForLocale(QTextCodec::codecForName(codepage));
    }
  }

  if ((query.queryItemValue("save_settings") == "dlna"))
  {
    const QStringList enabledAudioProfiles = MediaServer::mediaProfiles().enabledAudioProfiles();
    const QStringList enabledVideoProfiles = MediaServer::mediaProfiles().enabledVideoProfiles();
    const QStringList enabledImageProfiles = MediaServer::mediaProfiles().enabledImageProfiles();

    settings.beginGroup("DLNA");

    if (query.hasQueryItem("client"))
    {
      const QString group = "Client_" + query.queryItemValue("client");

      bool needEndGroup = false;
      if ((group.length() > 7) && (group != "Client__default"))
      {
        settings.beginGroup(group);
        needEndGroup = true;
      }

      if (query.hasQueryItem("save"))
      {
        const QString sizeName =
            QByteArray::fromPercentEncoding(
              query.queryItemValue("transcodesize", QUrl::FullyEncoded).replace('+', ' ').toLatin1());
        if (!sizeName.isEmpty())
          settings.setValue("TranscodeSize", sizeName);
        else
          settings.remove("TranscodeSize");

        const QString cropName =
            QString::fromUtf8(QByteArray::fromPercentEncoding(
                query.queryItemValue("cropmode", QUrl::FullyEncoded).replace('+', ' ').toLatin1()));

        if (!cropName.isEmpty())
          settings.setValue("TranscodeCrop", cropName);
        else
          settings.remove("TranscodeCrop");

        const QString encodeModeName =
            QString::fromUtf8(QByteArray::fromPercentEncoding(
                query.queryItemValue("encodemode", QUrl::FullyEncoded).replace('+', ' ').toLatin1()));

        if (!encodeModeName.isEmpty())
          settings.setValue("EncodeMode", encodeModeName);
        else
          settings.remove("EncodeMode");

        const QString channelsName =
            QString::fromUtf8(QByteArray::fromPercentEncoding(
                query.queryItemValue("channels", QUrl::FullyEncoded).replace('+', ' ').toLatin1()));

        if (!channelsName.isEmpty())
          settings.setValue("TranscodeChannels", channelsName);
        else
          settings.remove("TranscodeChannels");

        const QString musicChannelsName =
            QString::fromUtf8(QByteArray::fromPercentEncoding(
                query.queryItemValue("musicchannels", QUrl::FullyEncoded).replace('+', ' ').toLatin1()));

        if (!musicChannelsName.isEmpty())
          settings.setValue("TranscodeMusicChannels", musicChannelsName);
        else
          settings.remove("TranscodeMusicChannels");

        if (query.queryItemValue("musicaddvideo") == "on")
          settings.setValue("MusicAddBlackVideo", true);
        else
          settings.remove("MusicAddBlackVideo");

        const QString subtitleSizeName =
            QString::fromUtf8(QByteArray::fromPercentEncoding(
                query.queryItemValue("subtitlesize", QUrl::FullyEncoded).replace('+', ' ').toLatin1()));

        if (!subtitleSizeName.isEmpty())
          settings.setValue("SubtitleSize", subtitleSizeName);
        else
          settings.remove("SubtitleSize");

        QStringList audioProfiles;
        foreach (const QString &profile, enabledAudioProfiles)
        if (query.hasQueryItem("profile_" + profile))
          audioProfiles += profile;

        if (!audioProfiles.isEmpty())
          settings.setValue("SupportedAudioProfiles", audioProfiles);
        else
          settings.remove("SupportedAudioProfiles");

        QStringList videoProfiles;
        foreach (const QString &profile, enabledVideoProfiles)
        if (query.hasQueryItem("profile_" + profile))
          videoProfiles += profile;

        if (!videoProfiles.isEmpty())
          settings.setValue("SupportedVideoProfiles", videoProfiles);
        else
          settings.remove("SupportedVideoProfiles");

        QStringList imageProfiles;
        foreach (const QString &profile, enabledImageProfiles)
        if (query.hasQueryItem("profile_" + profile))
          imageProfiles += profile;

        if (!imageProfiles.isEmpty())
          settings.setValue("SupportedImageProfiles", imageProfiles);
        else
          settings.remove("SupportedImageProfiles");

        reset();
      }
      else if (query.hasQueryItem("defaults"))
      {
        settings.remove("TranscodeSize");
        settings.remove("TranscodeCrop");
        settings.remove("EncodeMode");
        settings.remove("TranscodeChannels");
        settings.remove("TranscodeMusicChannels");
        settings.remove("MusicAddBlackVideo");
        settings.remove("SubtitleSize");
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
