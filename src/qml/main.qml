import QtQuick 2.10
import QtQuick.Controls.Material 2.3
import QtQuick.Controls 2.3 as QQC2
import Fluid.Controls 1.0 as FluidControls
import "rrui" as RRUi
import "common"
import "user"
import "models"

RRUi.ApplicationWindow {
    id: mainWindow
    title: qsTr("Record Rack")
    visibility: FluidControls.ApplicationWindow.Maximized
    pageStack.anchors.leftMargin: sidebar.expanded ? sidebar.width : 0

    initialPage: LoginPage {
        QQC2.StackView.onRemoved: {
            mainWindow.appBar.visible = true;
            sidebar.expanded = true;
        }
        onLoggedIn: mainWindow.pageStack.replace(sidebar.model.get(0).fileName);
    }

    RRUi.Sidebar {
        id: sidebar
        expanded: false
        onCurrentIndexChanged: mainWindow.pageStack.replace(null, model.get(currentIndex).fileName);
    }
/*
function phoneFormatter() {
  $('.phone').on('input', function() {
    var number = $(this).val().replace(/[^\d]/g, '')
    if (number.length == 7) {
      number = number.replace(/(\d{3})(\d{4})/, "$1-$2");
    } else if (number.length == 10) {
      number = number.replace(/(\d{3})(\d{3})(\d{4})/, "($1) $2-$3");
    }
    $(this).val(number)
  });
};
*/
}
