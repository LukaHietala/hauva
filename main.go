package main

import (
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"strings"
	"time"
)

const clipFile = "koirankoppi.json"

type ClipManager struct {
	Clips []string `json:"clips"`
}

// loads all clips from json store (koirankoppi)
func (cm *ClipManager) Load() error {
	file, err := os.Open(clipFile)
	if err != nil {
		if os.IsNotExist(err) {
			cm.Clips = []string{}
			return nil
		}
		return err
	}
	defer file.Close()
	return json.NewDecoder(file).Decode(cm)
}

func (cm *ClipManager) Save() error {
	file, err := os.Create(clipFile)
	if err != nil {
		return err
	}
	defer file.Close()
	return json.NewEncoder(file).Encode(cm)
}

func checkRequiredPrograms() {
	// just wayland app for now, maybe X11 in the future
	// maybe wofi, rofi, etc too? now just dmenu because its the most common
	required := []string{"wl-paste", "wl-copy", "dmenu"}
	for _, program := range required {
		if _, err := exec.LookPath(program); err != nil {
			fmt.Printf("%s not found : %v\n", program, err)
			os.Exit(1)
		}
	}
}

func getClipboard() (string, error) {
	cmd := exec.Command("wl-paste")
	out, err := cmd.Output()
	return strings.TrimSpace(string(out)), err
}

func setClipboard(text string) error {
	cmd := exec.Command("wl-copy")
	cmd.Stdin = strings.NewReader(text)
	return cmd.Run()
}

func addClip() {
	cm := &ClipManager{}
	cm.Load()
	clip, err := getClipboard()
	if err != nil {
		fmt.Println("error getting clipboard", err)
		return
	}
	// prevent duplicates, if one already exist then move it first
	for i, c := range cm.Clips {
		if c == clip {
			cm.Clips = append(cm.Clips[:i], cm.Clips[i+1:]...)
			break
		}
	}
	cm.Clips = append([]string{clip}, cm.Clips...)
	// limit to 25, TODO: make optional
	if len(cm.Clips) > 25 {
		cm.Clips = cm.Clips[:25]
	}
	cm.Save()
}

func selectClip() {
	cm := &ClipManager{}
	cm.Load()
	if len(cm.Clips) == 0 {
		fmt.Println("no clips")
		return
	}
	// dmenu seperates options by \n so prepare the clips
	// -i match insensitively, -l list vertically with number of lines, -p message/prompt
	input := strings.Join(cm.Clips, "\n")
	cmd := exec.Command("dmenu", "-i", "-l", "10", "-p", "Select clip:")
	// pass stdin to dmenu to intrepret
	cmd.Stdin = strings.NewReader(input)
	// dmenu returns selected item as stdout
	out, err := cmd.Output()
	if err != nil {
		fmt.Println("dmenu error:", err)
		return
	}
	selected := strings.TrimSpace(string(out))
	setClipboard(selected)
}

func main() {
	// wl-copy, wl-paste, dmenu
	checkRequiredPrograms()
	if len(os.Args) > 1 && os.Args[1] == "--daemon" {
		// prevent same stuff being spammed to cm,
		lastClip := ""
		// check wl-paste every one second
		ticker := time.NewTicker(time.Second)
		defer ticker.Stop()
		for range ticker.C {
			clip, err := getClipboard()
			if err != nil {
				continue
			}
			if clip != lastClip && clip != "" {
				lastClip = clip
				addClip()
			}
		}
	} else {
		selectClip()
	}
}
