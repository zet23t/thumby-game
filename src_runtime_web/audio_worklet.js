class GameAudioProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this.port.onmessage = async (event) => {
            if (event.data.type === 'init') {
                this.instructionsQueue = [];
                const wasmModule = event.data.wasmModule;
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
                // console.log("sfxInstructions", sfxInstructions)
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