<img width="500" height="375" alt="image" src="https://github.com/user-attachments/assets/b9789ffa-75ed-406f-9205-0f5d068776b0" />

# Hauva 

Simple clipboard manager for  wayland. Supports only text. Requires wl-clipboard and dmenu.

## Setup

1. Clone the repo
2. Build
```bash
go build -o hauva main.go
```
3. Copy to /usr/local/bin
```bash
sudo cp hauva /usr/local/bin/
```
4. Add systemd service for daemon
```bash
mkdir -p ~/.config/systemd/user # or system wide
cp hauva.service ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable hauva.service
systemctl --user start hauva.service
```
5. Use command `hauva` to open clipboard menu

You can now use it in a wm for example. `bindsym $mod+Shift+v exec hauva`
