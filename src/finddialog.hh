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

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FindDialog(QWidget *parent = nullptr);
  ~FindDialog();

private:
  Ui::FindDialog *ui;
};

#endif // FINDDIALOG_HH
