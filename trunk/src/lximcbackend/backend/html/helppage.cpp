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

#include "helppage.h"
#include "platform/translator.h"

static const char help_css[] = {
#include "help.css.h"
}, help_svg[] = {
#include "help.svg.h"
};

static const unsigned char help_connect_direct_png[] = {
#include "help.connect-direct.png.h"
}, help_connect_network_png[] = {
#include "help.connect-network.png.h"
}, help_connect_powerline_png[] = {
#include "help.connect-powerline.png.h"
}, help_connect_wireless_png[] = {
#include "help.connect-wireless.png.h"
};

namespace html {

helppage::helppage(class mainpage &mainpage)
    : mainpage(mainpage)
{
    using namespace std::placeholders;

    mainpage.add_file("/css/help.css", mainpage::file { pupnp::upnp::mime_text_css, help_css, sizeof(help_css) });
    mainpage.add_file("/img/help.svg", mainpage::file { pupnp::upnp::mime_image_svg, help_svg, sizeof(help_svg) });

    mainpage.add_file("/img/help.connect-direct.png"    , mainpage::bin_file { pupnp::upnp::mime_image_png, help_connect_direct_png, sizeof(help_connect_direct_png) });
    mainpage.add_file("/img/help.connect-network.png"   , mainpage::bin_file { pupnp::upnp::mime_image_png, help_connect_network_png, sizeof(help_connect_network_png) });
    mainpage.add_file("/img/help.connect-powerline.png" , mainpage::bin_file { pupnp::upnp::mime_image_png, help_connect_powerline_png, sizeof(help_connect_powerline_png) });
    mainpage.add_file("/img/help.connect-wireless.png"  , mainpage::bin_file { pupnp::upnp::mime_image_png, help_connect_wireless_png, sizeof(help_connect_wireless_png) });

    mainpage.add_page("/help", mainpage::page
    {
        tr("Help"),
        "/img/help.svg",
        std::bind(&helppage::render_headers, this, _1, _2),
        std::bind(&helppage::render_page, this, _1, _2)
    });
}

helppage::~helppage()
{
    mainpage.remove_page("/help");
}

void helppage::render_headers(const struct pupnp::upnp::request &, std::ostream &out)
{
    out << "<link rel=\"stylesheet\" href=\"/css/help.css\" type=\"text/css\" media=\"screen, handheld, projection\" />";
}

int helppage::render_page(const struct pupnp::upnp::request &, std::ostream &out)
{
    out << "<h1>Quick start instructions</h1>"
           "<p>This page will briefly describe the steps to take to install LXiMediaCenter.</p>"
           "<h2>What is LXiMediaCenter?</h2>"
           "<p>LXiMediaCenter can be used to play media files "
           "from a PC over a local network on a DLNA capable device (e.g. "
           "television, game console, media center). LXiMediaCenter aims at "
           "compatibility with a large range of devices by transcoding all media "
           "to a standard MPEG2 stream that can be played on almost any DLNA "
           "capable device, also subtitles and audio language selection are "
           "supported. Furthermore, several common timing and synchronization "
           "errors in media files are fixed on the fly while transcoding.</p>"
           "<ul><li>DVD and High Definition resolutions (720p and 1080p)."
           "<li>Stereo and multi-channel (5.1 surround) audio.</li>"
           "<li>DVD title playback.</li>"
           "<li>Subtitle overlay, support for embedded subtitles and subtitle files.</li>"
           "<li>Selection of an audio stream and/or subtitle overlay from the DLNA device.</li></ul>"
           "<p>LXiMediaCenter runs as a daemon, or service, on "
           "your PC in the background and advertises itself on your local network "
           "as a DLNA media server.</p>"
           "<h2>System requirements</h2>"
           "<p>LXiMediaCenter always transcodes (i.e. decode and "
           "encode) media, even if it is already in a compatible format, this "
           "means the minimum processor requirements are relatively high. The "
           "processor needs to support at least SSE2 and it is not recommended to "
           "use systems with less than 512Mb of RAM. It is recommended to use a "
           "modern dual core processor or better for optimal user experience. "
           "Video encoding is performed at a high bit-rate for high image "
           "quality, therefore it is recommended to use a cabled network or a "
           "modern wireless network (802.11n).</p>"
           "<h2>Installation</h2>"
           "<p>First, obtain and install the package for the used "
           "operating system. After installation, the backend will be started and "
           "can be managed through its web interface at address "
           "<a href=\"/\">http://localhost:4280/</a>.</p>"
           "<h2>Connecting with a DLNA capable device</h2>"
           "<p>There are several options to connect a DLNA "
           "device, please consult the manual of the DLNA device for an overview "
           "of all options that are available. A few generic options are "
           "discussed below.</p>"
           "<table><tr><td>"
           "<h3>Direct connection</h3>"
           "<p><img src=\"/img/help.connect-direct.png\" alt=\"Direct connection\" /></p>"
           "<p>direct connection with a PC can be established by directly "
           "connecting an Ethernet cable from the PC to a DLNA device, by "
           "default the network should auto-configure itself within one "
           "minute.</p>"
           "</td><td>"
           "<h3>Home network</h3>"
           "<p><img src=\"/img/help.connect-network.png\" alt=\"Home network\" /></p>"
           "<p>A home network connection usually consists of a router, or a PC "
           "configured as a router, and one or more computers. In this case, "
           "the DLNA device can be connected directly to one of the “LAN” "
           "ports of the router with an Ethernet cable. The router should then "
           "automatically assign an IP address to the DLNA device using DHCP.<p>"
           "</td></tr><tr><td>"
           "<h3>Wireless</h3>"
           "<p><img src=\"/img/help.connect-wireless.png\" alt=\"Wireless\" /></p>"
           "<p>a wireless access point or wireless router is available and the "
           "DLNA device has support for wireless networks, a wireless "
           "connection is also possible. Please consult the manuals of the "
           "DLNA device and wireless router for details on the configuration.</p>"
           "</td><td>"
           "<h3>Network over powerline</h3>"
           "<p><img src=\"/img/help.connect-powerline.png\" alt=\"Network over powerline\" /></p>"
           "<p>situations where it is not convenient to have an Ethernet cable "
           "near the DLNA device, Network over power-line, or HomePlug, "
           "adapters may be a solution. These adapters are able to transmit a "
           "data signal over a home power-line, and can be used to replace an "
           "Ethernet cable. Note that, similar to wireless connections, the "
           "actual connection quality may vary depending on the circumstances.</p>"
           "</td></tr></table>"
           "<h3>Firewall</h3>"
           "<p>In case a firewall is enabled on the PC, it needs "
           "to be configured to allow the backend to communicate with DLNA "
           "devices. On Windows systems, the default Windows firewall is "
           "automatically configured by the installer. For other firewalls, "
           "please consult the manual of the firewall software for details on how "
           "to configure it. The backend receives incoming HTTP requests on port "
           "4280 and receives and multicast SSDP messages to/from "
           "239.255.255.250:1900.</p>"
           "<h2>Selecting media directories</h2>"
           "<p>The directories that are published can be selected "
           "from the <a href=\"/settings\">settings page</a> in the "
           "“Folders” field, all media files in the selected directories "
           "and sub-directories will become visible. By default, the backend "
           "(lximcbackend) runs as a restricted user. On Linux the user and group "
           "“lximediacenter” are created during installation for this "
           "purpose, on Windows the “Local Service” user is used. This means "
           "that all files that need to be accessed need to be accessible by this "
           "user. On Linux this can be done by setting the read permission for "
           "“other” users on the files and directories that need to be "
           "accessed by the backend. On windows this can be done by adding "
           "“Everyone” with the read permission set to the files and "
           "directories that need to be accessed by the backend.</p>"
           "<h2>Configuring High Definition video and surround sound</h2>"
           "<p>For maximum compatibility only DVD quality video "
           "with stereo sound is enabled by default, High Definition video and "
           "surround sound can be enabled from the <a href=\"/settings\">settings "
           "page</a> in the “DLNA” field. Note that higher settings require "
           "more CPU power and more network bandwidth. The “Letterbox” "
           "setting will add black bars to the image if the aspect ratio does not "
           "match, whereas the “Fullscreen” setting will zoom in and cut off "
           "part of the image. The “High quality” setting will use more CPU "
           "power but gives higher quality images than the “Fast” setting.</p>";

    out << "<h1>Troubleshooting</h1>"
           "<p>This page describes how to detect and resolve the most common problems.</p>"
           "<h2>LXiMediaCenter is not detected in the network</h2>"
           "<p>The usual suspect in such a case is a firewall or "
           "virus scanner. The installer should add an exception to the Windows "
           "firewall, if this fails you have to manually configure it to allow "
           "communication, please note that both the SSDP (UDP 1900) and HTTP "
           "(TCP 4280) are required.</p>"
           "<p>If you have more than one PC available, you can "
           "try installing LXiMediaCenter on another PC and test if the frontend "
           "applications on both PC's can find each other (the frontend "
           "application uses the same UPnP mechanism to detect all instances of "
           "LXiMediaCenter). If the frontend applications can't find each other, "
           "there is most probably an issue with a firewall or virus scanner that "
           "blocks communication. But if they can, there is most probably an "
           "issue with the network configuration of your media device (TV), in "
           "this case consult the manual of the device.</p>"
           "<h3>Manually adding an exception to the Windows firewall</h3>"
           "<p>This can be done by right-clicking “My Network "
           "Places” in the Start menu and clicking “Properties”. Then "
           "select “Local Area Connection” and click “Properties”, now "
           "the “Windows Firewall” dialog pops up. In the general tab you can "
           "see if the firewall is enabled. If it is, make sure the “Don't "
           "allow exceptions” box is unchecked. Then click the “Advanced” "
           "tab and press the “Settings” button in the “Windows Firewall” "
           "group. Next click the “Exceptions” tab and check if there is a "
           "“LeX-Interactive MediaCenter - Backend service” entry and if it "
           "is enabled.</p>"
           "<p>If there is no entry yet, one can be added by "
           "pressing the “Add Program” button. In the “Add Program” "
           "dialog, press “Browse” and browse to “C:\\Program "
           "Files\\LXiMediaCenter\\lximcbackend.exe” and press “Open”. Next, "
           "press “Ok” and check if the newly added item is enabled and press "
           "“Ok” again.</p>"
           "<h2>Files are not visible from LXiMediaCenter</h2>"
           "<p>By default, the backend (lximcbackend) runs as a "
           "restricted user. On Linux the user and group “lximediacenter” are "
           "created during installation for this purpose, on Windows the “Local "
           "Service” user is used. This means that all files that need to be "
           "accessed by LXiMediaCenter, need to be accessible by this user. On "
           "Linux this can be done by setting the read permission for “other” "
           "users on the files and directories that need to be accessed by the "
           "backend. On windows this can be done by adding “Everyone” with "
           "the read permission set to the files and directories that need to be "
           "accessed by the backend.</p>"
           "<h2>Device can not play files</h2>"
           "<p>Sometimes a device is not able to handle the files "
           "sent by the backend. The transcoding settings can be modified from "
           "the <a href=\"/settings\">settings page</a> in the “DLNA” "
           "field. Once a device has connected with the backend, a box will "
           "appear for that device. This box has a “Show profile settings” "
           "link, which shows a list of all supported DLNA profiles. If the "
           "device does not work properly with the default selected profiles, "
           "some of them can be deselected to hide these profiles from the "
           "device.</p>"
           "<h2>Codecs</h2>"
           "<p>LXiMediaCenter uses <a href=\"http://videolan.org/\">VLC media player</a>, "
           "therefore there is no need to install a codec pack. Nor will "
           "installing a codec pack harm the functioning of LXiMediaCenter. "
           "Unfortunately this also means that custom installed codecs are not "
           "recognized by LXiMediaCenter, only the codecs supported by this build "
           "of VLC are supported.</p>";

    return pupnp::upnp::http_ok;
}

}
