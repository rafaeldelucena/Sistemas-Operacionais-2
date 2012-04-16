// EPOS-- Thread Abstraction Implementation

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#include <thread.h>
#include <mmu.h>

__BEGIN_SYS

// Class attributes
Thread * volatile Thread::_running;
Thread::Ordered_Queue Thread::_ready;
Thread::Queue Thread::_suspended;

// Methods
int Thread::join() {
    db<Thread>(TRC) << "Thread::join(this=" << this
		    << ",state=" << _state << ")\n";

    if(Traits::active_scheduler)
	CPU::int_disable();
	
    if(_state != FINISHING)
	{
		Thread * requester = _running;
		_waiting.insert(&requester->_link);
		requester->suspend();
	}

    if(Traits::active_scheduler)
	CPU::int_enable();

    return *((int *)_stack);
}

void Thread::pass() {
    db<Thread>(TRC) << "Thread::pass(this=" << this << ")\n";

    if(Traits::active_scheduler)
	CPU::int_disable();

    Thread * old = _running;
    old->_state = READY;
    _ready.insert(&old->_link_ordered);

    _ready.remove(this);
    _state = RUNNING;
    _running = this;

//     old->_context->save(); // can be used to force an update
    db<Thread>(INF) << "old={" << old << "," 
		    << *old->_context << "}\n";
    db<Thread>(INF) << "new={" << _running << "," 
		    << *_running->_context << "}\n";
	
    CPU::switch_context(&old->_context, _context);

    if(Traits::active_scheduler)
	CPU::int_enable();
}

void  Thread::suspend()
{
    db<Thread>(TRC) << "Thread::suspend(this=" << this << ")\n";

    if(Traits::active_scheduler)
	CPU::int_disable();

    if(_running != this)
	_ready.remove(this);
    _state = SUSPENDED;
    _suspended.insert(&_link);

	_running = _ready.remove()->object();
	_running->_state = RUNNING;

// 	_context->save(); // can be used to force an update
	db<Thread>(INF) << "old={" << this << "," 
			<< *_context << "}\n";
	db<Thread>(INF) << "new={" << _running << "," 
			<< *_running->_context << "}\n";

	CPU::switch_context(&_context, _running->_context);

    if(Traits::active_scheduler)
	CPU::int_enable();
}	    

void  Thread::resume() {
    db<Thread>(TRC) << "Thread::resume(this=" << this << ")\n";

    if(Traits::active_scheduler)
	CPU::int_disable();

    _suspended.remove(this);
    _state = READY;
    _ready.insert(&_link_ordered);

    if(Traits::active_scheduler)
	CPU::int_enable();
}

void Thread::release_waiting()
{
	while(!_waiting.empty())
	{
		_waiting.remove();
	}
}

void Thread::wheres(Queue * wheres)
{
	_wheres = wheres;
}

void Thread::yield() {
    db<Thread>(TRC) << "Thread::yield()\n";
    if(Traits::active_scheduler)
	CPU::int_disable();

	Thread * old = _running;
	old->_state = READY;
	_ready.insert(&old->_link_ordered);
	
	_running = _ready.remove()->object();
	_running->_state = RUNNING;

// 	old->_context->save(); // can be used to force an update
	db<Thread>(INF) << "old={" << old << "," 
			<< *old->_context << "}\n";
	db<Thread>(INF) << "new={" << _running << "," 
			<< *_running->_context << "}\n";

	CPU::switch_context(&old->_context, _running->_context);

	CPU::int_enable();
}

void Thread::exit(int status)
{
    db<Thread>(TRC) << "Thread::exit(status=" << status << ")\n";

    if(Traits::active_scheduler)
	CPU::int_disable();

	_running->release_waiting();

	Thread * old = _running;
	old->_state = FINISHING;
	*((int *)(void *)old->_stack) = status;

	_running = _ready.remove()->object();
	_running->_state = RUNNING;

// 	old->_context->save(); // can be used to force an update
	db<Thread>(INF) << "old={" << old << "," 
			<< *old->_context << "}\n";
	db<Thread>(INF) << "new={" << _running << "," 
			<< *_running->_context << "}\n";

	CPU::switch_context(&old->_context, _running->_context);

	old->freeStack();

    if(Traits::active_scheduler)
	CPU::int_enable();
}

void Thread::idle()
{
    db<Thread>(TRC) << "Thread::idle()\n";

    db<Thread>(WRN) << "There are no runnable threads at the moment!\n";
    db<Thread>(WRN) << "Halting the CPU ...\n";

    CPU::int_enable();
    CPU::halt();
}

void Thread::freeStack()
{
	free(_stack);
}

__END_SYS
