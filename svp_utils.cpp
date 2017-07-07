#include "svp_utils.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include "svp_error.h"
#include "svp_log.h"
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>

namespace SVP {

str_t buff2hex( u8_t* buffer, u32_t size ) {

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    u32_t elem;
    for( u32_t i = 0; i < size; i++ ) {
        elem = static_cast<u32_t>(buffer[i]);
        ss << ' ' << elem;
    }
    return ss.str();
}

str_t buff2hex( buffer_t& buffer ) {

    return buffer.size() ? buff2hex(&buffer[0], buffer.size()) : "";
}

str_t get_module_path() {

    i8_t arg1[40];
    i8_t exepath[PATH_MAX + 1] = {0};
    sprintf(arg1, "/proc/%d/exe", getpid());
    if(readlink(arg1, exepath, PATH_MAX) == -1 )
        throw STATUS(STR("cant't get module path (%ld)", errno));
    for(int i = strlen(exepath) - 1; i >= 0; i--)
        if( exepath[i] == '/' ) {
            exepath[i] = 0;
            break;
        }
    /*strptr_t cut = strrchr(exepath, '/');
    if(cut)
        *cut = 0;//*/
    return str_t(exepath);
}

u32_t switch_endian(u32_t var) {

    u32_t result = (var & 0xFF) << 24;
    result |= (var & 0xFF00) << 8;
    result |= (var & 0xFF0000) >> 8;
    result |= (var & 0xFF000000) >> 24;
    return result;
}

u64_t get_tick_count() {

    struct timeval te = {0};
    gettimeofday(&te, NULL);
    // get current time

    return te.tv_sec*1000LL + te.tv_usec/1000;
    // caculate milliseconds
}


static const uint8_t kBase64Decode[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
    0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0,
    0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
};

static const uint8_t kBase64Encode[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

error_t base64encode(buffer_t& out, const buffer_t& in) {
    buffer_t::const_iterator input = in.begin();
    buffer_t::const_iterator input_end = in.end();

    if( input == input_end)
        return STATUS(E_OK);

    while(input_end - input > 2 ) {
        uint32_t data = 0;
        data |= *(input++) << 16;
        data |= *(input++) << 8;
        data |= *(input++);
        // grab all bytes in one integer

        out.push_back(kBase64Encode[ (data >> 18) & 0x3F ]);
        out.push_back(kBase64Encode[ (data >> 12) & 0x3F ]);
        out.push_back(kBase64Encode[ (data >> 6)  & 0x3F ]);
        out.push_back(kBase64Encode[ (data)       & 0x3F ]);
        // encode
    }

    size_t bytes_left = input_end - input;
    if( bytes_left ) { // not all data already converted
        uint32_t data = 0;
        data |= *(input++) << 16;
        if( bytes_left > 1 ) data |= *(input++) << 8;
        out.push_back(kBase64Encode[ (data >> 18) & 0x3F ]);
        out.push_back(kBase64Encode[ (data >> 12) & 0x3F ]);
        if( bytes_left > 1) {
            out.push_back(kBase64Encode[ (data >> 6)  & 0x3F ]);
        }
        else {
            out.push_back('=');
        }
        out.push_back('=');
    }
    // padding

    return STATUS(E_OK);
}

error_t base64decode(buffer_t& out, const buffer_t& in) {
    buffer_t::const_iterator input = in.begin();
    buffer_t::const_iterator input_end = in.end();

    while( input_end - input > 3 ) {
        // Grab four base-64 encoded (6-bit) bytes.
        uint32_t data = 0;
        data |= (kBase64Decode[*input++] << 18);
        data |= (kBase64Decode[*input++] << 12);
        data |= (kBase64Decode[*input++] <<  6);
        data |= (kBase64Decode[*input++]      );
        // And decode it to 3 (8-bit) bytes.
        out.push_back((data >> 16) & 0xff);
        out.push_back((data >>  8) & 0xff);
        out.push_back((data      ) & 0xff);

        // = is the base64 end marker. Remove decoded bytes if we see any.
        if (input[-1] == '=') out.pop_back();
        if (input[-2] == '=') out.pop_back();
    }
    if( input_end != input ) {
        return STATUS("Incoming base 64 data has incorrect size.");
    }
    return STATUS(E_OK);
}

str_t random_str(u32_t size) {

    str_t str;
    srand((u32_t)get_tick_count());
    for(u32_t i = 0; i < size; i++) {
        u32_t byte = rand() % 256;
        str += STR("%02x", byte);
    }
    return str;
}

i32_t getdir(str_t dir, std::vector<str_t>& files) {

    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL)
        return errno;

    while ((dirp = readdir(dp))!= NULL)
        if(dirp->d_name[0] != '.')
            files.push_back(str_t(dirp->d_name));
    closedir(dp);
    return 0;
}

i32_t get_file_size(cstrptr_t filename) {

    struct stat st = {0};
    if(stat(filename, &st))
        return -1;
    return (i32_t)st.st_size;
}

void replace_substring(str_t& src, cstrptr_t macro, const str_t& value) {

  size_t pos = 0;
  while( (pos = src.find(macro, pos)) != std::string::npos)
    src.replace(pos, strlen(macro), value);
}

error_t file2buff(cstrptr_t filename, buffer_t& buffer) {

    i32_t size = get_file_size(filename);
    if(size <= 0)
        return STATUS(STR("Can't get size for file %s", filename));

    int file = open(filename, O_RDONLY, 0);
    if(file < 0)
        return STATUS(STR("Can't open file %s", filename));

    buffer.resize((size_t)size);
    i32_t amount = read(file, &buffer[0], buffer.size());
    close(file);
    if(amount != size)
        return STATUS(STR("Can't read file %s -> %d", filename, amount));
    return STATUS(E_OK);
}

void gen_random(buffer_t& data, size_t size) {
    data.resize(0);
    data.reserve(size);
    while(size-- > 0) {
        data.push_back(rand());
    }
    return;
}

error_t write_file(str_t name, const buffer_t& data) {
    int file = open(name.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if(file < 0)
        return STATUS(STR("Can't open file %s -> %d", name.c_str(), file));

    write(file, &data[0], data.size());
    close(file);
    return STATUS(E_OK);
}

};	// namespace SVP

