#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#include <ctime>

class TimeUtils {
public:
  // 获取当前Unix时间戳（秒）
  static long long getCurrentTimestamp();

  // 时间格式化
  static std::string formatDuration(long long seconds);
  static std::string formatDateTime(long long timestamp);

  // 时间计算
  static long long getOfflineSeconds(long long lastSaveTime);
};

#endif