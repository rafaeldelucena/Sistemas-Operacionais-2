// EPOS-- Alarm Abstraction Implementation

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#include <alarm.h>
#include <display.h>

__BEGIN_SYS

// Class attributes
Timer Alarm::_timer;
volatile Alarm::Tick Alarm::_elapsed;
Alarm::Handler Alarm::_master;
Alarm::Tick Alarm::_master_ticks;
Alarm::Queue Alarm::_requests;

// Methods
Alarm::Alarm(const Microseconds & time, const Handler& handler, int times)
    : _ticks((time + period() / 2) / period()), _handler(new Thread(reinterpret_cast<int (*) ()>(&handler))),
      _times(times), _link(this), _semaphore(0), _alarm_delay(false)
{
    db<Alarm>(TRC) << "Alarm(t=" << time << ",h=" << (void *)handler
		   << ",x=" << times << ")\n";
    if(_ticks) {
		_requests.insert(&_link);
		_handler->suspend();
	}
    else
		_handler->resume();
}

Alarm::Alarm(const Microseconds & time)
    : _ticks((time + period() / 2) / period()), _handler(0),
      _times(1), _link(this), _semaphore(0), _alarm_delay(true)
{
    if(_ticks)
		_requests.insert(&_link);
    else
		_semaphore.v();
}

Alarm::~Alarm() {
    db<Alarm>(TRC) << "~Alarm()\n";
    _requests.remove(this);
	delete _handler;
}

void Alarm::master(const Microseconds & time, const Handler & handler)
{
    db<Alarm>(TRC) << "master(t=" << time << ",h="
		   << (void *)handler << ")\n";

    _master = handler;
    _master_ticks = (time + period() / 2) / period();
}

void Alarm::delay(const Microseconds & time)
{
    db<Alarm>(TRC) << "delay(t=" << time << ")\n";
	Alarm alarm(time);
	alarm.stop();
}

void Alarm::timer_handler(void)
{
    static Tick next;
    static Thread* handler;
	static Semaphore* semaphore;
	static bool alarm_delay;

    _elapsed++;
    
    if(Traits::visible) {
	Display display;
	int lin, col;
	display.position(&lin, &col);
	display.position(0, 79);
	display.putc(_elapsed);
	display.position(lin, col);
    }

    if(_master_ticks) {
	if(!(_elapsed % _master_ticks))
	    _master();
    }

    if(next)
	next--;
    if(!next) {
		if(alarm_delay) {
			if(semaphore) {
				semaphore->v();
			}
		}
		if(_requests.empty()) {
			handler = 0;
			semaphore = 0;
		}
		else {
			Queue::Element * e = _requests.remove();
			Alarm * alarm = e->object();
			next = alarm->_ticks;
			handler = alarm->_handler;
			semaphore = &(alarm->_semaphore);
		
			if(alarm->_times != -1)
				alarm->_times--;
			if(alarm->_times) {
				e->rank(alarm->_ticks);
				_requests.insert(e);
			}
		}
	}
}

__END_SYS
