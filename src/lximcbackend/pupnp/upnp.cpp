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

#include "upnp.h"
#include "../string.h"
#include <upnp/upnp.h>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

namespace pupnp {

const char  upnp::mime_application_octetstream[]  = "application/octet-stream";
const char  upnp::mime_audio_aac[]                = "audio/aac";
const char  upnp::mime_audio_ac3[]                = "audio/x-ac3";
const char  upnp::mime_audio_lpcm[]               = "audio/L16;rate=48000;channels=2";
const char  upnp::mime_audio_mp3[]                = "audio/mp3";
const char  upnp::mime_audio_mpeg[]               = "audio/mpeg";
const char  upnp::mime_audio_mpegurl[]            = "audio/x-mpegurl";
const char  upnp::mime_audio_ogg[]                = "audio/ogg";
const char  upnp::mime_audio_wave[]               = "audio/wave";
const char  upnp::mime_audio_wma[]                = "audio/x-ms-wma";
const char  upnp::mime_image_jpeg[]               = "image/jpeg";
const char  upnp::mime_image_png[]                = "image/png";
const char  upnp::mime_image_svg[]                = "image/svg+xml";
const char  upnp::mime_image_tiff[]               = "image/tiff";
const char  upnp::mime_video_3g2[]                = "video/3gpp";
const char  upnp::mime_video_asf[]                = "video/x-ms-asf";
const char  upnp::mime_video_avi[]                = "video/avi";
const char  upnp::mime_video_flv[]                = "video/x-flv";
const char  upnp::mime_video_matroska[]           = "video/x-matroska";
const char  upnp::mime_video_mpeg[]               = "video/mpeg";
const char  upnp::mime_video_mpegm2ts[]           = "video/vnd.dlna.mpeg-tts";
const char  upnp::mime_video_mpegts[]             = "video/x-mpegts";
const char  upnp::mime_video_mp4[]                = "video/mp4";
const char  upnp::mime_video_ogg[]                = "video/ogg";
const char  upnp::mime_video_qt[]                 = "video/quicktime";
const char  upnp::mime_video_wmv[]                = "video/x-ms-wmv";
const char  upnp::mime_text_css[]                 = "text/css;charset=\"utf-8\"";
const char  upnp::mime_text_html[]                = "text/html;charset=\"utf-8\"";
const char  upnp::mime_text_js[]                  = "text/javascript;charset=\"utf-8\"";
const char  upnp::mime_text_plain[]               = "text/plain;charset=\"utf-8\"";
const char  upnp::mime_text_xml[]                 = "text/xml;charset=\"utf-8\"";

upnp * upnp::me = nullptr;

std::string upnp::hostname()
{
  char buffer[64] = { '\0' };

  // On Windows this will ensure Winsock is initialized properly.
  if (::UpnpGetAvailableIpAddresses() != nullptr)
    gethostname(buffer, sizeof(buffer) - 1);

  return buffer;
}

upnp::upnp(class messageloop &messageloop)
  : messageloop(messageloop),
    basedir("/upnp/"),
    update_interfaces_timer(messageloop, std::bind(&upnp::update_interfaces, this)),
    update_interfaces_interval(10),
    clear_responses_timer(messageloop, std::bind(&upnp::clear_responses, this)),
    clear_responses_interval(15)
{
  assert(me == nullptr);

  me = this;
  port = 0;
  bind_public = false;
  initialized = false;
  webserver_enabled = false;
}

upnp::~upnp()
{
  upnp::close();

  assert(me != nullptr);
  me = nullptr;
}

const std::string &upnp::http_basedir() const
{
  return basedir;
}

void upnp::child_add(struct child &child)
{
  children.insert(&child);
}

void upnp::child_remove(struct child &child)
{
  children.erase(&child);
}

void upnp::http_callback_register(const std::string &path, const http_callback &callback)
{
  if (initialized && !webserver_enabled)
    enable_webserver();

  std::string p = path;
  if (!p.empty() && (p[p.length() - 1] != '/'))
    p += '/';

  if (http_callbacks.find(p) == http_callbacks.end())
  {
    http_callbacks[p] = callback;
    if (initialized)
      ::UpnpAddVirtualDir(p.c_str());
  }
}

void upnp::http_callback_unregister(const std::string &path)
{
  std::string p = path;
  if (!p.empty() && (p[p.length() - 1] != '/'))
    p += '/';

  auto i = http_callbacks.find(p);
  if (i != http_callbacks.end())
    http_callbacks.erase(i);
}

bool upnp::initialize(uint16_t port, bool bind_public)
{
  this->port = port;
  this->bind_public = bind_public;

  std::vector<const char *> addresses;
  for (char **i = ::UpnpGetAvailableIpAddresses(); i && *i; i++)
  {
    available_addresses.insert(*i);
    if (bind_public || is_local_address(*i))
      addresses.push_back(*i);
  }

  addresses.push_back(nullptr);
  int result = ::UpnpInit3(&(addresses[0]), port);
  initialized = result == UPNP_E_SUCCESS;
  if (!initialized)
    initialized = (result = ::UpnpInit3(&(addresses[0]), 0)) == UPNP_E_SUCCESS;

  if (initialized)
  {
    for (char **i = ::UpnpGetServerIpAddresses(); i && *i; i++)
      std::clog << "[" << this << "] pupnp::upnp: Bound " << *i << ":" << ::UpnpGetServerPort() << std::endl;

    if (!http_callbacks.empty())
      enable_webserver();

    for (auto i = http_callbacks.begin(); i != http_callbacks.end(); i++)
      ::UpnpAddVirtualDir(i->first.c_str());
  }
  else
    std::clog << "[" << this << "] pupnp::upnp: Failed to initialize libupnp:" << result << std::endl;

  update_interfaces_timer.start(update_interfaces_interval);

  if (initialized)
  for (auto *child : children)
    initialized &= child->initialize();

  return initialized;
}

void upnp::close(void)
{
  update_interfaces_timer.stop();

  for (auto *child : children)
    child->close();

  if (initialized)
  {
    initialized = false;

    if (webserver_enabled)
    {
      webserver_enabled = false;
      ::UpnpRemoveAllVirtualDirs();
    }

    // Ugly, but needed as UpnpFinish waits for callbacks from the HTTP server.
    std::atomic<bool> finished(false);
    std::thread finish([&finished] { ::UpnpFinish(); finished = true; });
    while (!finished) messageloop.process_events(std::chrono::milliseconds(16));
    finish.join();

    clear_responses();
  }
}

bool upnp::is_local_address(const char *address)
{
  return
      (strncmp(address, "10.", 3) == 0) ||
      (strncmp(address, "127.", 4) == 0) ||
      (strncmp(address, "169.254.", 8) == 0) ||
      (strncmp(address, "172.16.", 7) == 0) ||
      (strncmp(address, "172.17.", 7) == 0) ||
      (strncmp(address, "172.18.", 7) == 0) ||
      (strncmp(address, "172.19.", 7) == 0) ||
      (strncmp(address, "172.20.", 7) == 0) ||
      (strncmp(address, "172.21.", 7) == 0) ||
      (strncmp(address, "172.22.", 7) == 0) ||
      (strncmp(address, "172.23.", 7) == 0) ||
      (strncmp(address, "172.24.", 7) == 0) ||
      (strncmp(address, "172.25.", 7) == 0) ||
      (strncmp(address, "172.26.", 7) == 0) ||
      (strncmp(address, "172.27.", 7) == 0) ||
      (strncmp(address, "172.28.", 7) == 0) ||
      (strncmp(address, "172.29.", 7) == 0) ||
      (strncmp(address, "172.30.", 7) == 0) ||
      (strncmp(address, "172.31.", 7) == 0) ||
      (strncmp(address, "192.168.", 8) == 0);
}

bool upnp::is_my_address(const std::string &address) const
{
  const size_t colon = address.find_first_of(':');

  return available_addresses.find((colon != address.npos) ? address.substr(0, colon) : address) != available_addresses.end();
}

int upnp::handle_http_request(const struct request &request, std::string &content_type, std::shared_ptr<std::istream> &response)
{
  for (std::string path = request.url.path;;)
  {
    auto i = http_callbacks.find(path);
    if (i != http_callbacks.end())
      return i->second(request, content_type, response);

    const size_t slash = path.find_last_of('/', path.length() - 2);
    if (slash != path.npos)
      path = path.substr(0, slash + 1);
    else
      break;
  }

  return http_not_found;
}

void upnp::clear_responses()
{
  std::lock_guard<std::mutex> _(responses_mutex);

  responses.clear();
}

void upnp::update_interfaces()
{
  if (me->handles.empty())
  for (char **i = ::UpnpGetAvailableIpAddresses(); i && *i; i++)
  if (available_addresses.find(*i) == available_addresses.end())
  if (bind_public || is_local_address(*i))
  {
    close();
    initialize(port, bind_public);
    break;
  }
}

void upnp::enable_webserver()
{
  struct T
  {
    static int get_info(::Request_Info *request_info, const char *url, ::File_Info *info)
    {
      struct request request;
      request.user_agent = request_info->userAgent;
      request.source_address = request_info->sourceAddress;
      request.url = upnp::url("http://" + std::string(request_info->host) + url);

      std::string content_type;
      std::shared_ptr<std::istream> response;
      if (me->get_response(request, content_type, response, false) == http_ok)
      {
        info->file_length = -1;
        if (response)
        {
          auto i = response->tellg();
          if (i != decltype(i)(-1))
          {
            response->seekg(0, std::ios_base::end);
            info->file_length = response->tellg();
            response->seekg(i);
          }
        }

        info->last_modified = ::time(nullptr);
        info->is_directory = FALSE;
        info->is_readable = TRUE;
        info->is_cacheable = FALSE;
        info->content_type = ::ixmlCloneDOMString(content_type.c_str());

        return 0;
      }
      else
        return -1;
    }

    static ::UpnpWebFileHandle open(::Request_Info *request_info, const char *url, ::UpnpOpenFileMode mode)
    {
      struct request request;
      request.user_agent = request_info->userAgent;
      request.source_address = request_info->sourceAddress;
      request.url = upnp::url("http://" + std::string(request_info->host) + url);

      std::string content_type;
      std::shared_ptr<std::istream> response;
      if (me->get_response(request, content_type, response, true) == http_ok)
      {
        me->messageloop.send([&response]
        {
          me->handles[response.get()] = response;
        });

        return response.get();
      }

      return nullptr;
    }

    static int read(::UpnpWebFileHandle fileHnd, char *buf, size_t len)
    {
      if (fileHnd)
      {
        std::istream * const stream = reinterpret_cast<std::istream *>(fileHnd);
        stream->read(buf, len);
        return std::max(int(stream->gcount()), 0);
      }

      return UPNP_E_INVALID_HANDLE;
    }

    static int write(::UpnpWebFileHandle /*fileHnd*/, char */*buf*/, size_t /*len*/)
    {
      return UPNP_E_INVALID_HANDLE;
    }

    static int seek(::UpnpWebFileHandle fileHnd, off_t offset, int origin)
    {
      if (fileHnd)
      {
        std::istream * const stream = reinterpret_cast<std::istream *>(fileHnd);
        switch (origin)
        {
        case SEEK_SET:  return stream->seekg(offset, std::ios_base::beg) ? 0 : -1;
        case SEEK_CUR:  return stream->seekg(offset, std::ios_base::cur) ? 0 : -1;
        case SEEK_END:  return stream->seekg(offset, std::ios_base::end) ? 0 : -1;
        default:        return -1;
        }
      }

      return -1;
    }

    static int close(::UpnpWebFileHandle fileHnd)
    {
      if (fileHnd)
      {
        me->messageloop.post([fileHnd]
        {
          auto i = me->handles.find(fileHnd);
          if (i != me->handles.end())
            me->handles.erase(i);
        });

        return UPNP_E_SUCCESS;
      }

      return UPNP_E_INVALID_HANDLE;
    }
  };

  ::UpnpEnableWebserver(TRUE);
  static const ::UpnpVirtualDirCallbacks callbacks = { &T::get_info, &T::open, &T::read, &T::write, &T::seek, &T::close };
  ::UpnpSetVirtualDirCallbacks(const_cast< ::UpnpVirtualDirCallbacks * >(&callbacks));

  webserver_enabled = true;
}

int upnp::get_response(const struct request &request, std::string &content_type, std::shared_ptr<std::istream> &response, bool erase)
{
  {
    std::lock_guard<std::mutex> _(responses_mutex);

    auto i = responses.find(request.url.path);
    if (i != responses.end())
    {
      response = i->second.first;
      content_type = i->second.second;
      if (erase)
        responses.erase(i);

      return http_ok;
    }
  }

  int result = 0;
  messageloop.send([this, &request, &content_type, &response, erase, &result]
  {
    if (initialized)
    {
      result = handle_http_request(request, content_type, response);
      if (response && !erase && (result == http_ok))
        clear_responses_timer.start(clear_responses_interval, true);
    }
    else
      result = http_internal_server_error;
  });

  if (!erase && (result == http_ok))
  {
    std::lock_guard<std::mutex> _(responses_mutex);

    responses[request.url.path] = std::make_pair(response, content_type);
  }

  return result;
}

/*! Returns the MIME type for the specified filename, based on the extension.
 */
const char * upnp::mime_type(const std::string &filename)
{
  const size_t dot = filename.find_last_of('.');
  if (dot != filename.npos)
  {
    std::string ext = filename.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if      (ext == "js")     return mime_text_js;
    else if (ext == "aac")    return mime_audio_aac;
    else if (ext == "ac3")    return mime_audio_ac3;
    else if (ext == "lpcm")   return mime_audio_lpcm;
    else if (ext == "m3u")    return mime_audio_mpegurl;
    else if (ext == "mpa")    return mime_audio_mpeg;
    else if (ext == "mp2")    return mime_audio_mpeg;
    else if (ext == "mp3")    return mime_audio_mp3;
    else if (ext == "ac3")    return mime_audio_mpeg;
    else if (ext == "dts")    return mime_audio_mpeg;
    else if (ext == "oga")    return mime_audio_ogg;
    else if (ext == "wav")    return mime_audio_wave;
    else if (ext == "wma")    return mime_audio_wma;
    else if (ext == "jpeg")   return mime_image_jpeg;
    else if (ext == "jpg")    return mime_image_jpeg;
    else if (ext == "png")    return mime_image_png;
    else if (ext == "svg")    return mime_image_svg;
    else if (ext == "tiff")   return mime_image_tiff;
    else if (ext == "css")    return mime_text_css;
    else if (ext == "html")   return mime_text_html;
    else if (ext == "htm")    return mime_text_html;
    else if (ext == "txt")    return mime_text_plain;
    else if (ext == "log")    return mime_text_plain;
    else if (ext == "xml")    return mime_text_xml;
    else if (ext == "3g2")    return mime_video_3g2;
    else if (ext == "asf")    return mime_video_asf;
    else if (ext == "avi")    return mime_video_avi;
    else if (ext == "m2ts")   return mime_video_mpegts;
    else if (ext == "mkv")    return mime_video_matroska;
    else if (ext == "mpeg")   return mime_video_mpeg;
    else if (ext == "mpg")    return mime_video_mpeg;
    else if (ext == "mp4")    return mime_video_mp4;
    else if (ext == "ts")     return mime_video_mpeg;
    else if (ext == "ogg")    return mime_video_ogg;
    else if (ext == "ogv")    return mime_video_ogg;
    else if (ext == "ogx")    return mime_video_ogg;
    else if (ext == "spx")    return mime_video_ogg;
    else if (ext == "qt")     return mime_video_qt;
    else if (ext == "flv")    return mime_video_flv;
    else if (ext == "wmv")    return mime_video_wmv;
  }

  return "application/octet-stream";
}


upnp::url::url()
{
}

upnp::url::url(const std::string &url)
{
  size_t s = 0;
  if (starts_with(url, "http://"))
  {
    s = url.find_first_of('/', 7);
    if (s != url.npos)
      host = url.substr(7, s - 7);
    else
      s = 7;
  }

  const size_t q = url.find_first_of('?', s);
  if (q != url.npos)
  {
    path = url.substr(s, s - q);

    for (size_t i = q + 1; i != url.npos; )
    {
      const size_t n = url.find_first_of('&', i + 1);
      const size_t e = url.find_first_of('=', i + 1);
      if (e != url.npos)
        query[url.substr(i, e - i)] = url.substr(e + 1, n - e);
      else
        query[url.substr(i, n - i)] = std::string();

      i = n;
    }
  }
  else
    path = url.substr(s);
}

upnp::url::operator std::string() const
{
  std::string result = "http://" + host + path;

  char querysep = '?';
  for (auto &i : query)
  {
    result += querysep + i.first;
    if (!i.second.empty())
      result += '=' + i.second;

    querysep = '&';
  }

  return result;
}

} // End of namespace
