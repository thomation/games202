class PRTMaterial extends Material {

    constructor(color, translate, scale, vertexShader, fragmentShader) {
        console.log('PRTMaterial');

        super({
            'uSampler': { type: 'texture', value: color },
        }, [
            'aPrecomputeLT'
        ], vertexShader, fragmentShader, null);
    }
}

async function buildPRTMaterial(color, translate, scale, vertexPath, fragmentPath) {

    console.log('build prt material');
    console.log(precomputeL[guiParams.envmapId]);
    console.log(precomputeLT[guiParams.envmapId]);
    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    console.log('build prt material');
    return new PRTMaterial(color, translate, scale, vertexShader, fragmentShader);

}