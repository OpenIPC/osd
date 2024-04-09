#ifndef NET_H_
#define NET_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

#include "common.h"

    typedef struct
    {
        char *name, *value;
    } header_t;

    extern char *method,  // "GET" or "POST"
                *uri,     // "/index.html" things before '?'
                *query,   // "a=1&b=2"     things after  '?'
                *prot,    // "HTTP/1.1"
                *payload; // for POST
    extern int payload_size;

    void serve_forever();
    char *extract_key(char **pair);
    char *extract_pair(char **input);
    char *request_header(const char *name);

    void callback();
    void route();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif