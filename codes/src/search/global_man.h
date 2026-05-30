#ifndef GLOBAL_MAN_H
#define GLOBAL_MAN_H

#include "policy-repair/regression.h"

class global_man
{
public:
    global_man();
};


extern list<std::shared_ptr<PolicyItem>>  g_man_fsap; // Flag for debugging parts of the code
extern list<std::shared_ptr<PolicyItem>>  g_fixed_out;

#endif // GLOBAL_MAN_H
