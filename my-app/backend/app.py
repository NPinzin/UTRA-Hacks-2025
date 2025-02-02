from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import os
from audio import transcribe, getSymptoms, respond

app = Flask(__name__)
CORS(app)

UPLOAD_FOLDER = "uploads"
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

@app.route('/upload', methods=['POST'])
def upload_audio():
    if 'file' not in request.files:
        return jsonify({'error': 'No file part'}), 400

    file = request.files['file']
    if file.filename == '':
        return jsonify({'error': 'No selected file'}), 400

    if file:
        # Save uploaded audio file
        file_path = os.path.join(UPLOAD_FOLDER, file.filename)
        file.save(file_path)

        # Process the file
        transcription = transcribe(file_path)
        symptoms = getSymptoms(transcription)  # Save to DB if needed
        print(symptoms)

        # Generate AI speech response
        output_filename = "response_" + file.filename.replace(".wav", ".mp3")
        audio_url = respond(transcription, output_filename)

        return jsonify({'url': audio_url}), 200

# Serve uploaded audio files
@app.route('/uploads/<filename>')
def uploaded_file(filename):
    return send_from_directory(UPLOAD_FOLDER, filename)

if __name__ == '__main__':
    app.run(debug=True)
