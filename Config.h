#ifndef _CONFIG_H_INCLUDED
#define _CONFIG_H_INCLUDED

typedef struct Config
{
    char ssid[33];
    char password[65];
    char localhost[129];

    char mserver[129];
    char mpassword[65];
    char muser[65];
    unsigned short mport;

} Config_t;


#endif
