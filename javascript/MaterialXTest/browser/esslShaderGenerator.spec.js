// MaterialX is served through a script tag in the test setup.

function createStandardSurfaceMaterial(mx)
{
    const doc = mx.createDocument();
    const ssName = 'SR_default';
    const ssNode = doc.addChildOfCategory('standard_surface', ssName);
    ssNode.setType('surfaceshader');
    const smNode = doc.addChildOfCategory('surfacematerial', 'Default');
    smNode.setType('material');
    const shaderElement = smNode.addInput('surfaceshader');
    shaderElement.setType('surfaceshader');
    shaderElement.setNodeName(ssName);
    expect(doc.validate()).to.be.true;
    return doc;
}

describe('Generate ESSL Shaders', function ()
{
    let mx;
    const canvas = document.createElement('canvas');
    const gl = canvas.getContext('webgl2');

    this.timeout(60000);

    before(async function ()
    {
        mx = await MaterialX();
    });

    it('Compile Shaders', () =>
    {
        const doc = createStandardSurfaceMaterial(mx);
        const elem = mx.findRenderableElement(doc);

        const gen = new mx.EsslShaderGenerator();
        const genGLSL = new mx.GlslShaderGenerator();
        const genMSL = new mx.MslShaderGenerator();
        const genVK = new mx.VkShaderGenerator();

        const genContextGLSL = new mx.GenContext(genGLSL);
        const genContextMSL = new mx.GenContext(genMSL);
        const genContextVK = new mx.GenContext(genVK);

        //const stdlib3 = mx.loadStandardLibraries(genContextGLSL);
        //const stdlib4 = mx.loadStandardLibraries(genContextMSL);
        //const stdlib5 = mx.loadStandardLibraries(genContextVK);

        const genContext = new mx.GenContext(gen);
        const stdlib = mx.loadStandardLibraries(genContext);
        doc.importLibrary(stdlib);
        try
        {
            const genOSL = new mx.OslShaderGenerator();
            const genContextOSL = new mx.GenContext(genOSL);
            const stdlib2 = mx.loadStandardLibraries(genContextOSL);
            const mxShaderOSL = genOSL.generate(elem.getNamePath(), elem, genContextOSL);
            const fShaderOSL = mxShaderOSL.getSourceCode("pixel");
            console.log('------------------- START OSL Shader ------------\n', fShaderOSL);
            console.log('------------------- END OSL Shader ------------\n');
    
            const mxShaderGLSL = genGLSL.generate(elem.getNamePath(), elem, genContextGLSL);
            const fShaderGLSL = mxShaderGLSL.getSourceCode("pixel");
            console.log('------------------- START GLSL Shader ------------\n', fShaderGLSL);
            console.log('------------------- END GLSL Shader ------------\n');

            const mxShaderMSL = genMSL.generate(elem.getNamePath(), elem, genContextMSL);
            const fShaderMSL = mxShaderMSL.getSourceCode("pixel");
            console.log('------------------- Start Metal Shader ------------\n', fShaderMSL);
            console.log('------------------- End Metal Shader ------------\n', fShaderMSL);

            const mxShaderVK = genVK.generate(elem.getNamePath(), elem, genContextVK);
            const fShaderVK = mxShaderVK.getSourceCode("pixel");
            console.log('------------------- START Vulkan Shader ------------\n', fShaderVK);
            console.log('------------------- END Vulkan Shader ------------\n');

            const mxShader = gen.generate(elem.getNamePath(), elem, genContext);
            const fShader = mxShader.getSourceCode("pixel");
            const vShader = mxShader.getSourceCode("vertex");
            const glVertexShader = gl.createShader(gl.VERTEX_SHADER);
            gl.shaderSource(glVertexShader, vShader);
            gl.compileShader(glVertexShader);
            if (!gl.getShaderParameter(glVertexShader, gl.COMPILE_STATUS))
            {
                console.error("-------- VERTEX SHADER FAILED TO COMPILE: ----------------");
                console.error("--- VERTEX SHADER LOG ---");
                console.error(gl.getShaderInfoLog(glVertexShader));
                console.error("--- VERTEX SHADER START ---");
                console.error(fShader);
                console.error("--- VERTEX SHADER END ---");
            }
            expect(gl.getShaderParameter(glVertexShader, gl.COMPILE_STATUS)).to.equal(true);

            const glPixelShader = gl.createShader(gl.FRAGMENT_SHADER);
            gl.shaderSource(glPixelShader, fShader);
            gl.compileShader(glPixelShader);
            if (!gl.getShaderParameter(glPixelShader, gl.COMPILE_STATUS))
            {
                console.error("-------- PIXEL SHADER FAILED TO COMPILE: ----------------");
                console.error("--- PIXEL SHADER LOG ---");
                console.error(gl.getShaderInfoLog(glPixelShader));
                console.error("--- PIXEL SHADER START ---");
                console.error(fShader);
                console.error("--- PIXEL SHADER END ---");
            }
            expect(gl.getShaderParameter(glPixelShader, gl.COMPILE_STATUS)).to.equal(true);
        }
        catch (errPtr)
        {
            console.error("-------- Failed code generation: ----------------");
            console.error(mx.getExceptionMessage(errPtr));
        }
    });
});
