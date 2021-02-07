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

FindDialog::FindDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FindDialog),
  options(new FindDialog::Options)
{
  ui->setupUi(this);
  ui->extensionWidget->setVisible(false);
}

FindDialog::~FindDialog()
{
  delete options;
  delete ui;
}

void FindDialog::findClicked()
{
  if (ui->lineEdit->text().length() == 0) {
    ui->lineEdit->setFocus(Qt::OtherFocusReason);
    QApplication::beep();
    // TODO: "Flash" the widget background so it is obvious what is going on.
  } else {
    options->searchTerm = ui->lineEdit->text();
    options->fromStart = ui->fromStartCheckBox->isChecked();
    options->matchCase = ui->caseCheckBox->isChecked();
    options->matchWholeWords = ui->wholeCheckBox->isChecked();
    options->isRegexp = ui->regexCheckBox->isChecked();
    options->searchBackwards = ui->backwardsCheckBox->isChecked();
    emit findNext(options);
  }
}

void FindDialog::checkBoxStateChanged(int state)
{
  QCheckBox *chb = qobject_cast<QCheckBox *>(sender());
  if (chb == ui->caseCheckBox)
    emit matchCaseCheckBoxStateChanged(state);
  else if (chb == ui->fromStartCheckBox)
    emit fromStartCheckBoxStateChanged(state);
  else if (chb == ui->wholeCheckBox)
    emit wholeWordsCheckBoxStateChanged(state);
  else if (chb == ui->regexCheckBox)
    emit regularExpressionCheckBoxStateChanged(state);
  else
    emit searchBackwardsCheckBoxStateChanged(state);
}
