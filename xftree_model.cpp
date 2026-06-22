/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
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

#include "xftree_model.h"
#include "xformats.h"
#include <QDebug>

XFTreeModel::XFTreeModel(QObject *pParent) : QAbstractItemModel(pParent)
{
    m_pRootItem = nullptr;
    m_pXBinary = nullptr;
}

XFTreeModel::~XFTreeModel()
{
    clear();
}

void XFTreeModel::setData(XBinary *pXBinary, const QList<XBinary::XFHEADER> &listHeaders)
{
    beginResetModel();

    clear();

    m_pXBinary = pXBinary;

    m_pRootItem = new TREEITEM();
    m_pRootItem->pParent = nullptr;
    m_pRootItem->nRow = 0;

    buildTree(listHeaders);

    endResetModel();
}

QModelIndex XFTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!m_pRootItem || ((parent.isValid()) && (parent.column() != 0)) || !hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TREEITEM *pParentItem = nullptr;

    if (!parent.isValid()) {
        pParentItem = m_pRootItem;
    } else {
        pParentItem = static_cast<TREEITEM *>(parent.internalPointer());
    }

    if (pParentItem && (row >= 0) && (row < pParentItem->listChildren.count())) {
        TREEITEM *pChildItem = pParentItem->listChildren.at(row);
        return createIndex(row, column, pChildItem);
    }

    return QModelIndex();
}

QModelIndex XFTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    TREEITEM *pChildItem = static_cast<TREEITEM *>(child.internalPointer());

    if (!pChildItem) {
        return QModelIndex();
    }

    TREEITEM *pParentItem = pChildItem->pParent;

    if (!pParentItem || (pParentItem == m_pRootItem)) {
        return QModelIndex();
    }

    return createIndex(pParentItem->nRow, 0, pParentItem);
}

int XFTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!m_pRootItem || ((parent.isValid()) && (parent.column() != 0))) {
        return 0;
    }

    TREEITEM *pParentItem = nullptr;

    if (!parent.isValid()) {
        pParentItem = m_pRootItem;
    } else {
        pParentItem = static_cast<TREEITEM *>(parent.internalPointer());
    }

    if (pParentItem) {
        return pParentItem->listChildren.count();
    }

    return 0;
}

int XFTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return __COLUMN_SIZE;
}

QVariant XFTreeModel::data(const QModelIndex &index, int role) const
{
    QVariant result;

    if (!index.isValid()) {
        return result;
    }

    TREEITEM *pItem = static_cast<TREEITEM *>(index.internalPointer());

    if (!pItem) {
        return result;
    }

    qint32 nColumn = index.column();

    if (role == Qt::DisplayRole) {
        if (nColumn == COLUMN_NAME) {
            if (m_pXBinary) {
                QString sText = XFormats::getXFHeaderStructName(pItem->xfHeader);
                result = sText;
                // TODO if table number of records
            } else {
                result = QString::number(pItem->xfHeader.structID);
            }
        } /*else if (nColumn == COLUMN_TYPE) {
            if (pItem->xfHeader.xfType == XBinary::XFTYPE_HEADER) {
                result = QString("HEADER");
            } else if (pItem->xfHeader.xfType == XBinary::XFTYPE_TABLE) {
                result = tr("TABLE");
            } else {
                result = tr("Unknown");
            }
        } else if (nColumn == COLUMN_OFFSET) {
            result = XBinary::valueToHexEx(pItem->xfHeader.xLoc.nLocation);
        } else if (nColumn == COLUMN_INFO) {
            if (pItem->xfHeader.xfType == XBinary::XFTYPE_HEADER) {
                result = QString("fields: %1").arg(pItem->xfHeader.listFields.count());
            } else if (pItem->xfHeader.xfType == XBinary::XFTYPE_TABLE) {
                result = QString("rows: %1").arg(pItem->xfHeader.listRowLocations.count());
            }
        }*/
    } /*else if (role == Qt::TextAlignmentRole) {
        if (nColumn == COLUMN_OFFSET) {
            result = static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }
    }*/

    return result;
}

QVariant XFTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;

    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        if (section == COLUMN_NAME) {
            result = tr("Name");
        } /*else if (section == COLUMN_TYPE) {
            result = tr("Type");
        } else if (section == COLUMN_OFFSET) {
            result = tr("Offset");
        } else if (section == COLUMN_INFO) {
            result = tr("Info");
        }*/
    }

    return result;
}

