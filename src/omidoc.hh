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
#ifndef OMIDOC_HH
#define OMIDOC_HH

#include <QStringList>
#include <QDataStream>

class OmiDoc
{
public:
	OmiDoc();
	~OmiDoc();
	void addComment(const QString&);
	const QString& commentAt(int);
	void removeCommentAt(int);
	void replaceCommentAt(int, const QString&);
	void addFortune(const QString&);
	const QString& fortuneAt(int);
	void removeFortuneAt(int);
	void replaceFortuneAt(int, const QString&);
	int commentCount();
	int fortuneCount();
	const QString& commentAt(int) const;
	const QString& fortuneAt(int) const;
	int writeToStream(QDataStream&);

	static OmiDoc* newFromRawData(char*, unsigned int);

private:
	QStringList *commentList;
	QStringList *fortuneList;
};

#endif
