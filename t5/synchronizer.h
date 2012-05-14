// EPOS-- Synchronizer Abstractions Common Package

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#ifndef __synchronizer_h
#define __synchronizer_h

#include <system/config.h>
#include <cpu.h>
#include <thread.h>
#include <utility/spin.h>

__BEGIN_SYS

class Synchronizer_Common
{
protected:
    Synchronizer_Common() {}
    
private:
    static const bool busy_waiting = Traits<Thread>::busy_waiting;
	Queue<Thread> _sleeping;
	Spin _lock;

protected:
    // Atomic operations
    bool tsl(volatile bool & lock) { return CPU::tsl(lock); }
    int inc(volatile int & number) { return CPU::finc(number); }
    int dec(volatile int & number) { return CPU::fdec(number); }

    // Thread operations
    void sleep() {
		Thread* running = Thread::running();
		_sleeping.insert(&running->asleep_link());
		running->wheres(&_sleeping);
		if(!busy_waiting) {
			running->suspend();
		}
	}
    void wakeup() {
		if(!busy_waiting) {
    	if(!_sleeping.empty()) {
			Thread* waked = _sleeping.remove()->object();
			waked->wheres(0);
			waked->resume();
		}
		}
	}
    void wakeup_all() {
    	_lock.acquire();
		while(!_sleeping.empty()) {
			Thread* waked = _sleeping.remove()->object();
			waked->wheres(0);
			waked->resume();
		}
    	_lock.release();
	}
};

__END_SYS

#endif
