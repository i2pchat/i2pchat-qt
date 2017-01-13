/***************************************************************************
 *   Copyright (C) 2008 by I2P-Messenger   				   *
 *   Messenger-Dev@I2P-Messenger   					   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QtGui>
#include <QApplication>

#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QTime>
#include <QDir>

#include "form_Main.h"

#include "main.h"

#ifdef DEBUGLOGGING
QString debugLogDir;
#endif
bool quitting;


#ifdef DEBUGLOGGING
void enableDebugLogging(QString configPath);
#endif

QApplication* app_ptr;
QTranslator * translator;

void setLanguage(QString langCode) {
    qDebug() << "setLanguage:" << langCode;
    if(translator) {
        app_ptr->removeTranslator(translator);
        delete translator;
        translator = nullptr;
    }
    if(langCode=="en"){
        qDebug() << "setLanguage exiting ok (en).";
        return;
    }
    translator = new QTranslator;
    translator->load(QString("lang/")+langCode);//loads file langCode+".qm" produced by lrelease or qt linguist
    app_ptr->installTranslator(translator);
    qDebug() << "setLanguage exiting ok (not en).";
}

#ifdef DEBUGLOGGING
//void myMessageHandler(QtMsgType type, const char *msg);
void myMessageHandler(QtMsgType type,const QMessageLogContext &context,const QString &msg);
#endif

int main(int argc, char *argv[])
{
    quitting=false;
    translator=nullptr;
    QApplication app(argc, argv);
    app_ptr = &app;
	QString configPath;
#ifdef ANDROID
    QStringList loc = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    if(loc.size()<=0){
        QMessageBox::critical(
            NULL,
            "I2P-Messenger",
            "Error: no USER folder found");
        app.exec();
        quitting=true;
        app.closeAllWindows();
        return 1;
    }
    QString configPathParent=loc.at(0);
    QDir configPathParentDir(configPathParent);
    QString configPathDir("i2pchat");
    configPath=configPathParent+"/"+configPathDir;
    if(!configPathParentDir.mkpath(configPathDir)) {
        QMessageBox::critical(
            NULL,
            "I2P-Messenger",
            "Error: mkpath for config dir returned false");
        app.exec();
        quitting=true;
        app.closeAllWindows();
        return 1;
    }
    /*
    {
        QSettings settings(configPath+"/application.ini",QSettings::IniFormat);
        settings.beginGroup("General");
        settings.setValue("dummy","1");
        settings.endGroup();
        settings.sync();
        QString status = QString("status=(%1), folder=(%2).").arg((int)settings.status()).arg(configPath);
        QMessageBox::information(
            NULL,
            "I2P-Messenger status",
            status);
        app.exec();
        app.closeAllWindows();
        return 1;
    }
    */
#else
	configPath=QApplication::applicationDirPath();
	if(QFile::exists(configPath+"/UseHomeForConfigStore")==true){
		QStringList tmp=QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
		if(tmp.size()>=1){
			configPath=tmp.at(0);
			configPath+="/.I2P-Messenger";
		}
	}
#endif
#ifdef DEBUGLOGGING
    enableDebugLogging(configPath);
#endif
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    {
        QSettings settings(configPath+"/application.ini",QSettings::IniFormat);
        settings.beginGroup("General");
        QString langCode=settings.value("languageCode","en").toString();
        qDebug() << "LangCode.init:" << langCode;
        settings.endGroup();
        setLanguage(langCode);
    }


    form_MainWindow* mainForm= new form_MainWindow(configPath);
	mainForm->show();
    app.exec();
    quitting=true;
    app.closeAllWindows();

	return 0;
}

#ifdef DEBUGLOGGING
void enableDebugLogging(QString configPath)
{
	//is DebugPrint = enabled ?
    debugLogDir=configPath;
	QSettings settings(configPath+"/application.ini",QSettings::IniFormat);
	settings.beginGroup("General");
	  if(settings.value("DebugLogging","true").toBool()==true){
          qInstallMessageHandler(myMessageHandler);
	  }
	settings.endGroup();
	settings.sync();
}

void myMessageHandler(QtMsgType type,const QMessageLogContext &context,const QString &msg)
{
	QString txt;
	
	txt.append(QTime::currentTime().toString("hh:mm:ss"));
	
	switch (type) {
	case QtDebugMsg:{
		txt.append(QString(" Debug: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function));
		break;
	}
	case QtWarningMsg:{
		txt.append(QString(" Warning: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function));
		break;
	}
	case QtCriticalMsg:{
		txt.append(QString(" Critical: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function));
		break;
	}
    case QtFatalMsg:{
        txt.append(QString(" Fatal: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function));
        break;
    }
    default:{
        txt.append(QString(" Message: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function));
        break;
    }
    }
    QFile outFile(debugLogDir+"/DebugLog.txt");
	outFile.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream ts(&outFile);
	ts << txt << endl;
}
#endif
