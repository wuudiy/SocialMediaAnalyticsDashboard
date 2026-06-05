#include "chartimagebuilder.h"

#include <QBuffer>
#include <QFont>
#include <QFontMetrics>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRectF>

#include <algorithm>

/*
 * 根据下标返回图表颜色。
 *
 * 保证 HTML 导出中的多张图表视觉风格一致。
 */
QColor ChartImageBuilder::chartColor(int index)
{
    static const QList<QColor> colors = {
        QColor(QStringLiteral("#2563EB")),
        QColor(QStringLiteral("#10B981")),
        QColor(QStringLiteral("#F59E0B")),
        QColor(QStringLiteral("#EF4444")),
        QColor(QStringLiteral("#8B5CF6")),
        QColor(QStringLiteral("#06B6D4")),
        QColor(QStringLiteral("#EC4899")),
        QColor(QStringLiteral("#84CC16"))
    };

    return colors.at(index % colors.size());
}

/*
 * 数字格式化。
 *
 * 用英文区域格式化数字，方便生成 1,000 这种形式。
 */
QString ChartImageBuilder::numberText(qint64 value)
{
    return QLocale(QLocale::English, QLocale::UnitedStates).toString(value);
}

/*
 * 将 QImage 转换为 PNG 二进制数据。
 *
 * HTML 导出会继续将该二进制数据转成 base64；
 * 后续如果恢复图片类导出，也可以直接复用该二进制数据。
 */
QByteArray ChartImageBuilder::imageToPngData(const QImage& image)
{
    QByteArray data;
    QBuffer buffer(&data);

    if (!buffer.open(QIODevice::WriteOnly)) {
        return data;
    }

    image.save(&buffer, "PNG");
    return data;
}

/*
 * 将 PNG 二进制数据转换为 HTML 可直接使用的 data url。
 *
 * 这样导出的 HTML 文件不依赖外部图片文件，
 * 双击打开 HTML 即可完整显示图表。
 */
QString ChartImageBuilder::pngDataUrl(const QByteArray& pngData)
{
    return QStringLiteral("data:image/png;base64,%1")
    .arg(QString::fromLatin1(pngData.toBase64()));
}

/*
 * 生成平台帖子占比饼图。
 *
 * 输入：
 * - PlatformStatistics 列表。
 *
 * 输出：
 * - PNG 二进制数据。
 */
