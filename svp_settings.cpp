#include <stdio.h>
#include <stdlib.h>
#include "svp_settings.h"
#include "svp_ini.h"
#include "svp_log.h"
#include "svp_utils.h"

namespace SVP {

cstrptr_t settings::csv_file_path      = "CSV_FILE_PATH";

#define READ_STRING(key, ini_section, ini_key) { conf_value.clear();\
    if( config.read(str_t(ini_section), str_t(ini_key), conf_value) == E_OK )\
        strings[key] = conf_value;\
    value = getenv(key); if( value ) strings[key] = value;\
    SVP_INFO("SETTINGS(%s) = %s", key, strings[key].c_str()); }

#define READ_INTEGER(key, ini_section, ini_key) { conf_value.clear();\
    if( config.read(str_t(ini_section), str_t(ini_key), conf_value) == E_OK )\
        integers[key] = atoi(conf_value.c_str());\
    value = getenv(key); if( value ) integers[key] = atoi(value);\
    SVP_INFO("SETTINGS(%s) = %d", key, integers[key]); }

settings::settings() {

    error_t error;
    strptr_t value;
    str_t conf_value;

    ini config;
    THROW(config.open(CONFIG_FILE));
    // try to open config file

    READ_STRING(csv_file_path, "main", "csv_file_path");

    SVP_INFO("Reading settings done...");
    // read settings from config and/or environment */
}

#undef READ_STRING
#undef READ_INTEGER

settings::settings(const settings&) {

}

settings& settings::operator = (settings&) {

}

settings& settings::instance() {

    static settings  instance_;
    return instance_;
}

i32_t settings::get_integer(str_t key) {

    std::map<str_t, i32_t>::const_iterator found =
        integers.find(key);
    if( found == integers.end() )
        throw STATUS("Integer " + key + " not found in settings");
    return found->second;
}

str_t settings::get_string(str_t key) {

    std::map<str_t, str_t>::const_iterator found =
        strings.find(key);
    if( found == strings.end() )
        throw STATUS("String " + key + " not found in settings");
    return found->second;
}

void settings::save_in_log() {

    for(auto item : integers)
        SVP_INFO("SETTINGS(%s) = %d", item.first.c_str(), item.second);

    for(auto item : strings)
        SVP_INFO("SETTINGS(%s) = %s", item.first.c_str(), item.second.c_str());
}

};	// namespace SVP
