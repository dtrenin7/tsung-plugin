#ifndef SRC_SVP_SETTINGS_H_
#define SRC_SVP_SETTINGS_H_

#include "svp_error.h"
#include <map>

#define CONFIG_FILE "/etc/csv_reader_nif.ini"

namespace SVP {

class settings {

    std::map<str_t, i32_t>  integers;   ///< integer values
    std::map<str_t, str_t>  strings;    ///< string values

    settings();
    settings(const settings&);
    settings& operator=(settings&);

public:
    static cstrptr_t   csv_file_path;

    static settings& instance();

    i32_t get_integer(str_t key);
    str_t get_string(str_t key);
    void save_in_log();
};

};	// namespace SVP

#endif /* SRC_SVP_SETTINGS_H_ */
