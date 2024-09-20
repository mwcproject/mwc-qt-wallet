import QtQuick 2.15

Item {
    id: font

    property alias light: light.name
    property alias lightItalic: lightItalic.name

    property alias medium: medium.name
    property alias mediumItalic: mediumItalic.name

    property alias bold: bold.name
    property alias boldItalic: boldItalic.name


    FontLoader {
        id: light;
        source: "../font/Barlow-Light.ttf"
    }
    FontLoader {
        id: lightItalic;
        source: "../font/Barlow-LightItalic.ttf"
    }
    FontLoader {
        id: medium;
        source: "../font/Barlow-Medium.ttf"
    }
    FontLoader {
        id: mediumItalic;
        source: "../font/Barlow-MediumItalic.ttf"
    }
    FontLoader {
        id: bold
        source: "../font/Barlow-Bold.ttf"
    }
    FontLoader {
        id: boldItalic;
        source: "../font/Barlow-BoldItalic.ttf"
    }


}

/*##^##
Designer {
    D{i:0;autoSize:true;height:700;width:400}D{i:5;anchors_y:63}D{i:2;anchors_width:200;anchors_x:56;anchors_y:71}
D{i:6;anchors_height:200;anchors_width:200;anchors_x:100;anchors_y:290}
}
##^##*/
