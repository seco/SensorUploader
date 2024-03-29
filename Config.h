#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct Config
{
    char ssid[33];
    char password[65];
    char localhost[129];

    char mserver[129];
    char muser[65];
    char mpassword[65];
    unsigned short mport;
    
    char mprefix[129];

    bool mnjson;
    unsigned int minterval;

    char mfingerprint[129];
    bool mignfingerprint;

    bool debug;

} Config_t;


#endif
