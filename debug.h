#ifndef DEBUG_H
#define DEBUG_H


#ifndef DEBUG
#define DEBUG 0
#endif


#if DEBUG
/*  debug previously just #define'd itself as errmsg but i want to
    allow a high-level message routine (ie msg_log in libpetrifui)
    to #undefine errmsg without damaging debug messages.
 */
#define debug(...)                          \
{                                           \
    fprintf(stderr, "%40s:%5d  %-35s: ",    \
            __FILE__ + SRC_DIR_STRLEN + 1,  \
            __LINE__, __FUNCTION__);        \
    fprintf(stderr, __VA_ARGS__);           \
}
#define dbg(s) puts(s);
#else
#define debug(...)
#define dbg(s)
#endif


#endif
