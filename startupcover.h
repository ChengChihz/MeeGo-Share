// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
#ifndef N9_STARTUP_COVER_H
#define N9_STARTUP_COVER_H
#include <QtGui>
#include <QtDeclarative>
inline QWidget *createStartupCover(QDeclarativeView &view){
 view.resize(QApplication::desktop()->screenGeometry().size());
 view.showFullScreen();
 QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
 QSize size=view.viewport()->size();bool landscape=size.width()>size.height();
 QImage image(landscape?QSize(size.height(),size.width()):size,QImage::Format_RGB32);image.fill(QColor("#f4f8fd").rgb());
 QPainter painter(&image);int center=image.height()/2;
 QPixmap logo(QCoreApplication::applicationDirPath()+"/meeshare-logo.png");
 painter.drawPixmap((image.width()-96)/2,center-100,logo.scaled(96,96,Qt::KeepAspectRatio,Qt::SmoothTransformation));
 painter.setPen(QColor("#365f87"));QFont font=QApplication::font();font.setPixelSize(32);painter.setFont(font);
 painter.drawText(QRect(0,center+18,image.width(),55),Qt::AlignHCenter|Qt::AlignVCenter,"MeeShare");
 painter.setPen(QColor("#8d9eb2"));font.setPixelSize(14);painter.setFont(font);
 painter.drawText(QRect(20,image.height()-94,image.width()-40,22),Qt::AlignHCenter|Qt::AlignVCenter,"Unofficial LocalSend-compatible client");
 painter.drawText(QRect(20,image.height()-72,image.width()-40,22),Qt::AlignHCenter|Qt::AlignVCenter,QString::fromUtf8("MeeGo adaptation by Chih · LocalSend contributors"));
 painter.end();
 if(landscape)image=image.transformed(QTransform().rotate(270));
 QLabel *cover=new QLabel(view.viewport());cover->setGeometry(view.viewport()->rect());cover->setPixmap(QPixmap::fromImage(image));cover->show();
 QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);return cover;
}
#endif
