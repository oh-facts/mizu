/* date = September 18th 2024 11:48 pm */

#ifndef GLTF_LOADER_H
#define GLTF_LOADER_H

struct GLTF_Primitive
{
  
};

struct GLTF_Mesh
{
  GLTF_Primitive *primitives;
  u64 num_primitives;
};

struct GLTF_Model
{
  Str8 *textures;
  u32 num_textures;
  
  GLTF_Mesh *meshes;
  u64 num_meshes;
};

struct GLTF_It
{
  u64 mesh_index;
  GLTF_Model *model;
  Arena *arena;
};

function void gltf_traverse_node(GLTF_It *it, cgltf_node *node)
{
  if(node->mesh)
  {
    cgltf_mesh *node_mesh = node->mesh;
    
    GLTF_Mesh *mesh = it->model->meshes + it->mesh_index;
    mesh->num_primitives = node_mesh->primitives_count;
    mesh->primitives = push_array(it->arena, GLTF_Primitive, mesh->num_primitives);
    
    for(u32 i = 0; i < mesh->num_primitives; i++)
    {
      
    }
    
    
  }
  
}

function GLTF_Model gltf_load_mesh(Arena *arena, Str8 filepath)
{
  GLTF_Model out = {};
  Arena_temp temp = scratch_begin(0, 0);
  Str8 abs_path = str8_join(temp.arena, a_state->asset_dir, filepath);
  
  cgltf_options options = {};
  cgltf_data *data = 0;
  
  if(cgltf_parse_file(&options, (char*)abs_path.c, &data) == cgltf_result_success)
  {
    if(cgltf_load_buffers(&options, data, (char*)abs_path.c) == cgltf_result_success)
    {
      // load textures
      out.num_textures = data->textures_count;
      out.textures = push_array(arena, Str8, data->textures_count);
      
      for(u32 i = 0; i < data->textures_count; i++)
      {
        Str8 uri_str = str8((u8*)data->textures[i].image->uri, strlen(data->textures[i].image->uri));
        
        out.textures[i] = str8_join(arena, abs_path, uri_str);
      }
      
      // load meshes
      out.num_meshes = data->meshes_count;
      out.meshes = push_array(arena, GLTF_Mesh, out.num_meshes);
      
      GLTF_It it = {};
      it.model = &out;
      it.arena = arena;
      
      for(u32 i = 0; i < data->scenes_count; i++)
      {
        cgltf_scene *scene = data->scenes + i;
        
        for(u32 j = 0; j < scene->nodes_count; j++)
        {
          cgltf_node *node = scene->nodes[j];
          
          gltf_traverse_node(&it, node);
        }
      }
    }
  }
  
  scratch_end(&temp);
  
  
  return out;
}

#endif //GLTF_LOADER_H
