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
    const N = 'number', VS = 'void*', V = 'void';
    const init = Module.cwrap('init', V, [VS]);
    const update = Module.cwrap('update', V, [VS]);
    const RuntimeContext_create = Module.cwrap('RuntimeContext_create', VS, []);
    const RuntimeContext_setUTimeCallback = Module.cwrap('RuntimeContext_setUTimeCallback', V, [VS]);
    const RuntimeContext_updateInputs = Module.cwrap('RuntimeContext_updateInputs', V,
        [VS, N, N, N, N, N, N, N, N, N, N, N]);
    const RuntimeContext_getScreen = Module.cwrap('RuntimeContext_getScreen', VS, [VS]);
    const AudioContext_create = Module.cwrap('AudioContext_create', V, []);
    const AudioContext_audioUpdate = Module.cwrap('AudioContext_audioUpdate', V, [VS, VS, VS, N]);
    const AudioContext_beforeRuntimeUpdate = Module.cwrap('AudioContext_beforeRuntimeUpdate', V, [VS, VS]);
    const AudioContext_afterRuntimeUpdate = Module.cwrap('AudioContext_afterRuntimeUpdate', V, [VS, VS]);

    const AudioContext_getChannelStatus = Module.cwrap('AudioContext_getChannelStatus', VS,[VS, VS]);
    const AudioContext_setSfxInstructions = Module.cwrap('AudioContext_setSfxInstructions', V,[VS, VS]);
    const RuntimeContext_getSfxInstructions = Module.cwrap('RuntimeContext_getSfxInstructions', VS,[VS, VS]);
    const RuntimeContext_setSfxChannelStatus = Module.cwrap('RuntimeContext_setSfxChannelStatus', V,[VS, VS]);
    const RuntimeContext_clearSfxInstructions = Module.cwrap('RuntimeContext_clearSfxInstructions', V,[VS]);

    
    // Define the JavaScript function to return microseconds since the app started
    let startTime = performance.now();
    function getMicroseconds() {
        return (performance.now() - startTime) * 1000;
    }

    // Use addFunction to create a function pointer for the JavaScript function
    const getMicrosecondsPtr = Module.addFunction(getMicroseconds, 'i');

    const ctx = RuntimeContext_create();
    const exchangeBuffer = Module._malloc(2048);

    RuntimeContext_setUTimeCallback(ctx, getMicrosecondsPtr);


    let arrowKeys = { left: false, up: false, right: false, down: false, menu: false, shoulderRight: false, shoulderLeft: false, a: false, b: false, p: false };
    let keyMap = {
        'ArrowLeft': 'left', 'a': 'left',
        'ArrowUp': 'up', 'w': 'up',
        'ArrowRight': 'right', 'd': 'right',
        'ArrowDown': 'down', 's': 'down',
        'e': 'shoulderRight', 'q': 'shoulderLeft',
        'i': 'a', 'j': 'b', 'p': 'p',
        'm': 'menu'
    };
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

    let audioCtx = new (window.AudioContext || window.webkitAudioContext)(
        { sampleRate: 20050 }
    );
    let audioWorkletNode;
    let sharedArrayBuffer = new SharedArrayBuffer(2048 * 8);
    let int16Buffer = new Int16Array(sharedArrayBuffer);
    let sharedArrayBufferIndex = 0;
    let wasmBuffer = Module._malloc(2048 * 8);
    console.log("wasmBuffer", wasmBuffer, Module.HEAP16);

    
    // Load and compile the WebAssembly module
    async function loadWasmModule(url) {
        const response = await fetch(url);
        const bytes = await response.arrayBuffer();
        const wasmModule = await WebAssembly.compile(bytes);
        return wasmModule;
    }

    const wasmModule = await loadWasmModule('game.wasm');

    audioCtx.audioWorklet.addModule('audio_worklet.js').then(() => {
        audioWorkletNode = new AudioWorkletNode(audioCtx, 'game-audio-processor');
        audioWorkletNode.port.postMessage({ type: 'init', wasmModule: wasmModule });
        audioWorkletNode.port.onmessage = (event) => {
            let { channelStatus } = event.data;
            for (let i = 0; i < channelStatus.length; i++) {
                Module.HEAPU8[exchangeBuffer + i] = channelStatus[i];
            }
            RuntimeContext_setSfxChannelStatus(ctx, exchangeBuffer);
        };
        audioWorkletNode.connect(audioCtx.destination);
    });

    audioCtx.resume();

    // Call the init function
    init(ctx);

    // Set up the game loop
    let previousTime = performance.now() / 1000;
    let simTime = 0;
    let intPtr = Module._malloc(4);
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

        // AudioContext_beforeRuntimeUpdate(engineAudioCtx, ctx);
        update(ctx);
        let sfxPtr = RuntimeContext_getSfxInstructions(ctx, intPtr);
        let sfxPtrSize = Module.getValue(intPtr, 'i32');
        // Create a Uint8Array view of the data buffer
        let sfxInstructions = new Uint8Array(Module.HEAPU8.buffer, sfxPtr, sfxPtrSize);
        // Post the data buffer to the AudioWorkletProcessor
        if (audioWorkletNode)
        {
            let array = [];
            for (let i = 0; i < sfxInstructions.length; i++) {
                array.push(sfxInstructions[i]);
            }
            audioWorkletNode.port.postMessage({
                type: 'sfxInstructions',
                 data: array
            });
            RuntimeContext_clearSfxInstructions(ctx);
        }
        
        // AudioContext_afterRuntimeUpdate(engineAudioCtx, ctx);

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

// runGame().catch(console.error);