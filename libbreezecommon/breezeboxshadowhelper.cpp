/*
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "breezeboxshadowhelper.h"
#include "config-breezecommon.h"

#include <QVector>

#include <cmath>


namespace Breeze {
namespace BoxShadowHelper {

namespace {
// According to the CSS Level 3 spec, standard deviation must be equal to
// half of the blur radius. https://www.w3.org/TR/css-backgrounds-3/#shadow-blur
// Current window size is too small for sigma equal to half of the blur radius.
// As a workaround, sigma blur scale is lowered. With the lowered sigma
// blur scale, area under the kernel equals to 0.98, which is pretty enough.
// Maybe, it should be changed in the future.
const qreal BLUR_SIGMA_SCALE = 0.4375;
}

inline qreal radiusToSigma(qreal radius)
{
    return radius * BLUR_SIGMA_SCALE;
}

inline int boxSizeToRadius(int boxSize)
{
    return (boxSize - 1) / 2;
}

class BoxBlurProfile
{
public:
    BoxBlurProfile(int radius, int passes = 3);

    int padding() const;
    QVector<int> boxSizes() const;

private:
    int m_padding;
    QVector<int> m_boxSizes;
};

BoxBlurProfile::BoxBlurProfile(int radius, int passes)
{
    const qreal sigma = radiusToSigma(radius);

    // Box sizes are computed according to the "Fast Almost-Gaussian Filtering"
    // paper, see http://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf
    int lower = std::floor(std::sqrt(12 * std::pow(sigma, 2) / passes + 1));
    if (lower % 2 == 0) {
        lower--;
    }
    const int upper = lower + 2;

    const int threshold = std::round(
        (12 * std::pow(sigma, 2)
            - passes * std::pow(lower, 2)
            - 4 * passes * lower
            - 3 * passes)
        / (-4 * lower - 4));

    m_padding = radius;
    for (int i = 0; i < passes; ++i) {
        m_boxSizes.append(i < threshold ? lower : upper);
    }
}

int BoxBlurProfile::padding() const
{
    return m_padding;
}

QVector<int> BoxBlurProfile::boxSizes() const
{
    return m_boxSizes;
}

void boxBlurPass(const QImage &src, QImage &dst, int boxSize)
{
    const int alphaStride = src.depth() >> 3;
    const int alphaOffset = QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3;

    const int radius = boxSizeToRadius(boxSize);
    const qreal invSize = 1.0 / boxSize;

    const int dstStride = dst.width() * alphaStride;

    for (int y = 0; y < src.height(); ++y) {
        const uchar *srcAlpha = src.scanLine(y);
        uchar *dstAlpha = dst.scanLine(0);

        srcAlpha += alphaOffset;
        dstAlpha += alphaOffset + y * alphaStride;

        const uchar *left = srcAlpha;
        const uchar *right = left + alphaStride * radius;

        int window = 0;
        for (int x = 0; x < radius; ++x) {
            window += *srcAlpha;
            srcAlpha += alphaStride;
        }

        for (int x = 0; x <= radius; ++x) {
            window += *right;
            right += alphaStride;
            *dstAlpha = static_cast<uchar>(window * invSize);
            dstAlpha += dstStride;
        }

        for (int x = radius + 1; x < src.width() - radius; ++x) {
            window += *right - *left;
            left += alphaStride;
            right += alphaStride;
            *dstAlpha = static_cast<uchar>(window * invSize);
            dstAlpha += dstStride;
        }

        for (int x = src.width() - radius; x < src.width(); ++x) {
            window -= *left;
            left += alphaStride;
            *dstAlpha = static_cast<uchar>(window * invSize);
            dstAlpha += dstStride;
        }
    }
}

void boxBlurAlpha(QImage &image, const BoxBlurProfile &profile)
{
    // Temporary buffer is transposed so we always read memory
    // in linear order.
    QImage tmp(image.height(), image.width(), image.format());

    const auto boxSizes = profile.boxSizes();
    for (const int &boxSize : boxSizes) {
        boxBlurPass(image, tmp, boxSize); // horizontal pass
        boxBlurPass(tmp, image, boxSize); // vertical pass
    }
}

void boxShadow(QPainter *p, const QRect &box, const QPoint &offset,
               int radius, const QColor &color)
{
#if BREEZE_COMMON_USE_KDE4
    const qreal dpr = 1.0;
#else
    const qreal dpr = p->device()->devicePixelRatioF();
#endif
    const BoxBlurProfile profile(radius * dpr, 3);
    const QSize size = box.size() + 2 * QSize(profile.padding(), profile.padding());

    QImage shadow(size * dpr, QImage::Format_ARGB32_Premultiplied);
#if !BREEZE_COMMON_USE_KDE4
    shadow.setDevicePixelRatio(dpr);
#endif
    shadow.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&shadow);
    painter.fillRect(QRect(QPoint(profile.padding(), profile.padding()), box.size()), Qt::black);
    painter.end();

    // There is no need to blur RGB channels. Blur the alpha
    // channel and do compositing stuff later.
    boxBlurAlpha(shadow, profile);

    painter.begin(&shadow);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(shadow.rect(), color);
    painter.end();

    QRect shadowRect = shadow.rect();
    shadowRect.setSize(shadowRect.size() / dpr);
    shadowRect.moveCenter(box.center() + offset);
    p->drawImage(shadowRect, shadow);
}

} // BoxShadowHelper
} // Breeze
