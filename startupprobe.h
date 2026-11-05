// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
#ifndef N9_STARTUP_PROBE_H
#define N9_STARTUP_PROBE_H
#include <QtDeclarative>
#include "transfer.h"
class StartupProbe:public QObject {
 Q_OBJECT
 QDeclarativeView *view;Transfer *transfer;QTimer timer;int phase;
 bool check(QString expression){int split=expression.lastIndexOf(';');if(split>=0)expression="(function(){"+expression.left(split+1)+"return "+expression.mid(split+1)+";})()";QDeclarativeExpression e(QDeclarativeEngine::contextForObject(view->rootObject()),view->rootObject(),expression);QVariant result=e.evaluate();if(e.hasError()){qWarning()<<e.error();return false;}return result.toBool();}
 void done(bool success){timer.stop();qDebug()<<"startup_ui_probe passed="<<success<<"phase="<<phase;QCoreApplication::exit(success?0:8);}
 void capture(QString name){QImage image=QPixmap::grabWidget(view).toImage();if(image.width()>image.height())image=image.transformed(QTransform().rotate(90));image.save(QCoreApplication::applicationDirPath()+"/"+name+".png");}
public:
 StartupProbe(QDeclarativeView *v,Transfer *t):view(v),transfer(t),phase(0){connect(&timer,SIGNAL(timeout()),this,SLOT(tick()));timer.start(500);}
private slots:
 void tick(){bool ok=false;
 if(phase==0)ok=check("sendPage.item!==null && settingsPage.item!==null && !dialogLoader.item && !bannerLoader.item");
 if(phase==1)ok=check("sendPage.item.objectName='preloaded-send'; settingsPage.item.objectName='preloaded-settings'; tabs.currentTab=sendPage; sendPage.item.objectName==='preloaded-send' && settingsPage.item.objectName==='preloaded-settings'");
 if(phase==2)ok=check("sendPage.item.objectName='retained-send'; tabs.currentTab=settingsPage; settingsPage.item!==null && sendPage.item.objectName==='retained-send'");
 if(phase==3)ok=check("tabs.currentTab=receivePage; tabs.currentTab=sendPage; sendPage.item.objectName==='retained-send'");
 if(phase==4){QMetaObject::invokeMethod(transfer,"feedback",Q_ARG(QString,QString::fromUtf8("通知显示测试")));ok=check("bannerLoader.item!==null && bannerLoader.item.text.length>0");}
 if(phase==5){capture("startup-notice");ok=check("dialogLoader.needed=true; dialogLoader.item!==null");if(ok){check("dialogLoader.item.open(); true");timer.setInterval(1600);} }
 if(phase==6){qDebug()<<"dialog_open_state"<<check("dialogLoader.item.status===DialogStatus.Open");capture("startup-dialog");check("dialogLoader.item.close();true");done(true);return;}
 if(!ok){done(false);return;}++phase;
 }
};
#endif




