/* Copyright (c) 2017-2022 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "scanitemmodel.h"

ScanItemModel::ScanItemModel(QList<XBinary::SCANSTRUCT> *pListScanStructs, QObject *pParent, int nNumberOfColumns)
    : QAbstractItemModel(pParent)
{
    g_pRootItem=new ScanItem(tr("Result"),nullptr,nNumberOfColumns,true);
    XBinary::SCANSTRUCT emptySS={};
    g_pRootItem->setScanStruct(emptySS);

    QMap<QString,ScanItem *> mapParents;

    qint32 nNumberOfDetects=pListScanStructs->count();

    for(qint32 i=0;i<nNumberOfDetects;i++)
    {
        if(!mapParents.contains(pListScanStructs->at(i).id.sUuid))
        {
            ScanItem *_itemParent;

            if(pListScanStructs->at(i).parentId.sUuid=="")
            {
                _itemParent=g_pRootItem;
            }
            else
            {
                _itemParent=mapParents.value(pListScanStructs->at(i).parentId.sUuid);
            }

            QString sParent=XBinary::createTypeString(&pListScanStructs->at(i));

            ScanItem *pItemParent=new ScanItem(sParent,_itemParent,nNumberOfColumns,true);
            XBinary::SCANSTRUCT ss=XBinary::createHeaderScanStruct(&pListScanStructs->at(i));
            pItemParent->setScanStruct(ss);
            _itemParent->appendChild(pItemParent);

            mapParents.insert(pListScanStructs->at(i).id.sUuid,pItemParent);
        }

        if(pListScanStructs->at(i).sName!="")
        {
            ScanItem *pItemParent=mapParents.value(pListScanStructs->at(i).id.sUuid);

            QString sItem=XBinary::createResultString2(&pListScanStructs->at(i));
            ScanItem *pItem=new ScanItem(sItem,pItemParent,nNumberOfColumns,false);
            pItem->setScanStruct(pListScanStructs->at(i));
            pItemParent->appendChild(pItem);
        }
    }
}

ScanItemModel::~ScanItemModel()
{
    delete g_pRootItem;
}

QVariant ScanItemModel::headerData(int nSection, Qt::Orientation orientation, int nRole) const
{
    QVariant result;

    if((orientation==Qt::Horizontal)&&(nRole==Qt::DisplayRole))
    {
        result=g_pRootItem->data(nSection);
    }

    return result;
}

QModelIndex ScanItemModel::index(int nRow, int nColumn, const QModelIndex &parent) const
{
    QModelIndex result;

    if(hasIndex(nRow, nColumn, parent))
    {
        ScanItem *pParentItem=nullptr;

        if(!parent.isValid())
        {
            pParentItem=g_pRootItem;
        }
        else
        {
            pParentItem=static_cast<ScanItem *>(parent.internalPointer());
        }

        ScanItem *pChildItem=pParentItem->child(nRow);

        if(pChildItem)
        {
            result=createIndex(nRow, nColumn, pChildItem);
        }
    }

    return result;
}

QModelIndex ScanItemModel::parent(const QModelIndex &index) const
{
    QModelIndex result;

    if(index.isValid())
    {
        ScanItem *pChildItem=static_cast<ScanItem *>(index.internalPointer());
        ScanItem *pParentItem=pChildItem->getParentItem();

        if(pParentItem!=g_pRootItem)
        {
            result=createIndex(pParentItem->row(),0,pParentItem);
        }
    }

    return result;
}

int ScanItemModel::rowCount(const QModelIndex &parent) const
{
    int nResult=0;

    if(parent.column()<=0)
    {
        ScanItem *pParentItem=nullptr;

        if(!parent.isValid())
        {
            pParentItem=g_pRootItem;
        }
        else
        {
            pParentItem=static_cast<ScanItem *>(parent.internalPointer());
        }

        nResult=pParentItem->childCount();
    }

    return nResult;
}

int ScanItemModel::columnCount(const QModelIndex &parent) const
{
    int nResult=0;

    if(parent.isValid())
    {
        nResult=static_cast<ScanItem *>(parent.internalPointer())->columnCount();
    }
    else
    {
        nResult=g_pRootItem->columnCount();
    }

    return nResult;
}

QVariant ScanItemModel::data(const QModelIndex &index, int nRole) const
{
    QVariant result;

    if(index.isValid())
    {
        ScanItem *pItem=static_cast<ScanItem *>(index.internalPointer());

        if(nRole==Qt::DisplayRole)
        {
            result=pItem->data(index.column());
        }
        else if(nRole==Qt::UserRole+UD_FILETYPE)
        {
            result=pItem->scanStruct().id.fileType;
        }
        else if(nRole==Qt::UserRole+UD_NAME)
        {
            result=pItem->scanStruct().sName;
        }
        else if(nRole==Qt::UserRole+UD_INFO)
        {
            result=pItem->scanStruct().varInfo;
        }
#ifdef QT_GUI_LIB
        else if(nRole==Qt::ForegroundRole)
        {
            result=QVariant(pItem->scanStruct().colText);
        }
#endif
    }

    return result;
}

Qt::ItemFlags ScanItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result=Qt::NoItemFlags;

    if(index.isValid())
    {
        result=QAbstractItemModel::flags(index);
    }

    return result;
}

QString ScanItemModel::toXML()
{
    QString sResult;
    QXmlStreamWriter xml(&sResult);

    xml.setAutoFormatting(true);

    _toXML(&xml,g_pRootItem,0);

    return sResult;
}

QString ScanItemModel::toJSON()
{
    QString sResult;

    QJsonObject jsonResult;

    _toJSON(&jsonResult,g_pRootItem,0);

    QJsonDocument saveFormat(jsonResult);

    QByteArray baData=saveFormat.toJson(QJsonDocument::Indented);

    sResult=baData.data();

    return sResult;
}

QString ScanItemModel::toCSV()
{
    QString sResult;

    _toCSV(&sResult,g_pRootItem,0);

    return sResult;
}

QString ScanItemModel::toTSV()
{
    QString sResult;

    _toTSV(&sResult,g_pRootItem,0);

    return sResult;
}

QString ScanItemModel::toFormattedString()
{
    QString sResult;

    _toFormattedString(&sResult,g_pRootItem,0);

    return sResult;
}

QString ScanItemModel::toString(XBinary::FORMATTYPE formatType)
{
    QString sResult;

    if(formatType==XBinary::FORMATTYPE_XML)
    {
        sResult=toXML();
    }
    else if(formatType==XBinary::FORMATTYPE_JSON)
    {
        sResult=toJSON();
    }
    else if(formatType==XBinary::FORMATTYPE_CSV)
    {
        sResult=toCSV();
    }
    else if(formatType==XBinary::FORMATTYPE_TSV)
    {
        sResult=toTSV();
    }
    else
    {
        sResult=toFormattedString();
    }

    return sResult;
}

ScanItem *ScanItemModel::rootItem()
{
    return this->g_pRootItem;
}

void ScanItemModel::_toXML(QXmlStreamWriter *pXml, ScanItem *pItem, qint32 nLevel)
{
    if(pItem->childCount())
    {
        pXml->writeStartElement(pItem->data(0).toString());

        qint32 nNumberOfChildren=pItem->childCount();

        for(qint32 i=0;i<nNumberOfChildren;i++)
        {
            _toXML(pXml,pItem->child(i),nLevel+1);
        }

        pXml->writeEndElement();
    }
    else
    {
        XBinary::SCANSTRUCT ss=pItem->scanStruct();

        pXml->writeStartElement("detect");
        pXml->writeAttribute("type",ss.sType);
        pXml->writeAttribute("name",ss.sName);
        pXml->writeAttribute("version",ss.sVersion);
        pXml->writeAttribute("info",ss.sInfo);
        pXml->writeCharacters(pItem->data(0).toString());
        pXml->writeEndElement();
    }
}

void ScanItemModel::_toJSON(QJsonObject *pJsonObject, ScanItem *pItem,qint32 nLevel)
{
    if(pItem->childCount())
    {
        XBinary::SCANSTRUCT ss=pItem->scanStruct();

        QString sArrayName="detects";

        if(ss.id.filePart!=XBinary::FILEPART_UNKNOWN)
        {
            pJsonObject->insert("parentfilepart",XBinary::recordFilePartIdToString(ss.parentId.filePart));
            pJsonObject->insert("filetype",XBinary::fileTypeIdToString(ss.id.fileType));

            sArrayName="values";
        }

        QJsonArray jsArray;

        qint32 nNumberOfChildren=pItem->childCount();

        for(qint32 i=0;i<nNumberOfChildren;i++)
        {
            QJsonObject jsRecord;

            _toJSON(&jsRecord,pItem->child(i),nLevel+1);

            jsArray.append(jsRecord);
        }

        pJsonObject->insert(sArrayName,jsArray);
    }
    else
    {
        XBinary::SCANSTRUCT ss=pItem->scanStruct();

        pJsonObject->insert("type",ss.sType);
        pJsonObject->insert("name",ss.sName);
        pJsonObject->insert("version",ss.sVersion);
        pJsonObject->insert("info",ss.sInfo);
        pJsonObject->insert("string",pItem->data(0).toString());
    }
}

void ScanItemModel::_toCSV(QString *pString, ScanItem *pItem,qint32 nLevel)
{
    if(pItem->childCount())
    {
        qint32 nNumberOfChildren=pItem->childCount();

        for(qint32 i=0;i<nNumberOfChildren;i++)
        {
            _toCSV(pString,pItem->child(i),nLevel+1);
        }
    }
    else
    {
        XBinary::SCANSTRUCT ss=pItem->scanStruct();

        QString sResult=QString("%1;%2;%3;%4;%5\n")
                        .arg(ss.sType,
                             ss.sName,
                             ss.sVersion,
                             ss.sInfo,
                             pItem->data(0).toString());

        pString->append(sResult);
    }
}

void ScanItemModel::_toTSV(QString *pString, ScanItem *pItem,qint32 nLevel)
{
    if(pItem->childCount())
    {
        qint32 nNumberOfChildren=pItem->childCount();

        for(qint32 i=0;i<nNumberOfChildren;i++)
        {
            _toTSV(pString,pItem->child(i),nLevel+1);
        }
    }
    else
    {
        XBinary::SCANSTRUCT ss=pItem->scanStruct();

        QString sResult=QString("%1\t%2\t%3\t%4\t%5\n").arg(
                                ss.sType,
                                ss.sName,
                                ss.sVersion,
                                ss.sInfo,
                                pItem->data(0).toString()
                    );

        pString->append(sResult);
    }
}

void ScanItemModel::_toFormattedString(QString *pString, ScanItem *pItem, qint32 nLevel)
{
    if(nLevel)
    {
        QString sResult;
        sResult=sResult.leftJustified(4*(nLevel-1),' '); // TODO function
        sResult.append(QString("%1\n").arg(pItem->data(0).toString()));
        pString->append(sResult);
    }

    qint32 nNumberOfChildren=pItem->childCount();

    for(qint32 i=0;i<nNumberOfChildren;i++)
    {
        _toFormattedString(pString,pItem->child(i),nLevel+1);
    }
}
