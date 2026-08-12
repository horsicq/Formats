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

#include <limits>

SubDevice::SubDevice(QIODevice *pDevice, qint64 nOffset, qint64 nSize, QObject *pParent) : XIODevice(pParent)
{
    m_pDevice = pDevice;
    m_bIsRangeValid = false;

    const qint64 nDeviceSize = pDevice ? pDevice->size() : -1;
    if (pDevice && (nDeviceSize >= 0) && (nOffset >= 0) && (nOffset <= nDeviceSize) && (nSize >= -1)) {
        const qint64 nAvailable = nDeviceSize - nOffset;
        if (nSize == -1) {
            nSize = nAvailable;
        }

        if (nSize <= nAvailable) {
            setInitLocation((quint64)nOffset);
            setSize(nSize);
            m_bIsRangeValid = true;
        }
    }

    if (!pDevice) return;

    connect(pDevice, &QObject::destroyed, this, &SubDevice::handleBackingDeviceDestroyed);
    connect(pDevice, &QIODevice::aboutToClose, this, &SubDevice::handleBackingDeviceDestroyed);

    // Do not propagate raw-pointer dynamic properties. The backing QObject is
    // tracked by QPointer and getBackupDevice() recognizes SubDevice directly.
}

SubDevice::~SubDevice()
{
    if (isOpen()) {
        close();
    }
}

QIODevice *SubDevice::getOrigDevice()
{
    return m_pDevice.data();
}

bool SubDevice::open(OpenMode mode)
{
    if (isOpen()) {
        close();
    }

    const OpenMode accessMode = mode & QIODevice::ReadWrite;
    const qint64 nDeviceSize = m_pDevice ? m_pDevice->size() : -1;
    const quint64 nInitLocation = getInitLocation();
    if (!m_bIsRangeValid || !m_pDevice || !m_pDevice->isOpen() || m_pDevice->isSequential() ||
        (accessMode == NotOpen) || (nDeviceSize < 0) ||
        (nInitLocation > (quint64)(std::numeric_limits<qint64>::max)()) ||
        ((qint64)nInitLocation > nDeviceSize) || (size() < 0) || (size() > (nDeviceSize - (qint64)nInitLocation)) ||
        (mode & (QIODevice::Append | QIODevice::Truncate)) ||
        ((mode & QIODevice::ReadOnly) && !m_pDevice->isReadable()) ||
        ((mode & QIODevice::WriteOnly) && !m_pDevice->isWritable())) {
        return false;
    }

    if (!m_pDevice->seek((qint64)getInitLocation())) {
        return false;
    }

    if (!XIODevice::open(mode) || !QIODevice::seek(0)) {
        close();
        return false;
    }

    return true;
}

void SubDevice::close()
{
    XIODevice::close();
}

void SubDevice::handleBackingDeviceDestroyed()
{
    close();
}

bool SubDevice::seek(qint64 nPos)
{
    if (!isOpen() || !m_pDevice || !m_pDevice->isOpen() || (nPos < 0) || (nPos > size()) ||
        (getInitLocation() > (quint64)(std::numeric_limits<qint64>::max)()) ||
        (nPos > ((std::numeric_limits<qint64>::max)() - (qint64)getInitLocation()))) {
        return false;
    }

    const qint64 nAbsolutePosition = (qint64)getInitLocation() + nPos;
    const qint64 nOldBackingPosition = m_pDevice->pos();
    if (!m_pDevice->seek(nAbsolutePosition)) {
        return false;
    }

    if (!QIODevice::seek(nPos)) {
        if (nOldBackingPosition >= 0) m_pDevice->seek(nOldBackingPosition);
        return false;
    }

    return true;
}

bool SubDevice::reset()
{
    return seek(0);
}

qint64 SubDevice::readData(char *pData, qint64 nMaxSize)
{
    const qint64 nPosition = pos();
    if (!isOpen() || !isReadable() || !m_pDevice || !m_pDevice->isOpen() || !m_pDevice->isReadable() ||
        (nMaxSize < 0) || ((nMaxSize > 0) && !pData) || (nPosition < 0) || (nPosition > size())) return -1;
    nMaxSize = qMin(nMaxSize, size() - nPosition);

    // The backing device can be shared by multiple SubDevice instances (and by
    // parsers doing absolute reads).  Its cursor therefore cannot be assumed to
    // still match this view's logical cursor from the constructor or last seek.
    const qint64 nAbsolutePosition = (qint64)getInitLocation() + nPosition;
    if ((m_pDevice->pos() != nAbsolutePosition) && !m_pDevice->seek(nAbsolutePosition)) return -1;

    return m_pDevice->read(pData, nMaxSize);
}

qint64 SubDevice::writeData(const char *pData, qint64 nMaxSize)
{
    const qint64 nPosition = pos();
    if (!isOpen() || !isWritable() || !m_pDevice || !m_pDevice->isOpen() || !m_pDevice->isWritable() ||
        (nMaxSize < 0) || ((nMaxSize > 0) && !pData) || (nPosition < 0) || (nPosition > size())) return -1;
    nMaxSize = qMin(nMaxSize, size() - nPosition);

    const qint64 nAbsolutePosition = (qint64)getInitLocation() + nPosition;
    if ((m_pDevice->pos() != nAbsolutePosition) && !m_pDevice->seek(nAbsolutePosition)) return -1;

    return m_pDevice->write(pData, nMaxSize);
}
