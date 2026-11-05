// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
#ifndef N9_CONTROLLER_PROBE_H
#define N9_CONTROLLER_PROBE_H
#include "transfer.h"
// Integration fixture: real child processes and HTTPS responses, isolated QSettings.
class ControllerProbe : public QObject {
 Q_OBJECT
 static QString translated(const char *text){return QCoreApplication::translate("App",text,0,QCoreApplication::UnicodeUTF8);}
 Transfer *transfer;QTimer poll,deadline;int stage;bool receiveGuardChecked;
public:
 ControllerProbe(Transfer *t):transfer(t),stage(0),receiveGuardChecked(false){connect(t,SIGNAL(feedback(QString)),this,SLOT(result(QString)));connect(&poll,SIGNAL(timeout()),this,SLOT(tick()));connect(&deadline,SIGNAL(timeout()),this,SLOT(expired()));poll.start(100);deadline.setSingleShot(true);deadline.start(26000);}
private slots:
 void expired(){qCritical()<<"controller_probe timeout stage="<<stage;QCoreApplication::exit(2);}
 void tick(){if(stage==0&&!transfer->busy()){QVariantList list=transfer->devices();for(int i=0;i<list.size();++i)if(list.at(i).toMap().value("alias").toString()=="N9 Feedback Fixture"){transfer->selectDevice(i);stage=1;break;}}
  else if(stage==1&&!transfer->busy()&&transfer->connected()){transfer->choose("/home/user/MyDocs/localsend-controller-test.txt");qDebug()<<"controller_selection_first"<<transfer->selectedCount();transfer->choose(QString::fromUtf8("/home/user/MyDocs/localsend-controller-第二份.txt"));qDebug()<<"controller_selection_second"<<transfer->selectedCount();transfer->toggleSelection(QString::fromUtf8("/home/user/MyDocs/localsend-controller-第二份.txt"));if(transfer->selectedCount()!=1){QCoreApplication::exit(4);return;}transfer->toggleSelection(QString::fromUtf8("/home/user/MyDocs/localsend-controller-第二份.txt"));if(transfer->selectedCount()!=2){QCoreApplication::exit(4);return;}stage=2;transfer->send();}
  else if(stage==3&&!transfer->busy()){stage=4;transfer->send();}
  else if(stage==5&&!transfer->busy()){stage=6;transfer->send();}
  else if(stage==7){if(!receiveGuardChecked&&transfer->status()==translated("正在接收…")){QString active=AppLanguage::current(),choice=AppLanguage::choice();bool blocked=!transfer->requestLanguage("ja")&&!transfer->requestLanguage("ko");receiveGuardChecked=blocked&&AppLanguage::current()==active&&AppLanguage::choice()==choice;if(!receiveGuardChecked){QCoreApplication::exit(7);return;}qDebug()<<"controller_receive_language_guard=true";}QString path="/home/user/MyDocs/LocalSend-Controller-Test/fixture-received.txt";if(!QFile::exists(path)||!transfer->status().startsWith(translated("接收完成，已保存到 ")))return;QFile f(path);f.open(QIODevice::ReadOnly);bool bytes=f.readAll()=="controller autosave and directory test\n";bool unicode=transfer->status().endsWith(translated("。"))&&!transfer->status().contains(QChar(0xfffd));bool saved=QSettings().value("auto_save").toBool()&&QSettings().value("receive_directory").toString()=="/home/user/MyDocs/LocalSend-Controller-Test";transfer->setAutoSave(false);qDebug()<<"controller_probe reject=true busy=true send=true receive="<<bytes<<"preferences="<<saved<<"unicode="<<unicode;QCoreApplication::exit(bytes&&saved&&unicode&&receiveGuardChecked?0:3);}
 }
 void result(QString text){qDebug()<<"controller_state"<<text;if(stage==2&&text==translated("对方拒绝接收。")){qDebug()<<"controller_feedback rejected";stage=3;}
  else if(stage==4&&text==translated("对方正在处理其他传输，请稍后再试。")){qDebug()<<"controller_feedback busy";stage=5;}
  else if(stage==6&&text==translated("发送完成。")){qDebug()<<"controller_feedback success";QDir().mkpath("/home/user/MyDocs/LocalSend-Controller-Test");transfer->setReceiveDirectory("/home/user/MyDocs/LocalSend-Controller-Test");transfer->setAutoSave(true);stage=7;}
 }
};
#endif
