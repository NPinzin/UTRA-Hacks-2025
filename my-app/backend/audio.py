from openai import OpenAI
import os

# Parameters
OPENAI_API_KEY = os.environ.get('OPENAI_API_KEY')

def transcribe(filepath) -> str: 
    client = OpenAI(api_key=OPENAI_API_KEY)
    audio_file= open(filepath, "rb")
    transcription = client.audio.transcriptions.create(
        model="whisper-1", 
        file=audio_file,
        response_format="json"
    ) 
    return transcription.text

def getSymptoms(transcription: str) -> str:
    client = OpenAI(api_key=OPENAI_API_KEY)
    completion = client.chat.completions.create(
        model="gpt-4o-mini",
        messages=[
            {"role": "system", "content": "You are extracting symptoms from a provided transcription from a hospital visit, return them in a comma separated list, if there are no symptoms then return an empty string."},
            {
                "role": "user",
                "content": transcription
            }
        ]
    )
    return completion.choices[0].message.content

def respond(transcription: str) -> str:
    client = OpenAI(api_key=OPENAI_API_KEY)
    completion = client.chat.completions.create(
        model="gpt-4o-mini",
        messages=[
            {"role": "system", "content": "You are acting as a prescreening AI for a hospital, you need to respond to the patient's concerns with a detailed response with the assumption they will see the doctor shortly."},
            {
                "role": "user",
                "content": transcription
            }
        ]
    )
    #convert the response to an audio file with a text-to-speech API
    speech_file_path = "../frontend/src/uploads/speech.mp3"
    with client.audio.speech.with_streaming_response.create(
        model="tts-1",
        voice="coral",
        speed=1,
        input=completion.choices[0].message.content
    ) as response: #Ignores depreciation warning
        response.stream_to_file(speech_file_path)
    return "speech.mp3"

def main(): # For testing
    transcription = transcribe()
    symptoms = getSymptoms(transcription)
    print(symptoms)


if __name__ == "__main__":
    main()