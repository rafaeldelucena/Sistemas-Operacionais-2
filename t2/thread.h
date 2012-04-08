// EPOS-- Thread Abstraction Declarations

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#ifndef __thread_h
#define __thread_h

#include <system/config.h>
#include <utility/queue.h>
#include <utility/malloc.h>
#include <cpu.h>
#include <mmu.h>

__BEGIN_SYS

class Thread
{
private:
    typedef Traits<Thread> Traits;
    static const Type_Id TYPE = Type<Thread>::TYPE;

    typedef Queue<Thread> Queue;

    static const unsigned int STACK_SIZE = 
	__SYS(Traits)<Machine>::APPLICATION_STACK_SIZE;

    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Context Context;

public:
    enum Self {SELF};

    typedef short State;
    enum  {
        RUNNING,
        READY,
        SUSPENDED,
	FINISHING
    };

    typedef short Priority;
    enum {
	HIGH = 0,
	NORMAL = 15,
	LOW = 31
    };

public:
    // The int left on the stack between thread's arguments and its context
    // is due to the fact that the thread's function believes it's a normal
    // function that will be invoked with a call, which pushes the return
    // address on the stack
    Thread(int (* entry)(), 
	   const State & state = READY,
	   const Priority & priority = NORMAL,
	   unsigned int stack_size = STACK_SIZE)
	: _stack(malloc(stack_size)), 
	  _context(new (_stack + stack_size
			- sizeof(int) - sizeof(Context))
		   Context(entry)),
	  _state(state),
	  _priority(priority),
	  _link(this)
    {
	header(entry, stack_size);
	Log_Addr sp = _stack + stack_size;
	sp -= sizeof(int); *static_cast<unsigned int *>(sp) = 
			       reinterpret_cast<unsigned int>(&exit);
	body();
    }
    template<class T1>
    Thread(int (* entry)(T1 a1), T1 a1,
	   const State & state = READY,
	   const Priority & priority = NORMAL,
	   unsigned int stack_size = STACK_SIZE)
	: _stack(malloc(stack_size)), 
	  _context(new (_stack + stack_size - sizeof(T1)
			- sizeof(int) - sizeof(Context))
		   Context(entry)),
	  _state(state),
	  _priority(priority),
	  _link(this)
    {
	header(entry, stack_size);
	Log_Addr sp = _stack + stack_size;
	sp -= sizeof(T1); *static_cast<T1 *>(sp) = a1;
	sp -= sizeof(int); *static_cast<unsigned int *>(sp) = 
			       reinterpret_cast<unsigned int>(&exit);
	body();
    }
    template<class T1, class T2>
    Thread(int (* entry)(T1 a1, T2 a2), T1 a1, T2 a2,
	   const State & state = READY,
	   const Priority & priority = NORMAL,
	   unsigned int stack_size = STACK_SIZE)
	: _stack(malloc(stack_size)), 
	  _context(new (_stack + stack_size - sizeof(T2) - sizeof(T1)
			- sizeof(int) - sizeof(Context))
		   Context(entry)),
	  _state(state),
	  _priority(priority),
	  _link(this)
    {
	header(entry, stack_size);
	Log_Addr sp = _stack + stack_size;
	sp -= sizeof(T2); *static_cast<T2 *>(sp) = a2;
	sp -= sizeof(T1); *static_cast<T1 *>(sp) = a1;
	sp -= sizeof(int); *static_cast<unsigned int *>(sp) = 
			       reinterpret_cast<unsigned int>(&exit);
	body();
    }
    template<class T1, class T2, class T3>
    Thread(int (* entry)(T1 a1, T2 a2, T3 a3), T1 a1, T2 a2, T3 a3,
	   const State & state = READY,
	   const Priority & priority = NORMAL,
	   unsigned int stack_size = STACK_SIZE)
	: _stack(malloc(stack_size)), 
	  _context(new (_stack + stack_size - sizeof(T3) - sizeof(T2) 
			- sizeof(T1) - sizeof(int) - sizeof(Context))
		   Context(entry)),
	  _state(state),
	  _priority(priority),
	  _link(this)
    {
	header(entry, stack_size);
	Log_Addr sp = _stack + stack_size;
	sp -= sizeof(T3); *static_cast<T3 *>(sp) = a3;
	sp -= sizeof(T2); *static_cast<T2 *>(sp) = a2;
	sp -= sizeof(T1); *static_cast<T1 *>(sp) = a1;
	sp -= sizeof(int); *static_cast<unsigned int *>(sp) = 
			       reinterpret_cast<unsigned int>(&implicit_exit);
	body();
    }
    ~Thread() {
	_ready.remove(this);
	_suspended.remove(this);
	_waiting.remove(this);
	free(_stack);
    }

    volatile const State & state() const { return _state; }
    volatile const Priority & priority() const { return _priority; }
    void priority(const Priority & priority);

    int join();
    void pass();
    void suspend();
    void resume();

    static void yield();
    static void exit(int status = 0);

    static Thread * volatile  & running() { return _running; }
    static void running(Thread * r) { _running = r; }

    static int init(System_Info * si);

private:
    void header(Log_Addr entry, unsigned int stack_size) {
	db<Thread>(TRC) << "Thread(this=" << this 
			<< ",entry=" << (void *)entry 
			<< ",state=" << _state
			<< ",priority=" << _priority
			<< ",stack={b=" << _stack
			<< ",s=" << stack_size
			<< ",context={b=" << _context
			<< "," << *_context << "})\n";
    }
    void body() {
	if(Traits::active_scheduler)
	    CPU::int_disable();

	switch(_state) {
	case RUNNING: break;
	case SUSPENDED: _suspended.insert(&_link); break;
	default: _ready.insert(&_link);
	}

	if(Traits::active_scheduler)
	    CPU::int_enable();
    }

    static void implicit_exit() { exit(CPU::fr()); }
    static void reschedule() { yield(); }
    static void idle();

private:
    Log_Addr _stack;
    Context * volatile _context;
    volatile State _state;
    volatile Priority _priority;
    Queue::Element _link;

    static Thread * volatile _running;
    static Queue _ready;
    static Queue _suspended;
    static Queue _waiting;
};

__END_SYS

#endif