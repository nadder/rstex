#!/bin/bash
qmake QMAKE_CXX=$CXX QMAKE_LINK=$CXX QMAKE_CXXFLAGS+=-Wno-inconsistent-missing-override TeXFontViewerQt.pro
make
