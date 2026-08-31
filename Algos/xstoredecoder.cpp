/* Copyright (c) 2025-2026 hors<horsicq@gmail.com>
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
#include "xstoredecoder.h"
#include "algo_utils.h"

#include <limits>
#include <new>
#include <new>

XStoreDecoder::XStoreDecoder(QObject *parent) : QObject(parent)
{
}

bool XStoreDecoder::decompress(XBinary::DATAPROCESS_STATE *pDecompressState, XBinary::PDSTRUCT *pPdStruct)
{
    bool bResult = false;

    if (pDecompressState && pDecompressState->pDeviceInput && pDecompressState->pDeviceOutput && (pDecompressState->nInputOffset >= 0) &&
        (pDecompressState->nInputLimit >= -1)) {
        const bool bHasExpectedSize = pDecompressState->mapProperties.contains(XBinary::FPART_PROP_UNCOMPRESSEDSIZE);
        const qint64 nExpectedSize = pDecompressState->mapProperties.value(XBinary::FPART_PROP_UNCOMPRESSEDSIZE).toLongLong();

        // STORE has a one-to-one compressed/output byte mapping.  Reject a
        // bounded member whose declared output size disagrees before writing
        // anything; otherwise a truncated member could be reported as a
        // successful shorter file.
        if (bHasExpectedSize && ((nExpectedSize < 0) || ((pDecompressState->nInputLimit != -1) && (nExpectedSize != pDecompressState->nInputLimit)))) {
            return false;
        }

        if ((pDecompressState->nProcessedOffset < 0) || (pDecompressState->nProcessedLimit < -1) ||
            ((pDecompressState->nProcessedLimit != -1) &&
             (pDecompressState->nProcessedOffset > ((std::numeric_limits<qint64>::max)() - pDecompressState->nProcessedLimit)))) {
            pDecompressState->bWriteError = true;
            return false;
        }

        const qint32 nRequestedBufferSize = XBinary::getBufferSize(pPdStruct);
        if (nRequestedBufferSize <= 0) {
            return false;
        }
        const qint32 _nBufferSize = qBound((qint32)0x1000, nRequestedBufferSize, (qint32)0x100000);

        char *bufferIn = new (std::nothrow) char[_nBufferSize];
        if (!bufferIn) {
            return false;
        }

        Algo_utils::prepareState(pDecompressState);

        // Copy data from input to output
        for (qint64 nOffset = 0; ((pDecompressState->nInputLimit == -1) || (nOffset < pDecompressState->nInputLimit)) && XBinary::isPdStructNotCanceled(pPdStruct);) {
            qint32 nBufferSize = Algo_utils::getReadChunkSize(pDecompressState, _nBufferSize);

            if (nBufferSize <= 0) {
                break;
            }

            qint32 nRead = XBinary::_readDevice(bufferIn, nBufferSize, pDecompressState);

            if (nRead > 0) {
                if (XBinary::_writeDevice(bufferIn, nRead, pDecompressState) != nRead) break;
            } else {
                break;
            }

            if (pDecompressState->bReadError || pDecompressState->bWriteError) {
                break;
            }

            nOffset += nRead;
        }

        const bool bInputComplete = (pDecompressState->nInputLimit == -1) || (pDecompressState->nCountInput == pDecompressState->nInputLimit);
        const bool bOutputSizeMatches = !bHasExpectedSize || (pDecompressState->nCountOutput == nExpectedSize);
        bResult = bInputComplete && bOutputSizeMatches && XBinary::isPdStructNotCanceled(pPdStruct) && !pDecompressState->bReadError && !pDecompressState->bWriteError;

        delete[] bufferIn;
    }

    return bResult;
}
