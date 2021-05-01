attribute vec3 aVertexPosition;
attribute mat3 aPrecomputeLT;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

varying vec3 vColor;

void main(void) {

  gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix *
                vec4(aVertexPosition, 1.0);
  vColor.x = dot(aPrecomputeLT[0] , vec3(1.0, 1.0, 1.0));
  vColor.y = dot(aPrecomputeLT[1] , vec3(1.0, 1.0, 1.0));
  vColor.z = dot(aPrecomputeLT[2] , vec3(1.0, 1.0, 1.0));

}