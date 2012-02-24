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
#include "editdialog.hh"

EditDialog::EditDialog(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
}

QString EditDialog::textValue()
{
	return ui.valueEdit->toPlainText();
}

void EditDialog::setTextValue(const QString &value)
{
	ui.valueEdit->setPlainText(value);
}

void EditDialog::connectCutAction(QAction* action)
{
	connect(action, SIGNAL(triggered()), ui.valueEdit, SLOT(cut()));
}

void EditDialog::connectCopyAction(QAction* action)
{
	connect(action, SIGNAL(triggered()), ui.valueEdit, SLOT(copy()));
}

void EditDialog::connectPasteAction(QAction* action)
{
	connect(action, SIGNAL(triggered()), ui.valueEdit, SLOT(paste()));
}

void EditDialog::disconnectCutAction(QAction* action)
{
	disconnect(action, SIGNAL(triggered()), ui.valueEdit, SLOT(cut()));
}

void EditDialog::disconnectCopyAction(QAction* action)
{
	disconnect(action, SIGNAL(triggered()), ui.valueEdit, SLOT(copy()));
}

void EditDialog::disconnectPasteAction(QAction* action)
{
	disconnect(action, SIGNAL(triggered()), ui.valueEdit, SLOT(paste()));
}
