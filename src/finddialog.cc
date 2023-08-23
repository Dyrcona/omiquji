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
  if (ui->searchTextComboBox->currentText().length() == 0) {
    ui->searchTextComboBox->setFocus(Qt::OtherFocusReason);
    // TODO: Add code so the "beep" works on GNU/Linux with Gnome.
    QApplication::beep();
    // TODO: "Flash" the widget background so it is obvious what is going on.
  } else {
    options->searchText = ui->searchTextComboBox->currentText();
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

void FindDialog::addSearchTextItem(const QString &text)
{
  QComboBox *combo = ui->searchTextComboBox;
  bool add = true;
  int items = combo->count();
  if (items > 0) {
    for (int i = 0; i < items; i++) {
      if (combo->itemText(i) == text) {
        add = false;
        break;
      }
    }
  }
  if (add)
    combo->addItem(text);
}

void FindDialog::comboBoxCurrentTextChanged(const QString &text) {
  emit searchTextComboBoxTextChanged(text);
}
