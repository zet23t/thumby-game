class GameAudioProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this.port.onmessage = (event) => {
            this.int16Buffer = new Int16Array(event.data.sharedArrayBuffer);
            this.bufferPos = 0;
        };
    }

    process(inputs, outputs, parameters) {
        if (!this.int16Buffer) {
            return true;
        }
        let output = outputs[0];
        let buffer = output[0];

        let bufferPos = this.bufferPos;
        let int16Buffer = this.int16Buffer;
        let frameCount = buffer.length
        for (let i = 0; i < frameCount; i++) {
            let sample = int16Buffer[bufferPos++%int16Buffer.length];
            buffer[i] = sample / 32768;
        }
        this.bufferPos = bufferPos;
        this.port.postMessage({ frameCount: frameCount });
        
        return true;
    }
}

registerProcessor('game-audio-processor', GameAudioProcessor);