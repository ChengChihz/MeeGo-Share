// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
#ifndef N9_TRANSLATION_PROBE_H
#define N9_TRANSLATION_PROBE_H
#include "localization.h"
#include "parser.h"
inline int translationProbe(){
 QFile fixture(QCoreApplication::applicationDirPath()+"/translations/probe.json");if(!fixture.open(QIODevice::ReadOnly))return 3;bool parsed=false;QVariantMap data=QJson::Parser().parse(fixture.readAll(),&parsed).toMap();if(!parsed)return 3;
 QVariantList keys=data.value("keys").toList(),expected=data.value(AppLanguage::current()).toList();if(keys.size()!=expected.size())return 3;
 bool native=true;for(int i=0;i<keys.size();++i){QByteArray source=keys.at(i).toString().toUtf8();if(QCoreApplication::translate("App",source.constData(),0,QCoreApplication::UnicodeUTF8)!=expected.at(i).toString()){native=false;qWarning()<<"translation_mismatch"<<i;}}
 QDeclarativeEngine engine;TranslationBridge bridge;engine.rootContext()->setContextProperty("i18n",&bridge);QDeclarativeComponent component(&engine);component.setData("import QtQuick 1.1\nItem { property string label: i18n.text(\"\xe6\x8e\xa5\xe6\x94\xb6\") }",QUrl());QObject *root=component.create();bool qml=root&&root->property("label").toString()==expected.value(0).toString();delete root;
 bool fallbacks=AppLanguage::normalize("zh-Hant-TW")=="zh_TW"&&AppLanguage::normalize("zh-Hans")=="zh_CN"&&AppLanguage::normalize("en_US")=="en"&&AppLanguage::normalize("fr_FR")=="en"&&AppLanguage::normalize("ja_JP")=="ja"&&AppLanguage::normalize("ko_KR")=="ko";
 qDebug()<<"translation_probe language="<<AppLanguage::current()<<"messages="<<keys.size()<<"native="<<native<<"qml="<<qml<<"fallbacks="<<fallbacks;return native&&qml&&fallbacks?0:4;
}
#endif