const XFTreeModel::TREEITEM *XFTreeModel::rootItem() const
{
    return m_pRootItem;
}

const XFTreeModel::TREEITEM *XFTreeModel::itemFromIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return nullptr;
    }

    return static_cast<TREEITEM *>(index.internalPointer());
}

void XFTreeModel::clear()
{
    if (m_pRootItem) {
        // Delete all tree items using a stack (iterative to avoid deep recursion)
        QList<TREEITEM *> listToDelete;
        listToDelete.append(m_pRootItem);

        while (!listToDelete.isEmpty()) {
            TREEITEM *pItem = listToDelete.takeLast();

            for (TREEITEM *pChild : pItem->listChildren) {
                listToDelete.append(pChild);
            }

            delete pItem;
        }

        m_pRootItem = nullptr;
    }
}

void XFTreeModel::buildTree(const QList<XBinary::XFHEADER> &listHeaders)
{
    QMap<QString, TREEITEM *> mapItems;

    qint32 nCount = listHeaders.count();

    QList<TREEITEM *> listItems;

    for (qint32 i = 0; i < nCount; i++) {
        TREEITEM *pItem = new TREEITEM();
        pItem->xfHeader = listHeaders.at(i);
        pItem->pParent = nullptr;
        pItem->nRow = 0;
        listItems.append(pItem);
        mapItems.insert(pItem->xfHeader.sTag, pItem);
    }

    for (qint32 i = 0; i < nCount; i++) {
        TREEITEM *pItem = listItems.at(i);
        QString sParentTag = pItem->xfHeader.sParentTag;

        if (!sParentTag.isEmpty() && mapItems.contains(sParentTag)) {
            TREEITEM *pParentItem = mapItems.value(sParentTag);
            pItem->pParent = pParentItem;
            pItem->nRow = pParentItem->listChildren.count();
            pParentItem->listChildren.append(pItem);
        } else {
            pItem->pParent = m_pRootItem;
            pItem->nRow = m_pRootItem->listChildren.count();
            m_pRootItem->listChildren.append(pItem);
        }
    }
}

QString XFTreeModel::treeToString(XFTreeModel *pModel, const QString &sTitle)
{
    QStringList listLines;

    if (!pModel || !pModel->m_pRootItem) {
        return QString();
    }

    if (!sTitle.isEmpty()) {
        listLines.append(sTitle);
    }

    TREEITEM *pRoot = pModel->m_pRootItem;
    for (TREEITEM *pChild : pRoot->listChildren) {
        appendTreeLines(&listLines, pModel->m_pXBinary, pChild, "");
    }

    return listLines.join("\n");
}

void XFTreeModel::printToConsole(XFTreeModel *pModel, const QString &sTitle)
{
    QString sOutput = treeToString(pModel, sTitle);

    const QStringList listLines = sOutput.split("\n");

    for (const QString &sLine : listLines) {
        qDebug().noquote() << sLine;
    }
}

void XFTreeModel::appendTreeLines(QStringList *pListLines, XBinary *pXBinary, TREEITEM *pItem, const QString &sPrefix)
{
    QString sStructName;
    if (pXBinary) {
        sStructName = XFormats::getXFHeaderStructName(pItem->xfHeader);
    } else {
        sStructName = QString::number(pItem->xfHeader.structID);
    }

    QString sString = "[" + XBinary::xfHeaderToString(pItem->xfHeader, sStructName, pItem->xfHeader.sParentTag) + "]";

    QString sLine = sPrefix + sStructName + sString;

    pListLines->append(sLine);

    for (TREEITEM *pChild : pItem->listChildren) {
        appendTreeLines(pListLines, pXBinary, pChild, sPrefix + "    ");
    }
}

QString XFTreeModel::getItemName(XBinary *pXBinary, TREEITEM *pItem)
{
    QString sStructName;
    if (pXBinary) {
        sStructName = XFormats::getXFHeaderStructName(pItem->xfHeader);
    } else {
        sStructName = QString::number(pItem->xfHeader.structID);
    }
    return QString(sStructName).toUpper().remove(" ").remove("-");
}

