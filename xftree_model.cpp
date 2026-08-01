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
#include <QHash>
#include <QVector>
#include <algorithm>
#include <limits>


namespace {
qint64 getHeaderEndOffset(const XBinary::XFHEADER &xfHeader)
{
    if (xfHeader.xLoc.locType != XBinary::LT_OFFSET) {
        return -1;
    }

    if (xfHeader.xLoc.nLocation == (XBinary::XADDR)-1) {
        return -1;
    }

    if (xfHeader.xLoc.nLocation > (XBinary::XADDR)std::numeric_limits<qint64>::max()) {
        return -1;
    }

    qint64 nOffset = (qint64)xfHeader.xLoc.nLocation;
    qint64 nSize = xfHeader.nSize;

    if (xfHeader.xfType == XBinary::XFTYPE_TABLE) {
        qint64 nRowSize = nSize;

        if (nRowSize <= 0) {
            for (const XBinary::XFRECORD &record : xfHeader.listFields) {
                nRowSize = qMax(nRowSize, (qint64)record.nOffset + record.nSize);
            }
        }

        if ((nRowSize <= 0) || xfHeader.listRowLocations.isEmpty()) {
            return -1;
        }

        qint64 nEndOffset = -1;

        for (XBinary::XADDR nRowLocation : xfHeader.listRowLocations) {
            if ((nRowLocation > (XBinary::XADDR)std::numeric_limits<qint64>::max()) ||
                ((qint64)nRowLocation > std::numeric_limits<qint64>::max() - nRowSize)) {
                return -1;
            }

            nEndOffset = qMax(nEndOffset, (qint64)nRowLocation + nRowSize);
        }

        return nEndOffset;
    }

    if (nSize <= 0) {
        qint32 nNumberOfFields = xfHeader.listFields.count();
        for (qint32 i = 0; i < nNumberOfFields; i++) {
            const XBinary::XFRECORD &record = xfHeader.listFields.at(i);
            nSize = qMax(nSize, (qint64)record.nOffset + record.nSize);
        }

    }

    if (nSize <= 0) {
        return -1;
    }

    if (nOffset > std::numeric_limits<qint64>::max() - nSize) {
        return -1;
    }

    return nOffset + nSize;
}

bool isValidOffsetLocation(const XBinary::XLOC &xLoc)
{
    if (xLoc.locType != XBinary::LT_OFFSET) {
        return false;
    }

    if (xLoc.nLocation == (XBinary::XADDR)-1) {
        return false;
    }

    if (xLoc.nLocation > (XBinary::XADDR)std::numeric_limits<qint64>::max()) {
        return false;
    }

    return true;
}

qint32 findRightmostContainingPosition(const QVector<quint64> &listMaxEnds, qint32 nNode, qint32 nLeft, qint32 nRight,
                                       qint32 nMaximumPosition, quint64 nRequiredEnd)
{
    if ((nLeft > nMaximumPosition) || (listMaxEnds.at(nNode) < nRequiredEnd)) {
        return -1;
    }

    if (nLeft == nRight) {
        return nLeft;
    }

    qint32 nMiddle = nLeft + (nRight - nLeft) / 2;
    qint32 nResult = findRightmostContainingPosition(listMaxEnds, nNode * 2 + 1, nMiddle + 1, nRight, nMaximumPosition, nRequiredEnd);

    if (nResult == -1) {
        nResult = findRightmostContainingPosition(listMaxEnds, nNode * 2, nLeft, nMiddle, nMaximumPosition, nRequiredEnd);
    }

    return nResult;
}

XFTreeModel::TREEITEM *appendExtraTreeItem(XFTreeModel::TREEITEM *pRoot, XBinary::FT fileType, const QString &sTag, XBinary::STRUCTID structID)
{
    XFTreeModel::TREEITEM *pItem = new XFTreeModel::TREEITEM();
    pItem->xfHeader = {};
    pItem->xfHeader.sTag = sTag;
    pItem->xfHeader.fileType = fileType;
    pItem->xfHeader.structID = structID;
    pItem->xfHeader.xLoc.locType = XBinary::LT_OFFSET;
    pItem->xfHeader.xLoc.nLocation = 0;
    pItem->xfHeader.xfType = XBinary::XFTYPE_COMMAND;
    pItem->sStructName = sTag;
    pItem->sStructString = sTag;
    pItem->pParent = pRoot;
    pItem->nRow = pRoot->listChildren.count();
    pRoot->listChildren.append(pItem);

    return pItem;
}
}  // namespace

