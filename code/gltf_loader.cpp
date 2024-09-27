/* date = September 18th 2024 11:48 pm */
struct GLTF_Vertex
{
  v3f pos;
  f32 pad;
};

struct GLTF_Primitive
{
  u32 start;
  u32 count;
};

struct GLTF_Mesh
{
  GLTF_Primitive *primitives;
  u64 num_primitives;
  
  u32 *indices;
  u32 num_indices;
  
  GLTF_Vertex *vertices;
  u32 num_vertices;
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
  Arena *arena;
  u64 mesh_index;
  GLTF_Model *model;
  cgltf_data *data;
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
      cgltf_primitive *node_prim = node->mesh->primitives + i;
      cgltf_accessor *index_attrib = node_prim->indices;
      
      mesh->num_indices += index_attrib->count;
    }
    
    for(u32 i = 0; i < mesh->num_primitives; i++)
    {
      cgltf_primitive *node_prim = node->mesh->primitives + i;
      
      for(u32 j = 0; j < node_prim->attributes_count; j++)
      {
        cgltf_attribute *attrib = node_prim->attributes + j;
        
        if(attrib->type == cgltf_attribute_type_position)
        {
          cgltf_accessor *vert_attrib = attrib->data;
          mesh->num_vertices += vert_attrib->count;
        }
      }
    }
    
    mesh->indices = push_array(it->arena, u32, mesh->num_indices);
    mesh->vertices = push_array(it->arena, GLTF_Vertex, mesh->num_vertices);
    
    u64 init_vtx = 0;
    u64 init_index = 0;
    
    for(u32 i = 0; i < mesh->num_primitives; i++)
    {
      cgltf_primitive *node_prim = node->mesh->primitives + i;
      
      GLTF_Primitive *p = mesh->primitives + i;
      
      cgltf_accessor *index_attrib = node_prim->indices;
      
      p->start = init_index;
      p->count = index_attrib->count;
      
      // indices
      {
        for (u32 j = 0; j < index_attrib->count; j++)
        {
          size_t index = cgltf_accessor_read_index(index_attrib, j);
          
          mesh->indices[j + init_index] = index + init_vtx;
        }
        
        init_index += index_attrib->count;
      }
      
      // vertices
      for(u32 j = 0; j < node_prim->attributes_count; j++)
      {
        cgltf_attribute *attrib = node_prim->attributes + j;
        
        if(attrib->type == cgltf_attribute_type_position)
        {
          cgltf_accessor *vert_attrib = attrib->data;
          
          for(u32 k = 0; k < vert_attrib->count; k++)
          {
            cgltf_accessor_read_float(vert_attrib, k, mesh->vertices[k + init_vtx].pos.e, sizeof(f32));
          }
          
          init_vtx += vert_attrib->count;
        }
      }
      
      
    }
    
    it->mesh_index++;
  }
  
  for (u32 i = 0; i < node->children_count; ++i) 
  {
    gltf_traverse_node(it, node->children[i]);
  }
}

function void gltf_print(GLTF_Model *model)
{
  for(u32 i = 0; i < model->num_meshes; i++)
  {
    GLTF_Mesh *mesh = model->meshes + i;
    
    printf("indices %u\n", i);
    for(u32 j = 0; j < mesh->num_indices; j++)
    {
      printf("%u, ", mesh->indices[j]);
    }
    printf("\n");
    
    printf("verticess %u\n", i);
    for(u32 j = 0; j < mesh->num_vertices; j++)
    {
      GLTF_Vertex *vert = mesh->vertices + j;
      printf("[%f, %f, %f]", vert->pos.x, vert->pos.y, vert->pos.z);
    }
    printf("\n");
    
    for(u32 j = 0; j < mesh->num_primitives; j++)
    {
      GLTF_Primitive *p = mesh->primitives;
      
      printf("start: %u\n", p->start);
      printf("count: %u\n", p->count);
    }
    
    
    printf("\n");
  }
}

function GLTF_Model gltf_load_mesh(Arena *arena, Str8 filepath)
{
  GLTF_Model out = {};
  Arena_temp temp = scratch_begin(0, 0);
  Str8 abs_path = str8_join(temp.arena, a_state->asset_dir, filepath);
  //Str8 abs_path = str8_lit("C:\\dev\\game\\data\\assets\\gltf_test\\duck\\Duck.gltf");
  
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
      it.data = data;
      
      for(u32 i = 0; i < data->scenes_count; i++)
      {
        cgltf_scene *scene = data->scenes + i;
        
        for(u32 j = 0; j < scene->nodes_count; j++)
        {
          cgltf_node *node = scene->nodes[j];
          
          gltf_traverse_node(&it, node);
        }
      }
      
      //gltf_print(it.model);
      
    }
  }
  
  //scratch_end(&temp);
  
  
  return out;
}