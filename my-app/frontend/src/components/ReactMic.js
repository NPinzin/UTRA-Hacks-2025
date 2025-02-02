import React, { useState } from 'react';
import { ReactMic } from 'react-mic';

export const Mic = () => {
  const [record, setRecord] = useState(false);
  const [audioSrc, setAudioSrc] = useState('');

  const startRecording = () => {
    setRecord(true);
  };

  const stopRecording = () => {
    setRecord(false);
  };

  const onData = (recordedBlob) => {
    console.log('chunk of real-time data is: ', recordedBlob);
  };

  const onStop = async (recordedBlob) => {
    console.log('recordedBlob is: ', recordedBlob);

    const formData = new FormData();
    formData.append('file', recordedBlob.blob, 'recording.wav');

    try {
      const response = await fetch('http://localhost:5000/upload', {
        method: 'POST',
        body: formData,
      });
      const result = await response.json();
      console.log('Success:', result);
      setAudioSrc('../uploads/' + result.url);
      const audio = new Audio('../uploads/' + result.url);
      audio.play();
    } catch (error) {
      console.error('Error uploading file:', error);
    }
  };

  return (
    <div>
      <ReactMic
        record={record}
        className="sound-wave"
        onStop={onStop}
        onData={onData}
        strokeColor="#000000"
        backgroundColor="#FF4081"
      />
      <div>
      <button onClick={startRecording} type="button">Start</button>
      <button onClick={stopRecording} type="button">Stop</button>
      {audioSrc && <audio src={audioSrc} autoPlay />}
      </div>
    </div>
  );
};