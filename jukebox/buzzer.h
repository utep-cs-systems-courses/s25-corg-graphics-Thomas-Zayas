#ifndef buzzer_included
#define buzzer_included
#include <msp430.h>
void buzzer_init();
void buzzer_set_period(short cycles);
void buzz_update();
void update_note(short new);

extern short notesdv[100];
extern short lendv[100];
#endif // included
