// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
#ifndef N9_LANGUAGE_SWITCH_PROBE_H
#define N9_LANGUAGE_SWITCH_PROBE_H
#include "transfer.h"
class LanguageSwitchProbe : public QObject {
 Q_OBJECT
 QDeclarativeView *view;Transfer *transfer;TranslationBridge *bridge;QTimer poll,deadline;QString file,name,directory,oldChoice;bool oldPresent,automatic,listening;int phase,index;QStringList choices;
 QString tabText(){foreach(QGraphicsItem *item,view->scene()->items()){QDeclarativeItem *qml=dynamic_cast<QDeclarativeItem*>(item);if(qml&&qml->objectName()=="settingsTab")return qml->property("text").toString();}return QString();}
 void done(bool ok){poll.stop();bridge->applyChoice(oldChoice);if(!oldPresent){QSettings s;s.remove("language");s.sync();}QFile::remove(file);qDebug()<<"language_switch_probe passed="<<ok<<"choices="<<index<<"phase="<<phase;QCoreApplication::exit(ok?0:6);}
public:
 LanguageSwitchProbe(QDeclarativeView *v,Transfer *t,TranslationBridge *b):view(v),transfer(t),bridge(b),phase(0),index(0){QSettings s;oldPresent=s.contains("language");oldChoice=s.value("language","system").toString();file="/home/user/MyDocs/.localsend-language-probe-"+QString::number(QCoreApplication::applicationPid())+".txt";QFile f(file);if(!f.open(QIODevice::WriteOnly))qFatal("cannot create language fixture");f.write("language state fixture\n");f.close();choices<<"en"<<"zh_TW"<<"ja"<<"ko"<<"zh_CN"<<"system";connect(&poll,SIGNAL(timeout()),this,SLOT(tick()));connect(&deadline,SIGNAL(timeout()),this,SLOT(expired()));poll.start(300);deadline.setSingleShot(true);deadline.start(22000);}
private slots:
 void expired(){done(false);}
 void tick(){if(phase==0){if(transfer->busy()||!transfer->listening())return;transfer->choose(file);name=transfer->localName();directory=transfer->receiveDirectory();automatic=transfer->autoSave();listening=transfer->listening();phase=1;if(!transfer->requestLanguage(choices[0]))done(false);return;}
  if(phase==1){QString code=choices[index];QString expected=code=="system"?AppLanguage::systemLanguage():code;bool ok=AppLanguage::current()==expected&&bridge->languageChoice()==code&&QSettings().value("language").toString()==code&&AppLanguage::detect()==expected&&tabText()==bridge->text(QString::fromUtf8("设置"));ok=ok&&transfer->selectedCount()==1&&transfer->selected()==file&&transfer->localName()==name&&transfer->receiveDirectory()==directory&&transfer->autoSave()==automatic&&transfer->listening()==listening;if(!ok){qDebug()<<"language_switch_failed"<<code<<tabText();done(false);return;}qDebug()<<"language_switch_choice"<<code<<"state_preserved=true persisted=true qml=true";++index;if(index<choices.size()){if(!transfer->requestLanguage(choices[index]))done(false);return;}
   transfer->connectPeer();bool firstBlocked=!transfer->requestLanguage("en"),secondBlocked=!transfer->requestLanguage("ko");bool unchanged=bridge->languageChoice()=="system";transfer->cancel();if(!firstBlocked||!secondBlocked||!unchanged){done(false);return;}qDebug()<<"language_switch_busy_guard=true";phase=2;return;
  }
  if(phase==2&&!transfer->busy())done(true);
 }
};
#endif
