// ARM Harmattan package; includes only explicitly selected runtime assets.
const fs=require('fs'),path=require('path'),zlib=require('zlib'),crypto=require('crypto');
const base=__dirname,work=base;
function tar(entries){const parts=[];for(const e of entries){const b=Buffer.isBuffer(e.data)?e.data:Buffer.from(e.data||''),h=Buffer.alloc(512);if(Buffer.byteLength(e.name)>100)throw Error('tar name too long');const text=(v,o,n)=>h.write(String(v),o,n,'ascii'),oct=(v,o,n)=>text(v.toString(8).padStart(n-1,'0')+'\0',o,n);text(e.name,0,100);oct(e.mode||0o644,100,8);oct(0,108,8);oct(0,116,8);oct(b.length,124,12);oct(1793836800,136,12);h.fill(32,148,156);text(e.directory?'5':'0',156,1);text('ustar\0',257,6);text('00',263,2);text('root',265,32);text('root',297,32);text(h.reduce((a,b)=>a+b,0).toString(8).padStart(6,'0')+'\0 ',148,8);parts.push(h,b,Buffer.alloc((512-b.length%512)%512));}return Buffer.concat([...parts,Buffer.alloc(1024)]);}
function ar(entries){const p=[Buffer.from('!<arch>\n')],f=(v,n)=>String(v).padEnd(n,' ');for(const e of entries){p.push(Buffer.from(f(e.name+'/',16)+f(0,12)+f(0,6)+f(0,6)+f('100644',8)+f(e.data.length,10)+'`\n'),e.data);if(e.data.length%2)p.push(Buffer.from('\n'));}return Buffer.concat(p);}
const payload=[],add=(name,data,mode=0o644)=>payload.push({name:'./'+name,data:Buffer.isBuffer(data)?data:Buffer.from(data),mode});
const read=n=>fs.readFileSync(path.join(base,n));
for(const [src,dest] of [['build/ui/n9-lanshare','n9-lanshare'],['build/core/probe','probe']]){const b=fs.readFileSync(path.join(work,src));if(b.subarray(0,4).toString('hex')!=='7f454c46'||b.readUInt16LE(18)!==40)throw Error('expected ARM ELF');add('opt/localsend-meego/'+dest,b,0o755);}
for(const n of ['Main.qml','SettingsRow.qml','dice.svg','meeshare-logo.png','icon.png','alias-words.json',...['en','zh_TW','ja','ko'].map(l=>'alias-words.'+l+'.json')])add('opt/localsend-meego/'+n,read(n));
for(const l of ['en','zh_TW','ja','ko'])add('opt/localsend-meego/translations/app_'+l+'.qm',read('translations/app_'+l+'.qm'));
add('opt/localsend-meego/launch.sh',read('launch.sh'),0o755);
const icon=read('icon.png');if(icon.readUInt32BE(16)!==80||icon.readUInt32BE(20)!==80)throw Error('80px icon required');
add('usr/share/icons/hicolor/80x80/apps/localsend-meego.png',icon);
const description='为 Nokia N9 开发的 MeeShare，支持局域网文件收发，兼容 LocalSend 协议';
add('usr/share/applications/localsend-meego.desktop','[Desktop Entry]\nType=Application\nName=MeeShare\nComment='+description+'\nExec=/opt/localsend-meego/launch.sh\nIcon=/usr/share/icons/hicolor/80x80/apps/localsend-meego.png\nTerminal=false\nCategories=Utility;\n');
const doc='usr/share/doc/localsend-meego/';
for(const [src,dest] of [['LICENSE','LICENSE'],['NOTICE','NOTICE'],['third_party/localsend/LICENSE','LocalSend-LICENSE'],['third_party/qjson/COPYING.LIB','QJson-COPYING.LIB']])add(doc+dest,read(src));
add(doc+'MbedTLS-LICENSE',fs.readFileSync(path.join(work,'third_party/mbedtls-3.6.7/LICENSE')));
const qjson=fs.readdirSync(path.join(work,'third_party/qjson')).filter(n=>fs.statSync(path.join(work,'third_party/qjson',n)).isFile()).map(n=>({name:'qjson/'+n,data:fs.readFileSync(path.join(work,'third_party/qjson',n))}));
qjson.push({name:'qjson/COPYING.LIB',data:read('third_party/qjson/COPYING.LIB')}, {name:'qjson/BUILD.txt',data:'QJson sources compile as part of ui.pro and network.pro. See docs/BUILD.zh-CN.md.\n'});
add(doc+'qjson-source.tar.gz',zlib.gzipSync(tar(qjson)));
const kit=[...qjson];
for(const n of fs.readdirSync(base).filter(n=>/\.(cpp|h|pro|qml)$/.test(n)))kit.push({name:'app/'+n,data:read(n)});
for(const part of ['ui','core']){const dir=path.join(work,'build',part);for(const n of fs.readdirSync(dir).filter(n=>/\.o$/.test(n)))kit.push({name:part+'-objects/'+n,data:fs.readFileSync(path.join(dir,n))});}
kit.push({name:'libn9mbedtls.a',data:fs.readFileSync(path.join(work,'build/mbedtls/libn9mbedtls.a'))});
kit.push({name:'RELINK.txt',data:'Application sources are provided for modification and personal use. QJson remains LGPL-2.0-or-later.\nTo replace QJson: compile its four cpp/cc source files for Harmattan ARM with QtCore headers and -fPIC; replace parser.o, serializer.o, json_scanner.o and json_parser.o in both object directories.\nLink UI: arm-none-linux-gnueabi-g++ -o n9-lanshare ui-objects/*.o -L<sysroot>/usr/lib -lQtDeclarative -lQtGui -lQtNetwork -lQtCore -lcontentaction -lpthread\nLink network: arm-none-linux-gnueabi-g++ -o probe core-objects/*.o libn9mbedtls.a -L<sysroot>/usr/lib -lQtNetwork -lQtCore -lpthread -lrt\nUse Qt SDK Harmattan target compiler and sysroot; install resulting binaries in /opt/localsend-meego. No signing key is required to relink or build a replacement package.\n'});
add(doc+'relink-kit.tar.gz',zlib.gzipSync(tar(kit)));
add(doc+'copyright','MeeShare original client code, logo and icon: Copyright 2026 Chih, Apache-2.0\nLocalSend protocol and alias dictionaries: LocalSend contributors, Apache-2.0.\nMbed TLS 3.6.7: Mbed TLS contributors, Apache-2.0 (chosen option).\nQJson: Flavio Castelli and contributors, LGPL-2.0-or-later.\nQJson source, application sources and relinkable object files are included in relink-kit.tar.gz.\nQt runtime: provided by the device.\nIndependent compatible implementation; not an official LocalSend release.\n');
const dirs=new Set();for(const e of payload){let d=path.posix.dirname(e.name);while(d!=='.'){dirs.add(d+'/');d=path.posix.dirname(d);}}
const control='Package: localsend-meego\nVersion: 1.005\nArchitecture: armel\nMaintainer: Chih <Cheng_Chih@qq.com>\nSection: user/utilities\nPriority: optional\nDepends: libqt4-core (>= 4.7.4~), libqt4-gui, libqt4-network, libqt4-declarative, qt-components, libx11-6, libcontentaction0, gconf2, libc6, libstdc++6, libgcc1\nInstalled-Size: '+Math.ceil(payload.reduce((s,e)=>s+e.data.length,0)/1024)+'\nMaemo-Display-Name: MeeShare\nMaemo-Flags: visible\nAegis-Manifest: empty\nMaemo-Icon-26: '+icon.toString('base64')+'\nDescription: '+description+'\n';
const md5=payload.map(e=>crypto.createHash('md5').update(e.data).digest('hex')+'  '+e.name.slice(2)).join('\n')+'\n';
const deb=ar([{name:'debian-binary',data:Buffer.from('2.0\n')},{name:'control.tar.gz',data:zlib.gzipSync(tar([{name:'./control',data:control},{name:'./md5sums',data:md5}]))},{name:'data.tar.gz',data:zlib.gzipSync(tar([...dirs].sort((a,b)=>a.length-b.length).map(name=>({name,directory:true,mode:0o755})).concat(payload)))}]);
const out=path.join(base,'packages');fs.mkdirSync(out,{recursive:true});const file=path.join(out,'MeeShare_1.005_armel.deb');fs.writeFileSync(file,deb);fs.writeFileSync(file+'.sha256',crypto.createHash('sha256').update(deb).digest('hex')+'  '+path.basename(file)+'\n');console.log({file,bytes:deb.length,files:payload.length});




