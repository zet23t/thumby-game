function audioWorkletProcessor() {
    class GameAudioProcessor extends AudioWorkletProcessor {
        constructor() {
            super();
            this.instructionsQueue = [];
            console.log("Audio Worklet constructor exec, sending ready");
            this.port.postMessage({ type: 'ready' });
            this.port.onmessage = async (event) => {
                if (event.data.type === 'init') {
                    console.log("Audio Worklet init");
                    const bytes = event.data.wasmData;
                    const wasmModule = await WebAssembly.compile(bytes);
                    const wasmInstance = await WebAssembly.instantiate(wasmModule, {
                        env: {
                            memory: new WebAssembly.Memory({ initial: 16, maximum: 16 }),
                            _emscripten_memcpy_js: (dest, src, num) => {
                                const destView = new Uint8Array(this.wasmInstance.exports.memory.buffer, dest, num);
                                const srcView = new Uint8Array(this.wasmInstance.exports.memory.buffer, src, num);
                                destView.set(srcView);
                            },
                            emscripten_resize_heap: (size) => {
                                console.error('emscripten_resize_heap is not supported in this context');
                                return false;
                            },
                            abort: () => console.error('abort')
                        },
                        wasi_snapshot_preview1: {
                            fd_write: (fd, iovs, iovs_len, nwritten) => {
                                const memory = this.wasmInstance.exports.memory;
                                const HEAPU8 = new Uint8Array(memory.buffer);
                                const HEAP32 = new Int32Array(memory.buffer);
                                let written = 0;
                                let str = '';
                                for (let i = 0; i < iovs_len; i++) {
                                    const ptr = HEAP32[(iovs >> 2) + i * 2];
                                    const len = HEAP32[(iovs >> 2) + i * 2 + 1];
                                    for (let j = 0; j < len; j++) {
                                        str += String.fromCharCode(HEAPU8[ptr + j]);
                                    }
                                    written += len;
                                }
                                console.log(str);
                                HEAP32[nwritten >> 2] = written;
                                return 0; // WASI_ESUCCESS
                            }
                        }
                    });
                    this.wasmInstance = wasmInstance;
                    const AudioContext_audioUpdate = this.wasmInstance.exports.AudioContext_audioUpdate;
                    const AudioContext_create = this.wasmInstance.exports.AudioContext_create;
                    const audioCtx = AudioContext_create();
                    this.AudioContext_setSfxInstructions = this.wasmInstance.exports.AudioContext_setSfxInstructions;
                    this.AudioContext_getChannelStatus = this.wasmInstance.exports.AudioContext_getChannelStatus;
                    this.audioCtx = audioCtx;
                    this.AudioContext_audioUpdate = AudioContext_audioUpdate;
                    this.wasmBuffer = this.wasmInstance.exports.malloc(2048);
                    this.sfxInstructionBuffer = this.wasmInstance.exports.malloc(128);
                }
    
    
                if (event.data.type === 'sfxInstructions')
                {
                    const sfxInstructions = event.data.data
                    this.instructionsQueue.push(sfxInstructions);
                }
            };
        }
    
        process(inputs, outputs, parameters) {
            if (!this.wasmInstance) {
                return true;
            }
            let output = outputs[0];
            let buffer = output[0];
            let frameCount = buffer.length;
            let wasmBuffer = this.wasmBuffer;
            let audioCtx = this.audioCtx;
            let AudioContext_audioUpdate = this.AudioContext_audioUpdate;
            let AudioContext_setSfxInstructions = this.AudioContext_setSfxInstructions;
            let AudioContext_getChannelStatus = this.AudioContext_getChannelStatus;
    
            const memoryBuffer = this.wasmInstance.exports.memory.buffer;
            const HEAPU8 = new Uint8Array(memoryBuffer);
            const HEAP32 = new Int32Array(memoryBuffer);
    
            if (this.instructionsQueue.length > 0) {
                const sfxInstructions = this.instructionsQueue.shift();
                const sfxInstructionBuffer = this.sfxInstructionBuffer;
                for (let i = 0; i < sfxInstructions.length; i++) {
                    HEAPU8[sfxInstructionBuffer + i] = sfxInstructions[i];
                }
            }
    
            AudioContext_setSfxInstructions(audioCtx, this.sfxInstructionBuffer);
            AudioContext_audioUpdate(audioCtx, 20050, 16, wasmBuffer, frameCount);
            let channelStatusPtr = AudioContext_getChannelStatus(audioCtx, this.sfxInstructionBuffer)
            let channelStatus = new Uint8Array(memoryBuffer, channelStatusPtr, 16);
            let channelStatusArray = [];
            let channelStatusSize = HEAP32[this.sfxInstructionBuffer >> 2];
            for (let i = 0; i < channelStatusSize; i++) {
                channelStatusArray.push(channelStatus[i]);
            }
            this.port.postMessage({ channelStatus: channelStatusArray });
            for (let i = 0; i < 128; i++) {
                HEAPU8[this.sfxInstructionBuffer + i] = 0;
            }
    
            const int16View = new Int16Array(memoryBuffer, wasmBuffer, frameCount);
            for (let i = 0; i < frameCount; i++) {
                buffer[i] = int16View[i] / 32768;
            }
    
    
            return true;
        }
    }
    
    registerProcessor('game-audio-processor', GameAudioProcessor);
}

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
    let wasmBuffer = Module._malloc(2048 * 8);
    console.log("wasmBuffer", wasmBuffer, Module.HEAP16);
    
    // Load and the WebAssembly module for the audio worklet
    async function loadWasmData(url) {
        const response = await fetch(url);
        const bytes = await response.arrayBuffer();
        return bytes
    }

    const wasmData = await loadWasmData('game.wasm');

    const blob = new Blob([audioWorkletProcessor.toString(), 'audioWorkletProcessor()'], { type: 'application/javascript' });
    const url = URL.createObjectURL(blob);

    audioCtx.audioWorklet.addModule(url).then(() => {
        console.log('audio worklet loaded, setting up pipeline');
        audioWorkletNode = new AudioWorkletNode(audioCtx, 'game-audio-processor');
        audioWorkletNode.port.onmessage = (event) => {
            if (event.data.type === 'ready')
            {
                console.log("Audio Worklet signaled ready, sending init");
                audioWorkletNode.port.postMessage({ type: 'init', wasmData: wasmData });
                return;
            }
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

        update(ctx);
        let sfxPtr = RuntimeContext_getSfxInstructions(ctx, intPtr);
        let sfxPtrSize = Module.getValue(intPtr, 'i32');
        // Create a Uint8Array view of the data buffer
        let sfxInstructions = new Uint8Array(Module.HEAPU8.buffer, sfxPtr, sfxPtrSize);
        if (audioWorkletNode)
        {
            // Post the data buffer to the AudioWorkletProcessor using dumb array
            // because sending Uint8Array directly is super slow
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