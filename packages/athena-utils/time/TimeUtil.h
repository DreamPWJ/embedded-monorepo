#include <ctime>
#include <stdint.h>
#include <iostream>
#include <string>
#include <sys/time.h>

using std::string;

/**
* @author 潘维吉
* @date 2022/10/16 10:54
* @description 时间工具类
*/
class TimeUtil {

public:
    /**
     * 获取当前系统时间(精度秒），YY年 mm月 dd日 HH时 MM分 SS秒
     * 调用示例: TimeUtil::getDateTime()
     */
    static inline string getDateTime() {
        time_t now = time(0);
        tm *ltm = localtime(&now);

        char myDate[40], myTime[40], ims[10];
        strftime(myDate, 40, "%Y-%m-%d", ltm);
        strftime(myTime, 40, " %H:%M:%S", ltm);

        return string(myDate) + string(myTime);
    }

    /**
     * 获取当前系统时间(精确到纳秒）tv_nsec精确到纳秒（编译时加上-lrt）
     * */
    static inline int64_t getTimeNs() {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return ts.tv_sec * 1000000000 + ts.tv_nsec;
    }

    /**
     * 获取当前系统时间(精度微秒）tv_usec精确到微秒
     * */
    static inline int64_t getTimeUs() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    /**
     * 获取当前系统时间(精度毫秒）tv_usec精确到微秒
     * */
    static inline int64_t getTimeMs() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    /**
     * 根据纳秒时间返回 交易所时间格式：HHMMSSsss，其中LocalTime精确到纳秒
     **/
    static int getLocalTime_t(int64_t LocalTime) {
        time_t local_time = LocalTime / 1000000000;
        struct tm *tm;
        tm = localtime(&local_time);
        int time_ = 0;
        if (LocalTime / 1000000000 > 0) {
            time_ = (tm->tm_hour * 10000 + tm->tm_min * 100 + tm->tm_sec) * 1000 + LocalTime % 1000000000 / 1000000;
        } else {
            time_ = LocalTime % 1000000000 / 1000000;
        }
        return time_;    //精度毫秒
    }
};