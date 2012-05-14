// EPOS-- Alarm Abstraction Declarations

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#ifndef __alarm_h
#define __alarm_h

#include <system/config.h>
#include <utility/queue.h>
#include <tsc.h>
#include <rtc.h>
#include <timer.h>
#include <semaphore.h>
#include <thread.h>

__BEGIN_SYS

class Alarm
{
private:
    typedef Traits<Alarm> Traits;
    static const Type_Id TYPE = Type<Alarm>::TYPE;

    static const int FREQUENCY = __SYS(Traits)<Timer>::FREQUENCY;

    typedef Relative_Queue<Alarm> Queue;

    typedef TSC::Hertz Hertz;
    typedef TSC::Time_Stamp Time_Stamp;
    typedef RTC::Microseconds Microseconds;
    typedef RTC::Seconds Seconds;
    typedef Timer::Tick Tick;

public:
    // An alarm handler
    typedef void (* Handler)();

    // Infinite times (for alarms)
    enum { INFINITE = 0 };
	void stop(void) {_semaphore.p();}

public:
    Alarm(const Microseconds & time, const Handler& handler, int times = 1);
    Alarm(const Microseconds & time);
    ~Alarm();
    static void master(const Microseconds & time, const Handler & handler);

    static Hertz frequency() {	return _timer.frequency(); }

    static void delay(const Microseconds & time);

    static int init(System_Info *si);

private:
    static Microseconds period() { return 1000000 / frequency(); }
    static void timer_handler(void);

private:
    Tick _ticks;
    int _times;
    Queue::Element _link;
	Thread* _handler;
	Semaphore _semaphore;
	bool _alarm_delay;

    static Timer _timer;
    static volatile Tick _elapsed;
    static Handler _master;
    static Tick _master_ticks;
    static Queue _requests;
};

__END_SYS

#endif
