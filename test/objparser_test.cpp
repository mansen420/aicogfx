#include "aico/objparser.h"
#include "aico/opres.h"

#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv)
{
    const char* objpath="default.obj";
    if(argc>1)
        objpath=argv[1];
    else
        printf("No file path specified, assumed \"default.obj\"\n");

    aico::opres res;
    auto vertices=aico::parseobj(objpath, &res);
    
    printf("vertices size: %lu", vertices.size());

    if(res==aico::opres::SUCCESS)return EXIT_SUCCESS;
    else return EXIT_FAILURE;
}