XFTreeModel::XFTreeModel(QObject *pParent) : QAbstractItemModel(pParent)
{
    m_pRootItem = nullptr;
}

XFTreeModel::~XFTreeModel()
{
    clear();
}

void XFTreeModel::setData(const XBinary::INDATA &inData, const QList<XBinary::XFHEADER> &listHeaders, bool bExtraInfo)
{
    beginResetModel();

    clear();

    m_inData = inData;

    m_pRootItem = new TREEITEM();
    m_pRootItem->pParent = nullptr;
    m_pRootItem->nRow = 0;

    buildTree(listHeaders, bExtraInfo);

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
            if (pItem->xfHeader.sTag == "!VISUALIZATION") {
                result = tr("Visualization");
            } else if (pItem->xfHeader.sTag == "!HEX") {
                result = tr("Hex");
            } else if (pItem->xfHeader.sTag == "!DISASM") {
                result = tr("Disasm");
            } else if (pItem->xfHeader.sTag == "!NFDSCAN") {
                result = tr("Nauz File Detector");
            } else if (pItem->xfHeader.sTag == "!HASH") {
                result = tr("Hash");
            } else if (pItem->xfHeader.sTag == "!SIGNATURES") {
                result = tr("Signatures");
            } else if (pItem->xfHeader.sTag == "!MEMORYMAP") {
                result = tr("Memory map");
            } else if (pItem->xfHeader.sTag == "!ENTROPY") {
                result = tr("Entropy");
            } else if (pItem->xfHeader.sTag == "!EXTRACTOR") {
                result = tr("Extractor");
            } else if (pItem->xfHeader.sTag == "!SEARCH") {
                result = tr("Search");
            } else if (pItem->xfHeader.sTag == "!STRINGS") {
                result = tr("Strings");
            } else if (pItem->xfHeader.sTag == "!IMPORT") {
                result = tr("Import");
            } else if (pItem->xfHeader.sTag == "!EXPORT") {
                result = tr("Export");
            } else if (pItem->xfHeader.sTag == "!SYMBOLS") {
                result = tr("Symbols");
            } else if (pItem->xfHeader.sTag == "!RESOURCES") {
                result = tr("Resources");
            } else {
                result = pItem->sStructName;
            }
            // TODO if table number of records
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

void XFTreeModel::buildTree(const QList<XBinary::XFHEADER> &listHeaders, bool bExtraInfo)
{
    qint32 nCount = listHeaders.count();

    QList<TREEITEM *> listItems;
    listItems.reserve(nCount);
    QList<qint64> listHeaderEnds;
    listHeaderEnds.reserve(nCount);
    QHash<quint64, QString> mapStructNames;

    for (qint32 i = 0; i < nCount; i++) {
        TREEITEM *pItem = new TREEITEM();
        pItem->xfHeader = listHeaders.at(i);
        pItem->pParent = nullptr;
        pItem->nRow = -1;

        if (pItem->xfHeader.sTag.startsWith("!")) {
            pItem->sStructName = pItem->xfHeader.sTag;
            pItem->sStructString = pItem->xfHeader.sTag;
        } else {
            quint64 nStructKey = (static_cast<quint64>(static_cast<quint32>(pItem->xfHeader.fileType)) << 32) |
                                 static_cast<quint32>(pItem->xfHeader.structID);
            QHash<quint64, QString>::const_iterator it = mapStructNames.constFind(nStructKey);

            if (it == mapStructNames.constEnd()) {
                pItem->sStructName = XFormats::getXFHeaderStructName(pItem->xfHeader);

                if (pItem->sStructName.isEmpty()) {
                    pItem->sStructName = QString::number(pItem->xfHeader.structID);
                }

                mapStructNames.insert(nStructKey, pItem->sStructName);
            } else {
                pItem->sStructName = it.value();
            }

            pItem->sStructString = XBinary::xfHeaderToString(pItem->xfHeader, pItem->sStructName, pItem->xfHeader.sParentTag);
        }

        listItems.append(pItem);
        listHeaderEnds.append(getHeaderEndOffset(pItem->xfHeader));
    }

    QVector<qint32> listParentOrder;
    listParentOrder.reserve(nCount);

    for (qint32 i = 0; i < nCount; i++) {
        if (isValidOffsetLocation(listItems.at(i)->xfHeader.xLoc)) {
            listParentOrder.append(i);
        }
    }

    std::sort(listParentOrder.begin(), listParentOrder.end(), [&listItems](qint32 nLeftIndex, qint32 nRightIndex) {
        qint64 nLeftOffset = (qint64)listItems.at(nLeftIndex)->xfHeader.xLoc.nLocation;
        qint64 nRightOffset = (qint64)listItems.at(nRightIndex)->xfHeader.xLoc.nLocation;

        if (nLeftOffset != nRightOffset) {
            return nLeftOffset < nRightOffset;
        }

        // Earlier source items win ties, matching the previous forward scan.
        return nLeftIndex > nRightIndex;
    });

    QVector<qint32> listParentPositions(nCount, -1);
    QHash<qint64, qint32> mapLastParentPositions;

    for (qint32 i = 0; i < listParentOrder.count(); i++) {
        qint32 nSourceIndex = listParentOrder.at(i);
        qint64 nOffset = (qint64)listItems.at(nSourceIndex)->xfHeader.xLoc.nLocation;
        listParentPositions[nSourceIndex] = i;
        mapLastParentPositions.insert(nOffset, i);
    }

    qint32 nParentTreeBase = 1;
    while (nParentTreeBase < listParentOrder.count()) {
        nParentTreeBase *= 2;
    }

    QVector<quint64> listParentMaxEnds(nParentTreeBase * 2, 0);
    QHash<QString, TREEITEM *> mapLatestItemsByTag;

    // Build tree hierarchy in source order to keep parent references deterministic.
    for (qint32 i = 0; i < nCount; i++) {
        TREEITEM *pItem = listItems.at(i);
        QString sParentTag = pItem->xfHeader.sParentTag;

        TREEITEM *pBestParent = nullptr;

        if (!sParentTag.isEmpty()) {
            // An explicit tag describes a logical relationship. PE directory
            // children are commonly stored outside their parent's byte range,
            // so physical containment must not invalidate that relationship.
            pBestParent = mapLatestItemsByTag.value(sParentTag, nullptr);
        }

        if (!pBestParent) {
            // Without a resolvable explicit tag, infer the nearest containing
            // previous structure by physical offset. The interval tree keeps
            // this lookup logarithmic even for large sibling tables.
            if (isValidOffsetLocation(pItem->xfHeader.xLoc)) {
                qint64 nItemOffset = (qint64)pItem->xfHeader.xLoc.nLocation;
                quint64 nRequiredEnd = (quint64)nItemOffset + 1;

                if (listHeaderEnds.at(i) >= 0) {
                    nRequiredEnd = qMax(nRequiredEnd, (quint64)listHeaderEnds.at(i));
                }

                qint32 nMaximumPosition = mapLastParentPositions.value(nItemOffset, -1);

                if (nMaximumPosition >= 0) {
                    qint32 nParentPosition = findRightmostContainingPosition(listParentMaxEnds, 1, 0, nParentTreeBase - 1,
                                                                             nMaximumPosition, nRequiredEnd);

                    if ((nParentPosition >= 0) && (nParentPosition < listParentOrder.count())) {
                        pBestParent = listItems.at(listParentOrder.at(nParentPosition));
                    }
                }
            }
        }

        if (!pBestParent) {
            pBestParent = m_pRootItem;
        }

        pItem->pParent = pBestParent;
        pItem->nRow = pBestParent->listChildren.count();
        pBestParent->listChildren.append(pItem);

        if (!pItem->xfHeader.sTag.isEmpty()) {
            mapLatestItemsByTag.insert(pItem->xfHeader.sTag, pItem);
        }

        qint32 nParentPosition = listParentPositions.at(i);

        if (nParentPosition >= 0) {
            quint64 nParentEnd = (listHeaderEnds.at(i) >= 0) ? (quint64)listHeaderEnds.at(i) : std::numeric_limits<quint64>::max();
            qint32 nTreeIndex = nParentTreeBase + nParentPosition;
            listParentMaxEnds[nTreeIndex] = nParentEnd;

            while (nTreeIndex > 1) {
                nTreeIndex /= 2;
                listParentMaxEnds[nTreeIndex] = qMax(listParentMaxEnds.at(nTreeIndex * 2), listParentMaxEnds.at(nTreeIndex * 2 + 1));
            }
        }
    }

    if (bExtraInfo && m_pRootItem) {
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!VISUALIZATION", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!HEX", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!DISASM", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!NFDSCAN", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!HASH", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!SIGNATURES", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!MEMORYMAP", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!ENTROPY", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!EXTRACTOR", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!SEARCH", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!STRINGS", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!IMPORT", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!EXPORT", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!SYMBOLS", XBinary::STRUCTID_UNKNOWN);
        appendExtraTreeItem(m_pRootItem, m_inData.fileType, "!RESOURCES", XBinary::STRUCTID_UNKNOWN);
    }
}

