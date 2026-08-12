/* Copyright (c) 2017-2026 hors<horsicq@gmail.com>
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
#include "subdevice.h"

SubDevice::SubDevice(QIODevice *pDevice, qint64 nOffset, qint64 nSize, QObject *pParent) : XIODevice(pParent)
{
    m_pDevice = pDevice;

    qint64 nDeviceSize = pDevice ? pDevice->size() : 0;
    if (nDeviceSize < 0) nDeviceSize = 0;
    nOffset = qBound<qint64>(0, nOffset, nDeviceSize);
    const qint64 nAvailable = nDeviceSize - nOffset;
    if ((nSize == -1) || (nSize > nAvailable)) {
        nSize = nAvailable;
    } else if (nSize < 0) {
        nSize = 0;
    }

    setInitLocation(nOffset);
    setSize(nSize);

    if (!pDevice) return;

    pDevice->seek(nOffset);

    setProperty("BACKUPDEVICE", reinterpret_cast<quint64>(pDevice));
    const char *pMemory = reinterpret_cast<const char *>(pDevice->property("Memory").toULongLong());

    if (pMemory) {
        setProperty("Memory", reinterpret_cast<quint64>(pMemory) + nOffset);
    }
}

SubDevice::~SubDevice()
{
    if (isOpen()) {
        setOpenMode(NotOpen);
    }
}

QIODevice *SubDevice::getOrigDevice()
{
    return m_pDevice;
}

bool SubDevice::seek(qint64 nPos)
{
    bool bResult = false;

    if (m_pDevice && (nPos <= size()) && (nPos >= 0)) {
        if (m_pDevice->seek(getInitLocation() + nPos)) {
            bResult = QIODevice::seek(nPos);
        }
    }

    return bResult;
}

bool SubDevice::reset()
{
    return seek(0);
}

qint64 SubDevice::readData(char *pData, qint64 nMaxSize)
{
    if (!m_pDevice || (nMaxSize < 0) || ((nMaxSize > 0) && !pData) || (pos() < 0) || (pos() > size())) return -1;
    nMaxSize = qMin(nMaxSize, size() - pos());

    // The backing device can be shared by multiple SubDevice instances (and by
    // parsers doing absolute reads).  Its cursor therefore cannot be assumed to
    // still match this view's logical cursor from the constructor or last seek.
    const qint64 nAbsolutePosition = (qint64)getInitLocation() + pos();
    if ((m_pDevice->pos() != nAbsolutePosition) && !m_pDevice->seek(nAbsolutePosition)) return -1;

    return m_pDevice->read(pData, nMaxSize);
}

qint64 SubDevice::writeData(const char *pData, qint64 nMaxSize)
{
    if (!m_pDevice || (nMaxSize < 0) || ((nMaxSize > 0) && !pData) || (pos() < 0) || (pos() > size())) return -1;
    nMaxSize = qMin(nMaxSize, size() - pos());

    const qint64 nAbsolutePosition = (qint64)getInitLocation() + pos();
    if ((m_pDevice->pos() != nAbsolutePosition) && !m_pDevice->seek(nAbsolutePosition)) return -1;

    return m_pDevice->write(pData, nMaxSize);
}
