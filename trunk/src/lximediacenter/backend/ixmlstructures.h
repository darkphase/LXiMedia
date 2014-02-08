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

#ifndef IXMLSTRUCTURES_H
#define IXMLSTRUCTURES_H

#include <QtCore>
#include <LXiMediaCenter>

struct _IXML_Document;
struct _IXML_Element;
struct _IXML_Node;

namespace IXMLStructures {

class XmlStructure
{
public:
                                XmlStructure();
  explicit                      XmlStructure(_IXML_Document *&);
                                ~XmlStructure();

  _IXML_Element               * addElement(_IXML_Node *to, const char *name);
  _IXML_Element               * addElement(_IXML_Node *to, const char *ns, const char *name);
  _IXML_Element               * addTextElement(_IXML_Node *to, const char *name, const char *value);
  _IXML_Element               * addTextElement(_IXML_Node *to, const char *ns, const char *name, const char *value);
  void                          setAttribute(_IXML_Element *, const char *name, const char *value);

  QByteArray                    getTextElement(_IXML_Node *from, const char *name) const;

public:
  _IXML_Document        * const doc;

private:
  _IXML_Document       ** const dest;
};

class DeviceDescription : public RootDevice::DeviceDescription,
                          public XmlStructure
{
public:
  explicit                      DeviceDescription(const QString &host);

  virtual void                  setDeviceType(const QByteArray &, const QByteArray &dlnaDoc);
  virtual void                  setFriendlyName(const QString &);
  virtual void                  setManufacturer(const QString &manufacturer, const QString &url);
  virtual void                  setModel(const QString &description, const QString &name, const QString &url, const QString &number);
  virtual void                  setSerialNumber(const QByteArray &);
  virtual void                  setUDN(const QByteArray &);
  virtual void                  addIcon(const QString &url, const char *mimetype, int width, int height, int depth);
  virtual void                  addService(const QByteArray &serviceType, const QByteArray &serviceId, const QByteArray &descriptionFile, const QByteArray &controlFile, const QByteArray &eventFile);
  virtual void                  setPresentationURL(const QString &);

private:
  _IXML_Element         * const root;
  _IXML_Element               * device;
  _IXML_Element               * iconList;
  _IXML_Element               * serviceList;

  const QString                 host;
};

class ServiceDescription : public RootDevice::ServiceDescription,
                           public XmlStructure
{
public:
                                ServiceDescription();

  virtual void                  addAction(const char *name, const char * const *argname, const char * const *argdir, const char * const *argvar, int argcount);
  virtual void                  addStateVariable(const char *name, const char *type, bool sendEvents, const char * const *values, int valcount);

private:
  _IXML_Element         * const scpd;
  _IXML_Element               * actionList;
  _IXML_Element               * serviceStateTable;
};

class EventablePropertySet : public RootDevice::EventablePropertySet,
                             public XmlStructure
{
public:
                                EventablePropertySet();

  virtual void                  addProperty(const QString &name, const QString &value);

private:
  _IXML_Element         * const propertySet;
};

class ActionGetCurrentConnectionIDs : public ConnectionManager::ActionGetCurrentConnectionIDs,
                                      public XmlStructure
{
public:
  explicit                      ActionGetCurrentConnectionIDs(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void                  setResponse(const QList<qint32> &);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
};

class ActionGetCurrentConnectionInfo : public ConnectionManager::ActionGetCurrentConnectionInfo,
                                       public XmlStructure
{
public:
  explicit                      ActionGetCurrentConnectionInfo(_IXML_Node *, _IXML_Document *&, const char *);

  virtual qint32                getConnectionID() const;

  virtual void                  setResponse(const ConnectionManager::ConnectionInfo &info);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
};

class ActionGetProtocolInfo : public ConnectionManager::ActionGetProtocolInfo,
                              public XmlStructure
{
public:
  explicit                      ActionGetProtocolInfo(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void                  setResponse(const QByteArray &source, const QByteArray &sink);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
};

class ActionBrowse : public ContentDirectory::ActionBrowse,
                     public XmlStructure
{
public:
  explicit                      ActionBrowse(_IXML_Node *, _IXML_Document *&, const char *);

  virtual QByteArray            getObjectID() const;
  virtual BrowseFlag            getBrowseFlag() const;
  virtual QByteArray            getFilter() const;
  virtual quint32               getStartingIndex() const;
  virtual quint32               getRequestedCount() const;
  virtual QByteArray            getSortCriteria() const;

  virtual void                  addItem(const ContentDirectory::BrowseItem &);
  virtual void                  addContainer(const ContentDirectory::BrowseContainer &);
  virtual void                  setResponse(quint32 totalMatches, quint32 updateID);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
  XmlStructure                  result;
  _IXML_Element         * const didl;
  quint32                       numberReturned;
};

class ActionSearch : public ContentDirectory::ActionSearch,
                     public XmlStructure
{
public:
  explicit                      ActionSearch(_IXML_Node *, _IXML_Document *&, const char *);

  virtual QByteArray            getContainerID() const;
  virtual QByteArray            getSearchCriteria() const;
  virtual QByteArray            getFilter() const;
  virtual quint32               getStartingIndex() const;
  virtual quint32               getRequestedCount() const;
  virtual QByteArray            getSortCriteria() const;

  virtual void                  addItem(const ContentDirectory::BrowseItem &);
  virtual void                  setResponse(quint32 totalMatches, quint32 updateID);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
  XmlStructure                  result;
  _IXML_Element         * const didl;
  quint32                       numberReturned;
};

class ActionGetSearchCapabilities : public ContentDirectory::ActionGetSearchCapabilities,
                                    public XmlStructure
{
public:
  explicit                      ActionGetSearchCapabilities(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void                  setResponse(const QByteArray &);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
};

class ActionGetSortCapabilities : public ContentDirectory::ActionGetSortCapabilities,
                                  public XmlStructure
{
public:
  explicit                      ActionGetSortCapabilities(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void                  setResponse(const QByteArray &);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
};

class ActionGetSystemUpdateID : public ContentDirectory::ActionGetSystemUpdateID,
                                public XmlStructure
{
public:
  explicit                      ActionGetSystemUpdateID(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void                  setResponse(quint32);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
};

// Samsung GetFeatureList
class ActionGetFeatureList : public ContentDirectory::ActionGetFeatureList,
                             public XmlStructure
{
public:
  explicit                      ActionGetFeatureList(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void                  setResponse(const QList<QByteArray> &);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
};

// Microsoft MediaReceiverRegistrar IsAuthorized
class ActionIsAuthorized : public MediaReceiverRegistrar::ActionIsAuthorized,
                           public XmlStructure
{
public:
  explicit                      ActionIsAuthorized(_IXML_Node *, _IXML_Document *&, const char *);

  virtual QByteArray            getDeviceID() const;
  virtual void                  setResponse(int);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
};

// Microsoft MediaReceiverRegistrar IsValidated
class ActionIsValidated : public MediaReceiverRegistrar::ActionIsValidated,
                          public XmlStructure
{
public:
  explicit                      ActionIsValidated(_IXML_Node *, _IXML_Document *&, const char *);

  virtual QByteArray            getDeviceID() const;
  virtual void                  setResponse(int);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
};

// Microsoft MediaReceiverRegistrar RegisterDevice
class ActionRegisterDevice : public MediaReceiverRegistrar::ActionRegisterDevice,
                             public XmlStructure
{
public:
  explicit                      ActionRegisterDevice(_IXML_Node *, _IXML_Document *&, const char *);

  virtual QByteArray            getRegistrationReqMsg() const;
  virtual void                  setResponse(const QByteArray &);

private:
  _IXML_Node            * const src;
  const QByteArray              prefix;
};

}

#endif
