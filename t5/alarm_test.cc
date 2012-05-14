// EPOS-- Alarm Abstraction Test Program

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#include <utility/ostream.h>
#include <alarm.h>

__USING_SYS

const int iterations = 100;

void func_a(void);
void func_b(void);

OStream cout;

int main()
{
    cout << "Alarm test\n";

    cout << "I'm the first thread of the first task created in the system.\n";
    cout << "I'll now create two alarms and put myself in a delay ...\n";

    Alarm alarm_a(1000000, &func_a, iterations);
    Alarm alarm_b(1000000, &func_b, iterations);

    Alarm::delay(1000000 * (iterations * 2 + 1));

    cout << "I'm done, bye!\n";

    return 0;
}

void func_a()
{
    for(int i = 0; i < 79; i++)
	cout << "a";
    cout << "\n";
}

void func_b(void)
{
    for(int i = 0; i < 79; i++)
	cout << "b";
    cout << "\n";
}