QString XFTreeModel::getItemString(XBinary *pXBinary, TREEITEM *pItem)
{
    QString sStructName;
    if (pXBinary) {
        sStructName = XFormats::getXFHeaderStructName(pItem->xfHeader);
    } else {
        sStructName = QString::number(pItem->xfHeader.structID);
    }
    return XBinary::xfHeaderToString(pItem->xfHeader, sStructName, pItem->xfHeader.sParentTag);
}

QString XFTreeModel::getItemType(TREEITEM *pItem)
{
    return XBinary::xfTypeIdToFtString(pItem->xfHeader.xfType);
}

QString XFTreeModel::getItemFileType(TREEITEM *pItem)
{
    return XBinary::fileTypeIdToString(pItem->xfHeader.fileType);
}

QString XFTreeModel::getItemOffset(TREEITEM *pItem)
{
    return XBinary::valueToHexEx(pItem->xfHeader.xLoc.nLocation);
}

QString XFTreeModel::getItemSize(TREEITEM *pItem)
{
    return XBinary::valueToHexEx(pItem->xfHeader.nSize);
}

QString XFTreeModel::getItemRows(TREEITEM *pItem)
{
    return QString::number(pItem->xfHeader.listRowLocations.count());
}

QString XFTreeModel::xmlEscape(const QString &s)
{
    QString r = s;
    r.replace("&", "&amp;");
    r.replace("<", "&lt;");
    r.replace(">", "&gt;");
    r.replace("\"", "&quot;");
    return r;
}

QString XFTreeModel::jsonEscape(const QString &s)
{
    QString r = s;
    r.replace("\\", "\\\\");
    r.replace("\"", "\\\"");
    r.replace("\n", "\\n");
    r.replace("\r", "\\r");
    r.replace("\t", "\\t");
    return r;
}

QString XFTreeModel::svQuote(const QString &s, QChar cSep)
{
    if (s.contains(cSep) || s.contains('"') || s.contains('\n')) {
        return "\"" + QString(s).replace("\"", "\"\"") + "\"";
    }
    return s;
}

void XFTreeModel::appendXMLLines(QStringList *pListLines, XBinary *pXBinary, TREEITEM *pItem, const QString &sIndent)
{
    QString sTag = QString("%1<item name=\"%2\" type=\"%3\" string=\"%4\" fileType=\"%5\" offset=\"%6\" size=\"%7\" rows=\"%8\"")
                       .arg(sIndent)
                       .arg(xmlEscape(getItemName(pXBinary, pItem)))
                       .arg(xmlEscape(getItemType(pItem)))
                       .arg(xmlEscape(getItemString(pXBinary, pItem)))
                       .arg(xmlEscape(getItemFileType(pItem)))
                       .arg(xmlEscape(getItemOffset(pItem)))
                       .arg(xmlEscape(getItemSize(pItem)))
                       .arg(xmlEscape(getItemRows(pItem)));

    if (pItem->listChildren.isEmpty()) {
        pListLines->append(sTag + "/>");
    } else {
        pListLines->append(sTag + ">");
        for (TREEITEM *pChild : pItem->listChildren) {
            appendXMLLines(pListLines, pXBinary, pChild, sIndent + "  ");
        }
        pListLines->append(sIndent + "</item>");
    }
}

void XFTreeModel::appendJSONLines(QStringList *pListLines, XBinary *pXBinary, TREEITEM *pItem, const QString &sIndent, bool bLast)
{
    QString sInner = sIndent + "  ";
    pListLines->append(sIndent + "{");
    pListLines->append(sInner + QString("\"name\": \"%1\",").arg(jsonEscape(getItemName(pXBinary, pItem))));
    pListLines->append(sInner + QString("\"type\": \"%1\",").arg(jsonEscape(getItemType(pItem))));
    pListLines->append(sInner + QString("\"string\": \"%1\",").arg(jsonEscape(getItemString(pXBinary, pItem))));
    pListLines->append(sInner + QString("\"fileType\": \"%1\",").arg(jsonEscape(getItemFileType(pItem))));
    pListLines->append(sInner + QString("\"offset\": \"%1\",").arg(jsonEscape(getItemOffset(pItem))));
    pListLines->append(sInner + QString("\"size\": \"%1\",").arg(jsonEscape(getItemSize(pItem))));
    pListLines->append(sInner + QString("\"rows\": %1,").arg(getItemRows(pItem)));

    if (pItem->listChildren.isEmpty()) {
        pListLines->append(sInner + "\"children\": []");
    } else {
        pListLines->append(sInner + "\"children\": [");
        const qint32 nChildCount = pItem->listChildren.count();
        for (qint32 i = 0; i < nChildCount; i++) {
            appendJSONLines(pListLines, pXBinary, pItem->listChildren.at(i), sInner + "  ", i == nChildCount - 1);
        }
        pListLines->append(sInner + "]");
    }

    pListLines->append(sIndent + (bLast ? "}" : "},"));
}

