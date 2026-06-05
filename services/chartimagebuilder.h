#ifndef CHARTIMAGEBUILDER_H
#define CHARTIMAGEBUILDER_H

#include "../models/analyticsmodels.h"

#include <QByteArray>
#include <QColor>
#include <QImage>
#include <QList>
#include <QString>

/*
 * 图表图片生成工具。
 *
 * 负责：
 * - 将平台统计、互动趋势等分析数据绘制成 PNG 图片；
 * - 为 HTML 报表提供 base64 图片地址；
 * - 为后续图片类报表扩展预留可复用的图表图片能力。
 *
 * 不负责：
 * - 查询数据库；
 * - 生成 HTML 页面结构；
 * - 保存本地文件。
 */
class ChartImageBuilder
{
public:
    static QByteArray buildPlatformPieChartPng(const QList<PlatformStatistics>& stats);

    static QByteArray buildPlatformBarChartPng(const QList<PlatformStatistics>& stats);

    static QByteArray buildDailyTrendChartPng(const QList<DateTrend>& trends);

    static QString pngDataUrl(const QByteArray& pngData);

    static QByteArray imageToPngData(const QImage& image);

private:
    static QColor chartColor(int index);

    static QString numberText(qint64 value);
};

#endif // CHARTIMAGEBUILDER_H