```bash
go build -o hauva main.go
```

```bash
sudo cp hauva /usr/local/bin/
```

```bash
mkdir -p ~/.config/systemd/user # or system wide
cp hauva.service ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable hauva.service
systemctl --user start hauva.service
```