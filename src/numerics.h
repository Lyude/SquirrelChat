#ifndef __NUMERICS_H__
#define __NUMERICS_H__

#include "irc_network.h"

void init_numerics();
void rpl_isupport(struct irc_network * network,
                  char * hostmask,          
                  short argc,
                  char * argv[],
                  char * trailing);
#endif
