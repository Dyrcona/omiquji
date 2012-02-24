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
#include "omidoc.hh"
#include <QtEndian>
#include <QList>
#include <QByteArray>
#include <cstring>

struct TableEntry {
	quint32 offset;
	quint32 length;
};

struct OmikujiHeader {
	char signature[7];
	char version;
	TableEntry commentHeader;
	TableEntry fortuneHeader;
};

const char omikuji_version = 0;
const char *omikuji_signature = "omikuji";

bool checkOmikujiHeader(const OmikujiHeader header);
TableEntry *copyTableEntry(TableEntry *entry, char *data, quint32 offset);

OmiDoc::OmiDoc()
{
	commentList = new QStringList();
	fortuneList = new QStringList();
}

OmiDoc::~OmiDoc()
{
	delete commentList;
	delete fortuneList;
}

void OmiDoc::addComment(const QString &comment)
{
	commentList->append(comment);
}

void OmiDoc::addFortune(const QString &fortune)
{
	fortuneList->append(fortune);
}

void OmiDoc::removeCommentAt(int i)
{
	if (i < commentList->count())
		commentList->removeAt(i);
}

void OmiDoc::removeFortuneAt(int i)
{
	if (i < fortuneList->count())
		fortuneList->removeAt(i);
}

void OmiDoc::replaceCommentAt(int i, const QString& text)
{
	if (i < commentList->count())
		commentList->replace(i, text);
}

void OmiDoc::replaceFortuneAt(int i, const QString& text)
{
	if (i < fortuneList->count())
		fortuneList->replace(i, text);
}

int OmiDoc::commentCount()
{
	return commentList->size();
}

int OmiDoc::fortuneCount()
{
	return fortuneList->size();
}

const QString& OmiDoc::commentAt(int i)
{
	return commentList->at(i);
}

const QString& OmiDoc::fortuneAt(int i)
{
	return fortuneList->at(i);
}

int OmiDoc::writeToStream(QDataStream& stream)
{
	int bytesOut = 0;

	// Some handy variables for tracking things.
	quint32 offset = 0;
	quint32 comments = commentList->count();
	quint32 fortunes = fortuneList->count();

	// Make a header
	OmikujiHeader header;
	std::memcpy(header.signature, omikuji_signature, 7);
	header.version = omikuji_version;
	if (comments) {
		offset = sizeof(OmikujiHeader);
		header.commentHeader.offset = qToBigEndian(offset);
		header.commentHeader.length = qToBigEndian(comments);
		offset += comments * sizeof(TableEntry);
	} else {
		header.commentHeader.offset = 0;
		header.commentHeader.length = 0;
	}
	if (fortunes) {
		header.fortuneHeader.offset = qToBigEndian(offset);
		header.fortuneHeader.length = qToBigEndian(fortunes);
		offset += fortunes * sizeof(TableEntry);
	} else {
		header.fortuneHeader.offset = 0;
		header.fortuneHeader.length = 0;
	}
	// Write it to the stream.
	bytesOut += stream.writeRawData((const char*)&header,
									sizeof(OmikujiHeader));

	// List to hold the UTF-8 comment and fortune output:
	QList<QByteArray> *outputList = 0;

	// Write the comments table.
	if (comments) {
		if (!outputList)
			outputList = new QList<QByteArray>();
		TableEntry *commentTable = new TableEntry[comments];
		for (quint32 i = 0; i < comments; i++) {
			QString string = commentList->at(i);
			QByteArray entry = string.toUtf8();
			outputList->append(entry);
			commentTable[i].offset = qToBigEndian(offset);
			commentTable[i].length = qToBigEndian(entry.size());
			offset += entry.size();
		}
		bytesOut += stream.writeRawData((const char*)commentTable,
										comments * sizeof(TableEntry));
		delete commentTable;
	}

	// Write the fortunes table.
	if (fortunes) {
		if (!outputList)
			outputList = new QList<QByteArray>();
		TableEntry *fortuneTable = new TableEntry[fortunes];
		for (quint32 i = 0; i < fortunes; i++) {
			QString string = fortuneList->at(i);
			QByteArray entry = string.toUtf8();
			outputList->append(entry);
			fortuneTable[i].offset = qToBigEndian(offset);
			fortuneTable[i].length = qToBigEndian(entry.size());
			offset += entry.size();
		}
		bytesOut += stream.writeRawData((const char*)fortuneTable,
										fortunes * sizeof(TableEntry));
		delete fortuneTable;
	}

	// Write the outputList.
	if (outputList) {
		QList<QByteArray>::const_iterator i;
		for (i = outputList->constBegin(); i != outputList->constEnd(); i++)
			bytesOut += stream.writeRawData(i->data(), i->size());

		delete outputList;
	}

	return bytesOut;
}

