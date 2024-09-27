/* date = September 27th 2024 4:22 pm */

ED_CUSTOM_TAB(game_update_and_render)
{
  s32 tilemap[9][16] = 
  {
    {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
    {1, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 1},
    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
    {1, 1, 0, 1,  1, 0, 0, 0,  1, 1, 1, 0,  0, 0, 0, 0},
    {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 1},
    {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 1},
    {1, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1},
    {1, 0, 1, 0,  1, 0, 1, 0,  0, 0, 1, 0,  1, 0, 0, 1},
  };
  
  d_push_target(target);
  
  for(s32 row = 0; row < 9; row++)
  {
    for(s32 col = 0; col < 16; col++)
    {
      s32 tile_id = tilemap[row][col];
      v4f color = {};
      if(tile_id == 0)
      {
        color = D_COLOR_RED;
      }
      else
      {
        color = D_COLOR_GREEN;
      }
      
      f32 size = 64;
      
      Rect dst = {};
      dst.tl.x = col * size;
      dst.tl.y = row * size;
      dst.br.x = dst.tl.x + size;
      dst.br.y = dst.tl.y + size;
      
      d_rect(dst, color);
    }
  }
  
  d_pop_target();
}