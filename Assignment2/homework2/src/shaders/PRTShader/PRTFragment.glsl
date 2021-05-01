#ifdef GL_ES
precision mediump float;
#endif

// Phong related variables
uniform sampler2D uSampler;
uniform vec3 uKd;
uniform vec3 uKs;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform vec3 uLightRadiance;

varying highp vec2 vTextureCoord;
varying highp vec3 vFragPos;
varying highp vec3 vNormal;

#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586


varying vec4 vPositionFromLight;


void main(void) {

  gl_FragColor = vec4(0.5, 0.0, 0.0, 1.0);
}
