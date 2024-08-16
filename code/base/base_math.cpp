s32 floor_f32_to_s32(f32 a)
{
  s32 res = _mm_cvtss_si32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(a)));
  return res;
}

f32 v2f_dist_sq(v2f a, v2f b)
{
  f32 out = (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
  return out;
}

b32 operator==(v2s a, v2s b)
{
  return a.x == b.x && a.y == b.y;
}

v2s operator-(v2s a, v2s b)
{
  v2s out = {};
  out.x = a.x - b.x,
  out.y = a.y - b.y;
	
  return out;
}

v2s operator+(v2s a, v2s b)
{
  v2s out = {};
  out.x = a.x + b.x,
  out.y = a.y + b.y;
	
  return out;
}

void operator+=(v2s &a, v2s b)
{
  a.x += b.x;
  a.y += b.y;
}

void operator-=(v2s &a, v2s b)
{
  a.x -= b.x;
  a.y -= b.y;
}

v2f operator+(v2f a, f32 b)
{
  return(v2f){
    {
      a.x + b,
      a.y + b
    }
  };
}

v2f operator-(v2f a, f32 b)
{
  return(v2f){
    {
      a.x - b,
      a.y - b
    }
  };
}

v2f operator*(v2f a, f32 b)
{
  return(v2f){
    {
      a.x * b,
      a.y * b
    }
  };
}

v2f operator/(v2f a, f32 b)
{
  return(v2f){
    {
      a.x / b,
      a.y / b
    }
  };
}

void operator+=(v2f& a, f32 b)
{
  a.x += b;
  a.y += b;
}

void operator*=(v2f &a, f32 b)
{
  a.x *= b;
  a.y *= b;
}

v2f operator+(v2f a, v2f b)
{
  return (v2f){
    {
      a.x + b.x,
      a.y + b.y
    }
		
  };
}

v2f operator-(v2f a, v2f b)
{
  return (v2f){
    {
      a.x - b.x,
      a.y - b.y
    }
		
  };
}

void operator+=(v2f &a, v2f b)
{
  a.x += b.x;
  a.y += b.y;
}

void operator-=(v2f &a, v2f b)
{
  a.x -= b.x;
  a.y -= b.y;
}

f32 v3f_len(v3f v)
{
  return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

v3f v3f_normalize(v3f v)
{
  f32 l = v3f_len(v);
	
  return v3f{
    {
      v.x/l,
      v.y/l,
      v.z/l
    }};
}

f32 v3f_dot(v3f a, v3f b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

v3f v3f_cross(v3f a, v3f b)
{
  v3f result;
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}

v3f operator*(v3f a, f32 b)
{
  return (v3f){
    {
      a.x * b,
      a.y * b,
      a.z * b
    }
		
  };
}

v3f operator*(f32 a, v3f b)
{
  return b * a;
}

void operator+=(v3f &a, v3f b)
{
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
}

void operator*=(v3f &a, f32 b)
{
  a.x *= b;
  a.y *= b;
  a.z *= b;
}

v3f operator+(v3f a, v3f b)
{
  return (v3f){
    {
      a.x + b.x,
      a.y + b.y,
      a.z + b.z
    }
		
  };
}

v3f operator-(v3f a, v3f b)
{
  return v3f{{
      a.x - b.x,
      a.y - b.y,
      a.z - b.z
    }};
}

b32 operator==(v3f a, v3f b)
{
	if(a.x == b.x && a.y == b.y && a.z == b.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

v4f operator*(m4f a, v4f b)
{
  v4f out = {};
	
  for(u32 r = 0; r < 4; r ++)
  {
    out.e[r] =
      a.e[r][0] * b.e[0] +
      a.e[r][1] * b.e[1] +
      a.e[r][2] * b.e[2] +
      a.e[r][3] * b.e[3];
  }
	
  return out;
}

m4f m4f_identity()
{
  return (m4f){
    {
      {1, 0, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 1},
    }
  };
}

m4f m4f_make_trans(v3f v)
{
  return (m4f){
    {
      {1, 0, 0, v.x},
      {0, 1, 0, v.y},
      {0, 0, 1, v.z},
      {0, 0, 0, 1},
    }
  };
}

m4f m4f_make_trans(f32 x, f32 y, f32 z)
{
  return (m4f){
    {
      {1, 0, 0, x},
      {0, 1, 0, y},
      {0, 0, 1, z},
      {0, 0, 0, 1},
    }
  };
}

m4f m4f_make_scale(v3f v)
{
  return (m4f){
    {
      {v.x, 0, 0, 0},
      {0, v.y, 0, 0},
      {0, 0, v.z, 0},
      {0, 0, 0, 1},
    }
  };
}

m4f m4f_make_scale(f32 s)
{
  return (m4f){
    {
      {s, 0, 0, 0},
      {0, s, 0, 0},
      {0, 0, s, 0},
      {0, 0, 0, 1},
    }
  };
}

m4f m4f_make_rot_x(f32 rad)
{
  float c = cos(rad);
  float s = sin(rad);
	
  return (m4f){
    {
      {1, 0, 0, 0},
      {0, c, -s, 0},
      {0, s, c, 0},
      {0, 0, 0, 1}
    }
  };
}

m4f m4f_make_rot_y(f32 rad)
{
  float c = cos(rad);
  float s = sin(rad);
	
  return (m4f){
    {
      {c, 0, s, 0},
      {0, 1, 0, 0},
      {-s, 0, c, 0},
      {0, 0, 0, 1}
    }
  };
}

m4f m4f_make_rot_z(f32 rad)
{
  float c = cos(rad);
  float s = sin(rad);
	
  return (m4f){
    {
      {c,-s, 0, 0},
      {s, c, 0, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 1}
    }
  };
}

m4f m4f_translate(m4f m, v3f v)
{
  m4f out = m;
	
  out.e[3][0] = v.x;
  out.e[3][1] = v.y;
  out.e[3][2] = v.z;
	
  return out;
}

m4f m4f_scale(m4f m, v3f v)
{
  m4f out = m;
	
  out.e[0][0] = v.x;
  out.e[1][1] = v.y;
  out.e[2][2] = v.z;
	
  return out;
}

m4f m4f_rot(m4f m, v3f v)
{
  m4f out = m;
	
  out.e[0][0] = v.x;
  out.e[1][1] = v.y;
  out.e[2][2] = v.z;
	
  return out;
}

m4f_ortho_proj m4f_ortho(f32 left,f32 right,f32 bottom, f32 top, f32 _near, f32 _far)
{
  f32 rpl = right + left;
  f32 rml = right - left;
  f32 tpb = top + bottom;
  f32 tmb = top - bottom;
  f32 fpn = _far + _near;
  f32 fmn = _far - _near;
	
  m4f fwd = {{
      {2/rml,     0,          0,       -rpl/rml},
      {0,         2/tmb,      0,       -tpb/tmb},
      {0,         0,          -2/fmn,   - fpn/fmn},
      {0, 0,  0, 1}
    }};
	
  m4f inv = {{
      {rml/2,   0,       0,       rpl/2},
      {0,       tmb/2,   0,       tpb/2},
      {0,       0,       -fmn/2,  -fpn/2},
      {0,   0,   0,   1}
    }};
	
  return m4f_ortho_proj{fwd, inv};
}

m4f m4f_look_at(v3f eye, v3f center, v3f up)
{
  v3f z = v3f_normalize(center - eye);
  v3f x = v3f_normalize(v3f_cross(z, up));
  v3f y = v3f_cross(x, z);
	
  m4f mat = {
    {
      {x.x, y.x, -z.x, -v3f_dot(x,eye)},
      {x.y, y.y, -z.y, -v3f_dot(y,eye)},
      {x.z, y.z, -z.z, -v3f_dot(z,eye)},
      {0,0,0, 1}
    }
  };
	
  return mat;
}

f32 determinant3x3(f32 a, f32 b, f32 c,
                   f32 d, f32 e, f32 f,
                   f32 g, f32 h, f32 i) {
  return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}

m4f adjugate(const m4f& mat) {
  m4f adj;
	
  adj.e[0][0] =  determinant3x3(mat.e[1][1], mat.e[1][2], mat.e[1][3],
                                mat.e[2][1], mat.e[2][2], mat.e[2][3],
                                mat.e[3][1], mat.e[3][2], mat.e[3][3]);
  adj.e[1][0] = -determinant3x3(mat.e[1][0], mat.e[1][2], mat.e[1][3],
                                mat.e[2][0], mat.e[2][2], mat.e[2][3],
                                mat.e[3][0], mat.e[3][2], mat.e[3][3]);
  adj.e[2][0] =  determinant3x3(mat.e[1][0], mat.e[1][1], mat.e[1][3],
                                mat.e[2][0], mat.e[2][1], mat.e[2][3],
                                mat.e[3][0], mat.e[3][1], mat.e[3][3]);
  adj.e[3][0] = -determinant3x3(mat.e[1][0], mat.e[1][1], mat.e[1][2],
                                mat.e[2][0], mat.e[2][1], mat.e[2][2],
                                mat.e[3][0], mat.e[3][1], mat.e[3][2]);
	
  adj.e[0][1] = -determinant3x3(mat.e[0][1], mat.e[0][2], mat.e[0][3],
                                mat.e[2][1], mat.e[2][2], mat.e[2][3],
                                mat.e[3][1], mat.e[3][2], mat.e[3][3]);
  adj.e[1][1] =  determinant3x3(mat.e[0][0], mat.e[0][2], mat.e[0][3],
                                mat.e[2][0], mat.e[2][2], mat.e[2][3],
                                mat.e[3][0], mat.e[3][2], mat.e[3][3]);
  adj.e[2][1] = -determinant3x3(mat.e[0][0], mat.e[0][1], mat.e[0][3],
                                mat.e[2][0], mat.e[2][1], mat.e[2][3],
                                mat.e[3][0], mat.e[3][1], mat.e[3][3]);
  adj.e[3][1] =  determinant3x3(mat.e[0][0], mat.e[0][1], mat.e[0][2],
                                mat.e[2][0], mat.e[2][1], mat.e[2][2],
                                mat.e[3][0], mat.e[3][1], mat.e[3][2]);
	
  adj.e[0][2] =  determinant3x3(mat.e[0][1], mat.e[0][2], mat.e[0][3],
                                mat.e[1][1], mat.e[1][2], mat.e[1][3],
                                mat.e[3][1], mat.e[3][2], mat.e[3][3]);
  adj.e[1][2] = -determinant3x3(mat.e[0][0], mat.e[0][2], mat.e[0][3],
                                mat.e[1][0], mat.e[1][2], mat.e[1][3],
                                mat.e[3][0], mat.e[3][2], mat.e[3][3]);
  adj.e[2][2] =  determinant3x3(mat.e[0][0], mat.e[0][1], mat.e[0][3],
                                mat.e[1][0], mat.e[1][1], mat.e[1][3],
                                mat.e[3][0], mat.e[3][1], mat.e[3][3]);
  adj.e[3][2] = -determinant3x3(mat.e[0][0], mat.e[0][1], mat.e[0][2],
                                mat.e[1][0], mat.e[1][1], mat.e[1][2],
                                mat.e[3][0], mat.e[3][1], mat.e[3][2]);
	
  adj.e[0][3] = -determinant3x3(mat.e[0][1], mat.e[0][2], mat.e[0][3],
                                mat.e[1][1], mat.e[1][2], mat.e[1][3],
                                mat.e[2][1], mat.e[2][2], mat.e[2][3]);
  adj.e[1][3] =  determinant3x3(mat.e[0][0], mat.e[0][2], mat.e[0][3],
                                mat.e[1][0], mat.e[1][2], mat.e[1][3],
                                mat.e[2][0], mat.e[2][2], mat.e[2][3]);
  adj.e[2][3] = -determinant3x3(mat.e[0][0], mat.e[0][1], mat.e[0][3],
                                mat.e[1][0], mat.e[1][1], mat.e[1][3],
                                mat.e[2][0], mat.e[2][1], mat.e[2][3]);
  adj.e[3][3] =  determinant3x3(mat.e[0][0], mat.e[0][1], mat.e[0][2],
                                mat.e[1][0], mat.e[1][1], mat.e[1][2],
                                mat.e[2][0], mat.e[2][1], mat.e[2][2]);
	
  return adj;
}

f32 determinant(const m4f& mat) {
  f32 det = 0;
	
  det += mat.e[0][0] * determinant3x3(mat.e[1][1], mat.e[1][2], mat.e[1][3],
                                      mat.e[2][1], mat.e[2][2], mat.e[2][3],
                                      mat.e[3][1], mat.e[3][2], mat.e[3][3]);
  det -= mat.e[0][1] * determinant3x3(mat.e[1][0], mat.e[1][2], mat.e[1][3],
                                      mat.e[2][0], mat.e[2][2], mat.e[2][3],
                                      mat.e[3][0], mat.e[3][2], mat.e[3][3]);
  det += mat.e[0][2] * determinant3x3(mat.e[1][0], mat.e[1][1], mat.e[1][3],
                                      mat.e[2][0], mat.e[2][1], mat.e[2][3],
                                      mat.e[3][0], mat.e[3][1], mat.e[3][3]);
  det -= mat.e[0][3] * determinant3x3(mat.e[1][0], mat.e[1][1], mat.e[1][2],
                                      mat.e[2][0], mat.e[2][1], mat.e[2][2],
                                      mat.e[3][0], mat.e[3][1], mat.e[3][2]);
	
  return det;
}

m4f inverse(const m4f& mat) {
  m4f inv;
	
  f32 det = determinant(mat);
  if (det == 0) {
    
    return inv;
  }
	
  m4f adj = adjugate(mat);
	
  f32 inv_det = 1.0f / det;
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      inv.e[r][c] = adj.e[r][c] * inv_det;
    }
  }
	
  return inv;
}

m4f operator*(m4f a, m4f b)
{
  m4f out = {};
	
  for(u32 r = 0; r < 4; r ++)
  {
    for(u32 c = 0; c < 4; c ++)
    {
      out.e[r][c] =
        a.e[r][0] * b.e[0][c] +
        a.e[r][1] * b.e[1][c] +
        a.e[r][2] * b.e[2][c] +
        a.e[r][3] * b.e[3][c];
    }
  }
	
  return out;
}
