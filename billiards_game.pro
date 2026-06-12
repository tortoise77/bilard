TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle qt

HEADERS += GameObjects.h \
    GameObjects.h
SOURCES += main.cpp GameObjects.cpp

SFML_PATH = $$PWD/SFML-2.6.2-windows-gcc-13.1.0-mingw-64-bit/SFML-2.6.2
INCLUDEPATH += "$$SFML_PATH/include"
LIBS += -L"$$SFML_PATH/lib"

CONFIG(debug, debug|release){
    LIBS += -lsfml-audio-d -lsfml-graphics-d -lsfml-network-d -lsfml-system-d -lsfml-window-d
} else {
    LIBS += -lsfml-audio -lsfml-graphics -lsfml-network -lsfml-system -lsfml-window
}

CONFIG(debug, debug|release) {
    SFML_DLLS = sfml-system-d-2 sfml-window-d-2 sfml-graphics-d-2 sfml-audio-d-2 sfml-network-d-2
} else {
    SFML_DLLS = sfml-system-2 sfml-window-2 sfml-graphics-2 sfml-audio-2 sfml-network-2
}

for(dll, SFML_DLLS) {
    copydata.commands += $${QMAKE_COPY} \"$$shell_path($$SFML_PATH/bin/$${dll}.dll)\" \"$$shell_path($$OUT_PWD)\" $$escape_expand(\n\t)
}
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata