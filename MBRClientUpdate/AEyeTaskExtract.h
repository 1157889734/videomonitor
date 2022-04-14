#ifndef __AEYETASKEXTRACT_H__
#define __AEYETASKEXTRACT_H__

#include <QThread>
class  AEyeTaskExtract : public QThread {
    Q_OBJECT
signals:
    void downloaded(QString, QString);

public:
    AEyeTaskExtract(QString, QString);
    ~AEyeTaskExtract();
signals:
	void sigUnzipping(int ,int);
	void sigUnziped();
public:
    virtual void run();
    bool Extract(const QString& in_file_path, const QString& out_file_path);

private:
    QString m_strInFile;
    QString m_strOutDir;
    QString m_strURL;
};

#endif // !__AEYETASKEXTRACT_H__
