#ifndef CLOCK_FUNCTIONS_H
#define CLOCK_FUNCTIONS_H

void clock_setup();
void clock_loop_once(int hour, int minutes);
void set_clock(unsigned long unix_time);
String get_clock();


#endif