# WasteRecycling – Waste Collection Management (Qt/C++)

A desktop application (Qt/C++, Oracle database) for managing a waste-collection service: clients, employees, trucks, containers, machines and collection points, with a voice-control feature driven by an Arduino push-to-talk button.

> Academic team project – ESPRIT School of Engineering, 2nd year (2025–2026).
> Not runnable as-is: it needs an Oracle database and your own API keys, none of which are included.

## Modules

| Module | What it does |
|---|---|
| **Clients** | CRUD, loyalty points, filtered search, statistics chart |
| **Client messaging** | Personalized message per client, built from loyalty level (Bronze / Silver / Gold / VIP) and container stock; opens the system mail client (`mailto:`) to send it |
| **Voice control** | Arduino button starts/stops the PC microphone; speech is transcribed with **Whisper (Groq API)** to fill form fields, delete clients ("supprimer client ID 5", "supprimer tous les clients"), or create a client from a spoken note (LLM extracts name, phone, email, address, type) |
| **Employees** | Login, CRUD, search, input validation, role and security-question management |
| **Trucks** | CRUD, edit by double-click, search by plate number |
| **Containers** | CRUD, fill-level preview and alert threshold, search and sort |
| **Machines** | CRUD, search, statistics by waste type |
| **Collection points** | Display with progress bars, bar chart and donut chart |

## Tech stack

- **C++ / Qt 6** (Widgets, Charts, SQL, Network, Multimedia, SerialPort)
- **Oracle** database via ODBC (QODBC)
- **Arduino** over serial (push-to-talk button; sound feedback with an ISD1820 module)
- **Groq API** (Whisper `whisper-large-v3-turbo`) for speech-to-text
- **Anthropic API** (Claude) for extracting client data from a voice note
- Git / GitHub

## My contribution

Client module: CRUD, loyalty points, search, messaging and Arduino voice control.
the repository covers the whole team's work.

## Configuration (required to run)

Secrets are **not** stored in this repository. Replace the placeholders:

- `wasterecycling.h`: `YOUR_GROQ_API_KEY`, `YOUR_ANTHROPIC_API_KEY`, `YOUR_OPENAI_API_KEY`
- `connection.cpp`: `YOUR_DB_USER`, `YOUR_DB_PASSWORD` (and the Oracle data source name)

Never commit real keys.

## Build

1. Install Qt 6 with Qt Creator and the Oracle ODBC driver, and create the ODBC data source.
2. Create the Oracle tables used by the application
3. Open `WasteRecycling.pro` in Qt Creator, then build and run.
4. For voice control: connect the Arduino board (USB serial) and a microphone.

