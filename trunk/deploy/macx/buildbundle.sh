#!/bin/sh
PKGNAME=LXiMediaCenter
VERSION=`cat VERSION`

# Copy Qt libraries
mkdir -p ${PKGNAME}.app/Contents/Frameworks
cp /Library/Frameworks/phonon.framework/Versions/Current/phonon ${PKGNAME}.app/Contents/Frameworks/libphonon.4.dylib
cp /Library/Frameworks/QtCore.framework/Versions/Current/QtCore ${PKGNAME}.app/Contents/Frameworks/libQtCore.4.dylib
cp /Library/Frameworks/QtDBus.framework/Versions/Current/QtDBus ${PKGNAME}.app/Contents/Frameworks/libQtDBus.4.dylib
cp /Library/Frameworks/QtGui.framework/Versions/Current/QtGui ${PKGNAME}.app/Contents/Frameworks/libQtGui.4.dylib
cp -R /Library/Frameworks/QtGui.framework/Resources/* ${PKGNAME}.app/Contents/Resources/
cp /Library/Frameworks/QtNetwork.framework/Versions/Current/QtNetwork ${PKGNAME}.app/Contents/Frameworks/libQtNetwork.4.dylib
#cp /Library/Frameworks/QtScript.framework/Versions/Current/QtScript ${PKGNAME}.app/Contents/Frameworks/libQtScript.4.dylib
cp /Library/Frameworks/QtXml.framework/Versions/Current/QtXml ${PKGNAME}.app/Contents/Frameworks/libQtXml.4.dylib
cp /Library/Frameworks/QtWebKit.framework/Versions/Current/QtWebKit ${PKGNAME}.app/Contents/Frameworks/libQtWebKit.4.dylib

# Copy Qt Plugins
mkdir -p ${PKGNAME}.app/Contents/PlugIns/imageformats
cp /Developer/Applications/Qt/plugins/imageformats/libqjpeg.dylib ${PKGNAME}.app/Contents/PlugIns/imageformats/
cp /Developer/Applications/Qt/plugins/imageformats/libqtiff.dylib ${PKGNAME}.app/Contents/PlugIns/imageformats/

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
  install_name_tool -change phonon.framework/Versions/Current/phonon @executable_path/../Frameworks/libphonon.4.dylib $f
  install_name_tool -change phonon.framework/Versions/4/phonon @executable_path/../Frameworks/libphonon.4.dylib $f
  install_name_tool -change QtCore.framework/Versions/Current/QtCore @executable_path/../Frameworks/libQtCore.4.dylib $f
  install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/libQtCore.4.dylib $f
  install_name_tool -change QtDBus.framework/Versions/Current/QtDBus @executable_path/../Frameworks/libQtDBus.4.dylib $f
  install_name_tool -change QtDBus.framework/Versions/4/QtDBus @executable_path/../Frameworks/libQtDBus.4.dylib $f
  install_name_tool -change QtGui.framework/Versions/Current/QtGui @executable_path/../Frameworks/libQtGui.4.dylib $f
  install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/libQtGui.4.dylib $f
  install_name_tool -change QtNetwork.framework/Versions/Current/QtNetwork @executable_path/../Frameworks/libQtNetwork.4.dylib $f
  install_name_tool -change QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/libQtNetwork.4.dylib $f
  #install_name_tool -change QtScript.framework/Versions/Current/QtScript @executable_path/../Frameworks/libQtScript.4.dylib $f
  #install_name_tool -change QtScript.framework/Versions/4/QtScript @executable_path/../Frameworks/libQtScript.4.dylib $f
  install_name_tool -change QtWebKit.framework/Versions/Current/QtWebKit @executable_path/../Frameworks/libQtWebKit.4.dylib $f
  install_name_tool -change QtWebKit.framework/Versions/4/QtWebKit @executable_path/../Frameworks/libQtWebKit.4.dylib $f
  install_name_tool -change QtXml.framework/Versions/Current/QtXml @executable_path/../Frameworks/libQtXml.4.dylib $f
  install_name_tool -change QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/libQtXml.4.dylib $f

  install_name_tool -change libLXiCore.0.dylib @executable_path/../Frameworks/libLXiCore.0.dylib $f
  install_name_tool -change libLXiServer.0.dylib @executable_path/../Frameworks/libLXiServer.0.dylib $f
  install_name_tool -change libLXiStream.0.dylib @executable_path/../Frameworks/libLXiStream.0.dylib $f
  install_name_tool -change libLXiStreamGui.0.dylib @executable_path/../Frameworks/libLXiStreamGui.0.dylib $f
  install_name_tool -change libLXiMediaCenter.0.dylib @executable_path/../Frameworks/libLXiMediaCenter.0.dylib $f
done

rm -f ${PKGNAME}_${VERSION}.dmg
hdiutil create -format UDBZ -srcfolder ${PKGNAME}.app ${PKGNAME}_${VERSION}.dmg
