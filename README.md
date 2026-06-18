# J.A.R.V.I.S - Voice Assistant

Voice assistant with speech recognition, LLM integration, and app management.

## Stack

- C++17, MSVC
- Vosk (speech recognition)
- Windows TTS + VoxCPM2 (speech synthesis)
- Ollama + llama3.1:8b (LLM)
- cpp-httplib (web server)
- HTML/CSS/JS (frontend)

## Commands

"joke", "anecdote" - Joke,
"music", "open Spotify" - Launch Spotify,
"favorite tracks" = Saved tracks,
"next" / "pause" = Control,
"turn on/off shuffle" = Shuffle,
"volume [0-100]" = Spotify volume,
"browser", "open browser" = Launch browser,
"find [query]" = Google search,
"available commands" = Help,
Other questions - LLM.

## Launching

1. Install Ollama: `ollama pull llama3.1:8b`
2. Install dependencies: `pip install vosk pyaudio websockets`
3. Run `1startjarvis.bat` (administrator)

## Known issues

- Paths are hardcoded!
- VoxCPM2 is disabled (long loading time of 30-90 seconds)
- LLM without internet, sometimes hallucinates
- Vosk sometimes buffers commands

## License
```
MIT License

Copyright (c) 2026 EGRSKRUT

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
---
