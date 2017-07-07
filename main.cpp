#include <erl_nif.h>

#include <cstring>
#include <sstream>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include "svp_settings.h"
#include "svp_log.h"
#include "svp_utils.h"

static std::ifstream* file = NULL;
static ErlNifMutex* mutex = NULL;

extern "C"
{
    static int load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info) {

        //setlogmask(LOG_UPTO(LOG_NOTICE));
        openlog("csv_reader_nif", LOG_CONS|LOG_PID|LOG_NDELAY, LOG_LOCAL1);
        SVP_INFO("LOAD()\n");

        SVP::str_t path;
        try {
            path = SVP::settings::instance().get_string(SVP::settings::csv_file_path);
        }
        catch(SVP::error_t& err) {
            SVP_ERROR("Can't read CONFIG file: %s", CONFIG_FILE);
            return -1;
        };
        file = new std::ifstream(path.c_str());
        if(!file->is_open()) {
            SVP_INFO("Can't open CSV file: %s", path.c_str());
            delete file, file = NULL;
            return -1;
        }
        SVP_INFO("CSV file is successfully opened: %s", path.c_str());

        mutex = enif_mutex_create(const_cast<char*>("csv_reader_nif_mutex"));
        SVP_INFO("mutex created (%016llX).\nCSV plugin is loaded.", (SVP::u64_t)mutex);
        return 0;
    }

    static void unload(ErlNifEnv* env, void* priv_data) {

        SVP_INFO("UNLOAD()\n");
        if(file) {
            file->close();
            delete file, file = NULL;
            SVP_INFO("CSV file is closed.");
        }
        if(mutex) {
            enif_mutex_destroy(mutex), mutex = NULL;
            SVP_INFO("mutex destroyed.");
        }
        SVP_INFO("CSV plugin is unloaded.");
        closelog();
    }

#define READ_LINE(value) if(file->eof()) {\
            file->clear();\
            file->seekg(0, std::ios::beg);\
            if(file->eof()) {\
                enif_mutex_unlock(mutex);\
                SVP_ERROR("Cannot read " #value);\
                return enif_make_badarg(env); }}\
            std::getline(*file, value );

    static std::string get_type(ErlNifEnv* env, ERL_NIF_TERM& term) {
        if(enif_is_atom(env, term))
            return std::string("atom");
        if(enif_is_binary(env, term))
            return std::string("binary");
        if(enif_is_empty_list(env, term))
            return std::string("empty_list");
        if(enif_is_exception(env, term))
            return std::string("exception");
        if(enif_is_number(env, term))
            return std::string("number");
        if(enif_is_fun(env, term))
            return std::string("fun");
        if(enif_is_ref(env, term))
            return std::string("ref");
        if(enif_is_tuple(env, term))
            return std::string("tuple");
        if(enif_is_list(env, term))
            return std::string("list");
        return std::string("unknown");
    }

    void get_tuple(ErlNifEnv* env, const ERL_NIF_TERM& term) {
        int arity = 0;
        ERL_NIF_TERM* array = NULL;
        if(enif_get_tuple(env, term, &arity, const_cast<const ERL_NIF_TERM**>(&array))) {
            SVP_INFO("TUPLE = %d", arity);
            for(int i = 0; i < arity; i++) {
                std::string type = get_type(env, array[i]);
                SVP_INFO("[%d] = %s", i, type.c_str());
                if(type == "tuple") {
                    SVP_INFO("%d -->", i);
                    get_tuple(env, array[i]);
                    SVP_INFO("<-- %d", i);
                }
            }
        }
    }


    static void get_next_token(std::string& dst, const std::string& src, size_t& pos) {
	if( pos == std::string::npos )
	    return;

        const char delimiter = ';';
	size_t begin = pos;
    	pos = src.find(delimiter, begin);
        if( pos != std::string::npos ) { 
            dst = src.substr(begin, pos-begin);
	    pos++; // point to next char after delimiter
	}
	else {
	    dst = src.substr(begin, src.length() - begin);
	}
	return;
    }

    static ERL_NIF_TERM get_csv_line(ErlNifEnv* env, int argc, const ERL_NIF_TERM  argv[]) {

        if(!file) {
            SVP_ERROR("CSV file is not opened....");
            return enif_make_badarg(env);
        }

        if(!mutex) {
            SVP_ERROR("mutex is not created....");
            return enif_make_badarg(env);
        }

        std::string auth, handshake, data;
//        get_tuple(env, argv[0]);
        SVP::u64_t start = SVP::get_tick_count();
        enif_mutex_lock(mutex);
	std::string str;
	READ_LINE(str);
	size_t pos = 0;
	get_next_token(auth, str, pos);
	get_next_token(handshake, str, pos);
	get_next_token(data, str, pos);
        enif_mutex_unlock(mutex);
        SVP::u64_t end = SVP::get_tick_count() - start;
        SVP_INFO("%llu ms", end);

        return enif_make_list3(env,
            enif_make_string(env, auth.c_str(), ERL_NIF_LATIN1 ),
            enif_make_string(env, handshake.c_str(), ERL_NIF_LATIN1 ),
            enif_make_string(env, data.c_str(), ERL_NIF_LATIN1 )); //*/
/*        return enif_make_list3(env,
            enif_make_string(env, "QUJDCg==", ERL_NIF_LATIN1 ),
            enif_make_string(env, "QUJDCg==", ERL_NIF_LATIN1 ),
            enif_make_string(env, "QUJDCg==", ERL_NIF_LATIN1 )); //*/
    }

    static ErlNifFunc  csv_reader_funcs[] = {
        { "getline", 1, get_csv_line }
    };
}

ERL_NIF_INIT( csv_reader_nif, csv_reader_funcs, load, NULL, NULL, unload )
