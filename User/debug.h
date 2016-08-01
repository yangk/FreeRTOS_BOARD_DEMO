#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "string.h"
// ==== OSStatus ====
typedef int32_t         OSStatus;

#define kNoErr                      0       //! No error occurred.
#define kGeneralErr                -1       //! General error.
#define kInProgressErr              1       //! Operation in progress.

#if( !defined( unlikely ) )
   #define unlikely( EXPRESSSION )     !!(EXPRESSSION)
#endif

#define SHORT_FILE strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__
#define YesOrNo(x) (x ? "YES" : "NO")

#if TRACE
    #define custom_log_trace(N) do {printf("[%s: [TRACE] %s] %s()\r\n", N, SHORT_FILE, __PRETTY_FUNCTION__);}while(0==1)
#else  
    #define custom_log_trace(N)
#endif

#define custom_log(N, M, ...) do {printf("[%s: %s:%4d] " M "\r\n",  N, SHORT_FILE, __LINE__, ##__VA_ARGS__);}while(0==1)

#define debug_print_assert(A,B,C,D,E,F) do {printf("[RTOS:%s:%s:%4d] **ASSERT** %s""\r\n", D, F, E, (C!=NULL) ? C : "" );}while(0==1)

#if( !defined( require_action ) )
    #define require_action( X, LABEL, ACTION )                                                              \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );                 \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif
//---------------------------------------------------------------------------------------------
#define UIF_MAX_ARGS    (10)
#define UIF_MAX_LINE    (0x40)
//---------------------------------------------------------------------------------------------
typedef const struct
{
    char *cmd;
    int max_args;
    uint32_t flags;
    int (*func) (int, char **);
    char *help;
    char *usage;
} uif_cmd_t;
//---------------------------------------------------------------------------------------------
void debug_port_task_handler(void *pvParameters);
//---------------------------------------------------------------------------------------------
#endif
