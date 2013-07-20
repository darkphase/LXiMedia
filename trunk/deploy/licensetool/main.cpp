#include <QtCore>
#include <iostream>
#include "license.h"

void processDir(const QByteArray &license, const QDir &dir)
{
  static QStringList filters = QStringList() << "*.h" << "*.hpp" << "*.c" << "*.cpp";

  foreach (const QString &fileName, dir.entryList(filters, QDir::Files))
  {
    QFile file(dir.absoluteFilePath(fileName));
    if (file.open(QFile::ReadOnly))
    {
      QByteArray newFile, oldLicense;

      for (QByteArray line = file.readLine(); !line.isEmpty(); line = file.readLine())
      {
        if (newFile.isEmpty())
        {
          oldLicense += line;

          const QByteArray trimmedLicense = oldLicense.trimmed();
          if (trimmedLicense.startsWith("/*******") && trimmedLicense.endsWith("*******/"))
          {
            if (oldLicense != license)
            {
              std::cout << "Updating: " << file.fileName().toLatin1().data() << std::endl;

              newFile = license;
              newFile.reserve(file.size() + license.size());
            }
            else
              break;
          }
        }
        else
          newFile += line.replace("\r\n", "\n");
      }

      file.close();
      if (!newFile.isEmpty() && file.open(QFile::ReadWrite | QFile::Truncate))
        file.write(newFile);
    }
  }

  foreach (const QString &subdir, dir.entryList(QStringList() << "*", QDir::Dirs | QDir::NoDotAndDotDot))
    processDir(license, dir.absoluteFilePath(subdir));
}

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  if (app.arguments().count() >= 3)
  {
    QFile licenseFile(app.arguments()[1]);
    if (licenseFile.open(QFile::ReadOnly))
    {
      const QByteArray license = licenseFile.readAll();

      for (int i=2; i<app.arguments().count(); i++)
        processDir(license, QDir(app.arguments()[i]));

      return 0;
    }
    else
    {
      std::cerr << "Could not read: " << app.arguments()[1].toLatin1().data() << std::endl;

      return 1;
    }
  }
  else
  {
    std::cerr << "Usage: " << argv[0] << " <licensefile> <directory> ..." << std::endl;

    return 1;
  }
}
