class PRTMaterial extends Material {

    constructor(color, translate, scale, vertexShader, fragmentShader) {
        console.log('PRTMaterial');
        let envlight = precomputeL[0];
        console.log(envlight);
        var mr = new Float32Array(envlight.length);
        var mg = new Float32Array(envlight.length);
        var mb = new Float32Array(envlight.length);
        for(let i = 0; i < envlight.length; i ++) {
            mr[i] = envlight[i][0];
            mg[i] = envlight[i][1];
            mb[i] = envlight[i][2];
        }
        console.log(mr);
        super({
            'uSampler': { type: 'texture', value: color },
            'uPrecomputeLR':{type: 'matrix3fv', value: mr}, 
            'uPrecomputeLG':{type:'matrix3fv', value: mg},
            'uPrecomputeLB':{type:'matrix3fv', value: mb},
            'uLightWeight' :{type: '1f', value:0.5}
        }, [
            'aPrecomputeLT'
        ], vertexShader, fragmentShader, null);
    }
}

async function buildPRTMaterial(color, translate, scale, vertexPath, fragmentPath) {
    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);
    return new PRTMaterial(color, translate, scale, vertexShader, fragmentShader);

}