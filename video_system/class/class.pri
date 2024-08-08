INCLUDEPATH += $$PWD/api
INCLUDEPATH += $$PWD/app
INCLUDEPATH += $$PWD/usercontrol
INCLUDEPATH += $$PWD/devicevideo
INCLUDEPATH += $$PWD/devicecustom
INCLUDEPATH += $$PWD/deviceinterface

include ($$PWD/api/api.pri)
include ($$PWD/app/app.pri)
include ($$PWD/usercontrol/usercontrol.pri)
include ($$PWD/devicevideo/devicevideo.pri)
include ($$PWD/devicecustom/devicecustom.pri)
include ($$PWD/deviceinterface/deviceinterface.pri)
