// EPOS-- Mutex Abstraction Declarations

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#ifndef __mutex_h
#define __mutex_h

#include <common/synchronizer.h>

__BEGIN_SYS

class Mutex: public Synchronizer_Common
{
private:
    typedef Traits<Mutex> Traits;
    static const Type_Id TYPE = Type<Mutex>::TYPE;

public:
    Mutex() : _locked(false) { db<Mutex>(TRC) << "Mutex()\n"; }
    ~Mutex() { db<Mutex>(TRC) << "~Mutex()\n"; 
		wakeup_all();
	}

    void lock() { 
	db<Mutex>(TRC) << "Mutex::lock()\n";
	if(tsl(_locked))
	    sleep();
    }
    void unlock() { 
	db<Mutex>(TRC) << "Mutex::unlock()\n";
	_locked = false;
	wakeup(); 
    }

    static int init(System_Info *si);

private:
    volatile bool _locked;
};

__END_SYS

#endif
