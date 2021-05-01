attribute vec3 aVertexPosition;
attribute mat3 aPrecomputeLT;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat3 uPrecomputeLR;
uniform mat3 uPrecomputeLG;
uniform mat3 uPrecomputeLB;

varying vec3 vColor;

float compute_color(mat3 precomputeL)
{
  float color = 0.0;
  color += dot(aPrecomputeLT[0], precomputeL[0]);
  color += dot(aPrecomputeLT[1], precomputeL[1]);
  color += dot(aPrecomputeLT[2], precomputeL[2]);
  return color;
}
void main(void) {

  gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix *
                vec4(aVertexPosition, 1.0);
  vColor.x = compute_color(uPrecomputeLR);
  vColor.y = compute_color(uPrecomputeLG);
  vColor.z = compute_color(uPrecomputeLB);
}