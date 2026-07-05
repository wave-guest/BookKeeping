#pragma once

#include <QString>
#include <string>

// 纯业务模型：只定义数据结构，无任何数据库相关代码
struct TradeRecord {

    QString trade_time;               // 交易时间
    QString trade_category;           // 交易分类
    QString trade_type;               // 收支类型：收入/支出/不计收支
    double amount = 0.0;              // 交易金额
    QString goods_desc;               // 商品说明
    QString remark;                   // 备注
    QString source = "manual";        // 数据来源：manual/alipay
    QString from;                     // 资金来源
    QString to;                       // 资金去向
    QString id;                         // 流水编号
};

enum class TimeRange {
    Total,    // 总计
    Year,     // 年
    Month,    // 月
    Day       // 日
};

struct Statistics {
    double income = 0;
    double expense = 0;
    double profit = 0;

    // 便捷方法
    QString toString() const {
        return QString("收入:¥%1 支出:¥%2 利润:¥%3")
            .arg(income, 0, 'f', 2)
            .arg(expense, 0, 'f', 2)
            .arg(profit, 0, 'f', 2);
    }
};