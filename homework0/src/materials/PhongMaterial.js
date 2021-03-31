class PhongMaterial extends Material {
    constructor(color, colorMap, specular, intensity) {
        console.log("In PhoneMatterial");
        let textureSample = 0;
        if (colorMap != null) {
            console.log("with color map");
            textureSample = 1;
            super({
                'uSampler': { type: 'texture', value: colorMap },
                'uTextureSample': { type: '1i', value: textureSample },
                'uKd': { type: '3fv', value: color },
                'uKs': { type: '3fv', value: specular },
                'uLightIntensity': { type: '1f', value: intensity }
            }, [], PhongVertexShader, PhongFragmentShader);
        } else {
            console.log(color);
            super({
                'uTextureSample': { type: '1i', value: textureSample },
                'uKd': { type: '3fv', value: color },
                'uKs': { type: '3fv', value: specular },
                'uLightIntensity': { type: '1f', value: intensity }

            }, [], PhongVertexShader, PhongFragmentShader);
        }

    }

}