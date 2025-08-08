#include "aico/objparser.h"
#include "aico/opres.h"

#include "tiny_obj_loader.h"

#include <cstdio>
#include <vector>

using namespace aico;

[[nodiscard]]storage<vertex> aico::parseobj(const char *filename, opres *res)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    
    std::string warn, err;
    bool ok=tinyobj::LoadObj(&attrib, &shapes, nullptr,
        &warn, &err, filename);
    if(!warn.empty())printf("warn: %s\n", warn.c_str());
    if(!err.empty())printf("ERROR: %s\n", err.c_str());
    if(!ok) 
    {
        if(res) *res=opres::FAILURE;
        return storage<vertex>();
    }
    storage<vertex> vtx;
    vtx.rsvcpct([&](){size_t sum=0; 
        for(const auto& shape: shapes)sum+=shape.mesh.indices.size();
        return sum;}());

    bool has_norms=!attrib.normals.empty(), has_uv=!attrib.texcoords.empty();

    const auto& vertices=attrib.vertices;
    const auto& normals=attrib.normals;
    const auto& uvs=attrib.texcoords;

    for(const auto& shape: shapes)
        for(const auto& idx: shape.mesh.indices)
        {
            vertex v;
            const auto& vidx=idx.vertex_index;
            const auto& nidx=idx.normal_index;
            const auto& tidx=idx.texcoord_index;

            if(vidx>=0) v.pos=
                {vertices[vidx*3+0], vertices[vidx*3+1], vertices[vidx*3+2]};
            else puts("warn: missing vertices");
            
            if(nidx>=0) v.normal=
                {normals[nidx*3+0], normals[nidx*3+1], normals[nidx*3+2]};
            else if(has_norms) puts("warn: missing normals");

            if(tidx>=0) v.uv=
                {uvs[tidx*2+0], uvs[tidx*2+1]};
            else if(has_uv) puts("warn: missing UVs");

            vtx.push_back(v);
        }
    
    return vtx;
}
