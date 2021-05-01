class PRTMaterial extends Material {

    constructor(color, translate, scale, vertexShader, fragmentShader) {
        console.log('PRTMaterial');

        super({
            // Phong
            'uSampler': { type: 'texture', value: color },
        }, [], vertexShader, fragmentShader, null);
    }
}

async function buildPRTMaterial(color, translate, scale, vertexPath, fragmentPath) {

    console.log('build prt material');
    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    console.log('build prt material');
    return new PRTMaterial(color, translate, scale, vertexShader, fragmentShader);

}