#pragma once
#include "aico/sys/malc.h"
#include "aico/sys/opres.h"
#include "aico/sys/memory.h"

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

namespace aico
{
    //@warn won't work for file sizes >2GB due to ftell limit 
    //caller is responsible for contents ownership.
    inline opres_t readbf(const char* file_path, char*& contents)
    {
        FILE* fstream = fopen(file_path, "rb");
        if(!fstream)
            return FAIL;
        size_t len;
        {
            fseek(fstream, 0, SEEK_END);
            len = (size_t)ftell(fstream);
            if(len < 0)
            {
                fclose(fstream);
                return FAIL;
            }
            rewind(fstream);
        }
        contents = (char*)aico::sys::malc(len+1);
        if(!contents)
        {
            fclose(fstream);
            return FAIL;
        }
        if(fread(contents, 1, len, fstream) != len)
        {
            fclose(fstream);
            aico::sys::rel(contents);
            return FAIL;
        }
        fclose(fstream);
        contents[len]='\0';
        return OK;
    }
};
