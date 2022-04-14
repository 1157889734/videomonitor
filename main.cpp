#include "MySystem.h"
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
 #include <qapplication.h>
#include "testform.h"
#include <QMessageBox>
#include <QTextCodec>
#include "utility.h"
#ifdef _DEBUG
#else
#include "client/windows/handler/exception_handler.h"
#endif

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	
    static QMutex mymutex;

    mymutex.lock();
    QString text;

    switch(type)
    {

    case QtDebugMsg:

        text = QString("Debug:");

        break;

    case QtWarningMsg:

        text = QString("Warning:");

        break;

    case QtCriticalMsg:

        text = QString("Critical:");

        break;

    case QtFatalMsg:

        text = QString("Fatal:");
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);

    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");

    QString current_date = QString("(%1)").arg(current_date_time);

    QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);

    QString strLogPath =QDir::homePath()+"/Log";
    QDir dir(strLogPath);
    if(!dir.exists())
    {
       dir.mkdir(strLogPath);
    }
    QString fileName= QString("%1%2%3%4").arg(strLogPath).arg("/").arg( QDateTime::currentDateTime().toString("yyyy-MM-dd")).arg(".txt");


    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream text_stream(&file);

    text_stream << message << "\r\n";

    file.flush();

    file.close();

    mymutex.unlock();
	
}
#ifdef _DEBUG
#else
namespace {

	static bool callback(const wchar_t *dump_path, const wchar_t *id,
		void *context, EXCEPTION_POINTERS *exinfo,
		MDRawAssertionInfo *assertion,
		bool succeeded) {
			if (succeeded) {
				printf("dump guid is %ws\n", id);
			}
			else {
				printf("dump failed\n");
			}
			fflush(stdout);

			return succeeded;
	}
}  // namespace  
#endif
#if WIN32
void InitConsoleWindow()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	ShowWindow(GetConsoleWindow(), SW_MINIMIZE);
}
void DeInitConsoleWindow()
{
	fclose(stdout);
	FreeConsole();
}
#else
#endif
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef _DEBUG
#else
	google_breakpad::ExceptionHandler eh(
		L".", NULL, callback, NULL,
		google_breakpad::ExceptionHandler::HANDLER_ALL);
#endif

#if WIN32
	//InitConsoleWindow();
#endif
    //qInstallMessageHandler(outputMessage);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
	QString sPath = qApp->applicationDirPath();
	auto test =sPath.toStdString().c_str();
	std::wstring  strPath =PortSipUtility::Utf82Unicode_ND(sPath.toStdString());
	std::string   strFilePath = PortSipUtility::wstring2String(strPath);
    STATE_SetCCTVRunDir(strFilePath.c_str(),sPath.toStdString().length());

    MAIN_GetSdk();
    MySystem w;
    w.show();

  //  a.SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);

    return a.exec();
}
