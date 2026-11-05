// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
#ifndef N9_PICKER_PROBE_H
#define N9_PICKER_PROBE_H
#include "transfer.h"
class PickerProbe : public QObject {
 Q_OBJECT
 QDeclarativeView *view;Transfer *transfer;QTimer timer;QString fixture;bool photos;int step,waits;
 QList<QDeclarativeItem*> rows(){QList<QDeclarativeItem*> result;foreach(QGraphicsItem *item,view->scene()->items()){QDeclarativeItem *qml=dynamic_cast<QDeclarativeItem*>(item);if(qml&&qml->objectName()==(photos?"photoPickRow":"filePickRow")&&qml->isVisible()&&qml->property("itemPath").toString().startsWith(fixture+"/"))result<<qml;}return result;}
 QDeclarativeItem *check(QDeclarativeItem *row){foreach(QGraphicsItem *child,row->childItems()){QDeclarativeItem *qml=dynamic_cast<QDeclarativeItem*>(child);if(qml&&qml->objectName()==(photos?"photoPickCheck":"filePickCheck"))return qml;}return NULL;}
 void click(QDeclarativeItem *item,QPointF local){QPoint p=view->mapFromScene(item->mapToScene(local));QMouseEvent press(QEvent::MouseButtonPress,p,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(view->viewport(),&press);QMouseEvent release(QEvent::MouseButtonRelease,p,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);QApplication::sendEvent(view->viewport(),&release);}
 bool expect(QDeclarativeItem *row,int count,bool checked){QDeclarativeItem *box=check(row);return box&&transfer->selectedCount()==count&&box->property("checked").toBool()==checked;}
 void done(bool ok){qDebug()<<"picker_click_probe photos="<<photos<<"passed="<<ok<<"step="<<step;timer.stop();QImage image=QPixmap::grabWidget(view).toImage();if(image.width()>image.height())image=image.transformed(QTransform().rotate(90));image.save(QCoreApplication::applicationDirPath()+"/picker-click-preview.png");QFile::remove(fixture+"/one.png");QFile::remove(fixture+"/two.png");QDir().rmdir(fixture);QCoreApplication::exit(ok?0:5);}
public:
 PickerProbe(QDeclarativeView *v,Transfer *t,bool photo):view(v),transfer(t),photos(photo),step(0),waits(0){fixture="/home/user/MyDocs/.localsend-picker-probe-"+QString::number(QCoreApplication::applicationPid());if(!QDir().mkdir(fixture))qFatal("cannot create picker fixture");QImage image(160,120,QImage::Format_RGB32);image.fill(QColor("#77bbaa").rgb());image.save(fixture+"/one.png");image.fill(QColor("#aaccdd").rgb());image.save(fixture+"/two.png");view->rootContext()->setContextProperty("pickerFixturePath",fixture);connect(&timer,SIGNAL(timeout()),this,SLOT(tick()));timer.start(250);}
private slots:
 void tick(){QList<QDeclarativeItem*> list=rows();if(list.size()<2){if(++waits>32)done(false);return;}if(step==0&&++waits<6)return;QDeclarativeItem *row=list.first();
  switch(step){
   case 0:click(row,photos?QPointF(45,55):QPointF(110,38));++step;break;
   case 1:if(!expect(row,1,true)){done(false);return;}click(check(row),QPointF(20,20));++step;break;
   case 2:if(!expect(row,0,false)){done(false);return;}click(row,photos?QPointF(50,168):QPointF(210,58));++step;break;
   case 3:if(!expect(row,1,true)){done(false);return;}click(check(row),QPointF(20,20));++step;break;
   case 4:if(!expect(row,0,false)){done(false);return;}click(list.last(),photos?QPointF(45,55):QPointF(110,38));++step;break;
   case 5:if(!expect(list.last(),1,true)){done(false);return;}click(row,photos?QPointF(45,55):QPointF(110,38));++step;break;
   case 6:if(!expect(row,2,true)||!check(list.last())->property("checked").toBool()){done(false);return;}transfer->clearSelection();++step;break;
   case 7:done(expect(row,0,false)&&!check(list.last())->property("checked").toBool());break;
  }
 }
};
#endif
