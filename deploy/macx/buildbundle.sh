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
#cp ${QTDIR}/lib/QtScript.framework/Versions/Current/QtScript ${PKGNAME}.app/Contents/Frameworks/libQtScript.4.dylib
cp ${QTDIR}/lib/QtWebkit.framework/Versions/Current/QtWebkit ${PKGNAME}.app/Contents/Frameworks/libQtWebkit.4.dylib
cp ${QTDIR}/lib/QtXml.framework/Versions/Current/QtXml ${PKGNAME}.app/Contents/Frameworks/libQtXml.4.dylib

# Copy Qt Plugins
mkdir -p ${PKGNAME}.app/Contents/PlugIns/imageformats
cp ${QTDIR}/plugins/imageformats/libqjpeg.dylib ${PKGNAME}.app/Contents/PlugIns/imageformats/
cp ${QTDIR}/plugins/imageformats/libqtiff.dylib ${PKGNAME}.app/Contents/PlugIns/imageformats/

# Copy LXiMedia libraries
cp libLXiCore.0.dylib ${PKGNAME}.app/Contents/Frameworks/
cp libLXiServer.0.dylib ${PKGNAME}.app/Contents/Frameworks/
cp libLXiStream.0.dylib ${PKGNAME}.app/Contents/Frameworks/
cp libLXiStreamGui.0.dylib ${PKGNAME}.app/Contents/Frameworks/
cp libLXiMediaCenter.0.dylib ${PKGNAME}.app/Contents/Frameworks/

# Copy LXiMedia plugins
mkdir -p ${PKGNAME}.app/Contents/PlugIns/lximedia0
cp lximedia0/*.dylib ${PKGNAME}.app/Contents/PlugIns/lximedia0/

# Copy dcraw
cp dcraw ${PKGNAME}.app/Contents/MacOS/

# Copy lximcbackend
cp lximcbackend ${PKGNAME}.app/Contents/MacOS/

# Set the paths
for f in ${PKGNAME}.app/Contents/Frameworks/*.dylib ${PKGNAME}.app/Contents/PlugIns/imageformats/*.dylib ${PKGNAME}.app/Contents/PlugIns/lximedia0/*.dylib ${PKGNAME}.app/Contents/MacOS/*
do
  echo $f
  install_name_tool -change ${QTDIR}/lib/QtCore.framework/Versions/Current/QtCore @executable_path/../Frameworks/libQtCore.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/libQtCore.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtGui.framework/Versions/Current/QtGui @executable_path/../Frameworks/libQtGui.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/libQtGui.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtNetwork.framework/Versions/Current/QtNetwork @executable_path/../Frameworks/libQtNetwork.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/libQtNetwork.4.dylib $f
  #install_name_tool -change ${QTDIR}/lib/QtScript.framework/Versions/Current/QtScript @executable_path/../Frameworks/libQtScript.4.dylib $f
  #install_name_tool -change ${QTDIR}/lib/QtScript.framework/Versions/4/QtScript @executable_path/../Frameworks/libQtScript.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtWebkit.framework/Versions/Current/QtWebkit @executable_path/../Frameworks/libQtWebkit.4.dylib $f
  install_name_tool -change ${QTDIR}/lib/QtWebkit.framework/Versions/4/QtWebkit @executable_path/../Frameworks/libQtWebkit.4.dylib $f
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
