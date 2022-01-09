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
#include "subdevice.h"

SubDevice::SubDevice(QIODevice *pDevice, qint64 nOffset, qint64 nSize, QObject *pParent) : QIODevice(pParent)
{
    if(nOffset>pDevice->size())
    {
        nOffset=pDevice->size();
    }

    if(nOffset<0)
    {
        nOffset=0;
    }

    if((nSize+nOffset>pDevice->size())||(nSize==-1)) // TODO Check
    {
        nSize=pDevice->size()-nOffset;
    }

    if(nSize+nOffset<0)
    {
        nSize=0;
    }

    this->g_pDevice=pDevice;
    this->g_nOffset=nOffset;
    this->g_nSize=nSize;

    //    reset();
    pDevice->seek(nOffset);
}

SubDevice::~SubDevice()
{
    if(isOpen())
    {
        _close();
    }
}

qint64 SubDevice::getInitOffset()
{
    return g_nOffset;
}

qint64 SubDevice::size() const
{
    return g_nSize;
}

//qint64 SubDevice::bytesAvailable() const
//{
//    return nSize;
//}

bool SubDevice::isSequential() const
{
    return false;
}

bool SubDevice::seek(qint64 nPos)
{
    bool bResult=false;

    if((nPos<g_nSize)&&(nPos>=0))
    {
        if(g_pDevice->seek(g_nOffset+nPos))
        {
            bResult=QIODevice::seek(nPos);
        }
    }

    return bResult;
}

bool SubDevice::reset()
{
    return seek(0);
}

bool SubDevice::open(QIODevice::OpenMode mode)
{
    setOpenMode(mode);

    return true;
}

bool SubDevice::atEnd() const
{
    return (bytesAvailable()==0);
}

void SubDevice::close()
{
    _close();
}

qint64 SubDevice::pos() const
{
//    return pDevice->pos()-nOffset;
    return QIODevice::pos();
}

void SubDevice::_close()
{
    setOpenMode(NotOpen);
}

qint64 SubDevice::readData(char *pData, qint64 nMaxSize)
{
    nMaxSize=qMin(nMaxSize,g_nSize-pos());

    qint64 nLen=g_pDevice->read(pData,nMaxSize);

    return nLen;
}

qint64 SubDevice::writeData(const char *pData, qint64 nMaxSize)
{
    nMaxSize=qMin(nMaxSize,g_nSize-pos());

    qint64 nLen=g_pDevice->write(pData,nMaxSize);

    return nLen;
}

void SubDevice::setErrorString(const QString &sString)
{
    QIODevice::setErrorString(sString);
}
