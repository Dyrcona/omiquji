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
#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QtGlobal>
#include <QtWidgets>

#include "ui_mainwindow.h"
#include "omidoc.hh"
#include "finddialog.hh"
class EditDialog;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(bool shouldUpdateActions = false, QWidget *parent = 0);

signals:
  void commentAdded(QString&);
  void commentRemovedAt(int);
  void commentReplacedAt(int, QString&);
  void fortuneAdded(QString&);
  void fortuneRemovedAt(int);
  void fortuneReplacedAt(int, QString&);
  
protected:
  void closeEvent(QCloseEvent*);

private slots:
  void addComment();
  void editComment();
  void deleteComment();
  void addFortune();
  void editFortune();
  void deleteFortune();
  void addComments(const QStringList&);
  void addFortunes(const QStringList&);
  // Action methods:
  void newFile();
  void open();
  bool save();
  bool saveAs();
  void about();
  void openRecentFile();
  void clearRecentFiles();
  void searchComments();
  void searchFortunes();
  void findNextInComments(FindDialog::Options*);
  void findNextInFortunes(FindDialog::Options*);
  void toggleNewSearch(int);

private:
  void addComment(QString&);
  void removeCommentAt(int);
  void replaceCommentAt(int, QString&);
  void addFortune(QString&);
  void removeFortuneAt(int);
  void replaceFortuneAt(int, QString&);
  bool okToContinue();
  bool checkDocForSave();
  bool loadFile(const QString&);
  bool saveFile(const QString&);
  void setCurrentFile(const QString&);
  void connectEditMenu(EditDialog*);
  void disconnectEditMenu(EditDialog *dialog=0);
  void readSettings();
  void writeSettings();
  void openFile(const QString&);
  void updateRecentFileActions();
  void createStatusBar();
  void updateStatusBar();
  bool setupSearch(QListWidget*);
  void findNext(QListWidget*, FindDialog::Options*);
  void setupOmiDoc();

  Ui::MainWindow ui;
  OmiDoc *doc;
  QString currentFilename;
  QMenu *recentFileMenu;
  QList<QAction *> recentFileActions;
  QAction *separatorAction;
  QAction *clearRecentFilesAction;
  QLabel *commentsLabel;
  QLabel *fortunesLabel;
  QLabel *commentCounter;
  QLabel *fortuneCounter;
  FindDialog *findDialog;
  bool isNewSearch;
  int searchIndex;

  static int maxRecentFiles;
  static QSettings *settings;
  static QStringList recentFiles;
  static void addRecentFile(const QString&);
  static void cleanupRecentFiles();
  static void updateMainWindows();
}; 

#endif
