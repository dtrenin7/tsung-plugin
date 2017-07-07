#ifndef SRC_SVP_INI_H_
#define SRC_SVP_INI_H_

#include "svp_error.h"
#include <iostream>
#include <fstream>

namespace SVP {

class ini {

    std::ifstream*  file;   ///< file stream

public:
    ini();
    virtual ~ini();

    error_t open(cstrptr_t name);
    error_t read(const str_t& section, const str_t& key, str_t& value);
};

};	// namespace SVP

#endif /* SRC_SVP_INI_H_ */
