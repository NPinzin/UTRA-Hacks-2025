import React, { useState, useEffect } from 'react';
import { ReactMic } from 'react-mic';

export const Mic = ({ postId }) => {
  const [record, setRecord] = useState(false);

  useEffect(() => {
    // Reset conversation history when the page reloads
    fetch('http://localhost:5000/reset', { method: 'POST' })
      .then(response => response.json())
      .then(data => {
        const firstQuestion = new Audio(`${data.url}`, { autoplay: true });
        firstQuestion.play();
      })
      .catch(error => console.error("Error resetting conversation:", error));
  }, []);

  useEffect(() => {
    const cycleRecording = () => {
      setRecord(true);
      setTimeout(() => {
        setRecord(false);
        setTimeout(cycleRecording, 2000); // Wait 2 seconds before restarting
      }, 10000); // Record for 10 seconds
    };

    cycleRecording();
  }, []);

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
      const audio = new Audio(`${result.url}?t=${Date.now()}`);
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
    </div>
  );
};
