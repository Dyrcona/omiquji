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
#ifndef EDITDIALOG_HH
#define EDITDIALOG_HH

#include <QDialog>
#include "ui_editdialog.h"

class EditDialog : public QDialog
{
	Q_OBJECT

public:
	EditDialog(QWidget *parent=0);

	QString textValue();
	void setTextValue(const QString&);
	void connectCutAction(QAction*);
	void connectCopyAction(QAction*);
	void connectPasteAction(QAction*);
	void disconnectCutAction(QAction*);
	void disconnectCopyAction(QAction*);
	void disconnectPasteAction(QAction*);

private:
	Ui::EditDialog ui;
};

#endif
