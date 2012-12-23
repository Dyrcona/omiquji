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

int MainWindow::maxRecentFiles = 0;
QSettings *MainWindow::settings = 0;
QStringList MainWindow::recentFiles;

MainWindow::MainWindow(bool shouldUpdateActions) : QMainWindow()
{
	ui.setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);

	readSettings();

	recentFileMenu = 0;

	setCurrentFile("");

	doc = 0;

	disconnectEditMenu();

	if (shouldUpdateActions)
		updateRecentFileActions();

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
	EditDialog dlg(this);
	connectEditMenu(&dlg);
	dlg.setWindowTitle(tr("Add Comment"));

	if (dlg.exec() == QDialog::Accepted) {
		if (!doc)
			doc = new OmiDoc();
		QString text = dlg.textValue();
		ui.commentList->addItem(text);
		doc->addComment(text);
		setWindowModified(true);
	}
	disconnectEditMenu(&dlg);
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
	connectEditMenu(&dlg);
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
	disconnectEditMenu(&dlg);
}

void MainWindow::addFortune()
{
	EditDialog dlg(this);
	connectEditMenu(&dlg);
	dlg.setWindowTitle(tr("Add Fortune"));

	if (dlg.exec() == QDialog::Accepted) {
		if (!doc)
			doc = new OmiDoc();
		QString text = dlg.textValue();
		ui.fortuneList->addItem(text);
		doc->addFortune(text);
		setWindowModified(true);
	}
	disconnectEditMenu(&dlg);
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
	connectEditMenu(&dlg);
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
	disconnectEditMenu(&dlg);
}

void MainWindow::setCurrentFile(const QString& filename)
{
	currentFilename = filename;
	setWindowModified(false);
	QString shownName = tr("Untitled");
	if (!currentFilename.isEmpty()) {
		shownName = QFileInfo(currentFilename).fileName();
		MainWindow::addRecentFile(filename);
	}
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
	MainWindow *win = new MainWindow(true);
	win->show();
}

void MainWindow::open()
{
	QString filename =
		QFileDialog::getOpenFileName(this, tr("Open Omifile"), ".",
			tr("Omifiles (*.omi)"));
	if (!filename.isEmpty())
		this->openFile(filename);
}

void MainWindow::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		this->openFile(action->data().toString());
}

void MainWindow::openFile(const QString& filename)
{
	MainWindow *win;
	if (doc)
		win = new MainWindow();
	else
		win = this;
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
		if (!filename.isEmpty()) {
			bool result = saveFile(filename);
			if (result)
				MainWindow::addRecentFile(filename);
			return result;
		}
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
	if (okToContinue()) {
		this->writeSettings();
		event->accept();
	}
	else
		event->ignore();
}

void MainWindow::connectEditMenu(EditDialog* dialog)
{
	ui.actionCut->setEnabled(true);
	ui.actionCopy->setEnabled(true);
	ui.actionPaste->setEnabled(true);
	dialog->connectCutAction(ui.actionCut);
	dialog->connectCopyAction(ui.actionCopy);
	dialog->connectPasteAction(ui.actionPaste);
}

void MainWindow::disconnectEditMenu(EditDialog* dialog)
{
	if (dialog) {
		dialog->disconnectCutAction(ui.actionCut);
		dialog->disconnectCopyAction(ui.actionCopy);
		dialog->disconnectPasteAction(ui.actionPaste);
	}
	ui.actionCut->setEnabled(false);
	ui.actionCopy->setEnabled(false);
	ui.actionPaste->setEnabled(false);
}

void MainWindow::readSettings()
{
	if (!MainWindow::settings) {
		MainWindow::settings = new QSettings();
		MainWindow::maxRecentFiles = MainWindow::settings->value("maxRecentFiles", QVariant(10)).toInt();
		MainWindow::recentFiles = MainWindow::settings->value("recentFiles").toStringList();
		MainWindow::cleanupRecentFiles();
	}
	this->restoreGeometry(MainWindow::settings->value("geometry").toByteArray());
}

void MainWindow::writeSettings()
{
	if (MainWindow::settings) {
		MainWindow::settings->setValue("geometry", this->saveGeometry());
		MainWindow::settings->setValue("maxRecentFiles", MainWindow::maxRecentFiles);
		MainWindow::settings->setValue("recentFiles", MainWindow::recentFiles);
	}
}

void MainWindow::updateRecentFileActions()
{
	// A placeholder that we'll use again and again.
	QAction *action;

	// Clear the recentFileActions list:
	while (!this->recentFileActions.isEmpty()) {
		 action = this->recentFileActions.takeFirst();
		 if (this->recentFileMenu)
			 this->recentFileMenu->removeAction(action);
		 delete action;
	}

	if (!(this->recentFileMenu)) {
		this->recentFileMenu = new QMenu();
		ui.actionOpenRecentFiles->setMenu(this->recentFileMenu);
	}
	
	foreach (QString filename, MainWindow::recentFiles) {
		action = new QAction(filename, this->recentFileMenu);
		action->setData(QVariant(filename));
		connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
		this->recentFileActions.append(action);
		this->recentFileMenu->addAction(action);
	}

	if (this->recentFileActions.isEmpty())
		ui.actionOpenRecentFiles->setDisabled(true);
	else
		ui.actionOpenRecentFiles->setDisabled(false);
}

void MainWindow::addRecentFile(const QString& filename)
{
	if (!MainWindow::recentFiles.isEmpty()) {
		if (MainWindow::recentFiles.contains(filename))
			MainWindow::recentFiles.removeAll(filename);
		while (MainWindow::recentFiles.size() >= MainWindow::maxRecentFiles)
			MainWindow::recentFiles.removeLast();
		MainWindow::cleanupRecentFiles();
	}
	MainWindow::recentFiles.prepend(filename);
	foreach (QWidget *widget, QApplication::topLevelWidgets()) {
		if (MainWindow *win = qobject_cast<MainWindow *>(widget))
			win->updateRecentFileActions();
	}
}

void MainWindow::cleanupRecentFiles()
{
	QMutableStringListIterator i(MainWindow::recentFiles);
	while (i.hasNext())
		if (!QFile::exists(i.next())) i.remove();
}
