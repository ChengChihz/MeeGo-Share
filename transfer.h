// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
#ifndef N9_TRANSFER_H
#define N9_TRANSFER_H
#include <QtGui>
#include <QtDeclarative>
#include <QtNetwork>
#include "parser.h"
#include "localization.h"

class Transfer : public QObject {
 Q_OBJECT
 Q_PROPERTY(QString status READ status NOTIFY changed)
 Q_PROPERTY(QString localAddress READ localAddress NOTIFY changed)
 Q_PROPERTY(QString localName READ localName NOTIFY changed)
 Q_PROPERTY(QString offer READ offer NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(bool connected READ connected NOTIFY changed)
 Q_PROPERTY(bool listening READ listening NOTIFY changed)
 Q_PROPERTY(QString folder READ folder NOTIFY changed)
 Q_PROPERTY(QVariantList entries READ entries NOTIFY changed)
 Q_PROPERTY(QVariantList devices READ devices NOTIFY changed)
 Q_PROPERTY(QString selected READ selected NOTIFY changed)
 Q_PROPERTY(int selectedCount READ selectedCount NOTIFY changed)
 Q_PROPERTY(QString selectedSummary READ selectedSummary NOTIFY changed)
 Q_PROPERTY(QString peer READ peer NOTIFY changed)
 Q_PROPERTY(QString host READ host WRITE setHost NOTIFY changed)
 Q_PROPERTY(bool autoSave READ autoSave WRITE setAutoSave NOTIFY changed)
 Q_PROPERTY(QString receiveDirectory READ receiveDirectory NOTIFY changed)
 Q_PROPERTY(QString pickerMode READ pickerMode NOTIFY changed)
 QString state,message,dir,chosen,peerName,ip,pin,root,identity,receive,helper,ownName,operation;
 QVariantList listing,deviceList;QMap<QString,QVariantMap> known;QStringList selection;
 QProcess receiver,job;QTimer decisionTimer,expiry;QByteArray receiverBuffer,jobOutput;
 bool working,stopping,canceled,automatic,receiving;int peerPort;QString pickMode;
 bool canChangeService()const{return !working&&message.isEmpty()&&!receiving;}
 void restartService(){if(listening()){stopReceive();startReceive();}}
 static QString trUtf(const char *s){return QCoreApplication::translate("App",s,0,QCoreApplication::UnicodeUTF8);}
 void finish(QString text){state=text;emit changed();emit feedback(text);}
 void refreshList(){deviceList.clear();foreach(QVariantMap m,known)deviceList.append(m);emit changed();}
 void remember(QVariantMap m,QString address){QString name=m.value("alias").toString().trimmed().left(100);if(name.isEmpty()||QHostAddress(address).isNull())return;m["ip"]=address;m["lastSeen"]=QDateTime::currentDateTime().toTime_t();if(!m.contains("port"))m["port"]=53317;if(!m.contains("protocol"))m["protocol"]="https";known[address]=m;if(ip==address){peerName=name;peerPort=m.value("port").toInt();}refreshList();}
 void run(QString op,QStringList args){operation=op;jobOutput.clear();working=true;canceled=false;state=op=="info"?trUtf("正在连接设备…"):op=="send"?trUtf("等待对方接收…"):trUtf("正在准备设备名称…");job.start(helper,args);emit changed();}
 QString failureText(){QString e=QString::fromUtf8(jobOutput);if(e.contains("http_403")||e.contains("file_not_accepted"))return trUtf("对方拒绝接收。");if(e.contains("http_409"))return trUtf("对方正在处理其他传输，请稍后再试。");if(e.contains("http_401"))return trUtf("对方要求 PIN 验证，此版本暂不支持。");if(e.contains("http_429"))return trUtf("请求过于频繁，请稍后再试。");if(e.contains("timed out")||e.contains("timeout"))return trUtf("等待对方响应超时，请重试。");if(e.contains("peer_fingerprint_mismatch"))return trUtf("对方设备身份已变化，请重新连接后发送。");if(e.contains("file_missing")||e.contains("file_read"))return trUtf("无法读取选中的文件，请重新选择。");return operation=="send"?trUtf("发送失败，请检查网络及对方设备。"):trUtf("连接失败，请确认对方已打开 LocalSend。");}
public:
 Transfer():working(false),stopping(false),canceled(false),receiving(false),peerPort(53317){
  root="/home/user/MyDocs";dir=root;identity=QDir::homePath()+"/.config/n9-lanshare";receive=root+"/LocalSend";helper=QCoreApplication::applicationDirPath()+"/probe";
  QDir().mkpath(identity);QDir().mkpath(receive);QSettings settings;QString stored=QFileInfo(settings.value("receive_directory",receive).toString()).canonicalFilePath();if(stored==root||stored.startsWith(root+"/"))receive=stored;automatic=settings.value("auto_save",false).toBool();pickMode="files";ip=settings.value("peer_ip","192.168.1.13").toString();ownName=trUtf("Nokia N9");state=trUtf("正在准备局域网发现…");
  decisionTimer.setSingleShot(true);decisionTimer.setInterval(105000);connect(&decisionTimer,SIGNAL(timeout()),this,SLOT(rejectExpired()));
  expiry.setInterval(15000);connect(&expiry,SIGNAL(timeout()),this,SLOT(expirePeers()));expiry.start();
  receiver.setProcessChannelMode(QProcess::MergedChannels);connect(&receiver,SIGNAL(readyRead()),this,SLOT(readReceiver()));connect(&receiver,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(receiverEnded()));connect(&receiver,SIGNAL(error(QProcess::ProcessError)),this,SLOT(receiverError(QProcess::ProcessError)));
  job.setProcessChannelMode(QProcess::MergedChannels);connect(&job,SIGNAL(readyRead()),this,SLOT(readJob()));connect(&job,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(jobEnded(int,QProcess::ExitStatus)));connect(&job,SIGNAL(error(QProcess::ProcessError)),this,SLOT(jobError(QProcess::ProcessError)));
  browse(root);QTimer::singleShot(0,this,SLOT(initialize()));
 }
 ~Transfer(){disconnect(&receiver,0,this,0);disconnect(&job,0,this,0);receiver.kill();receiver.waitForFinished(2000);job.kill();job.waitForFinished(2000);}
 QString localAddress()const{foreach(QHostAddress a,QNetworkInterface::allAddresses()){quint32 n=a.toIPv4Address();if(a.protocol()==QAbstractSocket::IPv4Protocol&&((n>>24)==10||(n>>20)==0xac1||(n>>16)==0xc0a8))return a.toString();}return trUtf("请连接 Wi-Fi");}
 QString status()const{return state;}QString localName()const{return ownName;}QString offer()const{return message;}bool busy()const{return working;}bool listening()const{return receiver.state()!=QProcess::NotRunning;}QString folder()const{return dir;}QVariantList entries()const{return listing;}QVariantList devices()const{return deviceList;}QString selected()const{return chosen;}QString peer()const{return peerName;}QString host()const{return ip;}
 bool connected()const{return !pin.isEmpty();}bool autoSave()const{return automatic;}QString receiveDirectory()const{return receive;}QString pickerMode()const{return pickMode;}
 int selectedCount()const{return selection.size();}QString selectedSummary()const{return selection.isEmpty()?trUtf("选择要发送的文件"):selection.size()==1?QFileInfo(selection.first()).fileName():trUtf("已选择 %1 个文件").arg(selection.size());}
 Q_INVOKABLE bool isSelected(QString path)const{return selection.contains(path)||selection.contains(QFileInfo(path).canonicalFilePath());}
 Q_INVOKABLE void clearSelection(){selection.clear();chosen.clear();emit changed();}
 Q_INVOKABLE void toggleSelection(QString path){QFileInfo f(path);if(f.isDir()){browse(path);return;}QString canonical=f.canonicalFilePath();if(!canonical.startsWith(root+"/")||f.isSymLink()||!f.isFile()||!f.isReadable())return;if(selection.contains(canonical))selection.removeAll(canonical);else{if(selection.size()>=20){finish(trUtf("每次最多选择 20 个文件。"));return;}foreach(QString selected,selection)if(QFileInfo(selected).fileName()==f.fileName()){finish(trUtf("存在同名文件，请分开传输。"));return;}selection.append(canonical);}chosen=selection.isEmpty()?QString():selection.first();emit changed();}
 void setAutoSave(bool enabled){if(enabled==automatic)return;if(!canChangeService()){finish(trUtf("请先完成当前传输。"));return;}automatic=enabled;QSettings().setValue("auto_save",enabled);restartService();emit changed();}
 void setHost(QString s){if(s==ip)return;ip=s.trimmed();pin.clear();peerName=known.value(ip).value("alias").toString();peerPort=known.value(ip).value("port",53317).toInt();emit changed();}
 Q_INVOKABLE void browse(QString path){QString canonical=QFileInfo(path).canonicalFilePath();if(canonical!=root&&!canonical.startsWith(root+"/"))return;dir=canonical;listing.clear();QDir d(dir);foreach(QFileInfo f,d.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|QDir::Readable,QDir::DirsFirst|QDir::Name)){if(f.isSymLink())continue;if(!f.isDir()){if(pickMode=="folder")continue;if(pickMode=="photos"&&!QStringList(QStringList()<<"jpg"<<"jpeg"<<"png"<<"gif"<<"bmp"<<"webp").contains(f.suffix().toLower()))continue;}QVariantMap m;m["name"]=f.fileName();m["path"]=f.absoluteFilePath();m["directory"]=f.isDir();m["size"]=f.size();listing.append(m);}emit changed();}
 Q_INVOKABLE void up(){if(dir!=root)browse(QFileInfo(dir).path());}
 Q_INVOKABLE void openReceived(){browse(receive);}
 Q_INVOKABLE void beginPicker(QString mode){pickMode=mode;QString target=mode=="folder"?receive:mode=="photos"?root+"/DCIM":root;if(!QFileInfo(target).isDir())target=root;browse(target);}
 Q_INVOKABLE void setReceiveDirectory(QString path){if(!canChangeService()){finish(trUtf("请先完成当前传输。"));return;}QFileInfo f(path);QString canonical=f.canonicalFilePath();if(!f.isDir()||f.isSymLink()||(canonical!=root&&!canonical.startsWith(root+"/"))||!f.isWritable()){finish(trUtf("此目录不可用，请选择手机文件区域内的文件夹。"));return;}receive=canonical;QSettings().setValue("receive_directory",receive);restartService();finish(trUtf("保存目录已更新。"));}
 Q_INVOKABLE void choose(QString path){if(QFileInfo(path).isDir())browse(path);else if(!isSelected(path))toggleSelection(path);}
 Q_INVOKABLE void connectPeer(){if(working)return;QHostAddress a(ip);quint32 n=a.toIPv4Address();if(a.protocol()!=QAbstractSocket::IPv4Protocol||!((n>>24)==10||(n>>20)==0xac1||(n>>16)==0xc0a8)){finish(trUtf("请输入局域网 IPv4 地址。"));return;}QSettings().setValue("peer_ip",ip);run("info",QStringList()<<"--info"<<identity<<ip<<QString::number(peerPort));}
 Q_INVOKABLE void selectDevice(int index){if(working||index<0||index>=deviceList.size())return;QVariantMap m=deviceList.at(index).toMap();if(m.value("protocol").toString()!="https"){finish(trUtf("此设备使用未加密传输，请在对方开启加密后重试。"));return;}setHost(m.value("ip").toString());connectPeer();}
 Q_INVOKABLE void send(){if(working)return;if(pin.isEmpty()||selection.isEmpty()){finish(trUtf("请先连接设备并选择文件。"));return;}QStringList args;args<<"--send"<<identity<<ip<<QString::number(peerPort)<<pin;args<<selection;run("send",args);}
 Q_INVOKABLE void startReceive(){if(listening()||working)return;receiverBuffer.clear();stopping=false;receiver.start(helper,QStringList()<<"--serve"<<identity<<receive<<"lan"<<(automatic?"autosave":"consent"));state=trUtf("正在发现附近设备…");emit changed();}
 Q_INVOKABLE void stopReceive(){stopping=true;receiver.kill();receiver.waitForFinished(1000);decisionTimer.stop();receiving=false;message.clear();state=trUtf("接收与设备发现已关闭。");emit changed();}
 Q_INVOKABLE void scan(){if(working||!message.isEmpty())return;stopReceive();startReceive();}
 Q_INVOKABLE void regenerateName(){if(working||!message.isEmpty())return;stopReceive();run("identity",QStringList()<<"--new-name"<<identity);}
 Q_INVOKABLE void setLocalName(QString name){if(!canChangeService()){finish(trUtf("请先完成当前传输。"));return;}name=name.trimmed().left(50);if(name.isEmpty()){finish(trUtf("设备名称不能为空。"));return;}for(int i=0;i<name.size();++i)if(name.at(i).unicode()<32||name.at(i).unicode()==127)return;QFile f(identity+"/alias.txt");if(!f.open(QIODevice::WriteOnly)||f.write(name.toUtf8())!=name.toUtf8().size()){finish(trUtf("设备名称保存失败。"));return;}f.close();f.setPermissions(QFile::ReadOwner|QFile::WriteOwner);ownName=name;restartService();finish(trUtf("设备名称已保存。"));}
 Q_INVOKABLE void answer(bool accept){if(message.isEmpty())return;decisionTimer.stop();receiver.write(accept?"accept\n":"reject\n");receiving=accept;message.clear();state=accept?trUtf("正在接收…"):trUtf("已拒绝接收。");emit changed();}
 Q_INVOKABLE void cancel(){if(working){canceled=true;job.kill();}}
 Q_INVOKABLE bool requestLanguage(QString code){if(!canChangeService()){finish(trUtf("请先完成当前传输。"));return false;}if(!AppLanguage::validChoice(code))return false;emit languageRequested(code);return true;}
signals:
 void changed();void feedback(QString text);void languageRequested(QString code);
private slots:
 void languageApplied(){state=listening()?trUtf("已就绪，正在发现附近设备。"):trUtf("接收与设备发现已关闭。");emit changed();}
 void languageFailed(){finish(trUtf("语言切换失败，请重试。"));}
 void initialize(){run("identity",QStringList()<<"--identity"<<identity);}
 void rejectExpired(){answer(false);finish(trUtf("接收确认已超时，请重新发送。"));}
 void expirePeers(){uint now=QDateTime::currentDateTime().toTime_t();foreach(QString key,known.keys())if(now-known.value(key).value("lastSeen").toUInt()>75)known.remove(key);refreshList();}
 void readJob(){jobOutput+=job.readAll();if(jobOutput.size()>1048576)job.kill();}
 void jobError(QProcess::ProcessError e){if(e==QProcess::FailedToStart){working=false;finish(trUtf("无法启动传输组件，请重新打开应用。"));}}
 void receiverError(QProcess::ProcessError e){if(e==QProcess::FailedToStart)finish(trUtf("无法启动接收组件。"));}
 void jobEnded(int code,QProcess::ExitStatus exit){readJob();working=false;if(canceled){finish(trUtf("已取消发送。"));return;}if(code||exit!=QProcess::NormalExit){if(QCoreApplication::arguments().contains("--controller-probe"))qDebug()<<"controller_backend_error"<<jobOutput;finish(failureText());return;}
  if(operation=="identity"||operation=="info"){bool ok;QVariantMap data=QJson::Parser().parse(jobOutput,&ok).toMap();if(!ok){finish(trUtf("设备信息无法识别，请重试。"));return;}
   if(operation=="identity"){ownName=data.value("alias").toString();startReceive();emit changed();return;}
   if(data.value("status").toInt()!=200){finish(trUtf("连接失败，对方响应异常。"));return;}pin=data.value("certificateFingerprint").toString();QVariantMap info=data.value("info").toMap();if(info.value("alias").toString().trimmed().isEmpty())info["alias"]=known.value(ip).value("alias",trUtf("未命名设备"));remember(info,ip);finish(trUtf("已连接：")+peerName);return;
  }
  finish(trUtf("发送完成。"));
 }
 void receiverEnded(){receiving=false;decisionTimer.stop();message.clear();if(!stopping)finish(trUtf("接收服务已停止，请重新开启。"));emit changed();}
 void readReceiver(){receiverBuffer+=receiver.readAll();int at;while((at=receiverBuffer.indexOf('\n'))>=0){QByteArray line=receiverBuffer.left(at);receiverBuffer.remove(0,at+1);
   if(line.startsWith("PEER ")){bool ok;QVariantMap m=QJson::Parser().parse(line.mid(5),&ok).toMap();if(ok)remember(m,m.value("ip").toString());}
   else if(line.startsWith("OFFER ")){bool ok;QVariantMap m=QJson::Parser().parse(line.mid(6),&ok).toMap();if(ok){decisionTimer.start();QStringList names;foreach(QVariant v,m.value("names").toList())names<<v.toString();message=trUtf("来自 ")+m.value("sender").toString()+"\n\n"+names.join("\n")+"\n\n"+QString::number(m.value("bytes").toLongLong())+trUtf(" 字节");}}
   else if(line.startsWith("READY"))state=trUtf("已就绪，正在发现附近设备。");
   else if(line.startsWith("RECEIVE_OK")){receiving=false;finish(trUtf("接收完成，已保存到 ")+trUtf("文件")+receive.mid(root.size())+trUtf("。"));}
   else if(line.startsWith("RECEIVE_FAILED")){receiving=false;finish(trUtf("接收失败，未保存不完整文件，请重试。"));}
   else if(line.startsWith("PREPARE_ACCEPTED")){receiving=true;state=trUtf("正在接收…");}
   else if(line.startsWith("RECEIVE_CANCELED")){receiving=false;finish(trUtf("对方已取消发送。"));}
   else if(line.contains("PROBE_ERROR"))finish(trUtf("无法开启接收，可能端口正在使用。"));
   else if(line.startsWith("CONNECTION_ERROR")&&receiving){receiving=false;finish(trUtf("接收中断，请检查网络后重试。"));}
   emit changed();
  }if(receiverBuffer.size()>1048576)receiverBuffer.clear();
 }
};
#endif
