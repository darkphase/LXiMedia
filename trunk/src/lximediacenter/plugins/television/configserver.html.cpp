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

#include "configserver.h"

namespace LXiMediaCenter {


const char * const ConfigServer::htmlMain =
    " <table class=\"widgetsfull\">\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"widget\" width=\"33%\">\n"
    "    <p class=\"head\">{TR_TUNERS}</p>\n"
    "    <table class=\"full\">\n"
    "{TUNER_LIST}\n"
    "    </table>\n"
    "   </td>\n"
    "   <td class=\"widget\" width=\"33%\">\n"
    "    <p class=\"head\">{TR_TV_CHANNELS}</p>\n"
    "    <form name=\"channel\" action=\"\" method=\"get\">\n"
    "     <input type=\"hidden\" name=\"action\" value=\"update_channels\" />\n"
    "     <input type=\"hidden\" name=\"type\" value=\"{TYPE_TV}\" />\n"
    "     <table class=\"full\">\n"
    "{TV_CHANNEL_LIST}\n"
    "     </table>\n"
    "     <input type=\"submit\" value=\"{TR_SAVE}\" />\n"
    "    </form>\n"
    "   </td>\n"
    "   <td class=\"widget\" width=\"33%\">\n"
    "    <p class=\"head\">{TR_RADIO_STATIONS}</p>\n"
    "    <form name=\"channel\" action=\"\" method=\"get\">\n"
    "     <input type=\"hidden\" name=\"action\" value=\"update_channels\" />\n"
    "     <input type=\"hidden\" name=\"type\" value=\"{TYPE_RADIO}\" />\n"
    "     <table class=\"full\">\n"
    "{RADIO_CHANNEL_LIST}\n"
    "     </table>\n"
    "     <input type=\"submit\" value=\"{TR_SAVE}\" />\n"
    "    </form>\n"
    "   </td>\n"
    "  </tr>\n"
    " </table>\n";

const char * const ConfigServer::htmlAnalogScan =
    " <table class=\"widgetsfull\">\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"widget\" width=\"33%\">\n"
    "    <p class=\"head\">{TUNER}</p>\n"
    "    <table class=\"full\">\n"
    "     <tr valign=\"top\"><td align=\"left\">{TR_PROGRESS}:</td><td align=\"right\">{COMPLETED}</td></tr>\n"
    "     <tr valign=\"top\"><td align=\"left\">{TR_FREQUENCY}:</td><td align=\"right\">{FREQ}</td></tr>\n"
    "     <tr valign=\"top\"><td align=\"left\">{TR_LAST_CHANNEL}:</td><td align=\"right\">{LAST_CHANNEL}</td></tr>\n"
    "     <tr valign=\"top\"><td align=\"left\">{TR_LAST_PROGRAMME}:</td><td align=\"right\">{LAST_PROGRAMME}</td></tr>\n"
    "    </table>\n"
    "    <center>\n"
    "     <img src=\"signalplot.png\" alt=\"Signal\" /><br />\n"
    "     <img src=\"snapshot.jpeg?width=352&amp;height=288\" alt=\"Snapshot\" /><br />\n"
    "     <form name=\"scan\" action=\"\" method=\"get\">\n"
    "      <input type=\"hidden\" name=\"scan\" value=\"stop\" />\n"
    "      <input type=\"submit\" value=\"{TR_STOP_SCAN}\" />\n"
    "     </form>\n"
    "     <form name=\"scan\" action=\"\" method=\"get\">\n"
    "      <input type=\"hidden\" name=\"scan\" value=\"close\" />\n"
    "      <input type=\"submit\" value=\"{TR_CLOSE_SCAN}\" />\n"
    "     </form>\n"
    "    </center>\n"
    "   </td>\n"
    "   <td class=\"widget\" width=\"33%\">\n"
    "    <p class=\"head\">{TR_FOUND_CHANNELS}</p>\n"
    "    <form name=\"channel\" action=\"\" method=\"get\">\n"
    "     <input type=\"hidden\" name=\"action\" value=\"add_channels\" />\n"
    "     <table class=\"full\">\n"
    "{CHANNEL_LIST}"
    "     </table>\n"
    "     <input type=\"submit\" value=\"{TR_ADD}\" />\n"
    "    </form>\n"
    "   </td>\n"
    "  </tr>\n"
    " </table>\n";

const char * const ConfigServer::htmlDigitalScan =
    " <table class=\"widgetsfull\">\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"widget\" width=\"33%\">\n"
    "    <p class=\"head\">{TUNER}</p>\n"
    "    <table class=\"full\">\n"
    "     <tr valign=\"top\"><td align=\"left\">{TR_PROGRESS}:</td><td align=\"right\">{COMPLETED}</td></tr>\n"
    "     <tr valign=\"top\"><td align=\"left\">{TR_FREQUENCY}:</td><td align=\"right\">{FREQ}</td></tr>\n"
    "     <tr valign=\"top\"><td align=\"left\">{TR_PROVIDER}:</td><td align=\"right\">{PROVIDER}</td></tr>\n"
    "    </table>\n"
    "    <center>\n"
    "     <img src=\"signalplot.png\" alt=\"Signal\" /><br />\n"
    "     <img src=\"snapshot.jpeg\" alt=\"Snapshot\" /><br />\n"
    "     <form name=\"scan\" action=\"\" method=\"get\">\n"
    "      <input type=\"hidden\" name=\"scan\" value=\"stop\" />\n"
    "      <input type=\"submit\" value=\"{TR_STOP_SCAN}\" />\n"
    "     </form>\n"
    "     <form name=\"scan\" action=\"\" method=\"get\">\n"
    "      <input type=\"hidden\" name=\"scan\" value=\"close\" />\n"
    "      <input type=\"submit\" value=\"{TR_CLOSE_SCAN}\" />\n"
    "     </form>\n"
    "    </center>\n"
    "   </td>\n"
    "   <td class=\"widget\" width=\"33%\">\n"
    "    <p class=\"head\">{TR_FOUND_CHANNELS}</p>\n"
    "    <form name=\"channel\" action=\"\" method=\"get\">\n"
    "     <input type=\"hidden\" name=\"action\" value=\"add_channels\" />\n"
    "     <table class=\"full\">\n"
    "{CHANNEL_LIST}"
    "     </table>\n"
    "     <input type=\"submit\" value=\"{TR_ADD}\" />\n"
    "    </form>\n"
    "   </td>\n"
    "  </tr>\n"
    " </table>\n";

const char * const ConfigServer::htmlTunerItem =
    "     <tr>\n"
    "      <td class=\"left\">{NAME}</td>\n"
    "      <td class=\"right\">\n"
    "       <form name=\"scan\" action=\"\" method=\"get\">\n"
    "       <input type=\"hidden\" name=\"scan\" value=\"start\" />\n"
    "       <input type=\"hidden\" name=\"device\" value=\"{DEVICE}\" />\n"
    "       <input type=\"submit\" value=\"{TR_START_SCAN}\" />\n"
    "       </form>\n"
    "      </td>\n"
    "     </tr>\n"
    "     <tr>\n"
    "      <td class=\"right\" colspan=\"2\">\n"
    "       <form name=\"scan\" action=\"\" method=\"get\">\n"
    "       <input type=\"hidden\" name=\"scan\" value=\"add\" />\n"
    "       <input type=\"hidden\" name=\"device\" value=\"{DEVICE}\" />\n"
    "       <input type=\"text\" name=\"freq\" value=\"546000\" /> kHz\n"
    "       <input type=\"submit\" value=\"{TR_ADD_CHANNEL}\" />\n"
    "       </form>\n"
    "      </td>\n"
    "     </tr>\n";

const char * const ConfigServer::htmlChannelItem =
    "     <tr>\n"
    "      <td class=\"left\" rowspan=\"{ROWS}\">\n"
    "       <select name=\"newpreset_{PRESET}\">\n"
    "{PRESETS}"
    "       </select>\n"
    "      </td>\n"
    "      <td class=\"left\">\n"
    "       <input type=\"text\" name=\"newname_{PRESET}\" value=\"{NAME}\" />\n"
    "      </td>\n"
    "      <td class=\"left\">\n"
    "       <input type=\"checkbox\" name=\"delete_{PRESET}\" />{TR_DELETE}\n"
    "      </td>\n"
    "     </tr>\n";

const char * const ConfigServer::htmlFoundChannelItem =
    "      <tr>\n"
    "       <td class=\"left\">\n"
    "        <input type=\"checkbox\" name=\"add\" value=\"{ID}\" checked />\n"
    "        <input type=\"hidden\" name=\"type_{ID}\" value=\"{TYPE}\" />\n"
    "        <input type=\"hidden\" name=\"transponder_{ID}\" value=\"{TRANSPONDER}\" />\n"
    "        <input type=\"hidden\" name=\"serviceid_{ID}\" value=\"{SERVICEID}\" />\n"
    "        <input type=\"hidden\" name=\"device_{ID}\" value=\"{DEVICE}\" />\n"
    "        <input type=\"text\" name=\"name_{ID}\" value=\"{NAME}\" />\n"
    "        <br /><br />\n"
    "        {INFO}\n"
    "       </td>\n"
    "       <td class=\"right\">\n"
    "        <a href=\"{IMAGE}\" target=\"_blank\">\n"
    "         <img src=\"{THUMBNAIL}\" alt=\"Snapshot\" />\n"
    "        </a>\n"
    "       </td>\n"
    "      </tr>\n";

const char * const ConfigServer::htmlOption =
    "        <option value=\"{VALUE}\" {SELECTED}>{TEXT}</option>\n";

const char * const ConfigServer::htmlDeviceItem =
    "     <tr>\n"
    "      <td class=\"left\" colspan=\"2\">\n"
    "       <input type=\"checkbox\" name=\"use_{PRESET}\" value=\"{DEVICE}\" {CHECKED} />\n"
    "       <input type=\"hidden\" name=\"preset\" value=\"{PRESET}\" />\n"
    "       {NAME}\n"
    "      </td>\n"
    "     </tr>\n";

} // End of namespace
