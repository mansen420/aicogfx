#pragma once
#include "aico/malc.h"
#include "aico/opres.h"
#include "aico/memory.h"

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
    inline opres readbf(const char* file_path, char*& contents)
    {
        FILE* fstream = fopen(file_path, "rb");
        if(!fstream)
            return opres::FAILURE;
        size_t len;
        {
            fseek(fstream, 0, SEEK_END);
            len = (size_t)ftell(fstream);
            if(len < 0)
            {
                fclose(fstream);
                return opres::FAILURE;
            }
            rewind(fstream);
        }
        contents = (char*)aico::sys::malc(len+1);
        if(!contents)
        {
            fclose(fstream);
            return opres::FAILURE;
        }
        if(fread(contents, 1, len, fstream) != len)
        {
            fclose(fstream);
            aico::sys::rel(contents);
            return opres::FAILURE;
        }
        fclose(fstream);
        contents[len]='\0';
        return opres::SUCCESS;
    }
};
