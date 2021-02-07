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
qint64 writeStringListToStrfileStream(QDataStream &stream, QStringList *list,
                                      const char *separator, bool &wantSeparator);

OmiDoc::~OmiDoc() {
  delete commentList;
  delete fortuneList;
}

void OmiDoc::addComment(QString &comment) {
  commentList->append(comment);
}

void OmiDoc::addFortune(QString &fortune) {
  fortuneList->append(fortune);
}

void OmiDoc::removeCommentAt(int i) {
  if (i < commentList->count()) {
    QString current = commentList->at(i);
    commentList->removeAt(i);
  }
}

void OmiDoc::removeFortuneAt(int i) {
  if (i < fortuneList->count()) {
    QString current = fortuneList->at(i);
    fortuneList->removeAt(i);
  }
}

void OmiDoc::replaceCommentAt(int i, QString& text) {
  if (i < commentList->count()) {
    QString current = commentList->at(i);
    if (current != text) {
      commentList->replace(i, text);
    }
  }
}

void OmiDoc::replaceFortuneAt(int i, QString& text) {
  if (i < fortuneList->count()) {
    QString current = fortuneList->at(i);
    if (current != text) {
      fortuneList->replace(i, text);
    }
  }
}

void OmiDoc::insertComment(int i, QString &text) {
  commentList->insert(i, text);
}

void OmiDoc::insertFortune(int i, QString &text) {
  fortuneList->insert(i, text);
}

int OmiDoc::commentCount() {
  return commentList->count();
}

int OmiDoc::fortuneCount() {
  return fortuneList->count();
}

const QString& OmiDoc::commentAt(int i) {
  return commentList->at(i);
}

const QString& OmiDoc::fortuneAt(int i) {
  return fortuneList->at(i);
}

qint64 OmiDoc::writeToFile(QFile &output) {
  bool wantClose = false;
  qint64 bytesOut = 0;

  if (!output.isOpen()) {
    if (output.open(QIODevice::WriteOnly | QIODevice::Truncate))
      wantClose = true;
    else
      return -1;
  }
  QDataStream out(&output);
  if (output.fileName().endsWith(".omi")) {
    bytesOut = this->writeOmifileToStream(out);
  } else {
    bytesOut = this->writeStrfileToStream(out);
  }

  if (wantClose) output.close();
  return bytesOut;
}

qint64 OmiDoc::writeOmifileToStream(QDataStream& stream) {
  qint64 bytesOut = 0;

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

qint64 OmiDoc::writeStrfileToStream(QDataStream &stream) {
  qint64 bytesOut = 0;
  bool wantSeparator = false;
  const char *separator = "%\n";

  if (commentList->count())
    bytesOut += writeStringListToStrfileStream(stream, commentList, separator, wantSeparator);
  if (fortuneList->count())
    bytesOut += writeStringListToStrfileStream(stream, fortuneList, separator, wantSeparator);

  return bytesOut;
}

qint64 OmiDoc::readFromFile(QFile &input) {
  bool wantClose = false;
  qint64 bytesRead = 0;
  if (input.isReadable()) {
    if (!input.isOpen()) {
      if (input.open(QIODevice::ReadOnly))
        wantClose = true;
      else
        return -1;
    }
    if (input.fileName().endsWith(".omi")) {
      bytesRead = readFromOmifile(input);
    } else {
      bytesRead = readFromStrfile(input);
    }
    if (bytesRead > 0) {
      emit commentsAdded(*commentList);
      emit fortunesAdded(*fortuneList);
    }
  }
  if (wantClose) input.close();
  return bytesRead;
}

qint64 OmiDoc::readFromOmifile(QFile &file) {
  qint64 len = file.size();
  char *data = new char[len];
  if (!data) return -1;
  len = file.read(data, len);
  if (len < file.size()) {
    delete data;
    return -1;
  }

  qint64 bytesRead = 0;
  if (static_cast<unsigned long>(len) >= sizeof(OmikujiHeader)) {
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

      // Check for and copy comments.
      if (header.commentHeader.offset && header.commentHeader.length) {
        quint32 offset = header.commentHeader.offset;
        TableEntry entry;
        for (quint32 i = 0; i < header.commentHeader.length; i++) {
          if (offset >= sizeof(OmikujiHeader) &&
              offset + sizeof(TableEntry) <= static_cast<unsigned long>(len)) {
            copyTableEntry(&entry, data, offset);
            if (entry.offset >= sizeof(OmikujiHeader)
                && entry.offset + entry.length <= static_cast<unsigned long>(len)) {
              QString str = QString::fromUtf8((data + entry.offset),
                                              entry.length);
              this->addComment(str);
              bytesRead += entry.length;
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
              && offset + sizeof(TableEntry) <= static_cast<unsigned long>(len)) {
            copyTableEntry(&entry, data, offset);
            if (entry.offset >= sizeof(OmikujiHeader)
                && entry.offset + entry.length <= len) {
              QString str = QString::fromUtf8((data + entry.offset),
                                              entry.length);
              this->addFortune(str);
              bytesRead += entry.length;
            }
          }
          offset += sizeof(TableEntry);
        }
      }

    }
  }

  if (data) delete data;

  return bytesRead;
}

qint64 OmiDoc::readFromStrfile(QFile &file) {
  qint64 size = file.size();
  if (size > 0) {
    char *data = new char[size + 1];
    if (!data) return -1;
    qint64 length = file.read(data, size);
    if (length < size) {
      delete data;
      return -1;
    }
    else {
      data[length] = 0;
      char *start = data;
      while (true) {
        char *next = std::strstr(start, "\n%\n");
        if (next) {
          length = next - start + 1;
          if (length > 1) {
            QString str = QString::fromUtf8(start, length);
            this->addFortune(str);
          }
          start = next + 3;
          if (start >= (data + size)) break;
        } else {
          length = std::strlen(start);
          if (length > 0) {
            QString str = QString::fromUtf8(start, length);
            this->addFortune(str);
          }
          break;
        }
      }
      delete data;
    }
  }
  return size;
}

bool checkOmikujiHeader(const OmikujiHeader header) {
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

TableEntry *copyTableEntry(TableEntry *entry, char *data, quint32 offset) {
  std::memcpy(entry, (data + offset), sizeof(TableEntry));
  entry->offset = qFromBigEndian(entry->offset);
  entry->length = qFromBigEndian(entry->length);
  return entry;
}

qint64 writeStringListToStrfileStream(QDataStream &stream, QStringList *list,
                                      const char *separator, bool &wantSeparator) {
  qint64 bytesOut = 0;

  quint32 entries = list->count();
  if (entries) {
    for (quint32 i = 0; i < entries; i++) {
      QString string = list->at(i);
      QByteArray entry = string.toUtf8();
      if (entry.size() > 0) {
        if (wantSeparator)
          bytesOut += stream.writeRawData(separator, std::strlen(separator));
        bytesOut += stream.writeRawData(entry.data(), entry.size());
        if (!string.endsWith("\n"))
          bytesOut += stream.writeRawData("\n", 1);
        wantSeparator = true;
      }
    }
  }
  return bytesOut;
}
