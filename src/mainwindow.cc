/*
 * Copyright © 2012, 2021 Jason J.A. Stephenson <jason@sigio.com>
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

MainWindow::MainWindow(bool shouldUpdateActions, QWidget *parent) : QMainWindow(parent)
{
  ui.setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  readSettings();

  recentFileMenu = 0;

  separatorAction = new QAction(this);
  separatorAction->setSeparator(true);
  clearRecentFilesAction = new QAction(tr("Clear Recent Files"), this);
  connect(clearRecentFilesAction, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));

  setCurrentFile("");

  doc = 0;

  findDialog = nullptr;
  isNewSearch = true;
  searchIndex = 0;

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
  connect(ui.actionSearch_in_Comments, SIGNAL(triggered()), this, SLOT(searchComments()));
  connect(ui.actionSearch_in_Fortunes, SIGNAL(triggered()), this, SLOT(searchFortunes()));

  // Action buttons.
  connect(ui.addCommentButton, SIGNAL(clicked()), this, SLOT(addComment()));
  connect(ui.editCommentButton, SIGNAL(clicked()), this, SLOT(editComment()));
  connect(ui.deleteCommentButton, SIGNAL(clicked()), this, SLOT(deleteComment()));
  connect(ui.addFortuneButton, SIGNAL(clicked()), this, SLOT(addFortune()));
  connect(ui.editFortuneButton, SIGNAL(clicked()), this, SLOT(editFortune()));
  connect(ui.deleteFortuneButton, SIGNAL(clicked()), this, SLOT(deleteFortune()));
  connect(ui.searchCommentButton, SIGNAL(clicked()), this, SLOT(searchComments()));
  connect(ui.searchFortuneButton, SIGNAL(clicked()), this, SLOT(searchFortunes()));

  // Wire QListWidgets up for double click to edit.
  connect(ui.commentList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(editComment()));
  connect(ui.fortuneList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(editFortune()));

  createStatusBar();
}

void MainWindow::addComments(const QStringList &list) {
  ui.commentList->addItems(list);
}

void MainWindow::addFortunes(const QStringList &list) {
  ui.fortuneList->addItems(list);
}

void MainWindow::addComment(QString& text) {
  if (ui.commentList->currentItem()) {
    int index = ui.commentList->currentRow() + 1;
    ui.commentList->insertItem(index, text);
    ui.commentList->setCurrentRow(index, QItemSelectionModel::SelectCurrent);
    emit commentInserted(index, text);
  } else {
    ui.commentList->addItem(text);
    ui.commentList->setCurrentRow(ui.commentList->count() - 1, QItemSelectionModel::SelectCurrent);
    emit commentAdded(text);
  }
}

void MainWindow::addFortune(QString& text) {
  if (ui.fortuneList->currentItem()) {
    int index = ui.fortuneList->currentRow() + 1;
    ui.fortuneList->insertItem(index, text);
    ui.fortuneList->setCurrentRow(index, QItemSelectionModel::SelectCurrent);
    emit fortuneInserted(index, text);
  } else {
    ui.fortuneList->addItem(text);
    ui.fortuneList->setCurrentRow(ui.fortuneList->count() - 1, QItemSelectionModel::SelectCurrent);
    emit fortuneAdded(text);
  }
}

void MainWindow::replaceCommentAt(int index, QString& text) {
  if (index < ui.commentList->count()) {
    QString current = ui.commentList->item(index)->text();
    if (text != current) {
      ui.commentList->item(index)->setText(text);
      emit commentReplacedAt(index, text);
    }
  }
}

void MainWindow::replaceFortuneAt(int index, QString& text) {
  if (index < ui.fortuneList->count()) {
    QString current = ui.fortuneList->item(index)->text();
    if (text != current) {
      ui.fortuneList->item(index)->setText(text);
      emit fortuneReplacedAt(index, text);
    }
  }
}

void MainWindow::removeCommentAt(int index) {
  if (index < ui.commentList->count()) {
    delete ui.commentList->item(index);
    emit commentRemovedAt(index);
  }
}

void MainWindow::removeFortuneAt(int index) {
  if (index < ui.fortuneList->count()) {
    delete ui.fortuneList->item(index);
    emit fortuneRemovedAt(index);
  }
}

void MainWindow::addComment() {
  EditDialog dlg(this);
  connectEditMenu(&dlg);
  dlg.setWindowTitle(tr("Add Comment"));

  if (dlg.exec() == QDialog::Accepted) {
    if (!doc) setupOmiDoc();
    QString text = dlg.textValue();
    addComment(text);
    setWindowModified(true);
    updateStatusBar();
  }
  disconnectEditMenu(&dlg);
}

void MainWindow::deleteComment() {
  if (ui.commentList->currentItem()) {
    int index = ui.commentList->currentRow();
    removeCommentAt(index);
    setWindowModified(true);
    updateStatusBar();
  }
}

void MainWindow::editComment() {
  if (!ui.commentList->currentItem())
    return;

  QString entry = ui.commentList->currentItem()->text();
  EditDialog dlg(this);
  connectEditMenu(&dlg);
  dlg.setWindowTitle(tr("Edit Comment"));
  dlg.setTextValue(entry);

  if (dlg.exec() == QDialog::Accepted) {
    entry = dlg.textValue();
    replaceCommentAt(ui.commentList->currentRow(), entry);
    setWindowModified(true);
  }
  disconnectEditMenu(&dlg);
}

void MainWindow::addFortune() {
  EditDialog dlg(this);
  connectEditMenu(&dlg);
  dlg.setWindowTitle(tr("Add Fortune"));

  if (dlg.exec() == QDialog::Accepted) {
    if (!doc) setupOmiDoc();
    QString text = dlg.textValue();
    addFortune(text);
    setWindowModified(true);
    updateStatusBar();
  }
  disconnectEditMenu(&dlg);
}

void MainWindow::deleteFortune() {
  if (ui.fortuneList->currentItem()) {
    int index = ui.fortuneList->currentRow();
    removeFortuneAt(index);
    setWindowModified(true);
    updateStatusBar();
  }
}

void MainWindow::editFortune() {
  if (!ui.fortuneList->currentItem())
    return;

  QString entry = ui.fortuneList->currentItem()->text();
  EditDialog dlg(this);
  connectEditMenu(&dlg);
  dlg.setWindowTitle(tr("Edit Fortune"));
  dlg.setTextValue(entry);

  if (dlg.exec() == QDialog::Accepted) {
    entry = dlg.textValue();
    replaceFortuneAt(ui.fortuneList->currentRow(), entry);
    setWindowModified(true);
  }
  disconnectEditMenu(&dlg);
}

void MainWindow::searchComments() {
  if (setupSearch(ui.commentList)) {
    connect(findDialog, &FindDialog::findNext, this, &MainWindow::findNextInComments);
  } else {
    QMessageBox::warning(this, "omiquji", tr("There are no comments to search."),
                         QMessageBox::Cancel);
  }
}

void MainWindow::searchFortunes() {
  if (setupSearch(ui.fortuneList)) {
    connect(findDialog, &FindDialog::findNext, this, &MainWindow::findNextInFortunes);
  } else {
    QMessageBox::warning(this, "omiquji", tr("There are no fortunes to search."),
                         QMessageBox::Cancel);
  }
}

void MainWindow::findNextInComments(FindDialog::Options *findOpts) {
  findNext(ui.commentList, findOpts);
}

void MainWindow::findNextInFortunes(FindDialog::Options *findOpts) {
  findNext(ui.fortuneList, findOpts);
}

bool MainWindow::setupSearch(QListWidget* target) {
  if (target->count() > 0) {
    if (!findDialog) {
      findDialog = new FindDialog(this);
      connect(findDialog, &FindDialog::fromStartCheckBoxStateChanged, this,
              [=](int state){if (state == Qt::Checked) this->isNewSearch = true;});
      connect(findDialog, &FindDialog::searchTextComboBoxTextChanged, this, [=](){this->isNewSearch = true;});
      connect(this, &MainWindow::searchTextFound, findDialog, &FindDialog::addSearchTextItem);
    } else {
      disconnect(findDialog, &FindDialog::findNext, this, nullptr);
    }
    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
    isNewSearch = true;
    return true;
  }
  return false;
}

void MainWindow::findNext(QListWidget* target, FindDialog::Options *findOpts) {
  int step = (findOpts->searchBackwards) ? -1 : 1;
  int bound = (findOpts->searchBackwards) ? -1 : target->count();
  bool found = false;
  QRegularExpression re; // In case we need it.

  if (isNewSearch) {
    isNewSearch = false;
    if (findOpts->fromStart)
      searchIndex = (findOpts->searchBackwards) ? target->count() - 1 : 0;
    else
      searchIndex = (target->currentItem()) ? target->currentRow() : 0;
  } else {
    searchIndex += step;
    if (searchIndex < 0 || searchIndex >= target->count()) {
      // Start over
      if (findOpts->searchBackwards)
        searchIndex = target->count() - 1;
      else
        searchIndex = 0;
    }
  }

  if (findOpts->matchWholeWords)
    re.setPattern("\\W" + findOpts->searchText + "\\W");
  else if (findOpts->isRegexp)
    re.setPattern(findOpts->searchText);

  if (findOpts->matchWholeWords || findOpts->isRegexp) {
    if (!findOpts->matchCase)
      re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
  }

  while (searchIndex != bound && !found) {
    QString entry = target->item(searchIndex)->text();
    if (findOpts->matchWholeWords || findOpts->isRegexp)
      found = entry.contains(re);
    else
      found = entry.contains(findOpts->searchText, (findOpts->matchCase) ? Qt::CaseSensitive : Qt::CaseInsensitive);
    if (found) {
      target->setCurrentRow(searchIndex, QItemSelectionModel::SelectCurrent);
      emit searchTextFound(findOpts->searchText);
    }
    else
      searchIndex += step;
  }
  if (searchIndex == bound)
    QMessageBox::information(this, tr("Not Found"), tr("Search key not found."));
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
    success = (doc->writeToFile(file) > 0) ? true : false;
    if (success)
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
    setupOmiDoc();
    if (doc) {
      success = (doc->readFromFile(file) > 0) ? true: false;
      if (success) {
        setCurrentFile(filename);
        this->updateStatusBar();
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
      tr("Omifiles (*.omi);;Strfile (*)"));
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
        tr("Omifile (*.omi);;Strfile (*)"));
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
  else {
    this->recentFileMenu->removeAction(this->separatorAction);
    this->recentFileMenu->removeAction(this->clearRecentFilesAction);
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
  else {
    this->recentFileMenu->addAction(this->separatorAction);
    this->recentFileMenu->addAction(this->clearRecentFilesAction);
    ui.actionOpenRecentFiles->setDisabled(false);
  }
}

void MainWindow::clearRecentFiles()
{
  if (!MainWindow::recentFiles.isEmpty()) {
    MainWindow::recentFiles.clear();
    MainWindow::updateMainWindows();
  }
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
  MainWindow::updateMainWindows();
}

void MainWindow::cleanupRecentFiles()
{
  QMutableStringListIterator i(MainWindow::recentFiles);
  while (i.hasNext())
    if (!QFile::exists(i.next())) i.remove();
}

void MainWindow::updateMainWindows()
{
  foreach (QWidget *widget, QApplication::topLevelWidgets()) {
    if (MainWindow *win = qobject_cast<MainWindow *>(widget))
      win->updateRecentFileActions();
  }
}

void MainWindow::createStatusBar()
{
  commentsLabel = new QLabel(tr("Comments"));
  commentsLabel->setMinimumSize(commentsLabel->sizeHint());
  fortunesLabel = new QLabel(tr("Fortunes"));
  fortunesLabel->setMinimumSize(fortunesLabel->sizeHint());
  commentCounter = new QLabel("0");
  commentCounter->setLineWidth(2);
  commentCounter->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  fortuneCounter = new QLabel("0");
  fortuneCounter->setLineWidth(2);
  fortuneCounter->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  this->statusBar()->addWidget(commentsLabel);
  this->statusBar()->addWidget(commentCounter);
  this->statusBar()->addWidget(fortunesLabel);
  this->statusBar()->addWidget(fortuneCounter);
}

void MainWindow::updateStatusBar()
{
  commentCounter->setText(QString::number(ui.commentList->count()));
  fortuneCounter->setText(QString::number(ui.fortuneList->count()));
}

void MainWindow::setupOmiDoc() {
  if (!doc) {
    doc = new OmiDoc(this);
    // Connect our signals to OmiDoc's slots:
    connect(this, SIGNAL(commentAdded(QString&)), doc, SLOT(addComment(QString&)));
    connect(this, SIGNAL(commentRemovedAt(int)), doc, SLOT(removeCommentAt(int)));
    connect(this, SIGNAL(commentReplacedAt(int,QString&)), doc, SLOT(replaceCommentAt(int,QString&)));
    connect(this, SIGNAL(fortuneAdded(QString&)), doc, SLOT(addFortune(QString&)));
    connect(this, SIGNAL(fortuneRemovedAt(int)), doc, SLOT(removeFortuneAt(int)));
    connect(this, SIGNAL(fortuneReplacedAt(int,QString&)), doc, SLOT(replaceFortuneAt(int,QString&)));
    connect(this, SIGNAL(commentInserted(int,QString&)), doc, SLOT(insertComment(int,QString&)));
    connect(this, SIGNAL(fortuneInserted(int,QString&)), doc, SLOT(insertFortune(int,QString&)));
    // Connect OmiDoc's signals to our slots
    connect(doc, SIGNAL(commentsAdded(const QStringList&)), this, SLOT(addComments(const QStringList&)));
    connect(doc, SIGNAL(fortunesAdded(const QStringList&)), this, SLOT(addFortunes(const QStringList&)));
  }
}
