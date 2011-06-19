#!/bin/sh
QTDIR=~/QtSDK/Desktop/Qt/473/gcc
PKGNAME=LXiMediaCenter
VERSION=`cat VERSION`

# Copy Qt libraries
mkdir -p ${PKGNAME}.app/Contents/Frameworks
cp ${QTDIR}/lib/QtCore.framework/Versions/Current/QtCore ${PKGNAME}.app/Contents/Frameworks/libQtCore.4.dylib
cp ${QTDIR}/lib/QtGui.framework/Versions/Current/QtGui ${PKGNAME}.app/Contents/Frameworks/libQtGui.4.dylib
cp -R ${QTDIR}/lib/QtGui.framework/Resources/* ${PKGNAME}.app/Contents/Resources/
cp ${QTDIR}/lib/QtNetwork.framework/Versions/Current/QtNetwork ${PKGNAME}.app/Contents/Frameworks/libQtNetwork.4.dylib
cp ${QTDIR}/lib/QtSql.framework/Versions/Current/QtSql ${PKGNAME}.app/Contents/Frameworks/libQtSql.4.dylib
cp ${QTDIR}/lib/QtXml.framework/Versions/Current/QtXml ${PKGNAME}.app/Contents/Frameworks/libQtXml.4.dylib
cp ${QTDIR}/lib/QtGui.framework/Versions/Current/QtGui ${PKGNAME}.app/Contents/Frameworks/libQtGui.4.dylib

# Copy Qt Plugins
mkdir -p ${PKGNAME}.app/Contents/PlugIns/imageformats
cp ${QTDIR}/plugins/imageformats/libqjpeg.dylib ${PKGNAME}.app/Contents/PlugIns/imageformats/
cp ${QTDIR}/plugins/imageformats/libqgif.dylib ${PKGNAME}.app/Contents/PlugIns/imageformats/
mkdir -p ${PKGNAME}.app/Contents/PlugIns/sqldrivers
cp ${QTDIR}/plugins/sqldrivers/libqsqlite.dylib ${PKGNAME}.app/Contents/PlugIns/sqldrivers/

# Copy LXiMedia libraries
cp libLXiCore.0.dylib ${PKGNAME}.app/Contents/Frameworks/
cp libLXiServer.0.dylib ${PKGNAME}.app/Contents/Frameworks/
cp libLXiStream.0.dylib ${PKGNAME}.app/Contents/Frameworks/
cp libLXiStreamGui.0.dylib ${PKGNAME}.app/Contents/Frameworks/
cp libLXiMediaCenter.0.dylib ${PKGNAME}.app/Contents/Frameworks/

# Copy LXiMedia plugins
mkdir -p ${PKGNAME}.app/Contents/PlugIns/lximedia0
cp lximedia0/*.dylib ${PKGNAME}.app/Contents/PlugIns/lximedia0/

# Copy lximcbackend
cp lximcbackend ${PKGNAME}.app/Contents/MacOS/

# Set the paths
for f in ${PKGNAME}.app/Contents/Frameworks/*.dylib ${PKGNAME}.app/Contents/PlugIns/imageformats/*.dylib ${PKGNAME}.app/Contents/PlugIns/sqldrivers/*.dylib ${PKGNAME}.app/Contents/PlugIns/lximedia0/*.dylib ${PKGNAME}.app/Contents/MacOS/*
do
  echo $f
  install_name_tool -change ${QTDIR}/lib/QtCore.framework/Versions/Current/QtCore @executable_path/../Frameworks/libQtCore.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/libQtCore.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtGui.framework/Versions/Current/QtGui @executable_path/../Frameworks/libQtGui.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/libQtGui.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtNetwork.framework/Versions/Current/QtNetwork @executable_path/../Frameworks/libQtNetwork.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/libQtNetwork.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtSql.framework/Versions/Current/QtSql @executable_path/../Frameworks/libQtSql.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtSql.framework/Versions/4/QtSql @executable_path/../Frameworks/libQtSql.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtXml.framework/Versions/Current/QtXml @executable_path/../Frameworks/libQtXml.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/libQtXml.4.dylib $f

  install_name_tool -change libLXiCore.0.dylib @executable_path/../Frameworks/libLXiCore.0.dylib $f
  install_name_tool -change libLXiServer.0.dylib @executable_path/../Frameworks/libLXiServer.0.dylib $f
  install_name_tool -change libLXiStream.0.dylib @executable_path/../Frameworks/libLXiStream.0.dylib $f
  install_name_tool -change libLXiStreamGui.0.dylib @executable_path/../Frameworks/libLXiStreamGui.0.dylib $f
  install_name_tool -change libLXiMediaCenter.0.dylib @executable_path/../Frameworks/libLXiMediaCenter.0.dylib $f
done

rm -f ${PKGNAME}_${VERSION}.dmg
hdiutil create -format UDBZ -srcfolder ${PKGNAME}.app ${PKGNAME}_${VERSION}.dmg