void XFTreeModel::appendSVLines(QStringList *pListLines, XBinary *pXBinary, TREEITEM *pItem, QChar cSep)
{
    QStringList fields;
    fields << svQuote(getItemName(pXBinary, pItem), cSep) << svQuote(getItemType(pItem), cSep) << svQuote(getItemString(pXBinary, pItem), cSep)
           << svQuote(getItemFileType(pItem), cSep) << svQuote(getItemOffset(pItem), cSep) << svQuote(getItemSize(pItem), cSep) << svQuote(getItemRows(pItem), cSep);
    pListLines->append(fields.join(cSep));

    for (TREEITEM *pChild : pItem->listChildren) {
        appendSVLines(pListLines, pXBinary, pChild, cSep);
    }
}

QString XFTreeModel::toString(XBinary::FORMATTYPE formatType)
{
    if (formatType == XBinary::FORMATTYPE_UNKNOWN) {
        formatType = XBinary::FORMATTYPE_PLAINTEXT;
    }

    if (formatType == XBinary::FORMATTYPE_PLAINTEXT) {
        return toFormattedString();
    } else if (formatType == XBinary::FORMATTYPE_XML) {
        return toXML();
    } else if (formatType == XBinary::FORMATTYPE_JSON) {
        return toJSON();
    } else if (formatType == XBinary::FORMATTYPE_CSV) {
        return toCSV();
    } else if (formatType == XBinary::FORMATTYPE_TSV) {
        return toTSV();
    }

    return QString();
}

QString XFTreeModel::toFormattedString()
{
    QString sResult;

    if (m_pRootItem) {
        for (TREEITEM *pChild : m_pRootItem->listChildren) {
            _toFormattedString(&sResult, m_pXBinary, pChild, 1);
        }
    }

    return sResult;
}

void XFTreeModel::_toFormattedString(QString *pString, XBinary *pXBinary, TREEITEM *pItem, qint32 nLevel)
{
    QString sIndent;
    sIndent = sIndent.leftJustified(4 * (nLevel - 1), ' ');
    pString->append(QString("%1%2 %3\n").arg(sIndent).arg(getItemName(pXBinary, pItem)).arg(getItemString(pXBinary, pItem)));

    for (TREEITEM *pChild : pItem->listChildren) {
        _toFormattedString(pString, pXBinary, pChild, nLevel + 1);
    }
}

QString XFTreeModel::toXML() const
{
    if (!m_pRootItem) return QString();

    QStringList listLines;
    listLines.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    listLines.append("<tree>");

    for (TREEITEM *pChild : m_pRootItem->listChildren) {
        appendXMLLines(&listLines, m_pXBinary, pChild, "  ");
    }

    listLines.append("</tree>");
    return listLines.join("\n");
}

QString XFTreeModel::toJSON() const
{
    if (!m_pRootItem) return QString();

    QStringList listLines;
    listLines.append("[");

    qint32 nCount = m_pRootItem->listChildren.count();
    for (qint32 i = 0; i < nCount; i++) {
        appendJSONLines(&listLines, m_pXBinary, m_pRootItem->listChildren.at(i), "  ", i == nCount - 1);
    }

    listLines.append("]");
    return listLines.join("\n");
}

QString XFTreeModel::toCSV() const
{
    if (!m_pRootItem) return QString();

    QStringList listLines;
    listLines.append("Name,Type,String,FileType,Offset,Size,Rows");

    for (TREEITEM *pChild : m_pRootItem->listChildren) {
        appendSVLines(&listLines, m_pXBinary, pChild, ',');
    }

    return listLines.join("\n");
}

QString XFTreeModel::toTSV() const
{
    if (!m_pRootItem) return QString();

    QStringList listLines;
    listLines.append("Name\tType\tString\tFileType\tOffset\tSize\tRows");

    for (TREEITEM *pChild : m_pRootItem->listChildren) {
        appendSVLines(&listLines, m_pXBinary, pChild, '\t');
    }

    return listLines.join("\n");
}
