# qt-sd-animator

Qt6 C++ application for animating images using stable diffusion CLI.

## Features

- **File Management**: Open, save, and manage recently opened settings files
- **General Settings**: Configure SD-CLI binary path and default output directory
- **Model Manager**: Create, edit, duplicate, and delete model configurations
  - Diffusion model path (gguf/safetensors)
  - LLM path (gguf/safetensors)
  - VAE path (gguf/safetensors)
  - Width/height parameters
  - Model base path
- **Preset Manager**: Create, edit, duplicate, and delete prompt presets
  - Prompt text
  - Negative prompt text
- **Main Workspace**:
  - Model selector with new model dialog
  - Preset selector that auto-populates prompt fields
  - Background color picker
  - Source image browser (remembers last path)
  - Prompt and negative prompt text editors
  - Output destination selector with datetime checkbox
  - PROCESS and STOP buttons with state management
  - Real-time subprocess output window
- **Subprocess Management**:
  - Validates all inputs before execution
  - Creates output directory if missing
  - Streams stdout/stderr with prefixed logging
  - Logs constructed command for debugging
  - Generates companion JSON with execution metadata

## Building

### Requirements

- Qt 6.2 or higher
- CMake 3.16 or higher
- C++17 compatible compiler

### Build Steps

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

The binary will be located at `build/qt-sd-animator`.

## Usage

1. **Configure Settings**: Set SD-CLI binary path and output directory via File → Settings
2. **Manage Models**: Add model configurations with model paths and dimensions
3. **Manage Presets**: Create presets with prompt and negative prompt text
4. **Select Source**: Choose a source image using the file browser
5. **Configure Output**: Set output destination and toggle datetime in filename
6. **Process**: Click PROCESS to execute sd-cli with current settings
7. **Stop**: Click STOP to terminate a running process

## Output Filename Format

```
{source_basename}_{YYYYMMDD_HHMMSS}.avi
```

Example: `cat_20260824_143022.avi`

## Companion JSON

A `.json` file is created alongside each output video containing:

```json
{
  "command": "full command line executed",
  "model": { "name": "...", "diffusion-model": "...", ... },
  "preset": { "name": "...", "prompt": "...", "negative_prompt": "..." },
  "general_settings": { "output_path": "...", ... },
  "source_image": "/path/to/source.png",
  "bg_color": "#FF0000",
  "timestamp": "2026-08-24T14:30:22",
  "exit_code": 0
}
```

## Project Structure

```
qt-sd-animator/
├── CMakeLists.txt
├── .gitignore
├── README.md
├── include/
│   ├── MainWindow.h
│   ├── SettingsManager.h
│   ├── SubprocessHandler.h
│   └── dialogs/
│       ├── GeneralSettingsDialog.h
│       ├── ModelManagerDialog.h
│       ├── PresetManagerDialog.h
│       └── NewModelDialog.h
└── src/
    ├── main.cpp
    ├── MainWindow.cpp
    ├── SettingsManager.cpp
    ├── SubprocessHandler.cpp
    └── dialogs/
        ├── GeneralSettingsDialog.cpp
        ├── ModelManagerDialog.cpp
        ├── PresetManagerDialog.cpp
        └── NewModelDialog.cpp
```

## License

MIT
