// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
// Independent experimental implementation of the published LocalSend v2 protocol.
// Test receiver only: explicitly restricts sender IP and writes to a dedicated directory.
#include <QtCore>
#include <QtNetwork>
#include "parser.h"
#include "serializer.h"
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"
#include "mbedtls/sha256.h"

static QByteArray json(const QVariant &v){return QJson::Serializer().serialize(v);}
static QVariant parse(const QByteArray &s){bool ok=false; QVariant v=QJson::Parser().parse(s,&ok);if(!ok)throw QString("invalid_json");return v;}
static QByteArray readFile(QString p){QFile f(p);if(!f.open(QIODevice::ReadOnly))throw QString("file_read_failed");return f.readAll();}
static void save(QString p,QByteArray b){QFile f(p);if(!f.open(QIODevice::WriteOnly)||f.write(b)!=b.size())throw QString("file_write_failed");f.close();f.setPermissions(QFile::ReadOwner|QFile::WriteOwner);}
static QString randomId(){return QUuid::createUuid().toString().remove('{').remove('}');}
static QByteArray sha(const QByteArray &b){unsigned char out[32];mbedtls_sha256((const unsigned char*)b.constData(),b.size(),out,0);return QByteArray((char*)out,32).toHex();}
static void check(int r,const char *stage){if(r<0){char e[160];mbedtls_strerror(r,e,sizeof(e));throw QString("%1: %2 (%3)").arg(stage).arg(e).arg(r);}}
struct Identity {
 mbedtls_entropy_context entropy;mbedtls_ctr_drbg_context rng;mbedtls_pk_context key;mbedtls_x509_crt crt;QString fingerprint,alias;
 Identity(QString dir){mbedtls_entropy_init(&entropy);mbedtls_ctr_drbg_init(&rng);mbedtls_pk_init(&key);mbedtls_x509_crt_init(&crt);
 check(mbedtls_ctr_drbg_seed(&rng,mbedtls_entropy_func,&entropy,(const unsigned char*)"n9-lanshare",11),"rng");
 QDir().mkpath(dir);QString kp=dir+"/identity-key.pem",cp=dir+"/identity-cert.pem";
 if(!QFile::exists(kp)||!QFile::exists(cp)){
 check(mbedtls_pk_setup(&key,mbedtls_pk_info_from_type(MBEDTLS_PK_RSA)),"pk_setup");
 check(mbedtls_rsa_gen_key(mbedtls_pk_rsa(key),mbedtls_ctr_drbg_random,&rng,2048,65537),"rsa");
 unsigned char pem[8192];check(mbedtls_pk_write_key_pem(&key,pem,sizeof(pem)),"key_pem");save(kp,QByteArray((char*)pem));
 mbedtls_x509write_cert w;mbedtls_x509write_crt_init(&w);mbedtls_x509write_crt_set_version(&w,MBEDTLS_X509_CRT_VERSION_3);
 mbedtls_x509write_crt_set_md_alg(&w,MBEDTLS_MD_SHA256);mbedtls_x509write_crt_set_subject_key(&w,&key);mbedtls_x509write_crt_set_issuer_key(&w,&key);
 unsigned char serial[16];check(mbedtls_ctr_drbg_random(&rng,serial,sizeof(serial)),"serial");serial[0]&=0x7f;serial[0]|=1;
 check(mbedtls_x509write_crt_set_serial_raw(&w,serial,sizeof(serial)),"serial_set");
 check(mbedtls_x509write_crt_set_subject_name(&w,"CN=N9 LAN Share"),"subject");check(mbedtls_x509write_crt_set_issuer_name(&w,"CN=N9 LAN Share"),"issuer");
 check(mbedtls_x509write_crt_set_validity(&w,"20200101000000","20491231235959"),"validity");
 check(mbedtls_x509write_crt_set_basic_constraints(&w,0,-1),"constraints");
 check(mbedtls_x509write_crt_set_key_usage(&w,MBEDTLS_X509_KU_DIGITAL_SIGNATURE|MBEDTLS_X509_KU_KEY_ENCIPHERMENT),"usage");
 check(mbedtls_x509write_crt_pem(&w,pem,sizeof(pem),mbedtls_ctr_drbg_random,&rng),"cert_pem");save(cp,QByteArray((char*)pem));mbedtls_x509write_crt_free(&w);
 }else{QByteArray k=readFile(kp);check(mbedtls_pk_parse_key(&key,(const unsigned char*)k.constData(),k.size()+1,NULL,0,mbedtls_ctr_drbg_random,&rng),"parse_key");}
 check(mbedtls_x509_crt_parse_file(&crt,cp.toLocal8Bit().constData()),"parse_cert");fingerprint=sha(QByteArray((char*)crt.raw.p,crt.raw.len));
 QString namePath=dir+"/alias.txt";if(QFile::exists(namePath))alias=QString::fromUtf8(readFile(namePath)).trimmed();
 if(alias.isEmpty()){QString wordsPath=QCoreApplication::applicationDirPath()+"/alias-words."+QString::fromLatin1(qgetenv("N9_ALIAS_LANGUAGE"))+".json";if(!QFile::exists(wordsPath))wordsPath=QCoreApplication::applicationDirPath()+"/alias-words.json";QVariantMap words=parse(readFile(wordsPath)).toMap();QVariantList adj=words.value("adjectives").toList(),fruits=words.value("fruits").toList();if(adj.isEmpty()||fruits.isEmpty())throw QString("alias_words_missing");unsigned char random[8];check(mbedtls_ctr_drbg_random(&rng,random,8),"alias_random");quint32 x=0,y=0;for(int i=0;i<4;++i){x=(x<<8)|random[i];y=(y<<8)|random[i+4];}alias=words.value("combination").toString().replace("{adjective}",adj.at(x%adj.size()).toString()).replace("{fruit}",fruits.at(y%fruits.size()).toString());save(namePath,alias.toUtf8());}
 }
 ~Identity(){mbedtls_x509_crt_free(&crt);mbedtls_pk_free(&key);mbedtls_ctr_drbg_free(&rng);mbedtls_entropy_free(&entropy);}
 QVariantMap info(){QVariantMap m;m["alias"]=alias;m["version"]="2.0";m["deviceModel"]="Nokia N9";m["deviceType"]="mobile";m["fingerprint"]=fingerprint;m["port"]=53317;m["protocol"]="https";m["download"]=false;return m;}
};
struct Channel {
 mbedtls_ssl_context ssl;mbedtls_ssl_config conf;mbedtls_net_context net;QByteArray pending;
 Channel(Identity &id,bool server,int fd=-1){mbedtls_ssl_init(&ssl);mbedtls_ssl_config_init(&conf);mbedtls_net_init(&net);net.fd=fd;
 check(mbedtls_ssl_config_defaults(&conf,server?MBEDTLS_SSL_IS_SERVER:MBEDTLS_SSL_IS_CLIENT,MBEDTLS_SSL_TRANSPORT_STREAM,MBEDTLS_SSL_PRESET_DEFAULT),"ssl_config");
 mbedtls_ssl_conf_min_tls_version(&conf,MBEDTLS_SSL_VERSION_TLS1_2);mbedtls_ssl_conf_max_tls_version(&conf,MBEDTLS_SSL_VERSION_TLS1_2);
 // LocalSend uses self-signed peer certificates, not public CA validation.
 // Outbound transfer additionally pins the certificate hash selected at discovery.
 mbedtls_ssl_conf_authmode(&conf,MBEDTLS_SSL_VERIFY_NONE);mbedtls_ssl_conf_rng(&conf,mbedtls_ctr_drbg_random,&id.rng);
 mbedtls_ssl_conf_read_timeout(&conf,15000);check(mbedtls_ssl_conf_own_cert(&conf,&id.crt,&id.key),"own_cert");check(mbedtls_ssl_setup(&ssl,&conf),"ssl_setup");}
 ~Channel(){if(net.fd>=0)mbedtls_ssl_close_notify(&ssl);mbedtls_ssl_free(&ssl);mbedtls_ssl_config_free(&conf);mbedtls_net_free(&net);}
 void connect(QString host,int port){check(mbedtls_net_connect(&net,host.toLatin1().constData(),QByteArray::number(port).constData(),MBEDTLS_NET_PROTO_TCP),"connect");}
 void handshake(QString pin=QString()){struct timeval tv;tv.tv_sec=15;tv.tv_usec=0;setsockopt(net.fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));
 mbedtls_ssl_set_bio(&ssl,&net,mbedtls_net_send,mbedtls_net_recv,mbedtls_net_recv_timeout);int r;do{r=mbedtls_ssl_handshake(&ssl);}while(r==MBEDTLS_ERR_SSL_WANT_READ||r==MBEDTLS_ERR_SSL_WANT_WRITE);check(r,"handshake");
 if(!pin.isEmpty()){const mbedtls_x509_crt *c=mbedtls_ssl_get_peer_cert(&ssl);if(!c||QString::fromLatin1(sha(QByteArray((char*)c->raw.p,c->raw.len))).compare(pin,Qt::CaseInsensitive))throw QString("peer_fingerprint_mismatch");}}
 int read(char *b,int n){int r;do{r=mbedtls_ssl_read(&ssl,(unsigned char*)b,n);}while(r==MBEDTLS_ERR_SSL_WANT_READ||r==MBEDTLS_ERR_SSL_WANT_WRITE);if(r==MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)return 0;check(r,"read");return r;}
 void write(QByteArray b){int off=0;while(off<b.size()){int r=mbedtls_ssl_write(&ssl,(const unsigned char*)b.constData()+off,qMin(16384,b.size()-off));if(r==MBEDTLS_ERR_SSL_WANT_READ||r==MBEDTLS_ERR_SSL_WANT_WRITE)continue;check(r,"write");if(r==0)throw QString("write_closed");off+=r;}}
 QByteArray header(){while(!pending.contains("\r\n\r\n")){char b[4096];int n=read(b,sizeof(b));if(!n)throw QString("incomplete_headers");pending.append(b,n);if(pending.size()>32768)throw QString("headers_too_large");}int end=pending.indexOf("\r\n\r\n")+4;QByteArray h=pending.left(end);pending.remove(0,end);return h;}
 QByteArray take(int n){if(pending.isEmpty()){char b[16384];int r=read(b,qMin(n,16384));if(!r)throw QString("incomplete_body");pending.append(b,r);}QByteArray b=pending.left(n);pending.remove(0,b.size());return b;}
 QByteArray exact(int n){QByteArray b;while(b.size()<n)b+=take(n-b.size());return b;}
 QByteArray line(){QByteArray b;while(!b.endsWith("\r\n")){b+=take(1);if(b.size()>8192)throw QString("line_limit");}b.chop(2);return b;}
};
static bool chunked(QByteArray h){return h.toLower().contains("\r\ntransfer-encoding: chunked\r\n");}
static qint64 contentLength(QByteArray h){QList<QByteArray> lines=h.split('\n');qint64 len=-1;bool transfer=false;foreach(QByteArray l,lines){l=l.trimmed();if(l.toLower().startsWith("content-length:")){if(len!=-1)throw QString("duplicate_length");bool ok;len=l.mid(15).trimmed().toLongLong(&ok);if(!ok||len<0)throw QString("bad_length");}if(l.toLower().startsWith("transfer-encoding:")){if(transfer||l.mid(18).trimmed().toLower()!="chunked")throw QString("unsupported_transfer_encoding");transfer=true;}}if(transfer&&len!=-1)throw QString("ambiguous_body_length");return len;}
static QByteArray body(Channel &c,QByteArray h,int max=1048576){qint64 len=contentLength(h);if(len>max)throw QString("body_limit");QByteArray b;if(len<0){char a[4096];b=c.pending;c.pending.clear();for(;;){int n=c.read(a,sizeof(a));if(!n)break;b.append(a,n);if(b.size()>max)throw QString("body_limit");}}else while(b.size()<len)b+=c.take(len-b.size());return b;}
static void response(Channel &c,int status,QByteArray b=QByteArray()){c.write("HTTP/1.1 "+QByteArray::number(status)+" Result\r\nContent-Type: application/json\r\nContent-Length: "+QByteArray::number(b.size())+"\r\nConnection: close\r\n\r\n"+b);}
static QByteArray request(Identity &id,QString host,int port,QString pin,QByteArray path,QByteArray data,int *status=NULL){Channel c(id,false);c.connect(host,port);c.handshake(pin);mbedtls_ssl_conf_read_timeout(&c.conf,120000);c.write("POST "+path+" HTTP/1.1\r\nHost: "+host.toLatin1()+"\r\nContent-Type: application/json\r\nContent-Length: "+QByteArray::number(data.size())+"\r\nConnection: close\r\n\r\n"+data);QByteArray h=c.header();int s=h.split(' ').value(1).toInt();if(status)*status=s;QByteArray b=body(c,h);if(s<200||s>=300)throw QString("http_%1").arg(s);return b;}

