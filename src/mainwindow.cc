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
#include "mainwindow.hh"
#include "editdialog.hh"
#include "aboutdialog.hh"

MainWindow::MainWindow() : QMainWindow()
{
	ui.setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);

	setCurrentFile("");

	doc = 0;

	// Menu actions
	connect(ui.action_New, SIGNAL(triggered()), this, SLOT(newFile()));
	connect(ui.action_Open, SIGNAL(triggered()), this, SLOT(open()));
	connect(ui.action_Save, SIGNAL(triggered()), this, SLOT(save()));
	connect(ui.actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(ui.action_Close, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.action_Quit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
	connect(ui.action_About, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui.actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	// Action buttons.
	connect(ui.addCommentButton, SIGNAL(clicked()), this, SLOT(addComment()));
	connect(ui.editCommentButton, SIGNAL(clicked()), this, SLOT(editComment()));
	connect(ui.deleteCommentButton, SIGNAL(clicked()), this, SLOT(deleteComment()));
	connect(ui.addFortuneButton, SIGNAL(clicked()), this, SLOT(addFortune()));
	connect(ui.editFortuneButton, SIGNAL(clicked()), this, SLOT(editFortune()));
	connect(ui.deleteFortuneButton, SIGNAL(clicked()), this, SLOT(deleteFortune()));
}

void MainWindow::addComment()
{
	EditDialog dlg( this );
	dlg.setWindowTitle(tr("Add Comment"));

	if (dlg.exec() == QDialog::Accepted) {
		if (!doc)
			doc = new OmiDoc();
		QString text = dlg.textValue();
		ui.commentList->addItem(text);
		doc->addComment(text);
		setWindowModified(true);
	}
}

void MainWindow::deleteComment()
{
	if (doc) {	
		int idx = ui.commentList->currentRow();
		doc->removeCommentAt(idx);
	}
	delete ui.commentList->currentItem();
	setWindowModified(true);
}

void MainWindow::editComment()
{
	if (!ui.commentList->currentItem())
		return;

	QString entry = ui.commentList->currentItem()->text();
	EditDialog dlg(this);
	dlg.setWindowTitle(tr("Edit Comment"));
	dlg.setTextValue(entry);

	if (dlg.exec() == QDialog::Accepted) {
		if (doc) {
			int idx = ui.commentList->currentRow();
			doc->replaceCommentAt(idx, dlg.textValue());
		}
		ui.commentList->currentItem()->setText(dlg.textValue());
		setWindowModified(true);
	}
}

void MainWindow::addFortune()
{
	EditDialog dlg( this );
	dlg.setWindowTitle(tr("Add Fortune"));

	if (dlg.exec() == QDialog::Accepted) {
		if (!doc)
			doc = new OmiDoc();
		QString text = dlg.textValue();
		ui.fortuneList->addItem(text);
		doc->addFortune(text);
		setWindowModified(true);
	}
}

void MainWindow::deleteFortune()
{
	if (doc) {
		int idx = ui.fortuneList->currentRow();
		doc->removeFortuneAt(idx);
	}
	delete ui.fortuneList->currentItem();
	setWindowModified(true);
}

void MainWindow::editFortune()
{
	if (!ui.fortuneList->currentItem())
		return;

	QString entry = ui.fortuneList->currentItem()->text();
	EditDialog dlg(this);
	dlg.setWindowTitle(tr("Edit Fortune"));
	dlg.setTextValue(entry);

	if (dlg.exec() == QDialog::Accepted) {
		if (doc) {
			int idx = ui.fortuneList->currentRow();
			doc->replaceFortuneAt(idx, dlg.textValue());
		}
		ui.fortuneList->currentItem()->setText(dlg.textValue());
		setWindowModified(true);
	}
}

void MainWindow::setCurrentFile(const QString& filename)
{
	currentFilename = filename;
	setWindowModified(false);
	QString shownName = tr("Untitled");
	if (!currentFilename.isEmpty())
		shownName = QFileInfo(currentFilename).fileName();
	setWindowTitle(tr("%1[*] - omiquji").arg(shownName));
}

bool MainWindow::okToContinue()
{
	if (isWindowModified()) {
		int r = QMessageBox::warning(this, "omiquji",
			tr("The file has changed.\nDo you want to save your changes?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if (r == QMessageBox::Yes)
			return save();
		else if (r == QMessageBox::Cancel)
			return false;
	}
	return true;
}

bool MainWindow::checkDocForSave()
{
	if (doc)
		return true;
	else {
		QMessageBox::warning(this, "omiquji",
							 tr("There is no doument to save."),
							 QMessageBox::Ok);
		return false;
	}
}

bool MainWindow::saveFile(const QString& filename)
{
	QFile file(filename);
	bool success = file.open(QIODevice::WriteOnly | QIODevice::Truncate);
	if (success) {
		QDataStream out(&file);
		if (!doc->writeToStream(out))
			success = false;
		else
			setCurrentFile(filename);
		file.close();
	}
	return success;
}

bool MainWindow::loadFile(const QString& filename)
{
	QFile file(filename);
	bool success = file.open(QIODevice::ReadOnly);
	if (success) {
		char *buf = 0;
		qint64 size = file.size();
		if (size) {
			buf = new char[size];
			if (buf) {
				QDataStream in(&file);
				if (in.readRawData(buf, size)) {
					doc = OmiDoc::newFromRawData(buf, size);
					if (doc) {
						int count = doc->commentCount();
						int i;
						for (i = 0; i < count; i++)
							ui.commentList->addItem(doc->commentAt(i));
						count = doc->fortuneCount();
						for (i = 0; i < count; i++)
							ui.fortuneList->addItem(doc->fortuneAt(i));
						setCurrentFile(filename);
					}
					else
						success = false;
				} else {
					success = false;
				}
				delete buf;
			} else {
				success = false;
			}
		} else {
			success = false;
		}
		file.close();
	}
	return success;
}

void MainWindow::newFile()
{
	MainWindow *win = new MainWindow();
	win->show();
}

void MainWindow::open()
{
	MainWindow *win;
	if (doc)
		win = new MainWindow();
	else
		win = this;
	QString filename =
		QFileDialog::getOpenFileName(this, tr("Open Omifile"), ".",
									 tr("Omifiles (*.omi)"));
	if (!filename.isEmpty())
		win->loadFile(filename);
	if (win != this) {
		win->show();
		win->raise();
	}
}

bool MainWindow::save()
{
	if (currentFilename.isEmpty())
		return saveAs();
	else if (checkDocForSave())
		return saveFile(currentFilename);

	return false;
}

bool MainWindow::saveAs()
{
	if (checkDocForSave()) {
		QString filename =
			QFileDialog::getSaveFileName(this, tr("Save Omifile"), ".",
										 tr("Omifiles (*.omi)"));
		if (!filename.isEmpty())
			return saveFile(filename);
	}
	return false;
}

void MainWindow::about()
{
	AboutDialog *dlg = new AboutDialog(this);
	dlg->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (okToContinue())
		event->accept();
	else
		event->ignore();
}
