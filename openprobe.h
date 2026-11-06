// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
#ifndef MEESHARE_OPEN_PROBE_H
#define MEESHARE_OPEN_PROBE_H
#include "transfer.h"
class OpenProbe:public QObject {
 Q_OBJECT
 QDeclarativeView *view;Transfer *transfer;QTimer timer,deadline;QString directory;int phase;bool oldAuto;QVariant oldDirectory;
 bool expression(QString text){QDeclarativeExpression e(QDeclarativeEngine::contextForObject(view->rootObject()),view->rootObject(),text);QVariant value=e.evaluate();if(e.hasError())qWarning()<<e.error();return !e.hasError()&&value.toBool();}
 void capture(QString name){QImage image=QPixmap::grabWidget(view).toImage();if(image.width()>image.height())image=image.transformed(QTransform().rotate(90));image.save(QCoreApplication::applicationDirPath()+"/"+name+".png");}
 void done(bool ok){timer.stop();QSettings settings;settings.setValue("auto_save",oldAuto);if(oldDirectory.isValid())settings.setValue("receive_directory",oldDirectory);else settings.remove("receive_directory");settings.sync();qDebug()<<"open_probe passed="<<ok<<"phase="<<phase;QCoreApplication::exit(ok?0:9);}
 int visibleChecks(){int count=0;foreach(QGraphicsItem *item,view->scene()->items()){QDeclarativeItem *q=dynamic_cast<QDeclarativeItem*>(item);if(q&&q->objectName()=="filePickCheck"&&q->isVisible())++count;}return count;}
public:
 OpenProbe(QDeclarativeView *v,Transfer *t):view(v),transfer(t),phase(0){QSettings settings;oldAuto=settings.value("auto_save",false).toBool();oldDirectory=settings.value("receive_directory");directory="/home/user/MyDocs/.meeshare-open-probe-"+QString::number(QCoreApplication::applicationPid());QDir().mkpath(directory);QImage png(64,64,QImage::Format_RGB32);png.fill(QColor("#CDE4FF").rgb());png.save(directory+"/sample.png");QFile unknown(directory+"/unsupported.meetest");unknown.open(QIODevice::WriteOnly);QByteArray bytes;for(int i=0;i<256;++i)bytes.append(char(i));unknown.write(bytes);unknown.close();QFile::link(directory+"/sample.png",directory+"/link.png");connect(&timer,SIGNAL(timeout()),this,SLOT(tick()));connect(&deadline,SIGNAL(timeout()),this,SLOT(expired()));connect(transfer,SIGNAL(receiveCompleted()),this,SLOT(received()));timer.start(350);deadline.setSingleShot(true);deadline.start(60000);}
private slots:
 void expired(){done(false);}
 void received(){if(phase==4){bool multi=QCoreApplication::arguments().contains("--multi-file"),unsupported=QCoreApplication::arguments().contains("--unsupported-file");bool ok=transfer->receivedCount()==(multi?2:1)&&transfer->canOpenReceived()==(!multi&&!unsupported);if(!multi)ok=ok&&transfer->receivedSummary()==(unsupported?"unsupported-now.meetest":"open-now.png");qDebug()<<"open_probe completion_metadata="<<ok<<"count="<<transfer->receivedCount()<<"openable="<<transfer->canOpenReceived();if(!ok){done(false);return;}phase=5;}}
 void tick(){
  if(phase==0){if(transfer->busy()||!transfer->listening())return;bool png=transfer->canOpenFile(directory+"/sample.png"),unknown=transfer->canOpenFile(directory+"/unsupported.meetest"),link=transfer->canOpenFile(directory+"/link.png");qDebug()<<"open_probe handlers png="<<png<<"unknown="<<unknown<<"symlink="<<link;if(!png||unknown||link){done(false);return;}transfer->setReceiveDirectory(directory);transfer->setAutoSave(true);phase=1;return;}
  if(phase==1){if(!transfer->listening()||transfer->status()!=QCoreApplication::translate("App","已就绪，正在发现附近设备。",0,QCoreApplication::UnicodeUTF8))return;transfer->openReceived();if(!expression("(function(){pageStack.push(browser);return true;})()")){done(false);return;}phase=2;return;}
  if(phase==2){if(visibleChecks()!=0||transfer->pickerMode()!="received"){done(false);return;}int selected=transfer->selectedCount();transfer->activateEntry(directory+"/unsupported.meetest");if(transfer->selectedCount()!=selected||transfer->openFile(directory+"/missing.png")){done(false);return;}expression("(function(){if(bannerLoader.item)bannerLoader.item.hide();return true;})()");transfer->beginPicker("files");transfer->browse(directory);phase=3;return;}
  if(phase==3){if(visibleChecks()<2){done(false);return;}transfer->toggleSelection(directory+"/sample.png");if(transfer->selectedCount()!=1){done(false);return;}transfer->openReceived();phase=6;return;}
  if(phase==6){if(visibleChecks()!=0||transfer->selectedCount()!=1){done(false);return;}capture("received-browser");expression("(function(){pageStack.pop();return true;})()");phase=4;qDebug()<<"OPEN_PROBE_READY"<<directory;return;}
  if(phase==5){if(!expression("completionLoader.item && completionLoader.item.status===DialogStatus.Open"))return;if(!expression("completionLoader.item.acceptButtonText===(transfer.canOpenReceived?app.tr('立即打开'):app.tr('查看接收文件'))")){done(false);return;}capture("receive-open-now");qDebug()<<"open_probe native_completion_button=true";if(!expression("(function(){completionLoader.item.accept();return true;})()")){done(false);return;}qDebug()<<"open_probe native_open_requested="<<QCoreApplication::arguments().contains("--launch-file");phase=7;timer.setInterval(1200);return;}
  if(phase==7){if(!QCoreApplication::arguments().contains("--launch-file")){if(transfer->pickerMode()!="received"||visibleChecks()!=0){done(false);return;}capture("received-after-completion");}done(true);return;}
 }
};
#endif