QByteArray ChartImageBuilder::buildPlatformPieChartPng(
    const QList<PlatformStatistics>& stats)
{
    QImage image(QSize(820, 420), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(QStringLiteral("#FFFFFF")));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QFont titleFont(QStringLiteral("Microsoft YaHei"), 15, QFont::Bold);
    const QFont textFont(QStringLiteral("Microsoft YaHei"), 10);

    const QColor titleColor(QStringLiteral("#111827"));
    const QColor textColor(QStringLiteral("#374151"));
    const QColor mutedColor(QStringLiteral("#6B7280"));

    painter.setFont(titleFont);
    painter.setPen(titleColor);
    painter.drawText(QRectF(30, 18, 760, 32),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Platform Post Share"));

    int totalPosts = 0;

    for (const PlatformStatistics& item : stats) {
        totalPosts += item.postCount;
    }

    if (stats.isEmpty() || totalPosts <= 0) {
        painter.setFont(textFont);
        painter.setPen(mutedColor);
        painter.drawText(QRectF(30, 170, 760, 40),
                         Qt::AlignCenter,
                         QStringLiteral("No platform data available."));
        return imageToPngData(image);
    }

    const QRectF pieRect(56, 74, 270, 270);
    int startAngle = 90 * 16;
    int usedAngle = 0;

    for (int i = 0; i < stats.size(); ++i) {
        const PlatformStatistics& item = stats.at(i);

        int spanAngle = 0;

        /*
         * 最后一块直接补足剩余角度，
         * 避免四舍五入导致饼图出现细小缺口。
         */
        if (i == stats.size() - 1) {
            spanAngle = 360 * 16 - usedAngle;
        } else {
            spanAngle = qRound(360.0 * item.postCount / totalPosts * 16.0);
            usedAngle += spanAngle;
        }

        painter.setPen(QPen(QColor(QStringLiteral("#FFFFFF")), 2));
        painter.setBrush(chartColor(i));
        painter.drawPie(pieRect, startAngle, spanAngle);

        startAngle += spanAngle;
    }

    painter.setFont(textFont);

    int legendY = 82;

    for (int i = 0; i < stats.size(); ++i) {
        const PlatformStatistics& item = stats.at(i);
        const double share = static_cast<double>(item.postCount) / totalPosts * 100.0;

        painter.setPen(Qt::NoPen);
        painter.setBrush(chartColor(i));
        painter.drawRoundedRect(QRectF(380, legendY + 4, 14, 14), 3, 3);

        painter.setPen(textColor);
        painter.drawText(QRectF(405, legendY, 350, 24),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("%1  %2 posts  %3%")
                             .arg(item.platform)
                             .arg(numberText(item.postCount))
                             .arg(share, 0, 'f', 2));

        legendY += 32;
    }

    return imageToPngData(image);
}

/*
 * 生成平台互动量柱状图。
 */
QByteArray ChartImageBuilder::buildPlatformBarChartPng(
    const QList<PlatformStatistics>& stats)
{
    QImage image(QSize(820, 420), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(QStringLiteral("#FFFFFF")));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QFont titleFont(QStringLiteral("Microsoft YaHei"), 15, QFont::Bold);
    const QFont textFont(QStringLiteral("Microsoft YaHei"), 9);

    const QColor titleColor(QStringLiteral("#111827"));
    const QColor axisColor(QStringLiteral("#9CA3AF"));
    const QColor textColor(QStringLiteral("#374151"));
    const QColor mutedColor(QStringLiteral("#6B7280"));

    painter.setFont(titleFont);
    painter.setPen(titleColor);
    painter.drawText(QRectF(30, 18, 760, 32),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Platform Interactions"));

    if (stats.isEmpty()) {
        painter.setFont(textFont);
        painter.setPen(mutedColor);
        painter.drawText(QRectF(30, 170, 760, 40),
                         Qt::AlignCenter,
                         QStringLiteral("No platform data available."));
        return imageToPngData(image);
    }

    int maxValue = 0;

    for (const PlatformStatistics& item : stats) {
        maxValue = std::max(maxValue, item.totalInteractions);
    }

    if (maxValue <= 0) {
        maxValue = 1;
    }

    const QRectF chartRect(76, 64, 680, 270);
    const int gridCount = 4;

    painter.setPen(QPen(QColor(QStringLiteral("#E5E7EB")), 1));

    for (int i = 0; i <= gridCount; ++i) {
        const qreal y = chartRect.bottom() - chartRect.height() * i / gridCount;
        painter.drawLine(QPointF(chartRect.left(), y),
                         QPointF(chartRect.right(), y));
    }

    painter.setPen(QPen(axisColor, 1));
    painter.drawLine(chartRect.bottomLeft(), chartRect.bottomRight());
    painter.drawLine(chartRect.bottomLeft(), chartRect.topLeft());

    const int count = stats.size();
    const qreal slotWidth = chartRect.width() / count;
    const qreal barWidth = std::min<qreal>(70.0, slotWidth * 0.55);

    painter.setFont(textFont);

    for (int i = 0; i < count; ++i) {
        const PlatformStatistics& item = stats.at(i);

        const qreal ratio = static_cast<qreal>(item.totalInteractions) / maxValue;
        const qreal barHeight = chartRect.height() * ratio;
        const qreal x = chartRect.left() + slotWidth * i + (slotWidth - barWidth) / 2.0;
        const qreal y = chartRect.bottom() - barHeight;

        painter.setPen(Qt::NoPen);
        painter.setBrush(chartColor(i));
        painter.drawRoundedRect(QRectF(x, y, barWidth, barHeight), 5, 5);

        painter.setPen(textColor);
        painter.drawText(QRectF(x - 18, y - 22, barWidth + 36, 18),
                         Qt::AlignCenter,
                         numberText(item.totalInteractions));

        painter.drawText(QRectF(chartRect.left() + slotWidth * i,
                                chartRect.bottom() + 8,
                                slotWidth,
                                36),
                         Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                         item.platform);
    }

    return imageToPngData(image);
}

/*
 * 生成每日互动趋势折线图。
 */
QByteArray ChartImageBuilder::buildDailyTrendChartPng(
    const QList<DateTrend>& trends)
{
    QImage image(QSize(1200, 420), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(QStringLiteral("#FFFFFF")));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QFont titleFont(QStringLiteral("Microsoft YaHei"), 15, QFont::Bold);
    const QFont textFont(QStringLiteral("Microsoft YaHei"), 9);

    const QColor titleColor(QStringLiteral("#111827"));
    const QColor axisColor(QStringLiteral("#9CA3AF"));
    const QColor lineColor(QStringLiteral("#2563EB"));
    const QColor pointColor(QStringLiteral("#10B981"));
    const QColor textColor(QStringLiteral("#374151"));
    const QColor mutedColor(QStringLiteral("#6B7280"));

    painter.setFont(titleFont);
    painter.setPen(titleColor);
    painter.drawText(QRectF(30, 18, 1140, 32),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Interaction Trend"));

    if (trends.isEmpty()) {
        painter.setFont(textFont);
        painter.setPen(mutedColor);
        painter.drawText(QRectF(30, 170, 1140, 40),
                         Qt::AlignCenter,
                         QStringLiteral("No trend data available."));
        return imageToPngData(image);
    }

    int maxValue = 0;

    for (const DateTrend& trend : trends) {
        maxValue = std::max(maxValue, trend.totalInteractions);
    }

    if (maxValue <= 0) {
        maxValue = 1;
    }

    const QRectF chartRect(78, 64, 1064, 270);
    const int gridCount = 4;

    painter.setFont(textFont);

    for (int i = 0; i <= gridCount; ++i) {
        const qreal y = chartRect.bottom() - chartRect.height() * i / gridCount;

        painter.setPen(QPen(QColor(QStringLiteral("#E5E7EB")), 1));
        painter.drawLine(QPointF(chartRect.left(), y),
                         QPointF(chartRect.right(), y));

        painter.setPen(textColor);
        painter.drawText(QRectF(10, y - 9, 58, 18),
                         Qt::AlignRight | Qt::AlignVCenter,
                         numberText(maxValue * i / gridCount));
    }

    painter.setPen(QPen(axisColor, 1));
    painter.drawLine(chartRect.bottomLeft(), chartRect.bottomRight());
    painter.drawLine(chartRect.bottomLeft(), chartRect.topLeft());

    QPainterPath path;

    for (int i = 0; i < trends.size(); ++i) {
        const DateTrend& trend = trends.at(i);

        const qreal x = trends.size() == 1
                            ? chartRect.center().x()
                            : chartRect.left() + chartRect.width() * i / (trends.size() - 1);

        const qreal ratio = static_cast<qreal>(trend.totalInteractions) / maxValue;
        const qreal y = chartRect.bottom() - chartRect.height() * ratio;

        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }

    painter.setPen(QPen(lineColor, 3));
    painter.drawPath(path);

    painter.setPen(Qt::NoPen);
    painter.setBrush(pointColor);

    const int trendCount = static_cast<int>(trends.size());
    const int labelStep = std::max(1, trendCount / 6);

    for (int i = 0; i < trendCount; ++i) {
        const DateTrend& trend = trends.at(i);

        const qreal x = trendCount == 1
                            ? chartRect.center().x()
                            : chartRect.left() + chartRect.width() * i / (trendCount - 1);

        const qreal ratio = static_cast<qreal>(trend.totalInteractions) / maxValue;
        const qreal y = chartRect.bottom() - chartRect.height() * ratio;

        painter.drawEllipse(QPointF(x, y), 5, 5);

        if (i % labelStep == 0 || i == trendCount - 1) {
            painter.setPen(textColor);
            painter.drawText(QRectF(x - 45, chartRect.bottom() + 8, 90, 22),
                             Qt::AlignCenter,
                             trend.date.toString(QStringLiteral("MM-dd")));
            painter.setPen(Qt::NoPen);
        }
    }

    return imageToPngData(image);
}