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
#ifndef OMIDOC_HH
#define OMIDOC_HH

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDataStream>
#include <QFile>

class OmiDoc : public QObject
{
  Q_OBJECT

public:
  OmiDoc(QObject *parent = nullptr)
    : QObject(parent), commentList(new QStringList()),
      fortuneList(new QStringList()) {}
  ~OmiDoc();
  const QString& commentAt(int);
  const QString& fortuneAt(int);
  int commentCount();
  int fortuneCount();

public slots:
  void addComment(QString&);
  void removeCommentAt(int);
  void replaceCommentAt(int, QString&);
  void addFortune(QString&);
  void removeFortuneAt(int);
  void replaceFortuneAt(int, QString&);
  qint64 writeToFile(QFile&);
  qint64 readFromFile(QFile&);

signals:
  void commentAdded(int, QString&);
  void commentRemovedAt(int, QString&);
  void commentReplacedAt(int, QString&);
  void fortuneAdded(int, QString&);
  void fortuneRemovedAt(int, QString&);
  void fortuneReplacedAt(int, QString&);

private:
  QStringList *commentList;
  QStringList *fortuneList;
  qint64 writeOmifileToStream(QDataStream&);
  qint64 writeStrfileToStream(QDataStream&);
  qint64 readFromOmifile(QFile&);
  qint64 readFromStrfile(QFile&);

};

#endif
