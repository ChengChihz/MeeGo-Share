// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
#ifndef N9_LOCALIZATION_H
#define N9_LOCALIZATION_H
#include <QtCore>
namespace AppLanguage {
inline QString normalize(QString name){name=name.toLower().replace('-','_');if(name.startsWith("zh")){if(name.contains("tw")||name.contains("hk")||name.contains("mo")||name.contains("hant"))return "zh_TW";return "zh_CN";}if(name.startsWith("ja"))return "ja";if(name.startsWith("ko"))return "ko";return "en";}
inline bool validChoice(QString code){return QStringList(QStringList()<<"system"<<"zh_CN"<<"zh_TW"<<"en"<<"ja"<<"ko").contains(code);}
inline QString systemLanguage(){QProcess query;query.start("gconftool-2",QStringList()<<"--get"<<"/meegotouch/i18n/language");if(query.waitForFinished(1200)){QString value=QString::fromUtf8(query.readAllStandardOutput()).trimmed();if(!value.isEmpty())return normalize(value);}else{query.kill();query.waitForFinished(100);}return normalize(QLocale::system().name());}
inline QString choice(){QString code=QSettings().value("language","system").toString();return validChoice(code)?code:"system";}
inline QString detect(){foreach(QString arg,QCoreApplication::arguments())if(arg.startsWith("--language="))return normalize(arg.mid(11));QString saved=choice();return saved=="system"?systemLanguage():saved;}
inline QString current(){return QString::fromLatin1(qgetenv("N9_APP_LANGUAGE"));}
}
// Qt 4's script translation helper loses non-Latin source characters on this device.
// Keep Qt .qm catalogs, but pass the QML QString to translate explicitly as UTF-8.
class TranslationBridge : public QObject {
 Q_OBJECT
 Q_PROPERTY(int revision READ revision NOTIFY languageChanged)
 Q_PROPERTY(QString languageChoice READ languageChoice NOTIFY languageChanged)
 QTranslator *catalog;int generation;
public:
 TranslationBridge(QTranslator *translator=NULL):catalog(translator),generation(0){}
 int revision()const{return generation;}QString languageChoice()const{return AppLanguage::choice();}
 Q_INVOKABLE QString text(QString source)const{QByteArray utf8=source.toUtf8();return QCoreApplication::translate("App",utf8.constData(),0,QCoreApplication::UnicodeUTF8);}
public slots:
 void applyChoice(QString code){if(!catalog||!AppLanguage::validChoice(code)){emit failed();return;}QString target=code=="system"?AppLanguage::systemLanguage():code;QString previous=AppLanguage::current();
  if(target!="zh_CN"&&!catalog->load(QCoreApplication::applicationDirPath()+"/translations/app_"+target+".qm")){if(previous!="zh_CN")catalog->load(QCoreApplication::applicationDirPath()+"/translations/app_"+previous+".qm");emit failed();return;}
  QCoreApplication::removeTranslator(catalog);if(target!="zh_CN")QCoreApplication::installTranslator(catalog);
  qputenv("N9_APP_LANGUAGE",target.toLatin1());qputenv("N9_ALIAS_LANGUAGE",target.toLatin1());QSettings settings;settings.setValue("language",code);settings.sync();++generation;emit languageChanged();emit applied();
 }
signals:
 void languageChanged();void applied();void failed();
};
#endif
