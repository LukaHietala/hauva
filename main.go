package main

import (
	"os/exec"
	"strings"
	"time"
	"log"
)


// gets the current clipboard item
func getClipboard() (string, error) {
	cmd := exec.Command("wl-paste")
	out, err := cmd.Output()
	return strings.TrimSpace(string(out)), err
}


func main() {
	ticker := time.NewTicker(time.Second)
	defer ticker.Stop()
	for {
		select {
		case <-ticker.C:
			clip, err := getClipboard()
			if err != nil {
				continue
			}
			log.Println(clip)
		}
	}
}
