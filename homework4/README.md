# GAMES202 homework3 完成情况
* lut-gen项目在visual studio下编译有点问题，所以修改了部分代码
** uint8_t data[resolution * resolution * 3]; resolution非常量，无法编译，改成uint8_t *data = new uint8_t[resolution * resolution * 3];
** M_PI没在vc的math.h中定义， 添加 #define M_PI       3.14159265358979323846 
* Screen Space Ray Tracing
* 间接光照