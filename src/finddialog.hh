/*
 * Copyright © 2021 Jason J.A. Stephenson <jason@sigio.com>
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
#ifndef FINDDIALOG_HH
#define FINDDIALOG_HH

#include <QDialog>
#include <QString>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
  Q_OBJECT

public:
  struct Options
  {
    QString searchText;
    bool fromStart;
    bool matchCase;
    bool matchWholeWords;
    bool isRegexp;
    bool searchBackwards;
  };
  explicit FindDialog(QWidget *parent = nullptr);
  ~FindDialog();

signals:
  void findNext(FindDialog::Options*);
  void matchCaseCheckBoxStateChanged(int);
  void fromStartCheckBoxStateChanged(int);
  void searchBackwardsCheckBoxStateChanged(int);
  void regularExpressionCheckBoxStateChanged(int);
  void wholeWordsCheckBoxStateChanged(int);

private slots:
  void findClicked();
  void checkBoxStateChanged(int);

private:
  Ui::FindDialog *ui;
  FindDialog::Options *options;
  void addSearchTextItem(const QString&);
};

#endif // FINDDIALOG_HH
