#ifdef GL_ES
precision highp float;
#endif

uniform vec3 uLightDir;
uniform vec3 uCameraPos;
uniform vec3 uLightRadiance;
uniform sampler2D uGDiffuse;
uniform sampler2D uGDepth;
uniform sampler2D uGNormalWorld;
uniform sampler2D uGShadow;
uniform sampler2D uGPosWorld;

varying mat4 vWorldToScreen;
varying highp vec4 vPosWorld;

#define M_PI 3.1415926535897932384626433832795
#define TWO_PI 6.283185307
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309

float Rand1(inout float p) {
  p = fract(p * .1031);
  p *= p + 33.33;
  p *= p + p;
  return fract(p);
}

vec2 Rand2(inout float p) {
  return vec2(Rand1(p), Rand1(p));
}

float InitRand(vec2 uv) {
	vec3 p3  = fract(vec3(uv.xyx) * .1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}

vec3 SampleHemisphereUniform(inout float s, out float pdf) {
  vec2 uv = Rand2(s);
  float z = uv.x;
  float phi = uv.y * TWO_PI;
  float sinTheta = sqrt(1.0 - z*z);
  vec3 dir = vec3(sinTheta * cos(phi), sinTheta * sin(phi), z);
  pdf = INV_TWO_PI;
  return dir;
}

vec3 SampleHemisphereCos(inout float s, out float pdf) {
  vec2 uv = Rand2(s);
  float z = sqrt(1.0 - uv.x);
  float phi = uv.y * TWO_PI;
  float sinTheta = sqrt(uv.x);
  vec3 dir = vec3(sinTheta * cos(phi), sinTheta * sin(phi), z);
  pdf = z * INV_PI;
  return dir;
}

void LocalBasis(vec3 n, out vec3 b1, out vec3 b2) {
  float sign_ = sign(n.z);
  if (n.z == 0.0) {
    sign_ = 1.0;
  }
  float a = -1.0 / (sign_ + n.z);
  float b = n.x * n.y * a;
  b1 = vec3(1.0 + sign_ * n.x * n.x * a, sign_ * b, -sign_ * n.x);
  b2 = vec3(b, sign_ + n.y * n.y * a, -n.y);
}

vec4 Project(vec4 a) {
  return a / a.w;
}

float GetDepth(vec3 posWorld) {
  float depth = (vWorldToScreen * vec4(posWorld, 1.0)).w;
  return depth;
}

/*
 * Transform point from world space to screen space([0, 1] x [0, 1])
 *
 */
vec2 GetScreenCoordinate(vec3 posWorld) {
  vec2 uv = Project(vWorldToScreen * vec4(posWorld, 1.0)).xy * 0.5 + 0.5;
  return uv;
}

float GetGBufferDepth(vec2 uv) {
  float depth = texture2D(uGDepth, uv).x;
  if (depth < 1e-2) {
    depth = 1000.0;
  }
  return depth;
}

vec3 GetGBufferNormalWorld(vec2 uv) {
  vec3 normal = texture2D(uGNormalWorld, uv).xyz;
  return normal;
}

vec3 GetGBufferPosWorld(vec2 uv) {
  vec3 posWorld = texture2D(uGPosWorld, uv).xyz;
  return posWorld;
}

float GetGBufferuShadow(vec2 uv) {
  float visibility = texture2D(uGShadow, uv).x;
  return visibility;
}

vec3 GetGBufferDiffuse(vec2 uv) {
  vec3 diffuse = texture2D(uGDiffuse, uv).xyz;
  diffuse = pow(diffuse, vec3(2.2));
  return diffuse;
}

/*
 * Evaluate diffuse bsdf value.
 *
 * wi, wo are all in world space.
 * uv is in screen space, [0, 1] x [0, 1].
 *
 */
vec3 EvalDiffuse(vec3 wi, vec3 wo, vec2 uv) {
  vec3 normal = normalize(GetGBufferNormalWorld(uv));
  vec3 diffuse = max(dot(wi, normal), 0.0) * GetGBufferDiffuse(uv);
  return diffuse;
}

/*
 * Evaluate directional light with shadow map
 * uv is in screen space, [0, 1] x [0, 1].
 *
 */
vec3 EvalDirectionalLight(vec2 uv) {
  vec3 Le = uLightRadiance;
  float v = GetGBufferuShadow(uv);
  return Le * v;
}
#define STEP_NUM 30
#define STEP 0.1
#define EPS 0.05
bool RayMarch(vec3 ori, vec3 dir, out vec3 hitPos) {
  vec3 currentpos = ori + dir * 0.1; 
  vec3 inc = dir * STEP;
  for(int i = 0; i < STEP_NUM; i ++)
  {
    float currentdepth = GetDepth(currentpos);
    vec2 currentuv = GetScreenCoordinate(currentpos);
    float checkdepth = GetGBufferDepth(currentuv);
    if(abs(currentdepth - checkdepth) < EPS)
    {
      hitPos = currentpos;
      return true;
    }
    currentpos += inc; 
  }
  return false;
}

#define SAMPLE_NUM 50 

void main() {
  float s = InitRand(gl_FragCoord.xy);

  vec2 uv = GetScreenCoordinate(vPosWorld.xyz);
  vec3 viewDir = normalize(uCameraPos - vPosWorld.xyz);
  vec3 lightDir = normalize(uLightDir);
  vec3 b1, b2;
  vec3 normal = normalize(GetGBufferNormalWorld(uv));
  LocalBasis(normal, b1, b2);
  mat3 t = mat3(b1, b2, normal);
  vec3 color = vec3(0.0);
  // float pdf;
  // vec3 localdir = normalize(SampleHemisphereUniform(s, pdf)); 
  // gl_FragColor = vec4(t * localdir, 1.0);
  // 1  
  /*
  color = EvalDiffuse(lightDir, viewDir, uv) * EvalDirectionalLight(uv);
 */ 
  // 2
  /*
  vec3 hitPos = vec3(0.0);
  if(RayMarch(vPosWorld.xyz, normalize(reflect(-viewDir, normal)), hitPos))
    color = GetGBufferDiffuse(GetScreenCoordinate(hitPos));
 */ 
  //3
  // /*
  vec3 L2 = vec3(0.0);
  float pdf;
  for(int i = 0; i < SAMPLE_NUM; i ++) {
    vec3 localdir = normalize(SampleHemisphereUniform(s, pdf)); 
    // vec3 localdir = normalize(SampleHemisphereCos(s, pdf));
    vec3 dir = t * localdir;
    vec3 hitPos;
    if(RayMarch(vPosWorld.xyz, dir, hitPos)) {
      vec2 hituv = GetScreenCoordinate(hitPos);
      L2 += EvalDiffuse(dir, viewDir, uv) * EvalDiffuse(lightDir, -dir, hituv) * EvalDirectionalLight(hituv) / pdf;
    }
  }
  L2 /= float(SAMPLE_NUM);
  vec3 L = EvalDiffuse(lightDir, viewDir, uv) * EvalDirectionalLight(uv);
  color = L + L2;
//  */ 
  gl_FragColor = vec4(pow(clamp(color, vec3(0.0), vec3(1.0)), vec3(1.0 / 2.2)), 1.0);
}
