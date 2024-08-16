/* date = May 1st 2024 2:40 pm */

#ifndef BASE_MATH_H
#define BASE_MATH_H

#include <math.h>

// TODO(mizu): If sse 4.1 isn't available, don't include these and make rewrite functions
// that use intrinsics.

#include <xmmintrin.h>
#include <smmintrin.h>

#define PI (3.1415926535897f)
#define DEG_TO_RAD(deg) (deg * PI / 180.f)

union v2s
{
  s32 e[2];
  struct
  {
    s32 x;
    s32 y;
  };
};

union v2f
{
  f32 e[2];
  struct
  {
    f32 x;
    f32 y;
  };
};

union v3f
{
  f32 e[3];
  struct
  {
    f32 x;
    f32 y;
    f32 z;
  };
  struct
  {
    v2f xy;
    f32 _z;
  };
};

union v4f
{
  f32 e[4];
  struct
  {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
  };
	
  union v3f xyz;
};

union m4f
{
  f32 e[4][4];
};

struct m4f_ortho_proj
{
  m4f fwd;
  m4f inv;
};

function s32 floor_f32_to_s32(f32 a);

function f32 v2f_dist_sq(v2f a, v2f b);

function b32 operator==(v2s a, v2s b);
function v2s operator+(v2s a, v2s b);
function v2s operator-(v2s a, v2s b);
function void operator+=(v2s &a, v2s b);
function void operator-=(v2s &a, v2s b);

function v2f operator+(v2f a, f32 b);
function v2f operator-(v2f a, f32 b);
function v2f operator*(v2f a, f32 b);
function v2f operator/(v2f a, f32 b);
function void operator+=(v2f &a, f32 b);
function void operator*=(v2f &a, f32 b);

function v2f operator+(v2f a, v2f b);
function v2f operator-(v2f a, v2f b);
function void operator+=(v2f &a, v2f b);
function void operator-=(v2f &a, v2f b);

function f32 v3f_len(v3f v);
function v3f v3f_normalize(v3f v);
function f32 v3f_dot(v3f a, v3f b);
function v3f v3f_cross(v3f a, v3f b);

function v3f operator*(v3f a, f32 b);
function v3f operator*(f32 a, v3f b);
function void operator*=(v3f &a, f32 b);

function v3f operator+(v3f a, v3f b);
function v3f operator-(v3f a, v3f b);
function void operator+=(v3f &a, v3f b);
function b32 operator==(v3f a, v3f b);

function v4f operator*(m4f m, v4f b);

function m4f m4f_identity();

function m4f m4f_make_trans(v3f v);
function m4f m4f_make_trans(f32 x, f32 y, f32 z);
function m4f m4f_make_scale(v3f v);

function m4f m4f_make_rot_x(f32 rad);
function m4f m4f_make_rot_y(f32 rad);
function m4f m4f_make_rot_z(f32 rad);

function m4f m4f_translate(m4f m, v3f v);
function m4f m4f_scale(m4f m, v3f v);
function m4f m4f_rot(m4f m, v3f v);

function m4f_ortho_proj m4f_ortho(f32 left,f32 right,f32 bottom, f32 top, f32 _near, f32 _far);

function m4f m4f_look_at(v3f eye, v3f center, v3f up);
function m4f adjugate(const m4f& mat);
function f32 m4f_determinant(m4f m);
function m4f inverse(const m4f& mat);
function m4f operator*(m4f a, m4f b);

#endif //BASE_MATH_H