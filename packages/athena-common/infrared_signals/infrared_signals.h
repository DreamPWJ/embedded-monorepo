#ifndef EMBEDDED_MONOREPO_INFRARED_SIGNALS_H
#define EMBEDDED_MONOREPO_INFRARED_SIGNALS_H

/**
* @author 潘维吉
* @date 2022/9/19 9:50
* @description 红外信号发送接受数据
*/

int RECV_PIN = 6;

class infrared_signals {

public:

    void init(void);

    void get_data(void);

};


#endif
