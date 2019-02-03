import QtQuick 2.12
import com.gecko.rr 1.0 as RR

Item {
    id: tapTargetView
    property RR.TapTarget currentTapTarget: null
    property var tapTargets: []

    function queueTarget(tapTarget) {
        if (currentTapTarget == null)
            currentTapTarget = tapTarget;

        tapTargets.push(tapTarget);
    }

    Rectangle {
        x: currentTapTarget ? currentTapTarget.rect.x : 0
        y: currentTapTarget ? currentTapTarget.rect.y : 0
        width: currentTapTarget ? currentTapTarget.rect.width : 0
        height: currentTapTarget ? currentTapTarget.rect.height : 0
        color: "#80ff0000"
        radius: width / 2
    }
}
