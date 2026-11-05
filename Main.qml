// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
import QtQuick 1.1
import com.nokia.meego 1.0
import com.nokia.extras 1.0

PageStackWindow {
 id:app
 showStatusBar:true
 property color accent:"#365f87"
 property color muted:"#73869b"
 property bool promptShown:false
 function tr(source){if(i18n.revision<0)return source;return i18n.text(source);}
 function languageName(code){var names={zh_CN:"简体中文",zh_TW:"繁體中文",en:"English",ja:"日本語",ko:"한국어"};return code==="system"?app.tr("跟随系统"):names[code]||app.tr("跟随系统");}
 initialPage:Page {
  orientationLock:PageOrientation.LockPortrait
  Rectangle {anchors.fill:parent;color:"#f4f8fd"}
  tools:ToolBarLayout {
   ButtonRow {
    anchors.fill:parent
    style:TabButtonStyle {textColor:app.muted;checkedTextColor:app.accent}
    TabButton {text:app.tr("接收");tab:receivePage}
    TabButton {text:app.tr("发送");tab:sendPage}
    TabButton {objectName:"settingsTab";text:app.tr("设置");tab:settingsPage}
   }
  }
  Column {
   id:heading;x:26;y:18;width:parent.width-52;spacing:3
   Label {text:"MeeShare";font.pixelSize:32;font.bold:true;color:app.accent}
   Label {text:app.tr("兼容 LocalSend，Ver %1").arg("1.004");font.pixelSize:18;color:app.muted}
  }
  TabGroup {
   id:tabs;Component.onCompleted:{if(previewTab===1)currentTab=sendPage;else if(previewTab===2)currentTab=settingsPage;}
   anchors{left:parent.left;right:parent.right;top:heading.bottom;topMargin:16;bottom:statusPanel.top}
currentTab:receivePage
   Item {
    id:receivePage;anchors.fill:parent
    Item {anchors{left:parent.left;right:parent.right;top:parent.top;bottom:receivedFilesButton.top}
     Column {id:receiveContent;width:parent.width-52;anchors.centerIn:parent;anchors.verticalCenterOffset:-18;spacing:28
      Image {source:"meeshare-logo.png";width:112;height:112;smooth:true;anchors.horizontalCenter:parent.horizontalCenter}
      Label {width:parent.width;text:transfer.localName;horizontalAlignment:Text.AlignHCenter;wrapMode:Text.Wrap;font.pixelSize:36;color:"#24394f"}
      Button {width:300;anchors.horizontalCenter:parent.horizontalCenter;text:transfer.listening?app.tr("停止接收"):app.tr("开启接收");platformStyle:ButtonStyle {textColor:app.accent}
       onClicked:transfer.listening?transfer.stopReceive():transfer.startReceive()}
     }
    }
    Button {id:receivedFilesButton;width:300;anchors.horizontalCenter:parent.horizontalCenter;anchors.bottom:parent.bottom;anchors.bottomMargin:24;text:app.tr("查看接收文件");onClicked:{transfer.beginPicker("files");transfer.openReceived();pageStack.push(browser)}}
   }
   Item {
    id:sendPage;anchors.fill:parent
    property alias item:sendLoader.item
    Loader {id:sendLoader;anchors.fill:parent;sourceComponent:sendBody}
   }
   Component {id:sendBody
   Item {
    anchors.fill:parent
    Flickable {anchors.fill:parent;contentHeight:sendContent.height+36;clip:true
     Column {
      id:sendContent;x:26;y:8;width:parent.width-52;spacing:20
      Label {text:app.tr("选择");font.pixelSize:26;color:"#24394f"}
      Rectangle {
       width:parent.width;height:selection.height+36;radius:14;color:"#e5effb";border.color:"#cbdced"
       Column {id:selection;x:18;y:18;width:parent.width-36;spacing:15
        Label {width:parent.width;text:transfer.selectedSummary;font.pixelSize:22;color:"#24394f";wrapMode:Text.Wrap}
        Row {width:parent.width;spacing:12
         Button {width:(parent.width-12)/2;text:app.tr("照片");enabled:!transfer.busy;platformStyle:ButtonStyle {textColor:app.accent}
          onClicked:{transfer.beginPicker("photos");pageStack.push(browser)}}
         Button {width:(parent.width-12)/2;text:app.tr("文件");enabled:!transfer.busy;platformStyle:ButtonStyle {textColor:app.accent}
          onClicked:{transfer.beginPicker("files");pageStack.push(browser)}}
        }
       }
      }
      Item {width:1;height:12}
      Row {width:parent.width;spacing:8
       Label {width:parent.width-136;text:app.tr("附近的设备");font.pixelSize:26;color:"#24394f"}
       Button {width:128;text:app.tr("刷新");enabled:!transfer.busy;platformStyle:ButtonStyle {fontPixelSize:19;textColor:app.accent}
        onClicked:transfer.scan()}
      }
      Label {width:parent.width;visible:transfer.devices.length===0;text:app.tr("正在寻找设备，请确保目标连接到同一个WiFi网络。");font.pixelSize:20;color:app.muted;wrapMode:Text.Wrap}
      Column {width:parent.width;spacing:12
       Repeater {model:transfer.devices
        Rectangle {width:sendContent.width;height:100;radius:14;color:modelData.ip===transfer.host?"#cde4ff":"#e5effb";border.color:modelData.ip===transfer.host?"#7da8d5":"#cbdced"
         Column {x:20;anchors.verticalCenter:parent.verticalCenter;width:parent.width-40;spacing:8
          Label {width:parent.width;text:modelData.alias;font.pixelSize:25;color:"#24394f";elide:Text.ElideRight}
          Label {width:parent.width;text:modelData.deviceModel||app.tr("设备");font.pixelSize:19;color:app.muted;elide:Text.ElideRight}
         }
         MouseArea {anchors.fill:parent;enabled:!transfer.busy;onClicked:transfer.selectDevice(index)}
        }
       }
      }
      Button {width:parent.width;text:transfer.busy?app.tr("正在处理…"):app.tr("发送");enabled:!transfer.busy&&transfer.selectedCount>0&&transfer.connected;platformStyle:ButtonStyle {textColor:app.accent}
onClicked:transfer.send()}
      Button {width:parent.width;text:app.tr("清空选择");visible:transfer.selectedCount>0&&!transfer.busy;onClicked:transfer.clearSelection()}
      Button {width:parent.width;text:app.tr("取消发送");visible:transfer.busy;onClicked:transfer.cancel()}
     }
    }
   }
   }
   Item {
    id:settingsPage;anchors.fill:parent
    property alias item:settingsLoader.item
    Loader {id:settingsLoader;anchors.fill:parent;sourceComponent:settingsBody}
   }
   Component {id:settingsBody
   Item {
    anchors.fill:parent
    Flickable {anchors.fill:parent;contentHeight:settingsContent.height+40;clip:true
     Column {
      id:settingsContent;x:26;y:8;width:parent.width-52;spacing:16
      Label {text:app.tr("设置");font.pixelSize:28;color:"#24394f"}
      Rectangle {width:parent.width;height:100;color:"#edf3fc"
       Column {x:18;anchors.verticalCenter:parent.verticalCenter;width:parent.width-150;spacing:6
        Label {text:app.tr("自动保存");font.pixelSize:25;color:"#24394f"}
        Label {width:parent.width;text:app.tr("收到文件后直接保存");font.pixelSize:18;color:app.muted;wrapMode:Text.Wrap}
       }
       Switch {anchors.right:parent.right;anchors.rightMargin:16;anchors.verticalCenter:parent.verticalCenter;checked:transfer.autoSave;enabled:!transfer.busy;onCheckedChanged:transfer.autoSave=checked}
      }
      Column {width:parent.width
       SettingsRow {title:app.tr("保存目录");value:transfer.receiveDirectory.replace('/home/user/MyDocs',app.tr("文件"));onClicked:{transfer.beginPicker("folder");pageStack.push(browser)}}
       SettingsRow {title:app.tr("设备名称");value:transfer.localName;onClicked:pageStack.push(namePage)}
       SettingsRow {objectName:"languageSetting";title:app.tr("语言");value:app.languageName(i18n.languageChoice);onClicked:pageStack.push(languagePage)}
       SettingsRow {title:app.tr("高级设置");onClicked:pageStack.push(advancedPage)}
      }
      Item {width:1;height:4}
      SettingsRow {title:app.tr("关于");onClicked:pageStack.push(aboutPage)}
      Label {width:parent.width;text:app.tr("MeeShare · MeeGo 适配：Chih");font.pixelSize:14;color:"#8d9eb2";horizontalAlignment:Text.AlignHCenter}

     }
    }
   }
   }
  }
  Rectangle {
   id:statusPanel;anchors{left:parent.left;right:parent.right;bottom:parent.bottom}
height:Math.max(56,statusText.paintedHeight+24);color:"#e6effa"
   Label {id:statusText;x:24;y:12;width:parent.width-48;text:transfer.status;font.pixelSize:18;color:app.muted;wrapMode:Text.Wrap}
  }
 }
 Component {
  id:browser
  Page {
   orientationLock:PageOrientation.LockPortrait
   Rectangle {anchors.fill:parent;color:"#f4f8fd"}
   tools:ToolBarLayout {
    Item {width:parent.width;height:parent.height
     ToolIcon {x:8;anchors.verticalCenter:parent.verticalCenter;iconId:"toolbar-back";onClicked:pageStack.pop()}
     Button {id:doneSelection;width:180;platformStyle:ButtonStyle {fontPixelSize:22}
anchors.right:parent.right;anchors.rightMargin:20;anchors.verticalCenter:parent.verticalCenter;text:transfer.pickerMode==="folder"?app.tr("选择此目录"):app.tr("完成（%1）").arg(transfer.selectedCount);onClicked:{if(transfer.pickerMode==="folder")transfer.setReceiveDirectory(transfer.folder);pageStack.pop()}}
     Button {width:110;platformStyle:ButtonStyle {fontPixelSize:22}
anchors.right:doneSelection.left;anchors.rightMargin:12;anchors.verticalCenter:parent.verticalCenter;text:app.tr("上一级");onClicked:transfer.up()}
    }
   }
   Column {anchors.fill:parent;spacing:10
    Item {width:parent.width;height:80
     Label {x:24;y:8;width:parent.width-48;height:40;text:transfer.folder.replace('/home/user/MyDocs',app.tr("文件"));color:app.accent;elide:Text.ElideLeft;verticalAlignment:Text.AlignVCenter}
     Label {x:24;y:49;width:parent.width-48;visible:transfer.pickerMode!=="folder";text:app.tr("已选择 %1 个文件").arg(transfer.selectedCount);font.pixelSize:18;color:app.muted}
    }
    ListView {visible:transfer.pickerMode!=="photos";width:parent.width;height:parent.height-90;clip:true;model:transfer.entries
     delegate:Item {objectName:"filePickRow";property string itemPath:modelData.path;width:parent.width;height:76
      Label {x:24;anchors.verticalCenter:parent.verticalCenter;width:parent.width-100;text:(modelData.directory?"▸  ":"")+modelData.name;elide:Text.ElideMiddle;font.pixelSize:24;color:"#24394f"}
      Rectangle {anchors.bottom:parent.bottom;width:parent.width;height:1;color:"#d5e1ef"}
      CheckBox {objectName:"filePickCheck";z:2;anchors.right:parent.right;anchors.rightMargin:20;anchors.verticalCenter:parent.verticalCenter;visible:!modelData.directory&&transfer.pickerMode!=="folder";checked:transfer.selectedCount>=0&&transfer.isSelected(modelData.path);onClicked:transfer.toggleSelection(modelData.path)}
      MouseArea {z:3;anchors.fill:parent;onClicked:transfer.toggleSelection(modelData.path)}
     }
    }
    GridView {visible:transfer.pickerMode==="photos";width:parent.width;height:parent.height-90;clip:true;cellWidth:parent.width/3;cellHeight:178;model:transfer.entries
     delegate:Item {objectName:"photoPickRow";property string itemPath:modelData.path;width:GridView.view.cellWidth;height:178
      Rectangle {x:8;y:4;width:parent.width-16;height:112;radius:8;color:"#e0ebf8";clip:true
       Image {anchors.fill:parent;anchors.margins:4;source:modelData.directory?"":"file://"+modelData.path;sourceSize.width:240;sourceSize.height:240;fillMode:Image.PreserveAspectCrop;clip:true;smooth:true;asynchronous:true;cache:false}
       Label {anchors.centerIn:parent;visible:modelData.directory;text:app.tr("文件夹");font.pixelSize:21;color:app.accent}
      }
      Label {x:8;y:130;width:parent.width-16;visible:modelData.directory;text:modelData.name;font.pixelSize:19;color:app.muted;elide:Text.ElideMiddle;horizontalAlignment:Text.AlignHCenter}
      CheckBox {objectName:"photoPickCheck";z:2;x:(parent.width-42*0.8)/2;y:128;scale:0.8;transformOrigin:Item.TopLeft;visible:!modelData.directory;checked:transfer.selectedCount>=0&&transfer.isSelected(modelData.path);onClicked:transfer.toggleSelection(modelData.path)}
      MouseArea {z:3;anchors.fill:parent;onClicked:transfer.toggleSelection(modelData.path)}
     }
    }
   }
  }
 }
 Component {id:namePage
  Page {orientationLock:PageOrientation.LockPortrait
   Rectangle {anchors.fill:parent;color:"#f4f8fd"}
   tools:ToolBarLayout {ToolIcon {iconId:"toolbar-back";onClicked:pageStack.pop()}}
   Column {x:26;y:24;width:parent.width-52;spacing:24
    Label {text:app.tr("设备名称");font.pixelSize:30;color:app.accent}
    Label {width:parent.width;text:app.tr("其他设备会看到这个名称。");font.pixelSize:21;color:app.muted;wrapMode:Text.Wrap}
    Row {width:parent.width;spacing:12
     TextField {id:newName;width:parent.width-84;text:transfer.localName;maximumLength:50}
     Button {width:72;enabled:!transfer.busy;Image {anchors.centerIn:parent;width:38;height:38;source:"dice.svg"}onClicked:{newName.focus=false;transfer.regenerateName()}}
    }
    Button {width:parent.width;text:app.tr("保存名称");enabled:!transfer.busy;onClicked:{transfer.setLocalName(newName.text);pageStack.pop()}}
    Label {width:parent.width;text:app.tr("点击骰子，用 LocalSend 官方词库随机生成新名称。名称会保留，重新打开应用也不会改变。");font.pixelSize:19;color:app.muted;wrapMode:Text.Wrap}
   }
   Connections {target:transfer;onChanged:{if(!newName.activeFocus)newName.text=transfer.localName}}
  }
 }
 Component {id:languagePage
  Page {orientationLock:PageOrientation.LockPortrait
   Rectangle {anchors.fill:parent;color:"#f4f8fd"}
   tools:ToolBarLayout {ToolIcon {iconId:"toolbar-back";onClicked:pageStack.pop()}}
   Column {id:languageHeading;x:26;y:24;width:parent.width-52;spacing:16
    Label {text:app.tr("语言");font.pixelSize:30;color:app.accent}
    Label {width:parent.width;text:app.tr("选择后立即生效，跟随系统会使用手机的界面语言。");font.pixelSize:20;color:app.muted;wrapMode:Text.Wrap}
   }
   ListView {anchors{top:languageHeading.bottom;topMargin:24;left:parent.left;right:parent.right;bottom:parent.bottom}
clip:true
    model:ListModel {
     ListElement {code:"system"}
     ListElement {code:"zh_CN"}
     ListElement {code:"zh_TW"}
     ListElement {code:"en"}
     ListElement {code:"ja"}
     ListElement {code:"ko"}
    }
    delegate:Item {width:parent.width;height:78
     Rectangle {anchors.fill:parent;color:mouse.pressed?"#d5e7fc":"#edf3fc"}
     Label {x:26;width:parent.width-110;anchors.verticalCenter:parent.verticalCenter;text:app.languageName(code);font.pixelSize:25;color:"#24394f"}
     CheckBox {anchors.right:parent.right;anchors.rightMargin:24;anchors.verticalCenter:parent.verticalCenter;checked:i18n.languageChoice===code}
     Rectangle {anchors.bottom:parent.bottom;width:parent.width;height:1;color:"#d2dfef"}
     MouseArea {id:mouse;z:3;anchors.fill:parent;onClicked:{if(transfer.requestLanguage(code))pageStack.pop();}}
    }
   }
  }
 }
 Component {id:advancedPage
  Page {orientationLock:PageOrientation.LockPortrait
   Rectangle {anchors.fill:parent;color:"#f4f8fd"}
   tools:ToolBarLayout {ToolIcon {iconId:"toolbar-back";onClicked:pageStack.pop()}}
   Column {x:26;y:24;width:parent.width-52;spacing:22
    Label {text:app.tr("高级设置");font.pixelSize:30;color:app.accent}
    Label {width:parent.width;text:app.tr("通常无需修改。无法自动发现时，可尝试手动连接。");font.pixelSize:20;color:app.muted;wrapMode:Text.Wrap}
    Label {text:app.tr("对方设备的 IP 地址");font.pixelSize:23;color:"#24394f"}
    TextField {width:parent.width;text:transfer.host;enabled:!transfer.busy;onTextChanged:transfer.host=text;inputMethodHints:Qt.ImhNoPredictiveText}
    Button {width:parent.width;text:app.tr("手动连接");enabled:!transfer.busy;onClicked:transfer.connectPeer()}
    Label {width:parent.width;text:app.tr("本机 IP：")+transfer.localAddress+app.tr("\n端口：53317\n传输加密：HTTPS / TLS 1.2");font.pixelSize:20;color:app.muted;wrapMode:Text.Wrap}
   }
  }
 }
 Component {id:aboutPage
  Page {orientationLock:PageOrientation.LockPortrait
   onStatusChanged:{if(status===PageStatus.Active)aboutFlickable.contentY=0}
   Rectangle {anchors.fill:parent;color:"#f4f8fd"}
   tools:ToolBarLayout {ToolIcon {iconId:"toolbar-back";onClicked:pageStack.pop()}}
   Flickable {id:aboutFlickable;anchors.fill:parent;contentHeight:aboutContent.height+52;clip:true
    Column {id:aboutContent;x:26;y:24;width:parent.width-52;spacing:24
     Image {source:"meeshare-logo.png";width:80;height:80;smooth:true}
     Label {text:"MeeShare";font.pixelSize:30;color:app.accent}
     Label {text:"Ver 1.004";font.pixelSize:20;color:app.muted}
     Label {width:parent.width;text:app.tr("为 Nokia N9 开发的局域网文件传输应用，兼容 LocalSend 协议");font.pixelSize:22;color:"#24394f";wrapMode:Text.Wrap}
     Label {width:parent.width;text:app.tr("支持局域网文件收发，当前每个接收文件最多 64MB，传输时请保持应用打开");font.pixelSize:20;color:app.muted;wrapMode:Text.Wrap}
     Label {width:parent.width;text:app.tr("收发设备需连接同一 WiFi 网络。收到的文件可在保存目录中查看");font.pixelSize:19;color:app.muted;wrapMode:Text.Wrap}
     Item {width:1;height:12}
     Label {width:parent.width;text:app.tr("客户端开发、MeeGo 适配与界面：Chih\nLocalSend 协议与随机名称词库：LocalSend 项目及贡献者\n网络与数据组件：Qt、Mbed TLS、QJson\n\n独立兼容实现，与 LocalSend 项目无官方隶属关系");font.pixelSize:19;color:"#889aaf";wrapMode:Text.Wrap;lineHeight:1.2}
     Label {width:parent.width;text:app.tr("LocalSend 原项目：localsend.org");font.pixelSize:18;color:"#889aaf";wrapMode:Text.Wrap}
    }
   }
  }
 }
 Timer {interval:250;running:previewTab>=3;repeat:false;onTriggered:{if(previewTab===3)pageStack.push(namePage);else if(previewTab===4)pageStack.push(advancedPage);else if(previewTab===5)pageStack.push(aboutPage);else if(previewTab===6){transfer.beginPicker('photos');var chosen=0;for(var i=0;i<transfer.entries.length&&chosen<2;i++){if(!transfer.entries[i].directory){transfer.toggleSelection(transfer.entries[i].path);chosen++;}}pageStack.push(browser);}else if(previewTab===7){transfer.beginPicker('folder');pageStack.push(browser);}else if(previewTab===8){app.notify(app.tr("已连接：测试设备"));}else if(previewTab===9){transfer.beginPicker('files');pageStack.push(browser);}else if(previewTab===12){pageStack.push(languagePage);}else if(previewTab===10||previewTab===11){transfer.beginPicker(previewTab===11?'photos':'files');transfer.browse(pickerFixturePath);pageStack.push(browser);}}}
 Item {id:popupHost;anchors.fill:parent;z:1000}
 Component {id:bannerComponent
 InfoBanner {id:resultBanner;objectName:"transferNotice";topMargin:44;z:1000;timerShowTime:4500}
 }
 QtObject {id:bannerLoader;property variant item:null}

 Component {id:dialogComponent
 QueryDialog {id:receiveDialog;titleText:app.tr("接收文件？");message:transfer.offer;acceptButtonText:app.tr("接收");rejectButtonText:app.tr("拒绝");onAccepted:{app.promptShown=false;transfer.answer(true)}
onRejected:{app.promptShown=false;transfer.answer(false)}}
 }
 QtObject {id:dialogLoader;property variant item:null;property bool needed:false;onNeededChanged:{if(needed&&!item)item=dialogComponent.createObject(popupHost);}}
 function notify(text){if(!bannerLoader.item)bannerLoader.item=bannerComponent.createObject(popupHost);if(bannerLoader.item){bannerLoader.item.text=text;bannerLoader.item.show();}}
 Connections {target:transfer;onFeedback:app.notify(text);onChanged:{if(transfer.offer.length&&!app.promptShown){dialogLoader.needed=true;if(dialogLoader.item&&!app.promptShown){app.promptShown=true;dialogLoader.item.open();}}else if(!transfer.offer.length&&app.promptShown){app.promptShown=false;if(dialogLoader.item)dialogLoader.item.close();}}}
}








