async function runGame() {
    // Create the WASM module and wait for it to initialize
    const Module = await createModule();
    let canvas = document.getElementById('canvas');
    let canvasCtx = canvas.getContext('2d');
    let screenData = canvasCtx.createImageData(128, 128);
    let screenCanvas = document.createElement('canvas');
    screenCanvas.width = 128;
    screenCanvas.height = 128;
    let screenCanvasCtx = screenCanvas.getContext('2d');

    // Wrap the init and update functions
    const init = Module.cwrap('init', 'void', ['void*']);
    const update = Module.cwrap('update', 'void', ['void*']);
    const RuntimeContext_create = Module.cwrap('RuntimeContext_create', 'void*', []);
    const RuntimeContext_setUTimeCallback = Module.cwrap('RuntimeContext_setUTimeCallback', 'void', ['void*']);
    const N = 'number'
    const RuntimeContext_updateInputs = Module.cwrap('RuntimeContext_updateInputs', 'void', 
        ['void*', N, N, N, N, N, N, N, N, N, N, N]);
    const RuntimeContext_getScreen = Module.cwrap('RuntimeContext_getScreen', 'void*', ['void*']);

    // Define the JavaScript function to return microseconds since the app started
    let startTime = performance.now();
    function getMicroseconds() {
        return (performance.now() - startTime) * 1000;
    }

    // Use addFunction to create a function pointer for the JavaScript function
    const getMicrosecondsPtr = Module.addFunction(getMicroseconds, 'i');

    const ctx = RuntimeContext_create();

    RuntimeContext_setUTimeCallback(ctx, getMicrosecondsPtr);


    let arrowKeys = { left: false, up: false, right: false, down: false, menu: false, shoulderRight: false, shoulderLeft: false, a: false, b: false, p:false};
    let keyMap = { 
        'ArrowLeft': 'left', 'a': 'left',
        'ArrowUp': 'up', 'w': 'up',
        'ArrowRight': 'right', 'd': 'right',
        'ArrowDown': 'down',  's': 'down',
        'e': 'shoulderRight', 'q': 'shoulderLeft',
        'i': 'a', 'j': 'b', 'p': 'p', 
        'm': 'menu' };
    // Add event listeners for arrow keys
    window.addEventListener('keydown', (event) => {
        let key = keyMap[event.key];
        if (key) {
            arrowKeys[key] = true;
        }
    });

    window.addEventListener('keyup', (event) => {
        let key = keyMap[event.key];
        if (key) {
            arrowKeys[key] = false;
        }
    });



    // Call the init function
    init(ctx);

    // Set up the game loop
    let previousTime = performance.now() / 1000;
    let simTime = 0;
    function gameLoop() {
        let currentTime = performance.now() / 1000;
        let dt = (currentTime - previousTime);
        previousTime = currentTime;

        // cap dt to 0.2
        if (dt > 0.2) {
            dt = 0.2;
        }
        simTime += dt;
        RuntimeContext_updateInputs(ctx, currentTime, dt, 
            arrowKeys.up, arrowKeys.right, arrowKeys.down, arrowKeys.left, arrowKeys.a, arrowKeys.b, arrowKeys.menu, arrowKeys.shoulderLeft, arrowKeys.shoulderRight);
        update(ctx);
        
        // Get the screen data and update the canvas
        let screenPtr = RuntimeContext_getScreen(ctx);
        let screenDataArray = new Uint8Array(Module.HEAPU8.buffer, screenPtr, 128 * 128 * 4);
        screenData.data.set(screenDataArray);
        // remove alpha channel data (which is z data)
        const pixels = screenData.data;
        for (let i = 3, n = canvas.width * canvas.height * 4; i < n; i += 4) {
            pixels[i] = 255;
        }
        canvasCtx.imageSmoothingEnabled = false;
        screenCanvasCtx.putImageData(screenData, 0, 0);
        canvasCtx.drawImage(screenCanvas, 0, 0, canvas.width, canvas.height);
        requestAnimationFrame(gameLoop);
    }

    requestAnimationFrame(gameLoop);
}

// Run the game
runGame().catch(console.error);