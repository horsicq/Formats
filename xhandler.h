/* Copyright (c) 2026-2026 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef XHANDLER_H
#define XHANDLER_H

#include "xbinary.h"

class XHandler : public QObject {
    Q_OBJECT

public:
    enum XHANDLER {
        XHANDLER_UNKNOWN = 0,
        XHANDLER_REMOVE,
        XHANDLER_COPY,
        XHANDLER_MOVE,
    };

    enum RECORD_OPTION {
        RECORD_OPTION_UNKNOWN = 0,
        RECORD_OPTION_FILENAME_SOURCE,
        RECORD_OPTION_FILENAME_DEST
    };

    struct RECORD {
        XHANDLER xhandler;
        QMap<RECORD_OPTION, QVariant> mapOptions;
    };

    explicit XHandler(QObject *pParent = nullptr);

    static void addRecord_Remove(QList<RECORD> *pListRecords, const QString &sFilename);
    static void addRecord_Copy(QList<RECORD> *pListRecords, const QString &sFilenameSource, const QString &sFilenameDest);
    static void addRecord_Move(QList<RECORD> *pListRecords, const QString &sFilenameSource, const QString &sFilenameDest);

    void processRecords(QList<RECORD> *pListRecords, XBinary::PDSTRUCT *pDStruct = nullptr);

signals:
    void errorMessage(const QString &sText);

};

#endif  // XHANDLER_H
