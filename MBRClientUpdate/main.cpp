#include "stdafx.h"
#include "MBRClientUpdate.h"
#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QString>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    
    QTextCodec *codec = QTextCodec::codecForLocale();
    QTextCodec::setCodecForLocale(codec);   
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    QApplication::setFont(QFont("Microsoft YaHei")); 

    QTranslator t ;
    t.load(":/qm/qt_zh_CN.qm");
    a.installTranslator (&t);

    QString strURL = argc >= 2 ? argv[1] : "";
    MBRClientUpdate w(argv[1]);
    w.showWidget();

    return a.exec();
}
