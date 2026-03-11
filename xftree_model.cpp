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
    if (!m_pRootItem) {
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
    if (!m_pRootItem) {
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
                result = m_pXBinary->structIDToString(pItem->xfHeader.structID);
            } else {
                result = QString::number(pItem->xfHeader.structID);
            }
        } else if (nColumn == COLUMN_TYPE) {
            if (pItem->xfHeader.xfType == XBinary::XFTYPE_HEADER) {
                result = tr("HEADER");
            } else if (pItem->xfHeader.xfType == XBinary::XFTYPE_TABLE) {
                result = tr("TABLE");
            } else {
                result = tr("UNKNOWN");
            }
        } else if (nColumn == COLUMN_OFFSET) {
            result = XBinary::valueToHexEx(pItem->xfHeader.xLoc.nLocation);
        } else if (nColumn == COLUMN_INFO) {
            if (pItem->xfHeader.xfType == XBinary::XFTYPE_HEADER) {
                result = QString("fields: %1").arg(pItem->xfHeader.listFields.count());
            } else if (pItem->xfHeader.xfType == XBinary::XFTYPE_TABLE) {
                result = QString("rows: %1").arg(pItem->xfHeader.listRowLocations.count());
            }
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (nColumn == COLUMN_OFFSET) {
            result = (int)(Qt::AlignRight | Qt::AlignVCenter);
        }
    }

    return result;
}

QVariant XFTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;

    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        if (section == COLUMN_NAME) {
            result = tr("Name");
        } else if (section == COLUMN_TYPE) {
            result = tr("Type");
        } else if (section == COLUMN_OFFSET) {
            result = tr("Offset");
        } else if (section == COLUMN_INFO) {
            result = tr("Info");
        }
    }

    return result;
}

XFTreeModel::TREEITEM *XFTreeModel::rootItem() const
{
    return m_pRootItem;
}

XFTreeModel::TREEITEM *XFTreeModel::itemFromIndex(const QModelIndex &index) const
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

            for (qint32 i = 0; i < pItem->listChildren.count(); i++) {
                listToDelete.append(pItem->listChildren.at(i));
            }

            delete pItem;
        }

        m_pRootItem = nullptr;
    }
}

void XFTreeModel::buildTree(const QList<XBinary::XFHEADER> &listHeaders)
{
    // Build a map from GUID -> TREEITEM for parent lookup
    QMap<QString, TREEITEM *> mapItems;

    qint32 nCount = listHeaders.count();

    // First pass: create all tree items
    QList<TREEITEM *> listItems;

    for (qint32 i = 0; i < nCount; i++) {
        TREEITEM *pItem = new TREEITEM();
        pItem->xfHeader = listHeaders.at(i);
        pItem->pParent = nullptr;
        pItem->nRow = 0;
        listItems.append(pItem);
        mapItems.insert(pItem->xfHeader.sTag, pItem);
    }

    // Second pass: establish parent-child relationships
    for (qint32 i = 0; i < nCount; i++) {
        TREEITEM *pItem = listItems.at(i);
        QString sParentTag = pItem->xfHeader.sParentTag;

        if (!sParentTag.isEmpty() && mapItems.contains(sParentTag)) {
            TREEITEM *pParentItem = mapItems.value(sParentTag);
            pItem->pParent = pParentItem;
            pItem->nRow = pParentItem->listChildren.count();
            pParentItem->listChildren.append(pItem);
        } else {
            // Root-level item
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
    qint32 nChildCount = pRoot->listChildren.count();

    for (qint32 i = 0; i < nChildCount; i++) {
        bool bIsLast = (i == nChildCount - 1);
        appendTreeLines(&listLines, pModel->m_pXBinary, pRoot->listChildren.at(i), "", bIsLast);
    }

    return listLines.join("\n");
}

void XFTreeModel::printToConsole(XFTreeModel *pModel, const QString &sTitle)
{
    QString sOutput = treeToString(pModel, sTitle);

    QStringList listLines = sOutput.split("\n");

    for (qint32 i = 0; i < listLines.count(); i++) {
        qDebug().noquote() << listLines.at(i);
    }
}

void XFTreeModel::appendTreeLines(QStringList *pListLines, XBinary *pXBinary, TREEITEM *pItem, const QString &sPrefix, bool bIsLast)
{
    QString sConnector = bIsLast ? QString::fromUtf8("\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80 ") : QString::fromUtf8("\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 ");

    QString sTypeName;
    if (pItem->xfHeader.xfType == XBinary::XFTYPE_HEADER) {
        sTypeName = "HEADER";
    } else if (pItem->xfHeader.xfType == XBinary::XFTYPE_TABLE) {
        sTypeName = "TABLE";
    } else {
        sTypeName = "UNKNOWN";
    }

    QString sInfo;
    if (pItem->xfHeader.xfType == XBinary::XFTYPE_HEADER) {
        sInfo = QString("fields: %1").arg(pItem->xfHeader.listFields.count());
    } else if (pItem->xfHeader.xfType == XBinary::XFTYPE_TABLE) {
        sInfo = QString("rows: %1").arg(pItem->xfHeader.listRowLocations.count());
    }

    QString sStructName;
    if (pXBinary) {
        sStructName = pXBinary->structIDToString(pItem->xfHeader.structID);
    } else {
        sStructName = QString::number(pItem->xfHeader.structID);
    }

    QString sOffset = XBinary::valueToHexEx(pItem->xfHeader.xLoc.nLocation);

    QString sTag = "[" + XBinary::xfHeaderToTag(pItem->xfHeader, sStructName, pItem->xfHeader.sParentTag) + "]";

    QString sLine = sPrefix + sConnector + sStructName + " [" + sTypeName + "] offset=" +
                    sOffset + " " + sInfo + " " + sTag;

    pListLines->append(sLine);

    QString sChildPrefix = sPrefix + (bIsLast ? "    " : QString::fromUtf8("\xe2\x94\x82   "));

    qint32 nChildCount = pItem->listChildren.count();

    for (qint32 i = 0; i < nChildCount; i++) {
        bool bChildIsLast = (i == nChildCount - 1);
        appendTreeLines(pListLines, pXBinary, pItem->listChildren.at(i), sChildPrefix, bChildIsLast);
    }
}
