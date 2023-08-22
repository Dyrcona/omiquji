# Copyright © 2012, 2021, 2023 Jason J.A. Stephenson <jason@sigio.com>

# This file is part of omiquji.

# omiquji is free software: you can redistribute it and/or modify it
# under the terms of the Lesser GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# omiquji is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# Lesser GNU General Public License for more details.

# You should have received a copy of the Lesser GNU General Public License
# along with omiquji.  If not, see <http://www.gnu.org/licenses/>.
TEMPLATE = app
TARGET = omiquji
QT += widgets
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x050F00

DESTDIR=../

RESOURCES = ../omiquji.qrc
SOURCES += main.cc mainwindow.cc editdialog.cc omidoc.cc aboutdialog.cc \
    finddialog.cc
HEADERS += mainwindow.hh editdialog.hh omidoc.hh aboutdialog.hh \
    finddialog.hh
FORMS   += mainwindow.ui editdialog.ui aboutdialog.ui \
    finddialog.ui
