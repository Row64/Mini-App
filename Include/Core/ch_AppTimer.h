#ifndef CH_APPTIMER_H
#define CH_APPTIMER_H

#include <iostream>
#include <string>
#include <chrono>

struct AppTimer {

    std::chrono::high_resolution_clock::time_point      StartTime;

    void   reset() { StartTime = std::chrono::high_resolution_clock::now(); }
    double elapse() {
        auto now  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - StartTime;
        reset();
        return elapsed.count();
    }
    double elapseNR() {     // no reset
        auto now  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - StartTime;
        return elapsed.count();
    }
    double elapse_millisecond() {
        auto now  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - StartTime;
        reset();
        return elapsed.count();
    }
    double elapse_millisecondNR() {     // no reset
        auto now  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - StartTime;
        return elapsed.count();
    }
    
    AppTimer() { reset(); }
    ~AppTimer() = default;

};



#endif /* CH_APPTIMER_H */
