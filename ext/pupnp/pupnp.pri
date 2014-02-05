PUPNP_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/ext/pupnp
include($${PWD}/pupnp-version.pri)

unix|win32-g++ {
  INCLUDEPATH += $${PUPNP_DIR}/libupnp-$${PUPNP_VERSION}/upnp/inc/
  LIBS += -L$${PUPNP_DIR}/libupnp-$${PUPNP_VERSION}/upnp/.libs/

  INCLUDEPATH += $${PUPNP_DIR}/libupnp-$${PUPNP_VERSION}/ixml/inc/
  LIBS += -L$${PUPNP_DIR}/libupnp-$${PUPNP_VERSION}/ixml/.libs/

  LIBS += -L$${PUPNP_DIR}/libupnp-$${PUPNP_VERSION}/threadutil/.libs/
}

unix:LIBS += -lupnp -lixml -lthreadutil
win32-g++:LIBS += -lupnp -lixml -lthreadutil
win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(PUPNP_DIR,/,\\)libupnp-$${PUPNP_VERSION}\\upnp\\.libs\\libupnp.a $$replace(PUPNP_DIR,/,\\)libupnp-$${PUPNP_VERSION}\\ixml\\.libs\\libixml.a $$replace(PUPNP_DIR,/,\\)libupnp-$${PUPNP_VERSION}\\threadutil\\.libs\\libthreadutil.a
