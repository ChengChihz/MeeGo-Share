// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
#ifndef N9_SINGLE_INSTANCE_H
#define N9_SINGLE_INSTANCE_H
#include <QtGui>
#include <QtNetwork>
#include <unistd.h>
class SingleInstance : public QObject {
 Q_OBJECT
 QLocalServer server;QWidget *window;QString name;
 bool activateExisting(){QLocalSocket socket;socket.connectToServer(name);if(!socket.waitForConnected(200))return false;socket.write("activate\n");socket.waitForBytesWritten(200);socket.disconnectFromServer();return true;}
public:
 SingleInstance(QWidget *view):window(view),name("chih-localsend-meego-"+QString::number(getuid())){connect(&server,SIGNAL(newConnection()),this,SLOT(activate()));}
 bool acquire(){if(activateExisting())return false;if(server.listen(name))return true;if(activateExisting())return false;if(server.serverError()==QAbstractSocket::AddressInUseError){QLocalServer::removeServer(name);return server.listen(name);}return false;}
private slots:
 void activate(){while(server.hasPendingConnections()){QLocalSocket *socket=server.nextPendingConnection();socket->close();socket->deleteLater();}if(window){window->showFullScreen();window->raise();window->activateWindow();}}
};
#endif
