from flask import Flask, request, jsonify
from flask_cors import CORS
import os
from audio import transcribe, getSymptoms, respond

app = Flask(__name__)
CORS(app)

@app.route('/upload', methods=['POST'])
def upload_audio():
    if 'file' not in request.files:
        return jsonify({'error': 'No file part'}), 400
    file = request.files['file']
    if file.filename == '':
        return jsonify({'error': 'No selected file'}), 400
    if file:
        file_path = os.path.join('../frontend/src/uploads', file.filename)
        file.save(file_path)
        # Process the audio file
        transcription = transcribe(file_path)
        symptoms = getSymptoms(transcription) #Have to upload to MongoDB
        path = respond(transcription)
        return jsonify({'url': path}), 200

if __name__ == '__main__':
    if not os.path.exists('../frontend/src/uploads'):
        os.makedirs('../frontend/src/uploads')
    app.run(debug=True)
