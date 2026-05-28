#ifndef PLATFORMCONSTANTS_H
#define PLATFORMCONSTANTS_H

#include <QString>
#include <QStringList>

/*
 * 平台名称统一放在这里，避免不同页面出现 WeChat / Wechat 这种大小写不一致。
 *
 * 当前项目已有 posts 数据使用：
 * - Weibo
 * - Douyin
 * - Bilibili
 * - Xiaohongshu
 * - Wechat
 *
 * 所以后续所有页面、导入、导出、统计都应该从这里取平台名称。
 */
namespace PlatformConstants
{
inline const QString Weibo = QStringLiteral("Weibo");
inline const QString Douyin = QStringLiteral("Douyin");
inline const QString Bilibili = QStringLiteral("Bilibili");
inline const QString Xiaohongshu = QStringLiteral("Xiaohongshu");
inline const QString Wechat = QStringLiteral("Wechat");

/*
 * 返回系统支持的平台列表。
 *
 * 注意：
 * 这里不放 “All Platforms”，因为 “全部平台” 只是页面筛选用的显示项，
 * 不应该作为真实平台名称写入 posts 表。
 */
inline QStringList availablePlatforms()
{
    return {
        Weibo,
        Douyin,
        Bilibili,
        Xiaohongshu,
        Wechat
    };
}
}

#endif // PLATFORMCONSTANTS_H
