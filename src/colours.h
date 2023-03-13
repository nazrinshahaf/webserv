#ifndef _COLORS_
#define _COLORS_

/* FOREGROUND */
#define RESET  "\x1B[0m"
#define RED  "\x1B[31m"
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"
#define WHITE  "\x1B[37m"

#define FRED(x) RED x RESET
#define FGRN(x) GREEN x RESET
#define FYEL(x) YELLOW x RESET
#define FBLU(x) BLUE x RESET
#define FMAG(x) MAGENTA x RESET
#define FCYN(x) CYAN x RESET
#define FWHT(x) WHITE x RESET

#define BOLD(x)	"\x1B[1m" x RESET
#define UNDL(x)	"\x1B[4m" x RESET


#endif	/* _COLORS_ */
