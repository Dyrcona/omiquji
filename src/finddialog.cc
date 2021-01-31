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
#include "finddialog.hh"
#include "ui_finddialog.h"
#include <QCloseEvent>

FindDialog::FindDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FindDialog),
  findOpts(new FindOptions)
{
  ui->setupUi(this);
  ui->extensionWidget->setVisible(false);
}

FindDialog::~FindDialog()
{
  delete findOpts;
  delete ui;
}

void FindDialog::findClicked()
{
  if (ui->lineEdit->text().length() == 0) {
    ui->lineEdit->setFocus(Qt::OtherFocusReason);
    QApplication::beep();
    // TODO: "Flash" the widget background so it is obvious what is going on.
  } else {
    findOpts->searchTerm = ui->lineEdit->text();
    findOpts->fromStart = ui->fromStartCheckBox->isChecked();
    findOpts->matchCase = ui->caseCheckBox->isChecked();
    findOpts->matchWholeWords = ui->wholeCheckBox->isChecked();
    findOpts->isRegexp = ui->regexCheckBox->isChecked();
    findOpts->reverse = ui->backwardsCheckBox->isChecked();
    emit findNext(findOpts);
  }
}

void FindDialog::closeEvent(QCloseEvent *event)
{
  disconnect(this, SIGNAL(findNext(FindOptions*)), this->parentWidget(), nullptr);
  event->accept();
}
