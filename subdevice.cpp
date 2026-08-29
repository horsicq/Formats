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

    const qint64 nDeviceSize = m_pDevice ? m_pDevice->size() : -1;
    if (m_pDevice && (nDeviceSize >= 0) && (nOffset >= 0) && (nOffset <= nDeviceSize) && (nSize >= -1)) {
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

    if (!m_pDevice) return;

    connect(m_pDevice.data(), &QObject::destroyed, this, &SubDevice::handleBackingDeviceDestroyed);
    if (!m_pDevice) return;
    connect(m_pDevice.data(), &QIODevice::aboutToClose, this, &SubDevice::handleBackingDeviceDestroyed);

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
    QPointer<SubDevice> guardedThis(this);
    if (isOpen()) {
        close();
        if (!guardedThis) return false;
    }

    const OpenMode accessMode = mode & QIODevice::ReadWrite;
    QPointer<QIODevice> guardedDevice(m_pDevice);
    if (!m_bIsRangeValid || !guardedDevice || (accessMode == NotOpen) || (mode & (QIODevice::Append | QIODevice::Truncate))) {
        return false;
    }

    const qint64 nDeviceSize = guardedDevice->size();
    if (!guardedThis || !guardedDevice) return false;
    const quint64 nInitLocation = getInitLocation();
    const qint64 nRangeSize = size();
    if ((nDeviceSize < 0) || (nInitLocation > (quint64)(std::numeric_limits<qint64>::max)()) || ((qint64)nInitLocation > nDeviceSize) || (nRangeSize < 0) ||
        (nRangeSize > (nDeviceSize - (qint64)nInitLocation))) {
        return false;
    }

    const bool bBackingOpen = guardedDevice->isOpen();
    if (!guardedThis || !guardedDevice || !bBackingOpen) return false;
    const bool bSequential = guardedDevice->isSequential();
    if (!guardedThis || !guardedDevice || bSequential) return false;
    if (mode & QIODevice::ReadOnly) {
        const bool bReadable = guardedDevice->isReadable();
        if (!guardedThis || !guardedDevice || !bReadable) return false;
    }
    if (mode & QIODevice::WriteOnly) {
        const bool bWritable = guardedDevice->isWritable();
        if (!guardedThis || !guardedDevice || !bWritable) return false;
    }

    const bool bPositioned = guardedDevice->seek((qint64)nInitLocation);
    if (!guardedThis || !guardedDevice || !bPositioned) return false;

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
    QPointer<SubDevice> guardedThis(this);
    QPointer<QIODevice> guardedDevice(m_pDevice);
    const qint64 nRangeSize = size();
    const quint64 nInitLocation = getInitLocation();
    if (!isOpen() || !guardedDevice || (nPos < 0) || (nPos > nRangeSize) || (nInitLocation > (quint64)(std::numeric_limits<qint64>::max)()) ||
        (nPos > ((std::numeric_limits<qint64>::max)() - (qint64)nInitLocation))) {
        return false;
    }

    const bool bBackingOpen = guardedDevice->isOpen();
    if (!guardedThis || !guardedDevice || !bBackingOpen) return false;
    const qint64 nAbsolutePosition = (qint64)nInitLocation + nPos;
    const qint64 nOldBackingPosition = guardedDevice->pos();
    if (!guardedThis || !guardedDevice) return false;
    const bool bPositioned = guardedDevice->seek(nAbsolutePosition);
    if (!guardedThis || !guardedDevice || !bPositioned) return false;

    if (!QIODevice::seek(nPos)) {
        if ((nOldBackingPosition >= 0) && guardedDevice) {
            guardedDevice->seek(nOldBackingPosition);
        }
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
    QPointer<SubDevice> guardedThis(this);
    QPointer<QIODevice> guardedDevice(m_pDevice);
    const qint64 nPosition = pos();
    const qint64 nRangeSize = size();
    if (!isOpen() || !isReadable() || !guardedDevice || (nMaxSize < 0) || ((nMaxSize > 0) && !pData) || (nPosition < 0) || (nPosition > nRangeSize)) return -1;
    const bool bBackingOpen = guardedDevice->isOpen();
    if (!guardedThis || !guardedDevice || !bBackingOpen) return -1;
    const bool bBackingReadable = guardedDevice->isReadable();
    if (!guardedThis || !guardedDevice || !bBackingReadable) return -1;
    nMaxSize = qMin(nMaxSize, nRangeSize - nPosition);

    // The backing device can be shared by multiple SubDevice instances (and by
    // parsers doing absolute reads).  Its cursor therefore cannot be assumed to
    // still match this view's logical cursor from the constructor or last seek.
    const qint64 nAbsolutePosition = (qint64)getInitLocation() + nPosition;
    const qint64 nBackingPosition = guardedDevice->pos();
    if (!guardedThis || !guardedDevice) return -1;
    if (nBackingPosition != nAbsolutePosition) {
        const bool bPositioned = guardedDevice->seek(nAbsolutePosition);
        if (!guardedThis || !guardedDevice || !bPositioned) return -1;
    }

    const qint64 nResult = guardedDevice->read(pData, nMaxSize);
    return (guardedThis && guardedDevice) ? nResult : -1;
}

qint64 SubDevice::writeData(const char *pData, qint64 nMaxSize)
{
    QPointer<SubDevice> guardedThis(this);
    QPointer<QIODevice> guardedDevice(m_pDevice);
    const qint64 nPosition = pos();
    const qint64 nRangeSize = size();
    if (!isOpen() || !isWritable() || !guardedDevice || (nMaxSize < 0) || ((nMaxSize > 0) && !pData) || (nPosition < 0) || (nPosition > nRangeSize)) return -1;
    const bool bBackingOpen = guardedDevice->isOpen();
    if (!guardedThis || !guardedDevice || !bBackingOpen) return -1;
    const bool bBackingWritable = guardedDevice->isWritable();
    if (!guardedThis || !guardedDevice || !bBackingWritable) return -1;
    nMaxSize = qMin(nMaxSize, nRangeSize - nPosition);

    const qint64 nAbsolutePosition = (qint64)getInitLocation() + nPosition;
    const qint64 nBackingPosition = guardedDevice->pos();
    if (!guardedThis || !guardedDevice) return -1;
    if (nBackingPosition != nAbsolutePosition) {
        const bool bPositioned = guardedDevice->seek(nAbsolutePosition);
        if (!guardedThis || !guardedDevice || !bPositioned) return -1;
    }

    const qint64 nResult = guardedDevice->write(pData, nMaxSize);
    return (guardedThis && guardedDevice) ? nResult : -1;
}
