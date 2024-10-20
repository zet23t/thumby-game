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


                if (event.data.type === 'sfxInstructions') {
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

const hasMouse = window.matchMedia('(pointer:fine)').matches;
let baseSize = 0

function FillCircle(ctx, x, y, radius) {
    ctx.beginPath();
    ctx.arc(x, y, radius, 0, 2 * Math.PI);
    ctx.fill();
}

function FillRoundedRect(ctx, x, y, width, height, radius) {
    ctx.beginPath();
    ctx.ellipse(x + radius, y + radius, radius, radius, 0, Math.PI, -Math.PI * 0.5);
    ctx.lineTo(x + width - radius, y);
    ctx.ellipse(x + width - radius, y + radius, radius, radius, 0, -Math.PI * 0.5, 0);
    ctx.lineTo(x + width, y + height - radius);
    ctx.ellipse(x + width - radius, y + height - radius, radius, radius, 0, 0, Math.PI * 0.5);
    ctx.lineTo(x + radius, y + height);
    ctx.ellipse(x + radius, y + height - radius, radius, radius, 0, Math.PI * 0.5, Math.PI);
    ctx.lineTo(x, y + radius);
    ctx.fill();
}

function DrawHandheld(canvas, canvasCtx, baseSize) {
    canvasCtx.clearRect(0, 0, canvas.width, canvas.height);
    canvasCtx.fillStyle = '#831';
    FillRoundedRect(canvasCtx, 0, 0, canvas.width, canvas.height, 20);
    let width = canvas.width;
    let height = canvas.height;

    let hotkeySize = baseSize * .075
    let hotkeyFont = hotkeySize * .7 + 'px sans-serif'
    let drawHotKeyInfo = (x, y, key) => {
        canvasCtx.fillStyle = '#35a'
        FillRoundedRect(canvasCtx, x - hotkeySize * .5, y - hotkeySize * .5,
            hotkeySize, hotkeySize, hotkeySize * .2)
        canvasCtx.stroke()
        canvasCtx.font = hotkeyFont
        canvasCtx.fillStyle = '#fff'
        canvasCtx.textAlign = 'center';
        canvasCtx.textBaseline = 'middle';
        canvasCtx.fillText(key, x, y)
    }
    const bs = baseSize
    if (width > height) {
        // draw landscape handheld

        // dpad
        canvasCtx.fillStyle = '#000';
        let x = bs * .25;
        let y = bs * .5;
        let r = bs * .18;
        let offset = bs * .01;
        FillCircle(canvasCtx, x, y, r);
        canvasCtx.fillStyle = '#555';
        FillCircle(canvasCtx, x, y, r - offset);
        canvasCtx.fillStyle = '#777';

        FillRoundedRect(canvasCtx, x - r * .8, y - r * .3, r * 1.6, r * .6, r * .15);
        FillRoundedRect(canvasCtx, x - r * .3, y - r * .8, r * .6, r * 1.6, r * .15);
        drawHotKeyInfo(x, y - r * .5, "w")
        drawHotKeyInfo(x, y + r * .5, "s")
        drawHotKeyInfo(x - r * .5, y, "a")
        drawHotKeyInfo(x + r * .5, y, "d")

        // menu button
        canvasCtx.fillStyle = "#000";
        FillRoundedRect(canvasCtx, x - bs * .1, y + bs * .3, bs * .3, bs * .1, bs * .03)
        canvasCtx.fillStyle = "#a00";
        FillRoundedRect(canvasCtx, x - bs * .1 + offset, y + bs * .3 + offset, 
            bs * .3 - offset * 2, bs * .1 - offset * 2, bs * .03 - offset * .5)
        canvasCtx.fillStyle = "#fff"
        canvasCtx.font = bs * .05 + "px sans-serif"
        canvasCtx.fillText("MENU", x - bs * .1 + bs * .15, y + bs * .3 + bs * .055)
        drawHotKeyInfo(x - bs * .1, y + bs * .3 + bs * .1, 'm')

        // shoulder button
        let shoulderButton = function (bx, by, label, hotkey) {
            const h = bs * .15
            const w = bs * .4
            const x = bx - w * .5
            const y = by - bs * 0.075
            canvasCtx.fillStyle = '#000';
            FillRoundedRect(canvasCtx, x, y, w, h, h * .25)
            canvasCtx.fillStyle = '#f80';
            FillRoundedRect(canvasCtx, x + offset, y + offset, w - offset * 2, h - offset * 2, h * .25 - offset * .5)
            canvasCtx.fillStyle = "#000"
            canvasCtx.font = h * .5 + "px sans-serif"
            canvasCtx.fillText(label, bx, by)
            drawHotKeyInfo(x + h * .5, by, hotkey)
        }

        shoulderButton(bs * .25, bs * .15, "L", "q")
        shoulderButton(width - bs * .25, bs * .15, "R", "e")

        // button A / B
        let drawButton = function (bx, by, label, hotkey) {
            canvasCtx.fillStyle = '#000';
            FillCircle(canvasCtx, bx, by, bs * .1);
            canvasCtx.fillStyle = '#f80';
            FillCircle(canvasCtx, bx, by, bs * .1 - offset);
            canvasCtx.fillStyle = '#000';
            canvasCtx.font = (r * .5) + 'px sans-serif';
            canvasCtx.textAlign = 'center';
            canvasCtx.textBaseline = 'middle';
            canvasCtx.fillText(label, bx, by + r * .015);

            drawHotKeyInfo(bx, by + bs * .1, hotkey)
        }

        drawButton(width - bs * .15, bs * .5, 'A', 'i')
        drawButton(width - bs * .35, bs * .65, 'B', 'j')


    }
    canvasCtx.fillStyle = '#fff';
}

async function setupPlaceholder() {
    let canvas = document.getElementById('canvas');
    if (!canvas) {
        // not ready yet, try again in 30ms
        setTimeout(setupPlaceholder, 30);
        return;
    }
    baseSize = canvas.width;
    if (screen.width < screen.height) {
        canvas.width = baseSize;
        canvas.height = baseSize * 2;
    }
    else {
        canvas.width = baseSize * 2;
        canvas.height = baseSize;
    }

    let canvasCtx = canvas.getContext('2d');
    canvas.tabIndex = 0;


    // draw button like placeholder that says "click to start" and calls runGame

    DrawHandheld(canvas, canvasCtx, baseSize);
    canvasCtx.fillStyle = 'black';
    canvasCtx.fillRect(
        (canvas.width - baseSize) * .5,
        (canvas.height - baseSize) * .5, baseSize, baseSize);

    canvasCtx.fillStyle = 'white';
    canvasCtx.font = '24px sans-serif';
    canvasCtx.textAlign = 'center';
    canvasCtx.textBaseline = 'middle';
    const startMessage = !hasMouse ? 'Tap to start' : 'Click to start';
    canvasCtx.fillText(startMessage, canvas.width / 2, canvas.height / 2);

    let onClick = () => {
        canvasCtx.clearRect(0, 0, canvas.width, canvas.height);
        canvasCtx.fillStyle = 'black';
        canvasCtx.fillRect(0, 0, canvas.width, canvas.height);

        canvasCtx.fillStyle = 'white';
        canvasCtx.font = '24px sans-serif';
        canvasCtx.textAlign = 'center';
        canvasCtx.textBaseline = 'middle';
        canvasCtx.fillText('Loading...', canvas.width / 2, canvas.height / 2);

        runGame().catch(console.error);
        canvas.removeEventListener('click', onClick);
    };
    canvas.addEventListener('click', onClick);
}

async function runGame() {
    let canvas = document.getElementById('canvas');
    let canvasCtx = canvas.getContext('2d');
    // Create the WASM module and wait for it to initialize
    const Module = await createModule();
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
    const RuntimeContext_getSfxInstructions = Module.cwrap('RuntimeContext_getSfxInstructions', VS, [VS, VS]);
    const RuntimeContext_setSfxChannelStatus = Module.cwrap('RuntimeContext_setSfxChannelStatus', V, [VS, VS]);
    const RuntimeContext_clearSfxInstructions = Module.cwrap('RuntimeContext_clearSfxInstructions', V, [VS]);


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
    canvas.addEventListener('keydown', (event) => {
        let key = keyMap[event.key];
        if (key) {
            arrowKeys[key] = true;
        }
    });

    canvas.addEventListener('keyup', (event) => {
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
            if (event.data.type === 'ready') {
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


    let isPaused = false;
    canvas.addEventListener('blur', () => {
        audioCtx.suspend();
        isPaused = true;
        console.log("paused");
    });
    canvas.addEventListener('focus', () => {
        audioCtx.resume();
        isPaused = false;
        console.log("resumed");
    });

    // Call the init function
    init(ctx);

    // Set up the game loop
    let previousTime = performance.now() / 1000;
    let simTime = 0;
    let intPtr = Module._malloc(4);
    let screenWidth, screenHeight;
    function gameLoop() {
        let currentTime = performance.now() / 1000;
        let dt = (currentTime - previousTime);
        previousTime = currentTime;

        if (screenWidth !== screen.width || screenHeight !== screen.height) {
            screenWidth = screen.width;
            screenHeight = screen.height;
            if (screen.width < screen.height) {
                canvas.width = baseSize;
                canvas.height = baseSize * 2;
            }
            else {
                canvas.width = baseSize * 2;
                canvas.height = baseSize;
            }
        }

        // draw rounded rectangle
        DrawHandheld(canvas, canvasCtx, baseSize);



        canvasCtx.fillStyle = 'white';

        if (!isPaused) {
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
            if (audioWorkletNode) {
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
            canvasCtx.drawImage(screenCanvas,
                (canvas.width - baseSize) * .5,
                (canvas.height - baseSize) * .5,
                baseSize, baseSize);
        }
        else {
            // pause

            DrawHandheld(canvas, canvasCtx, baseSize)
            canvasCtx.drawImage(screenCanvas,
                (canvas.width - baseSize) * .5,
                (canvas.height - baseSize) * .5,
                baseSize, baseSize);

            canvasCtx.fillStyle = 'rgba(0, 0, 0, 0.25)';
            canvasCtx.fillRect(0, 0, canvas.width, canvas.height);
            canvasCtx.fillStyle = 'rgba(0, 0, 0, 0.75)';
            canvasCtx.fillRect(0, canvas.height / 2 - 25, canvas.width, 50);
            canvasCtx.fillStyle = 'white';
            canvasCtx.font = '24px sans-serif';
            canvasCtx.textAlign = 'center';
            canvasCtx.textBaseline = 'middle';
            const pauseMessage = !hasMouse ? 'Paused - tap to continue' : 'Paused - click to continue';

            canvasCtx.fillText(pauseMessage, canvas.width / 2, canvas.height / 2);

        }
        requestAnimationFrame(gameLoop);
    }

    requestAnimationFrame(gameLoop);
}

// runGame().catch(console.error);
setupPlaceholder().catch(console.error);