<img width="500" height="375" alt="image" src="https://github.com/user-attachments/assets/b9789ffa-75ed-406f-9205-0f5d068776b0" />

## Hauva

Simple clipboard manager for Wayland

### Features

- Text-only clipboard history (default 50 entries)
- De-duplication and persistent history (`~/.cache/hauva_history`)
- Fast IPC via Unix sockets (`/tmp/hauva.sock`)
- Integrates with `wl-clipboard` and `wmenu` for selection and copy

### Requirements

- Wayland compositor
- `wmenu` (entry selection)
- `wl-clipboard` (`wl-copy`, `wl-paste`)
- `gcc`, `make`

### Build & Install

```sh
make
make install
```

### Usage

Start the daemon:
```sh
./build/hauvad &
```

The hauva daemon maintains history and listens to clipboard

Client commands:
- `hauva add` — Add clipboard entry from stdin (manual addition)
- `hauva list` — List entries (one per line, for menu pickers)  
- `hauva copy` — Copy selected entry (from stdin, e.g. wmenu output)

### Systemd services

Example user service file (edit paths as needed):

`~/.config/systemd/user/hauva.service`
```ini
[Unit]
Description=Hauva daemon

[Service]
ExecStart=/usr/local/bin/hauvad
Restart=always

[Install]
WantedBy=default.target
```

### Tips

- Bind `hauva-ui` to some key on your wm
- Use with dmenu or other menus by editing the hauva-ui.sh script

### Future stuff

- X11 support
- Images and other MIME types
- Qt/gtk app