static void emitJson(const char *prefix,const QVariant &data){QByteArray line=QByteArray(prefix)+json(data)+"\n";fwrite(line.constData(),1,line.size(),stdout);fflush(stdout);}
static bool privateIp(quint32 ip){return (ip>>24)==10||(ip>>20)==0xac1||(ip>>16)==0xc0a8;}
static bool onLan(QString host){QHostAddress a(host);if(a.protocol()!=QAbstractSocket::IPv4Protocol||!privateIp(a.toIPv4Address()))return false;foreach(QNetworkInterface nic,QNetworkInterface::allInterfaces()){if(!(nic.flags()&QNetworkInterface::IsUp))continue;foreach(QNetworkAddressEntry e,nic.addressEntries()){quint32 ip=e.ip().toIPv4Address(),mask=e.netmask().toIPv4Address();if(privateIp(ip)&&mask&&(ip&mask)==(a.toIPv4Address()&mask))return true;}}return false;}
static void peerEvent(QVariantMap peer,QString host,QString own){QString name=peer.value("alias").toString().trimmed().left(100);if(name.isEmpty()||peer.value("fingerprint").toString().compare(own,Qt::CaseInsensitive)==0||!onLan(host))return;peer["alias"]=name;peer["ip"]=host;if(!peer.contains("port"))peer["port"]=53317;if(!peer.contains("protocol"))peer["protocol"]="https";emitJson("PEER ",peer);}
struct Discovery {
 int fd;QList<quint32> addresses;
 Discovery(){fd=socket(AF_INET,SOCK_DGRAM,0);if(fd<0)return;int yes=1;setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));struct sockaddr_in addr;memset(&addr,0,sizeof(addr));addr.sin_family=AF_INET;addr.sin_port=htons(53317);addr.sin_addr.s_addr=htonl(INADDR_ANY);if(::bind(fd,(sockaddr*)&addr,sizeof(addr))){::close(fd);fd=-1;return;}
 foreach(QNetworkInterface nic,QNetworkInterface::allInterfaces()){if(!(nic.flags()&QNetworkInterface::IsUp))continue;foreach(QNetworkAddressEntry e,nic.addressEntries()){quint32 ip=e.ip().toIPv4Address();if(!privateIp(ip)||addresses.contains(ip))continue;addresses<<ip;struct ip_mreq group;group.imr_multiaddr.s_addr=inet_addr("224.0.0.167");group.imr_interface.s_addr=htonl(ip);setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&group,sizeof(group));}}}
 ~Discovery(){if(fd>=0)::close(fd);}
 void announce(Identity &id,bool request=true){if(fd<0)return;QVariantMap info=id.info();info["announce"]=request;QByteArray bytes=json(info);struct sockaddr_in to;memset(&to,0,sizeof(to));to.sin_family=AF_INET;to.sin_port=htons(53317);to.sin_addr.s_addr=inet_addr("224.0.0.167");foreach(quint32 ip,addresses){struct in_addr source;source.s_addr=htonl(ip);setsockopt(fd,IPPROTO_IP,IP_MULTICAST_IF,&source,sizeof(source));sendto(fd,bytes.constData(),bytes.size(),0,(sockaddr*)&to,sizeof(to));}}
 void read(Identity &id){char bytes[8193];struct sockaddr_in from;socklen_t len=sizeof(from);int n=recvfrom(fd,bytes,sizeof(bytes),0,(sockaddr*)&from,&len);if(n<=0||n>8192)return;QString host=QString::fromLatin1(inet_ntoa(from.sin_addr));try{QVariantMap m=parse(QByteArray(bytes,n)).toMap();if(m.value("fingerprint").toString().compare(id.fingerprint,Qt::CaseInsensitive)==0)return;peerEvent(m,host,id.fingerprint);if(m.value("announce").toBool()&&onLan(host))announce(id,false);}catch(QString){}}
};