OmiDoc* OmiDoc::newFromRawData(char* data, int len)
{
	OmiDoc *doc = 0;

	// Currently only support creation from omifiles, so check for the
	// header.
	if (len >= sizeof(OmikujiHeader)) {
		// Minimum size of an omikuji file is 24 bytes for the header.
		OmikujiHeader header;
		std::memcpy(&header, data, sizeof(OmikujiHeader));
		if (checkOmikujiHeader(header)) {
			// Fix the other header fields.
			header.commentHeader.offset =
				qFromBigEndian(header.commentHeader.offset);
			header.commentHeader.length =
				qFromBigEndian(header.commentHeader.length);
			header.fortuneHeader.offset =
				qFromBigEndian(header.fortuneHeader.offset);
			header.fortuneHeader.length =
				qFromBigEndian(header.fortuneHeader.length);

			// Create the doc:
			doc = new OmiDoc();

			// Check for and copy comments.
			if (header.commentHeader.offset && header.commentHeader.length) {
				quint32 offset = header.commentHeader.offset;
				TableEntry entry;
				for (quint32 i = 0; i < header.commentHeader.length; i++) {
					if (offset >= sizeof(OmikujiHeader) &&
						offset + sizeof(TableEntry) <= len) {
						copyTableEntry(&entry, data, offset);
						if (entry.offset >= sizeof(OmikujiHeader)
							&& entry.offset + entry.length <= len) {
							QString str =
								QString::fromUtf8((data + entry.offset),
												  entry.length);
							doc->addComment(str);
						}
					}
					offset += sizeof(TableEntry);
				}
			}

			// Ditto for the fortunes.
			if (header.fortuneHeader.offset && header.fortuneHeader.length) {
				quint32 offset = header.fortuneHeader.offset;
				TableEntry entry;
				for (quint32 i = 0; i < header.fortuneHeader.length; i++) {
					if (offset >= sizeof(OmikujiHeader)
						&& offset + sizeof(TableEntry) <= len) {
						copyTableEntry(&entry, data, offset);
						if (entry.offset >= sizeof(OmikujiHeader)
							&& entry.offset + entry.length <= len) {
							QString str =
								QString::fromUtf8((data + entry.offset),
												  entry.length);
							doc->addFortune(str);
						}
					}
					offset += sizeof(TableEntry);
				}
			}

		}
	}

	return doc;
}

bool checkOmikujiHeader(const OmikujiHeader header)
{
	bool isValid = false;

	// Only support 1 version of omikji files, since only 1 exists.
	if (header.version == omikuji_version) {
		isValid = true;
		for (int i = 0; i < 7; i++) {
			if (header.signature[i] != omikuji_signature[i]) {
				isValid = false;
				break;
			}
		}
	}

	return isValid;
}

TableEntry *copyTableEntry(TableEntry *entry, char *data, quint32 offset)
{
	std::memcpy(entry, (data + offset), sizeof(TableEntry));
	entry->offset = qFromBigEndian(entry->offset);
	entry->length = qFromBigEndian(entry->length);
	return entry;
}
