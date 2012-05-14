// EPOS-- Thread Test Program

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#include <utility/ostream.h>
#include <thread.h>

__USING_SYS

const int iterations = 100;

int func_a(void);
int func_b(void);

Thread * a;
Thread * b;
Thread * m;

OStream cout;

int main()
{
    cout << "Thread test\n";

//    m = new Thread(Thread::SELF);

//    cout << "I'm the first thread of the first task created in the system.\n";
//    cout << "I'll now create two threads and then suspend myself ...\n";

    a = new Thread(&func_a);
    b = new Thread(&func_b);

    m->suspend();

    cout << "Both threads are now done and have suspended themselves. I'll now wake them up so they can exit ...\n";

    a->resume();
    b->resume();

    int status_a = a->join();
    int status_b = b->join();

    cout << "Thread A exited with status " << status_a 
	 << " and thread B exited with status " << status_b << "\n";

    delete a;
    delete b;
    delete m;
    
    cout << "I'm also done, bye!\n";

    return 0;
}

int func_a(void)
{
    for(int i = iterations; i > 0; i--) {
	for(int i = 0; i < 79; i++)
	    cout << "a";
	cout << "\n";
	Thread::yield();
    }

//     Thread self(Thread::SELF);
//     self.suspend();

    return 'A';   
}

int func_b(void)
{
    for(int i = iterations; i > 0; i--) {
	for(int i = 0; i < 79; i++)
	    cout << "b";
	cout << "\n";
	Thread::yield();
    }

    m->resume();

//     Thread self(Thread::SELF);
//     self.suspend();

    return 'B';   
}
