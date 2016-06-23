#ifndef _CONFIG_H_INCLUDED
#define _CONFIG_H_INCLUDED

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

    char mswitchtopic[65];
    char mswitchmsgon[65];


    char mstatustopic[65];
    char mstatusmsg[65];

    char mswitchmsgoff[65];

    bool debug;
} Config_t;


#endif
