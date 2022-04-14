#include "stdafx.h"
#include "AEyeTaskExtract.h"
#include "quazipfile.h"

AEyeTaskExtract::AEyeTaskExtract(QString strInFile, QString strOutDir)
    : m_strInFile(strInFile)
    , m_strOutDir(strOutDir)
     {

}

AEyeTaskExtract::~AEyeTaskExtract() {

}

void AEyeTaskExtract::run() {
     Extract(m_strInFile, m_strOutDir);
    emit downloaded(m_strOutDir, m_strURL);
}

bool AEyeTaskExtract::Extract(const QString& in_file_path, const QString& out_file_path) {
    QString path = out_file_path;
    if (!path.endsWith("/") && !out_file_path.endsWith("\\")) {
        path += "/";
    }

    QDir dir(out_file_path);
    if (!dir.exists()) {
        dir.mkpath(out_file_path);
    }

    QuaZip archive(in_file_path);
    if (!archive.open(QuaZip::mdUnzip)) {
		emit sigUnziped();
        return false;
    }
	int nPos=0,nCount  = archive.getEntriesCount();  // 压缩实体数量（文件或文件夹）;
    for( bool f = archive.goToFirstFile(); f; f = archive.goToNextFile() ) {
        QString filePath = archive.getCurrentFileName();
        QuaZipFile zFile(archive.getZipName(), filePath);
        zFile.open(QIODevice::ReadOnly );
        QByteArray ba = zFile.readAll();
        zFile.close();

        if (filePath.endsWith("/")) {
            dir.mkpath(filePath);
			nPos++;
			emit sigUnzipping(nPos, nCount);
        } else {
            QFile dstFile(path + filePath);
            if (!dstFile.open(QIODevice::WriteOnly))
                return false;
            dstFile.write(ba);
            dstFile.close();
			nPos++;

			emit sigUnzipping(nPos, nCount);

        }
    }
	emit sigUnziped();
    return true;
}