int main(int argc,char **argv){QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));QCoreApplication app(argc,argv);signal(SIGPIPE,SIG_IGN);QStringList a=app.arguments();
 try{if(a.size()<3)throw QString("usage: --discover identityDir | --serve identityDir receiveDir allowedIP | --send identityDir host port fingerprint file");
 if(a[1]=="--new-name")QFile::remove(a[2]+"/alias.txt");
 Identity id(a[2]);
 if(a[1]=="--identity"||a[1]=="--new-name"){emitJson("",id.info());return 0;}
 if(a[1]=="--info"&&a.size()==5){Channel c(id,false);c.connect(a[3],a[4].toInt());c.handshake();const mbedtls_x509_crt *peer=mbedtls_ssl_get_peer_cert(&c.ssl);if(!peer)throw QString("missing_peer_certificate");QVariantMap result;result["certificateFingerprint"]=QString::fromLatin1(sha(QByteArray((char*)peer->raw.p,peer->raw.len)));QByteArray registration=json(id.info());c.write("POST /api/localsend/v2/register HTTP/1.1\r\nHost: "+a[3].toLatin1()+"\r\nContent-Type: application/json\r\nContent-Length: "+QByteArray::number(registration.size())+"\r\nConnection: close\r\n\r\n"+registration);QByteArray h=c.header();result["status"]=h.split(' ').value(1).toInt();result["info"]=parse(body(c,h));emitJson("",result);return 0;}
 if(a[1]=="--discover"){
 QUdpSocket udp;if(!udp.bind(QHostAddress::Any,53317,QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint))throw QString("udp_bind_failed");
 // Qt 4.7 has no joinMulticastGroup: join through the native descriptor.
 struct ip_mreq group;group.imr_multiaddr.s_addr=inet_addr("224.0.0.167");group.imr_interface.s_addr=htonl(INADDR_ANY);
 if(setsockopt(udp.socketDescriptor(),IPPROTO_IP,IP_ADD_MEMBERSHIP,&group,sizeof(group)))throw QString("multicast_join_failed");
 QVariantMap info=id.info();info["announce"]=true;udp.writeDatagram(json(info),QHostAddress("224.0.0.167"),53317);
 QTime timer;timer.start();QSet<QString> seen;while(timer.elapsed()<8000){udp.waitForReadyRead(500);while(udp.hasPendingDatagrams()){QByteArray b;b.resize(udp.pendingDatagramSize());QHostAddress host;quint16 port;udp.readDatagram(b.data(),b.size(),&host,&port);try{QVariantMap m=parse(b).toMap();if(m.value("fingerprint").toString()==id.fingerprint)continue;QString key=host.toString()+m.value("fingerprint").toString();if(seen.contains(key))continue;seen.insert(key);m["ip"]=host.toString();qDebug()<<json(m);}catch(QString){}}}return 0;
 }

 if(a[1]=="--send"&&a.size()>=7){
 QStringList paths=a.mid(6),order;QVariantMap files;QMap<QString,QString> locations;QSet<QString> names;
 if(paths.size()>20)throw QString("too_many_files");
 foreach(QString path,paths){QFileInfo stat(path);QFile f(path);if(!stat.isFile()||!f.open(QIODevice::ReadOnly))throw QString("send_file_missing");QString name=stat.fileName();if(names.contains(name))throw QString("duplicate_file_name");names.insert(name);QString fid=randomId();QVariantMap file;file["id"]=fid;file["fileName"]=name;file["size"]=f.size();file["fileType"]="application/octet-stream";files[fid]=file;locations[fid]=path;order<<fid;}
 QVariantMap prep;prep["info"]=id.info();prep["files"]=files;
 QVariantMap reply=parse(request(id,a[3],a[4].toInt(),a[5],"/api/localsend/v2/prepare-upload",json(prep))).toMap();QString sid=reply.value("sessionId").toString();QVariantMap accepted=reply.value("files").toMap();
 if(sid.isEmpty())throw QString("file_not_accepted");
 foreach(QString fid,order)if(accepted.value(fid).toString().isEmpty()){try{request(id,a[3],a[4].toInt(),a[5],"/api/localsend/v2/cancel?sessionId="+QUrl::toPercentEncoding(sid),QByteArray());}catch(QString){}throw QString("file_not_accepted");}
 qint64 total=0;int sent=0;
 foreach(QString fid,order){QFile f(locations.value(fid));if(!f.open(QIODevice::ReadOnly)||f.size()!=files.value(fid).toMap().value("size").toLongLong())throw QString("send_file_changed");
 Channel c(id,false);c.connect(a[3],a[4].toInt());c.handshake(a[5]);QByteArray path="/api/localsend/v2/upload?sessionId="+QUrl::toPercentEncoding(sid)+"&fileId="+QUrl::toPercentEncoding(fid)+"&token="+QUrl::toPercentEncoding(accepted.value(fid).toString());
 c.write("POST "+path+" HTTP/1.1\r\nHost: "+a[3].toLatin1()+"\r\nContent-Type: application/octet-stream\r\nContent-Length: "+QByteArray::number(f.size())+"\r\nConnection: close\r\n\r\n");while(!f.atEnd()){QByteArray bytes=f.read(65536);if(bytes.isEmpty()&&f.error()!=QFile::NoError)throw QString("send_file_read");c.write(bytes);}QByteArray h=c.header();int status=h.split(' ').value(1).toInt();if(status!=200)throw QString("upload_http_%1").arg(status);total+=f.size();++sent;qDebug()<<"SENT_FILE count="<<sent;
 }
 qDebug()<<"SEND_OK files="<<sent<<"bytes="<<total;return 0;
 }
 if(a[1]=="--serve"&&(a.size()==5||a.size()==6)){bool consent=a.size()==6&&a[5]=="consent",autosave=a.size()==6&&a[5]=="autosave";QString dir=QDir(a[3]).absolutePath();QDir().mkpath(dir);QString allowed=a[4];if(allowed!="lan"&&QHostAddress(allowed).isNull())throw QString("sender_ip_required");if(allowed=="lan"&&!consent&&!autosave)throw QString("lan_requires_consent");
 mbedtls_net_context listener;mbedtls_net_init(&listener);check(mbedtls_net_bind(&listener,NULL,"53317",MBEDTLS_NET_PROTO_TCP),"listen");
 QString session,source;QVariantMap accepted;QMap<QString,QString> tokens;QSet<QString> complete;Discovery discovery;QTime announced;announced.start();
 qDebug()<<"READY TLS1.2 restricted_test_receiver fingerprint="<<id.fingerprint;
 discovery.announce(id);
 for(;;){if(announced.elapsed()>15000){discovery.announce(id);announced.restart();}fd_set sockets;FD_ZERO(&sockets);FD_SET(listener.fd,&sockets);int highest=listener.fd;if(discovery.fd>=0){FD_SET(discovery.fd,&sockets);highest=qMax(highest,discovery.fd);}struct timeval tick;tick.tv_sec=2;tick.tv_usec=0;int ready=select(highest+1,&sockets,NULL,NULL,&tick);if(ready<=0)continue;if(discovery.fd>=0&&FD_ISSET(discovery.fd,&sockets))discovery.read(id);if(!FD_ISSET(listener.fd,&sockets))continue;
 mbedtls_net_context incoming;mbedtls_net_init(&incoming);unsigned char ip[16];size_t iplen;int r=mbedtls_net_accept(&listener,&incoming,ip,sizeof(ip),&iplen);if(r<0)continue;char addr[INET6_ADDRSTRLEN];inet_ntop(iplen==4?AF_INET:AF_INET6,ip,addr,sizeof(addr));QString peer(addr);
 if((allowed=="lan"&&!onLan(peer))||(allowed!="lan"&&peer!=allowed)){mbedtls_net_free(&incoming);continue;}
 try{Channel c(id,true,incoming.fd);incoming.fd=-1;c.handshake();QByteArray h=c.header();QList<QByteArray> parts=h.split(' ');QUrl url(QString::fromUtf8(parts.value(1)));QString path=url.path();
 if(path=="/api/localsend/v2/info"||path=="/api/localsend/v2/register"){if(path.endsWith("register")){QVariantMap metadata=parse(body(c,h,65536)).toMap();peerEvent(metadata,peer,id.fingerprint);}response(c,200,json(id.info()));continue;}
 if(path=="/api/localsend/v2/prepare-upload"){
 if(!session.isEmpty()&&complete.size()!=accepted.size()){response(c,409);continue;}
 QVariantMap req=parse(body(c,h)).toMap(),files=req.value("files").toMap();if(files.isEmpty()||files.size()>20){response(c,400);continue;}
 bool valid=true;QSet<QString> names;qint64 total=0;for(QVariantMap::const_iterator it=files.begin();it!=files.end();++it){QVariantMap m=it.value().toMap();QString name=m.value("fileName").toString();qint64 size=m.value("size").toLongLong();if(name.isEmpty()||name=="."||name==".."||name.contains('/')||name.contains('\\')||name.contains(QChar(0))||names.contains(name)||QFile::exists(dir+"/"+name)||size<0||size>64*1024*1024){valid=false;break;}names.insert(name);total+=size;}if(!valid||total>128*1024*1024){response(c,400);continue;}
 if(consent){QVariantMap offer;offer["sender"]=req.value("info").toMap().value("alias");offer["ip"]=peer;offer["count"]=files.size();offer["bytes"]=total;QVariantList names;foreach(QString key,files.keys())names.append(files.value(key).toMap().value("fileName"));offer["names"]=names;emitJson("OFFER ",offer);
 fd_set set;FD_ZERO(&set);FD_SET(0,&set);struct timeval wait;wait.tv_sec=110;wait.tv_usec=0;if(select(1,&set,NULL,NULL,&wait)<=0){response(c,403);continue;}char decision[32];if(!fgets(decision,sizeof(decision),stdin)||QByteArray(decision).trimmed()!="accept"){response(c,403);continue;}}
 session=randomId();source=peer;accepted=files;tokens.clear();complete.clear();QVariantMap replyTokens;foreach(QString key,files.keys()){QString t=randomId();tokens[key]=t;replyTokens[key]=t;}QVariantMap reply;reply["sessionId"]=session;reply["files"]=replyTokens;response(c,200,json(reply));qDebug()<<"PREPARE_ACCEPTED files="<<files.size();continue;
 }
 if(path=="/api/localsend/v2/upload"){
 QString fid=url.queryItemValue("fileId");if(session.isEmpty()||peer!=source||url.queryItemValue("sessionId")!=session||!tokens.contains(fid)||url.queryItemValue("token")!=tokens.value(fid)||complete.contains(fid)){response(c,403);continue;}
 QVariantMap meta=accepted.value(fid).toMap();qint64 size=meta.value("size").toLongLong();if(!chunked(h)&&contentLength(h)!=size){response(c,400);continue;}contentLength(h);
 QString final=dir+"/"+meta.value("fileName").toString(),temp=dir+"/."+randomId()+".part";QFile f(temp);if(!f.open(QIODevice::WriteOnly)){response(c,500);continue;}f.setPermissions(QFile::ReadOwner|QFile::WriteOwner);
 mbedtls_sha256_context hash;mbedtls_sha256_init(&hash);mbedtls_sha256_starts(&hash,0);qint64 received=0;bool ok=true;try{bool done=false;while(!done){qint64 block=size-received;if(chunked(h)){QByteArray line=c.line().split(';').value(0);bool valid=false;block=line.toLongLong(&valid,16);if(!valid||block<0||block>size-received)throw QString("invalid_chunk_size");if(block==0){int trailers=0;while(!c.line().isEmpty()){if(++trailers>20)throw QString("trailer_limit");}done=true;}}else done=true;
 while(block>0){QByteArray b=c.take(qMin(qint64(16384),block));if(f.write(b)!=b.size())throw QString("disk_write");mbedtls_sha256_update(&hash,(unsigned char*)b.constData(),b.size());received+=b.size();block-=b.size();}if(chunked(h)&&!done&&c.exact(2)!="\r\n")throw QString("invalid_chunk_end");}if(received!=size)throw QString("size_mismatch");}catch(QString){ok=false;}
 unsigned char digest[32];mbedtls_sha256_finish(&hash,digest);mbedtls_sha256_free(&hash);f.close();QString actual=QByteArray((char*)digest,32).toHex(),expected=meta.value("sha256").toString();if(!expected.isEmpty()&&expected.compare(actual,Qt::CaseInsensitive))ok=false;
 if(!ok||!QFile::rename(temp,final)){QFile::remove(temp);response(c,422);qDebug()<<"RECEIVE_FAILED";session.clear();accepted.clear();tokens.clear();complete.clear();continue;}complete.insert(fid);response(c,200);if(complete.size()==accepted.size())qDebug()<<"RECEIVE_OK bytes="<<received<<"sha256="<<actual;else qDebug()<<"RECEIVE_PROGRESS files="<<complete.size();continue;
 }
 if(path=="/api/localsend/v2/cancel"&&peer==source&&url.queryItemValue("sessionId")==session){session.clear();accepted.clear();tokens.clear();complete.clear();response(c,200);qDebug()<<"RECEIVE_CANCELED";continue;}
 response(c,404);
 }catch(QString error){qDebug()<<"CONNECTION_ERROR"<<error;mbedtls_net_free(&incoming);}
 }
 }
 throw QString("invalid_arguments");
 }catch(QString error){qCritical()<<"PROBE_ERROR"<<error;return 1;}
}
