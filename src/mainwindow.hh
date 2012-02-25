/*
 * Copyright © 2012 Jason J.A. Stephenson <jason@sigio.com>
 *
 * This file is part of omiquji.
 *
 * omiquji is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * omiquji is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with omiquji.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QtGui>
#include "ui_mainwindow.h"
#include "omidoc.hh"

class EditDialog;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

public slots:

protected:
	void closeEvent(QCloseEvent*);

private slots:
	void addComment();
	void editComment();
	void deleteComment();
	void addFortune();
	void editFortune();
	void deleteFortune();
	// Action methods:
	void newFile();
	void open();
	bool save();
	bool saveAs();
	void about();

private:
	bool okToContinue();
	bool checkDocForSave();
	bool loadFile(const QString&);
	bool saveFile(const QString&);
	void setCurrentFile(const QString&);
	QString strippedName(const QString&);
	void connectEditMenu(EditDialog*);
	void disconnectEditMenu(EditDialog *dialog=0);

	Ui::MainWindow ui;
	OmiDoc *doc;
	QString currentFilename;
}; 

#endif