QString XFTreeModel::getItemSize(TREEITEM *pItem)
{
    if (!pItem) {
        return "?";
    }

    qint64 nSize = pItem->xfHeader.nSize;

    if (nSize > 0) {
        if ((pItem->xfHeader.xfType == XBinary::XFTYPE_TABLE) && !pItem->xfHeader.listRowLocations.isEmpty()) {
            qint64 nRows = pItem->xfHeader.listRowLocations.count();
            if (nSize <= std::numeric_limits<qint64>::max() / nRows) {
                nSize *= nRows;
            }
        }
        return XBinary::valueToHexEx(nSize);
    }

    qint64 nEndOffset = getHeaderEndOffset(pItem->xfHeader);

    if (nEndOffset < 0) {
        return "?";
    }

    if ((qint64)pItem->xfHeader.xLoc.nLocation >= 0) {
        nSize = nEndOffset - (qint64)pItem->xfHeader.xLoc.nLocation;
    }

    if (nSize > 0) {
        return XBinary::valueToHexEx(nSize);
    }

    return "?";
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
        appendTreeLines(&listLines, nullptr, pChild, "");
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
    Q_UNUSED(pXBinary)

    QString sStructName = pItem->sStructName;
    QString sString = "[" + pItem->sStructString + "]";

    QString sLine = sPrefix + sStructName + sString;

    pListLines->append(sLine);

    for (TREEITEM *pChild : pItem->listChildren) {
        appendTreeLines(pListLines, pXBinary, pChild, sPrefix + "    ");
    }
}

QString XFTreeModel::getItemName(XBinary *pXBinary, TREEITEM *pItem)
{
    Q_UNUSED(pXBinary)

    return QString(pItem->sStructName).toUpper().remove(" ").remove("-");
}

QString XFTreeModel::getItemString(XBinary *pXBinary, TREEITEM *pItem)
{
    Q_UNUSED(pXBinary)

    return pItem->sStructString;
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
            _toFormattedString(&sResult, nullptr, pChild, 1);
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
        appendXMLLines(&listLines, nullptr, pChild, "  ");
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
        appendJSONLines(&listLines, nullptr, m_pRootItem->listChildren.at(i), "  ", i == nCount - 1);
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
        appendSVLines(&listLines, nullptr, pChild, ',');
    }

    return listLines.join("\n");
}

QString XFTreeModel::toTSV() const
{
    if (!m_pRootItem) return QString();

    QStringList listLines;
    listLines.append("Name\tType\tString\tFileType\tOffset\tSize\tRows");

    for (TREEITEM *pChild : m_pRootItem->listChildren) {
        appendSVLines(&listLines, nullptr, pChild, '\t');
    }

    return listLines.join("\n");
}






