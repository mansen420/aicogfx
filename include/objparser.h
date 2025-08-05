#pragma once

#include "opres.h"
#include "vec.h"
#include "storage.h"

#include "tiny_obj_loader.h"
#include <vector>

namespace aico
{
    vec3 s;
    struct vertex{vec3 pos, normal; vec2 uv;};
    
    storage<vertex> parseobj(const char* filename, opres* res)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        
        std::string warn, err;
        bool ok=tinyobj::LoadObj(&attrib, &shapes, nullptr,
            &warn, &err, filename);
        if(!warn.empty())std::cout<<"Warn: "<<warn<<"\n";
        if(!err.empty())std::cout<<"Error: "<<err<<"\n";
        if(!ok) 
        {
            if(res) *res=opres::FAILURE;
            return storage<vertex>();
        }
        storage<vertex> vtx;
        vtx.rsvcpct(attrib.vertices.size());
        
        bool has_norms=!attrib.normals.empty(), has_uv=!attrib.texcoords.empty();

        for(const auto& shape: shapes)
        {
            for(const auto& idx: shape.mesh.indices)
            {
                vertex v;

                vtx.push_back(v);
            }
        }
        
        return vtx;
    }

    //TODO
    class obj;
}
