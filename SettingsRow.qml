// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Chih
import QtQuick 1.1
import com.nokia.meego 1.0
Item {
 id:row
 property string title
 property string value:""
 signal clicked()
 width:parent?parent.width:432
 height:value.length?88:72
 Rectangle {anchors.fill:parent;color:mouse.pressed?"#d5e7fc":"#edf3fc"}
 Column {x:18;anchors.verticalCenter:parent.verticalCenter;width:parent.width-54;spacing:5
  Label {width:parent.width;text:row.title;font.pixelSize:25;color:"#24394f";elide:Text.ElideRight}
  Label {width:parent.width;text:row.value;visible:text.length>0;font.pixelSize:19;color:"#73869b";elide:Text.ElideMiddle}
 }
 Label {anchors.right:parent.right;anchors.rightMargin:18;anchors.verticalCenter:parent.verticalCenter;text:"›";font.pixelSize:34;color:"#8199b3"}
 Rectangle {anchors.bottom:parent.bottom;width:parent.width;height:1;color:"#d2dfef"}
 MouseArea {id:mouse;anchors.fill:parent;onClicked:row.clicked()}
}

